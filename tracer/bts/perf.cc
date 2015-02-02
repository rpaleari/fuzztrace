//
// Copyright 2015, Roberto Paleari (@rpaleari) and Aristide Fattori (@joystick)
//

#include "./perf.h"

#include <asm/unistd.h>
#include <cstring>

#include "common/logging.h"
#include "./bts_trace.h"

/* There is no glibc wrapper for syscall perf_event_open(), so we provide a
   simple wrapper here */
int64_t perf_event_open(struct perf_event_attr *attr, pid_t pid,
                        int cpu, int group_fd, uint64_t flags) {
  return syscall(__NR_perf_event_open, attr, pid, cpu, group_fd, flags);
}

void perf_init(struct perf_event_attr *attr, int mmap_pages) {
  memset(attr, 0, sizeof(struct perf_event_attr));
  attr->type = PERF_TYPE_HARDWARE;
  attr->size = sizeof(struct perf_event_attr);
  attr->config = PERF_COUNT_HW_BRANCH_INSTRUCTIONS;

  attr->branch_sample_type = PERF_SAMPLE_BRANCH_USER | PERF_SAMPLE_BRANCH_ANY;

  attr->disabled = 1;
  attr->enable_on_exec = 1;
  attr->exclude_kernel = 1;
  attr->exclude_hv = 1;
  attr->exclude_idle = 1;
  attr->precise_ip = 2;
  attr->wakeup_watermark = mmap_pages*getpagesize()/32/4;
  attr->watermark = 1;
  attr->mmap = 1;

  attr->sample_period = 1;
  attr->sample_type = PERF_SAMPLE_IP | PERF_SAMPLE_TID | PERF_SAMPLE_ADDR;
}
