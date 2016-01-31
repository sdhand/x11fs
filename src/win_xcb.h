#pragma once
#include "x11fs.h"
#include <stdbool.h>

X11FS_STATUS xcb_init();
void xcb_cleanup();

bool exists(int wid);
