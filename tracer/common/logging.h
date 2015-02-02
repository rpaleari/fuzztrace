//
// Copyright 2015, Roberto Paleari (@rpaleari) and Aristide Fattori (@joystick)
//

#ifndef _COMMON_LOGGING_H_
#define _COMMON_LOGGING_H_

#include <stdio.h>
#include <stdlib.h>

// #define DEBUG_MODE

#define _msg(tag, fmt, ...)                                     \
  fprintf(stderr, "[" tag "] " fmt "\n", ## __VA_ARGS__)

#ifdef DEBUG_MODE
#define LOG_DEBUG(fmt, ...) _msg("D", fmt, ## __VA_ARGS__)
#else
#define LOG_DEBUG(fmt, ...)
#endif

#define LOG_INFO(fmt, ...) _msg("*", fmt, ## __VA_ARGS__)
#define LOG_WARN(fmt, ...) _msg("!", fmt, ## __VA_ARGS__)
#define LOG_FATAL(fmt, ...) {                   \
    _msg("#", fmt, ## __VA_ARGS__);             \
    exit(-1);                                   \
  }

#endif  // _COMMON_LOGGING_H_
