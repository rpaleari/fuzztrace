//
// Copyright 2015, Roberto Paleari (@rpaleari) and Aristide Fattori (@joystick)
//
// Execution monitor.
//

#ifndef _COMMON_BBMAP_H
#define _COMMON_BBMAP_H

#include <map>
#include <utility>

#include "./common.h"

typedef std::pair<target_addr, target_addr> bbmap_edge;
typedef std::map<bbmap_edge, unsigned int >::const_iterator bbmap_iterator;

class BBMap {
 public:
  explicit BBMap() {}

  // Record the execution of a CFG edge, identified by a pair of basic block
  // addresses
  void AddEdge(target_addr prev, target_addr next);

  // Return a 32-bit hash of this basic block map
  uint32_t ComputeHash() const;

  // Iterate over the (edge, #hit) pairs
  bbmap_iterator map_begin() const { return bb_map_.begin(); }
  bbmap_iterator map_end() const { return bb_map_.end(); }
  int size() const { return bb_map_.size(); }

 private:
  std::map<bbmap_edge, unsigned int > bb_map_;
};

#endif  // _COMMON_BBMAP_H
