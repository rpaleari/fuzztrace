//
// Copyright 2015, Roberto Paleari (@rpaleari) and Aristide Fattori (@joystick)
//

#ifndef _IMAGES_H
#define _IMAGES_H

#include "pin.H"

BOOL Images_IsInterestingAddress(ADDRINT addr);

// Image loading callback
VOID Images_CallbackNewImage(IMG img, VOID *v);

#endif  // _IMAGES_H
