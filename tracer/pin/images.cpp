//
// Copyright 2015, Roberto Paleari (@rpaleari) and Aristide Fattori (@joystick)
//

#include <algorithm>
#include <string>
#include <utility>
#include <vector>

#include "images.H"
#include "common/serialize.h"

extern ExecutionTrace gbl_execution_trace;

// Blacklisted image names
static const std::string gbl_bad_images[] = {
#if 0
  "/lib/x86_64-linux-gnu/libacl.so.1",
  "/lib/x86_64-linux-gnu/libattr.so.1",
  "/lib/x86_64-linux-gnu/libc.so.6",
  "/lib/x86_64-linux-gnu/libdl.so.2",
  "/lib/x86_64-linux-gnu/libpcre.so.3",
  "/lib/x86_64-linux-gnu/libselinux.so.1",
  "/lib64/ld-linux-x86-64.so.2",
#endif
};

// Vector of std::string for blacklisted memory images
static const std::vector<std::string> gbl_img_blacklist(
  gbl_bad_images,
  gbl_bad_images + sizeof(gbl_bad_images) / sizeof(gbl_bad_images[0])
);

// Lower/upper bounds of active images (i.e., those we are interested to trace)
static std::vector<std::pair<ADDRINT, ADDRINT> > gbl_img_active;

BOOL Images_IsInterestingAddress(ADDRINT addr) {
  BOOL b = FALSE;
  for (std::vector<std::pair<ADDRINT, ADDRINT> >::iterator it =
         gbl_img_active.begin();
       it != gbl_img_active.end();
       it++) {
    if (addr >= it->first && addr <= it->second) {
      b = TRUE;
      break;
    }
  }
  return b;
}

VOID Images_CallbackNewImage(IMG img, VOID *v) {
  // Check if the image is blacklisted
  const std::string name = IMG_Name(img);
  if (std::find(gbl_img_blacklist.begin(), gbl_img_blacklist.end(), name)
      !=  gbl_img_blacklist.end()) {
    return;
  }

  // Otherwise, add the image address range to the global "gbl_img_active"
  // vector, to keep track of which instructions must be processed
  std::pair<ADDRINT, ADDRINT> range(IMG_LowAddress(img), IMG_HighAddress(img));
  gbl_img_active.push_back(range);

  MemoryRegion region;
  region.base = IMG_LowAddress(img);
  region.size = IMG_HighAddress(img) - IMG_LowAddress(img);
  region.filename = IMG_Name(img);
  gbl_execution_trace.memory_regions.push_back(region);
}
