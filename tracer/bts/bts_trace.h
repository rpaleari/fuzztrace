//
// Copyright 2015, Roberto Paleari (@rpaleari) and Aristide Fattori (@joystick)
//

#ifndef _BTS_TRACE_H_
#define _BTS_TRACE_H_

#define __STDC_FORMAT_MACROS

#include <linux/perf_event.h>
#include <inttypes.h>
#include <signal.h>

#define MMAP_PAGES 512

// Global status of perf_event monitor
struct perf_global_status {
  void *mmap;                   // Pointer to mmap'ed area
  unsigned char *data;          // Backup copy of BTS data for processing
  int data_size;                // Size of mmap'ed data area (without hdr)
  int fd_evt;
  uint64_t prev_head;
  int n_events;
  int pid_child;
  volatile sig_atomic_t data_ready;
};

extern struct perf_global_status gbl_status;

#endif  // _BTS_TRACE_H_
