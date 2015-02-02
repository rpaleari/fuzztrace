//
// Copyright 2015, Roberto Paleari (@rpaleari) and Aristide Fattori (@joystick)
//

#include "./bbmap.h"

#include <cstdint>

void BBMap::AddEdge(target_addr prev, target_addr next) {
  bbmap_edge edge(prev, next);

  if (bb_map_.find(edge) == bb_map_.end()) {
    bb_map_[edge] = 1;
  } else {
    bb_map_[edge]  += 1;
  }
}

uint32_t BBMap::ComputeHash() const {
  uint32_t hash = 0;
  for (bbmap_iterator it = map_begin(); it != map_end(); it++) {
    bbmap_edge edge = it->first;
    hash ^= edge.first ^ edge.second;
  }
  return hash;
}
