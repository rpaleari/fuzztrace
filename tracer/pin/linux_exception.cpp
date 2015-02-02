//
// Copyright 2015, Roberto Paleari (@rpaleari) and Aristide Fattori (@joystick)
//

#include <cassert>
#include <csignal>

#include "linux_exception.H"

#include "exception.H"

Exception* Linux_BuildException(THREADID tid, INT32 sig,
                                const EXCEPTION_INFO *pExceptInfo) {
  assert(sig == SIGSEGV);
  assert(pExceptInfo != NULL);
  assert(PIN_GetExceptionClass(PIN_GetExceptionCode(pExceptInfo)) ==
         EXCEPTCLASS_ACCESS_FAULT);

  ADDRINT pc = PIN_GetExceptionAddress(pExceptInfo);

  // TODO(roby): check PIN_GetFaultyAccessAddress() return value
  ADDRINT faulty_addr = 0;
  PIN_GetFaultyAccessAddress(pExceptInfo, &faulty_addr);

  FAULTY_ACCESS_TYPE faulty_type = PIN_GetFaultyAccessType(pExceptInfo);

  Exception *exception =
    new Exception(tid, ExceptionAccessViolation, pc, faulty_addr, faulty_type);

  return exception;
}
