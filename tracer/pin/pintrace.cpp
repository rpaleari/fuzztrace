//
// Copyright 2015, Roberto Paleari (@rpaleari) and Aristide Fattori (@joystick)
//
// Basic BB tracer.
//

#include <iostream>
#include <map>
#include <string>
#include <memory>
#include <vector>

#include "pin.H"

#include "common/bbmap.h"
#include "common/exception.h"
#include "common/serialize.h"
#include "images.H"

#ifdef _WIN32
// Windows
#include "win_exception.H"
#else
// Linux
#include <csignal>
#include "linux_exception.H"
#endif

static const std::string DEFAULT_OUTFILE = "/dev/shm/trace.bin";
static const int MAX_STACKTRACE_SIZE = 16;

// Globals
ExecutionTrace gbl_execution_trace;

// Map to keep track of last observed basic block, for each application thread
static std::map<THREADID, ADDRINT> gbl_last_bb;

// Command line switches
KNOB<string> gbl_outfile(KNOB_MODE_WRITEONCE, "pintool", "f", DEFAULT_OUTFILE,
                         "specify file name for FuzzTrace output");

// Print help message
INT32 Usage() {
  cerr << KNOB_BASE::StringKnobSummary() << endl;
  return -1;
}

static void build_stacktrace(const CONTEXT *context, Exception *exc) {
  // Generate a stack trace for this process
  ADDRINT retaddr;
  ADDRINT fp = PIN_GetContextReg(context, REG_GBP);

  for (int i = 0; i < MAX_STACKTRACE_SIZE; i++) {
    // Read the return address of current stack frame
    size_t nbytes =
      PIN_SafeCopy(&retaddr,
                   reinterpret_cast<const VOID*>(fp + sizeof(ADDRINT)),
                   sizeof(ADDRINT));

    if (nbytes != sizeof(ADDRINT)) {
      break;
    }

    // Save this return address
    exc->stacktrace_push(retaddr);
    // printf("ret%d @0x%lx -> 0x%lx\n", i, fp+sizeof(ADDRINT), retaddr);

    // Read the address of the next stack frame
    nbytes =
      PIN_SafeCopy(&fp, reinterpret_cast<const VOID*>(fp), sizeof(ADDRINT));

    if (nbytes != sizeof(ADDRINT)) {
      break;
    }
  }
}

// Instrumentation callback
VOID CallbackTrace(TRACE trace, VOID *v) {
  // Visit every basic block in the trace
  for (BBL bbl = TRACE_BblHead(trace); BBL_Valid(bbl); bbl = BBL_Next(bbl)) {
    ADDRINT bb_current = BBL_Address(bbl);

    if (!Images_IsInterestingAddress(bb_current)) {
      continue;
    }

    // Compute the (bb_previous, bb_current) pair, that determines the brach
    // that has been taken by the application
    THREADID tid = PIN_ThreadId();

    // If we have no previous BB, skip
    if (gbl_last_bb.find(tid) != gbl_last_bb.end()) {
      ADDRINT bb_previous = gbl_last_bb[tid];
      gbl_execution_trace.basic_blocks.AddEdge(bb_previous, bb_current);
    }

    gbl_last_bb[tid] = bb_current;
  }
}

#ifdef _WIN32
// Windows-only
VOID CallbackContextChangeWindows(THREADID tid, CONTEXT_CHANGE_REASON reason,
                                  const CONTEXT *from, CONTEXT *to, INT32 info,
                                  VOID *v) {
  if (reason != CONTEXT_CHANGE_REASON_EXCEPTION) {
    return;
  }
}
#else
// Linux-only
BOOL CallbackSignalLinux(THREADID tid, INT32 sig, CONTEXT *ctxt,
                         BOOL hasHandler, const EXCEPTION_INFO *pExceptInfo,
                         VOID *v) {
  Exception *exception = Linux_BuildException(tid, sig, pExceptInfo);
  build_stacktrace(ctxt, exception);
  gbl_execution_trace.exceptions.push_back(std::shared_ptr<Exception>
                                           (exception));

  // Always pass the exception to the application
  return TRUE;
}
#endif

// Termination callback
VOID CallbackFini(INT32 code, VOID *v) {
  string filename = gbl_outfile.Value();

  serialize_trace(filename, gbl_execution_trace);
}

int main(int argc, char **argv) {
  // Initialize PIN library and check command-line
  if (PIN_Init(argc, argv)) {
    return Usage();
  }

  IMG_AddInstrumentFunction(Images_CallbackNewImage, 0);
  TRACE_AddInstrumentFunction(CallbackTrace, 0);

#ifdef _WIN32
  // Callback for context changes (e.g., exceptions). Windows only.
  PIN_AddContextChangeFunction(CallbackContextChangeWindows, 0);
#else
  // Callback for SEGVs. Linux only.
  PIN_InterceptSignal(SIGSEGV, CallbackSignalLinux, 0);
#endif

  // Callback for application termination
  PIN_AddFiniFunction(CallbackFini, 0);

  // Start the program, never returns
  PIN_StartProgram();

  return 0;
}
