//
// Copyright 2015, Roberto Paleari (@rpaleari) and Aristide Fattori (@joystick)
//
// Serialize trace data to file.
//

#ifndef _COMMON_SERIALIZE_H
#define _COMMON_SERIALIZE_H

#include <string>
#include <vector>

#include "./common.h"
#include "./bbmap.h"
#include "./exception.h"

typedef struct {
  target_addr base;
  unsigned int size;
  std::string filename;
} MemoryRegion;

typedef struct {
  // Map of basic block edges
  BBMap basic_blocks;

  // Tracked exceptions
  exceptions_list exceptions;

  // Mapped memory regions
  std::vector<MemoryRegion> memory_regions;
} ExecutionTrace;

void serialize_trace(const std::string &filename,
                     const ExecutionTrace &execution_trace);

#endif  // _COMMON_SERIALIZE_H
