//
// Copyright 2015, Roberto Paleari (@rpaleari) and Aristide Fattori (@joystick)
//

#ifndef _PERF_H_
#define _PERF_H_

#include <linux/perf_event.h>
#include <stdint.h>
#include <unistd.h>

struct sample_id {
  uint64_t id;
};

struct perf_record_lost {
  struct perf_event_header header;
  uint64_t id;
  uint64_t lost;
  struct sample_id sample_id;
};

// A branch trace record in perf_event
struct perf_event_bts {
  uint64_t from;
  uint32_t pid, tid;
  uint64_t to;
};

// A perf_event branch trace sample
struct perf_event_bts_sample {
  struct perf_event_header header;
  struct perf_event_bts bts;
};

// mmap()'ing of executable areas
struct perf_event_mmap {
  struct perf_event_header header;
  uint32_t pid, tid;
  uint64_t addr;
  uint64_t len;
  uint64_t pgoff;
  char filename[];
  struct sample_id sample_id;
};

// Process exit event
struct perf_event_exit {
  struct perf_event_header header;
  uint32_t pid, tid;
  uint64_t time;
  struct sample_id sample_id;
};

// Process fork event
struct perf_event_fork {
  struct perf_event_header header;
  uint32_t pid, ppid;
  uint32_t tid, ptid;
  uint64_t time;
  struct sample_id sample_id;
};

int64_t perf_event_open(struct perf_event_attr *attr, pid_t pid,
                        int cpu, int group_fd, uint64_t flags);
void perf_init(struct perf_event_attr *attr, int mmap_pages);

#if defined(__i386__)
#define rmb() asm volatile("lock; addl $0,0(%%esp)" ::: "memory")
#define mb() asm volatile("lock; addl $0,0(%%esp)" ::: "memory")

#elif defined(__x86_64)
#define rmb() asm volatile("lfence":::"memory")
#define mb() asm volatile("mfence":::"memory")
#endif

#endif  // _PERF_H_
