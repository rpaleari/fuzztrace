//
// Copyright 2015, Roberto Paleari (@rpaleari) and Aristide Fattori (@joystick)
//

#include "./monitor.h"

#include <linux/perf_event.h>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/user.h>
#include <sys/wait.h>
#include <cassert>
#include <cerrno>
#include <cstring>

#include <map>
#include <memory>

#include "common/common.h"
#include "common/serialize.h"
#include "./bts_trace.h"
#include "./perf.h"

static const int MAX_STACKTRACE_SIZE = 16;

static ExecutionTrace gbl_execution_trace;

static inline bool is_kernel_addr(target_addr addr) {
  return (addr >> 47) != 0;
}

// Memory mapping of a new executable section. Extract details of the
// mmap()'ped region and add to the global structure
static void monitor_add_mmap(struct perf_event_mmap *mmap_event) {
  MemoryRegion region;
  region.base = mmap_event->addr;
  region.size = mmap_event->len;
  region.filename = mmap_event->filename;
  gbl_execution_trace.memory_regions.push_back(region);

  LOG_DEBUG("mmap()'ing image '%s' at range [0x%lx-0x%lx]",
  region.filename.c_str(), region.base, region.base+region.size-1);
}

// Add a new branch event
static void monitor_add_sample(struct perf_event_bts_sample *bts_sample) {
  const target_addr bb_previous = bts_sample->bts.from;
  const target_addr bb_current = bts_sample->bts.to;

#ifdef DEBUG_MODE
  fprintf(stderr, "[tid %d] from: 0x%016" PRIx64 ", to: 0x%016" PRIx64 "\n",
    bts_sample->bts.tid, bb_previous, bb_current);
#endif

  // Skip kernel addresses
  if (is_kernel_addr(bb_previous) || is_kernel_addr(bb_current)) {
    return;
  }

  gbl_execution_trace.basic_blocks.AddEdge(bb_previous, bb_current);
  gbl_status.n_events++;
}

static void monitor_process_events(void) {
  struct perf_event_mmap_page *control_page;
  struct perf_event_header *event;
  uint64_t head, prev_head_wrap;
  void *data_mmap;
  int size, offset;

  control_page = (struct perf_event_mmap_page*) gbl_status.mmap;
  data_mmap = (unsigned char*) gbl_status.mmap + getpagesize();

  if (control_page == NULL) {
    LOG_WARN("Skipping invalid control page");
    return;
  }

  head = control_page->data_head;
  rmb();

  size = head - gbl_status.prev_head;

  prev_head_wrap = gbl_status.prev_head % gbl_status.data_size;

  LOG_DEBUG("Current head 0x%016" PRIx64 ", previous head 0x%016" PRIx64
            ", size %d data_size %d prev_head_wrap 0x%016" PRIx64, head,
            gbl_status.prev_head, size, gbl_status.data_size, prev_head_wrap);


  // Copy (possibly wrapped) data to the work area
  memcpy(gbl_status.data, (unsigned char*) data_mmap + prev_head_wrap,
         gbl_status.data_size - prev_head_wrap);
  memcpy(gbl_status.data + gbl_status.data_size - prev_head_wrap,
         (unsigned char*) data_mmap, prev_head_wrap);

  offset = 0;
  while (offset < size) {
    event = (struct perf_event_header *) &gbl_status.data[offset];

    switch (event->type) {
    case PERF_RECORD_MMAP:
      monitor_add_mmap(reinterpret_cast<struct perf_event_mmap *>(event));
      break;

    case PERF_RECORD_LOST:
      LOG_DEBUG("Lost %lu events", ((struct perf_record_lost*) event)->lost);
      break;

    case PERF_RECORD_THROTTLE:
    case PERF_RECORD_UNTHROTTLE:
      LOG_DEBUG("Received a (un)throttle event, ignoring");
      break;

    case PERF_RECORD_SAMPLE:
      assert(event->size == sizeof(struct perf_event_bts_sample));
      monitor_add_sample(reinterpret_cast<
       struct perf_event_bts_sample *>(event));
      break;

    case PERF_RECORD_FORK: {
      LOG_DEBUG("Process %d (thread %d) created",
                reinterpret_cast<struct perf_event_fork *>(event)->pid,
                reinterpret_cast<struct perf_event_fork *>(event)->tid);
      break;
    }

    case PERF_RECORD_EXIT: {
      LOG_DEBUG("Process %d (thread %d) has exited",
                reinterpret_cast<struct perf_event_exit *>(event)->pid,
                reinterpret_cast<struct perf_event_exit *>(event)->tid);
      break;
    }

    default:
      LOG_FATAL("Unsupported perf record %d", event->type);
    }

    // Skip event
    offset += event->size;
  }

  assert(offset == size);

  mb();
  control_page->data_tail = head;
  gbl_status.prev_head = head;
}

static void monitor_handle_signal(pid_t pid, int status) {
  LOG_DEBUG("Read signal info (pid %d)", pid);
  siginfo_t si;
  int ret = ptrace(PTRACE_GETSIGINFO, pid, NULL, &si);
  assert(ret != -1);

  // Exception type
  ExceptionType exc_type;
  target_addr exc_faulty;

  switch (si.si_signo) {
  case SIGSEGV:
    exc_type = ExceptionAccessViolation;
    exc_faulty = reinterpret_cast<target_addr>(si.si_addr);
    break;
  default:
    exc_type = ExceptionUnknown;
    exc_faulty = 0;
    break;
  }

  LOG_DEBUG("Reading child registers (pid %d)", pid);
  struct user_regs_struct regs;
  ret = ptrace(PTRACE_GETREGS, pid, NULL, &regs);
  assert(ret != -1);

  std::shared_ptr<Exception>
    exc(new Exception(pid, exc_type, regs.rip, exc_faulty, 0));

  // Generate a stack trace for this process
  target_addr fp = regs.rsp;
  LOG_DEBUG("Starting stack walking at 0x%016lx", fp);

  for (int i = 0; i < MAX_STACKTRACE_SIZE; i++) {
    // Read the return address of current stack frame
    target_addr retaddr = ptrace(PTRACE_PEEKTEXT, pid,
         fp+sizeof(target_addr), 0);
    if (retaddr == static_cast<target_addr>(-1)) {
      LOG_DEBUG("Failed reading %d-th return address @0x%lx, giving up",
                i, fp+sizeof(target_addr));
      break;
    }

    // Save this return address
    exc->stacktrace_push(retaddr);
    LOG_DEBUG("ret%d @0x%lx -> 0x%lx", i, fp+sizeof(target_addr), retaddr);

    // Read the address of the next stack frame
    fp = ptrace(PTRACE_PEEKTEXT, pid, fp, 0);
    if (fp == static_cast<target_addr>(-1)) {
      LOG_DEBUG("Failed reading %d-th frame pointer @0x%lx, giving up", i, fp);
      break;
    }
  }

  gbl_execution_trace.exceptions.push_back(exc);
}

void monitor_loop(pid_t pid_child, const std::string s_outfile) {
  int ret, status;
  pid_t pid;

  // Wait until child terminates
  while (1) {
    pid = waitpid(pid_child, &status, 0);
    if (pid == -1 && errno == EINTR) {
      continue;
    }

    assert(pid == pid_child);

    // Process any pending event
    if (gbl_status.data_ready > 0) {
      gbl_status.data_ready--;
      monitor_process_events();
    }

    if (WIFEXITED(status)) {
      LOG_DEBUG("Child terminated with status %d", WEXITSTATUS(status));
      break;
    } else if (WIFSIGNALED(status)) {
      LOG_DEBUG("Child terminated by signal #%d", WTERMSIG(status));
      break;
    } else if (WIFSTOPPED(status) && WSTOPSIG(status) != SIGTRAP) {
      LOG_DEBUG("Child stopped by signal #%d", WSTOPSIG(status));
      monitor_handle_signal(pid_child, status);
      break;
    }

    // Resume child process
    ret = ptrace(PTRACE_CONT, pid_child, 0, 0);
    assert(ret != -1);
  }

  // Process final events before terminating
  monitor_process_events();

  // Serialize to file
  LOG_INFO("Got %d events (%d CFG edges)", gbl_status.n_events,
           gbl_execution_trace.basic_blocks.size());

  if (s_outfile.length() > 0) {
    LOG_INFO("Serializing to %s", s_outfile.c_str());
    serialize_trace(s_outfile, gbl_execution_trace);
  }
}
