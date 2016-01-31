#pragma once
#include <unistd.h>
void border_color_write(int wid, const char *buf);

int border_width_read(int wid, char *buf, size_t size);
void border_width_write(int wid, const char *buf);


int geometry_width_read(int wid, char *buf, size_t size);
void geometry_width_write(int wid, const char *buf);

int geometry_height_read(int wid, char *buf, size_t size);
void geometry_height_write(int wid, const char *buf);

int geometry_x_read(int wid, char *buf, size_t size);
void geometry_x_write(int wid, const char *buf);

int geometry_y_read(int wid, char *buf, size_t size);
void geometry_y_write(int wid, const char *buf);


int mapped_read(int wid, char *buf, size_t size);
void mapped_write(int wid, const char *buf);


int ignored_read(int wid, char *buf, size_t size);
void ignored_write(int wid, const char *buf);


void stack_write(int wid, const char *buf);


int title_read(int wid, char *buf, size_t size);
void title_write(int wid, const char *buf);


int class_read(int wid, char *buf, size_t size);


int event_read(int wid, char *buf, size_t size);


int focused_read(int wid, char *buf, size_t size);
void focused_write(int wid, const char *buf);


int generic_event_read(int wid, char *buf, size_t size);
