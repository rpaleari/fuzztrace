//
// Copyright 2015, Roberto Paleari (@rpaleari) and Aristide Fattori (@joystick)
//

#include "./exception.h"

Exception::Exception(int tid, ExceptionType type, target_addr pc,
                     target_addr faulty_addr, int faulty_type)
  : tid_(tid), type_(type), pc_(pc), faulty_addr_(faulty_addr),
    faulty_type_(faulty_type) {
}
