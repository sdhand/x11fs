#pragma once
#include "x11fs.h"
#include <stdbool.h>

X11FS_STATUS xcb_init();
void xcb_cleanup();

bool exists(int wid);
int *list_windows();
void close_window(int wid);

int focused();
void focus(int wid);

int get_width(int wid);
void set_width(int wid, int width);

int get_height(int wid);
void set_height(int wid, int height);

int get_x(int wid);
void set_x(int wid, int x);

int get_y(int wid);
void set_y(int wid, int y);

int get_border_width(int wid);
void set_border_width(int wid, int width);

void set_border_color(int wid, int color);

bool get_mapped(int wid);
void set_mapped(int wid, bool mapped);

bool get_ignored(int wid);
void set_ignored(int wid, bool ignored);

char *get_title(int wid);

char **get_class(int wid);

void raise(int wid);
void lower(int wid);

char *get_events(int wid);

char *get_root_events();
