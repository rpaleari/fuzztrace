//
// Copyright 2015, Roberto Paleari (@rpaleari) and Aristide Fattori (@joystick)
//

#include <fstream>
#include <string>

#include "./bbtrace.pb.h"
#include "./serialize.h"

// Populate a protobuf Edge object
static inline void serialize_populate_edge(bbtrace::Edge *output,
                                           const bbmap_edge &edge,
                                           unsigned int hit) {
    output->set_prev(edge.first);
    output->set_next(edge.second);
    output->set_hit(hit);
}

// Populate a protobuf Exception object
static inline void
serialize_populate_exception(bbtrace::Exception *output,
                             const std::shared_ptr<Exception> &exc) {
    output->set_tid(exc->tid());
    output->set_pc(exc->pc());
    output->set_faultyaddr(exc->faulty_addr());

    // Set exception type
    switch (exc->type()) {
    case ExceptionAccessViolation:
      output->set_type(bbtrace::Exception::TYPE_VIOLATION);
      break;
    default:
      output->set_type(bbtrace::Exception::TYPE_UNKNOWN);
      break;
    }

    // Set faulty access type
    switch (exc->faulty_type()) {
    case ExceptionFaultyRead:
      output->set_access(bbtrace::Exception::ACCESS_READ);
      break;
    case ExceptionFaultyWrite:
      output->set_access(bbtrace::Exception::ACCESS_WRITE);
      break;
    case ExceptionFaultyExecute:
      output->set_access(bbtrace::Exception::ACCESS_EXECUTE);
      break;
    default:
      output->set_access(bbtrace::Exception::ACCESS_UNKNOWN);
      break;
    }

    // Stack trace
    for (stacktrace_iterator it_stack = exc->stacktrace_begin();
         it_stack != exc->stacktrace_end(); it_stack++) {
      output->add_stacktrace(*it_stack);
    }
}

// Populate a protobuf Exception object
static inline void
serialize_populate_region(bbtrace::MemoryRegion *output,
                          const MemoryRegion &region) {
  output->set_base(region.base);
  output->set_size(region.size);
  output->set_name(region.filename);
}

void serialize_trace(const std::string &filename,
                     const ExecutionTrace &execution_trace) {
  bbtrace::Trace trace;

  // Create the trace header
  bbtrace::TraceHeader *header = trace.mutable_header();
  header->set_magic(bbtrace::TraceHeader::TRACE_MAGIC);
  header->set_timestamp(time(NULL));
  header->set_hash(execution_trace.basic_blocks.ComputeHash());

  // Output basic block information
  for (bbmap_iterator it = execution_trace.basic_blocks.map_begin();
       it != execution_trace.basic_blocks.map_end(); it++) {
    bbtrace::Edge *edge = trace.add_edge();
    serialize_populate_edge(edge, it->first, it->second);
  }

  // Output recorded exceptions
  for (exceptions_iterator it = execution_trace.exceptions.begin();
       it != execution_trace.exceptions.end(); it++) {
    bbtrace::Exception *exception = trace.add_exception();
    serialize_populate_exception(exception, *it);
  }

  // Output memory-mapped regions
  for (auto it = execution_trace.memory_regions.begin();
       it != execution_trace.memory_regions.end(); it++) {
    bbtrace::MemoryRegion *region = trace.add_region();
    serialize_populate_region(region, *it);
  }

  std::fstream outstream(filename.c_str(),
                         std::ios::out | std::ios::trunc | std::ios::binary);
  trace.SerializeToOstream(&outstream);
}
