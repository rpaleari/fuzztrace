//
// Copyright 2015, Roberto Paleari (@rpaleari) and Aristide Fattori (@joystick)
//

#ifndef _COMMON_COMMON_H
#define _COMMON_COMMON_H

#include "cstdint"

#include "./logging.h"

#if __x86_64__
typedef uint64_t target_addr;
#else
typedef uint32_t target_addr;
#endif

#endif  // _COMMON_COMMON_H
