//
// Copyright 2015, Roberto Paleari (@rpaleari) and Aristide Fattori (@joystick)
//
// OS-independent exception handling.
//

#ifndef _COMMON_EXCEPTION_H
#define _COMMON_EXCEPTION_H

#include <memory>
#include <vector>

#include "./common.h"

typedef std::vector<target_addr>::const_iterator stacktrace_iterator;

enum ExceptionType {
  ExceptionUnknown = 0,
  ExceptionAccessViolation = 1,
};

enum ExceptionFaultyType {
  ExceptionFaultyUnknown = 0,
  ExceptionFaultyRead = 1,
  ExceptionFaultyWrite = 2,
  ExceptionFaultyExecute = 3,
};

class Exception {
 public:
  explicit Exception(int tid, ExceptionType type, target_addr pc,
                     target_addr faulty_addr, int faulty_type);

  // Getters
  int tid() const { return tid_; }
  ExceptionType type() const { return type_; }
  target_addr pc() const { return pc_; }
  target_addr faulty_addr() const { return faulty_addr_; }
  int faulty_type() const { return faulty_type_; }

  // Push a new entry to the stack trace
  void stacktrace_push(target_addr addr) {
    stacktrace_.push_back(addr);
  }

  stacktrace_iterator stacktrace_begin() const { return stacktrace_.begin(); }
  stacktrace_iterator stacktrace_end() const { return stacktrace_.end(); }

 private:
  int tid_;                       // Thread ID
  ExceptionType type_;            // Exception type
  target_addr pc_;                // Program counter
  target_addr faulty_addr_;       // Faulty address
  int faulty_type_;               // Type of faulty access
  std::vector<target_addr> stacktrace_;  // Stack trace (list of retaddr)
};

typedef std::vector<std::shared_ptr<Exception> > exceptions_list;
typedef exceptions_list::const_iterator exceptions_iterator;

#endif  // _COMMON_EXCEPTION_H
