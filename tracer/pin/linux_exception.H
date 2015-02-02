//
// Copyright 2015, Roberto Paleari (@rpaleari) and Aristide Fattori (@joystick)
//
// Exception handling, Linux version.
//

#ifndef _LINUX_EXCEPTION_H
#define _LINUX_EXCEPTION_H

#include "pin.H"
#include "common/exception.h"

Exception* Linux_BuildException(THREADID tid, INT32 sig,
                                const EXCEPTION_INFO *pExceptInfo);

#endif  // _LINUX_EXCEPTION_H
