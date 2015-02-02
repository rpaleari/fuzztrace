//
// Copyright 2015, Roberto Paleari (@rpaleari) and Aristide Fattori (@joystick)
//
// Trace executed branches, using perf_event_open() and leveraging Intel BTS.
//
// References:
// - https://svn.physiomeproject.org/svn/opencmissextras/cm/trunk/external/packages/PAPI/papi-4.2.0/src/libpfm4/perf_examples/x86/bts_smpl.c
// - https://github.com/deater/perf_event_tests/blob/master/tests/record_sample/sample_branch_stack.c
//

#include "./bts_trace.h"

#include <assert.h>
#include <fcntl.h>
#include <getopt.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/ptrace.h>

#include "common/logging.h"
#include "./monitor.h"
#include "./perf.h"

struct perf_global_status gbl_status = {
  NULL,                         // mmap
  NULL,                         // data
  0,                            // data_size
  0,                            // fd_evt
  0,                            // prev_head
  0,                            // n_events
  0,                            // pid_child
  0                             // data_ready
};

static pid_t child_start(char **argv) {
  pid_t pid;
  int ret;

  pid = fork();
  assert(pid >= 0);
  if (pid == 0) {
    // Child
    ret = ptrace(PTRACE_TRACEME, 0, 0, 0);
    assert(ret != -1);

    raise(SIGTRAP);
    execvp(argv[0], argv);

    // Unreachable
    assert(0);
  }

  // Parent
  return pid;
}

static void show_help(char **argv) {
  fprintf(stderr, "Syntax: %s [-f <filename>] cmdline\n", argv[0]);
}

// Signal handler to process perf events.
static void sig_handler(int signum, siginfo_t *siginfo, void *dummy) {
  if (signum == SIGIO) {
    kill(gbl_status.pid_child, SIGTRAP);
    gbl_status.data_ready++;
  }
}

int main(int argc, char **argv) {
  struct perf_event_attr pe;
  struct sigaction sa;
  pid_t pid_child;
  int opt;
  std::string s_outfile;

  while ((opt = getopt(argc, argv, "f:h")) != -1) {
    switch (opt) {
    case 'f':
      s_outfile = optarg;
      break;
    default:
    case 'h':
      show_help(argv);
      exit(1);
    }
  }

  // Allocate work area for processing events
  gbl_status.data_size = MMAP_PAGES*getpagesize();
  gbl_status.data = (unsigned char*) malloc(gbl_status.data_size);
  assert(gbl_status.data > 0);

  memset(gbl_status.data, 0, gbl_status.data_size);

  // Prepare the child process
  pid_child = child_start(argv+optind);
  LOG_DEBUG("Started child with pid %d", pid_child);
  gbl_status.pid_child = pid_child;

  // Setup signals
  memset(&sa, 0, sizeof(struct sigaction));
  sa.sa_sigaction = sig_handler;
  sa.sa_flags = SA_SIGINFO;
  if (sigaction(SIGIO, &sa, NULL) < 0) {
    LOG_FATAL("Error setting up signal handler");
  }

  // Initialize perf structure
  perf_init(&pe, MMAP_PAGES);

  gbl_status.fd_evt = perf_event_open(&pe, pid_child, -1, -1, 0);
  if (gbl_status.fd_evt == -1) {
    perror("perf_event_open");
    exit(EXIT_FAILURE);
  }

  // Allocate mmap'ed area
  gbl_status.mmap = mmap(NULL, (MMAP_PAGES+1)*getpagesize(),
      PROT_READ | PROT_WRITE, MAP_SHARED, gbl_status.fd_evt, 0);
  assert(gbl_status.mmap != reinterpret_cast<void*>(-1));

  fcntl(gbl_status.fd_evt, F_SETFL, O_RDWR|O_NONBLOCK|O_ASYNC);
  fcntl(gbl_status.fd_evt, F_SETSIG, SIGIO);
  fcntl(gbl_status.fd_evt, F_SETOWN, getpid());

  // Monitor child until it terminates
  monitor_loop(pid_child, s_outfile);

  ioctl(gbl_status.fd_evt, PERF_EVENT_IOC_DISABLE, 0);
  close(gbl_status.fd_evt);
  munmap(gbl_status.mmap, (MMAP_PAGES+1)*getpagesize());

  return 0;
}
