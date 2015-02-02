//
// Copyright 2015, Roberto Paleari (@rpaleari) and Aristide Fattori (@joystick)
//

#ifndef _MONITOR_H_
#define _MONITOR_H_

#include <unistd.h>

#include <string>
#include <vector>

void monitor_loop(pid_t pid_child, const std::string s_outfile);

#endif  // _MONITOR_H_
