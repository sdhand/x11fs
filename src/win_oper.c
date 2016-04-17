#include "win_oper.h"
#include "win_xcb.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <syslog.h>
#include "x11fs.h"

//Specific read and write functions for each file

void border_color_write(int wid, const char *buf)
{
	set_border_color(wid, strtol(buf, NULL, 16));
}

#define DECLARE_NORM_READER(cat, prop, getter) \
	char * cat##_##prop##_read (int wid) {\
		int i = getter(wid);\
		if ( i == -1 ) {\
			errno = -EIO;\
			return NULL;\
		}\
		\
		char * str = malloc(snprintf(NULL, 0, "%d\n", i) + 1);\
		if ( !str ) {\
			syslog(LOG_ERR, "failed to allocate in %s: %s\n", __func__, strerror(ENOMEM));\
		}\
		\
		if ( sprintf(str, "%d\n", i) < 0 ) {\
			syslog(LOG_ERR, "failed to store value in %s\n", __func__);\
		}\
		\
		return str;\
	}

DECLARE_NORM_READER(border,   width,  get_border_width);
DECLARE_NORM_READER(geometry, width,  get_width);
DECLARE_NORM_READER(geometry, height, get_height);
DECLARE_NORM_READER(geometry, x,      get_x);
DECLARE_NORM_READER(geometry, y,      get_y);

#define DECLARE_NORM_WRITER(cat, prop, setter) \
	void cat##_##prop##_write (int wid, const char * buf) {\
		setter(wid, atoi(buf));\
	}

DECLARE_NORM_WRITER(border,   width,  set_border_width);
DECLARE_NORM_WRITER(geometry, width,  set_width);
DECLARE_NORM_WRITER(geometry, height, set_height);
DECLARE_NORM_WRITER(geometry, x,      set_x);
DECLARE_NORM_WRITER(geometry, y,      set_y);

char *root_width_read(int wid)
{
	(void) wid;
	return geometry_width_read(-1);
}

char *root_height_read(int wid)
{
	(void) wid;
	return geometry_height_read(-1);
}

char *mapped_read(int wid)
{
	return strdup(get_mapped(wid) ? "true\n" : "false\n");
}

void mapped_write(int wid, const char *buf)
{
	if(!strcmp(buf, "true\n"))
		set_mapped(wid, true);
	if(!strcmp(buf, "false\n"))
		set_mapped(wid, false);
}

char *ignored_read(int wid)
{
	return strdup(get_ignored(wid) ? "true\n" : "false\n");
}

void ignored_write(int wid, const char *buf)
{
	if(!strcmp(buf, "true\n"))
		set_ignored(wid, true);
	if(!strcmp(buf, "false\n"))
		set_ignored(wid, false);
}

void stack_write(int wid, const char *buf)
{
	if(!strcmp(buf, "raise\n"))
		raise(wid);
	if(!strcmp(buf, "lower\n"))
		lower(wid);
}

char *title_read(int wid)
{
	char *title=get_title(wid);
	size_t title_len = strlen(title);
	char *title_string=malloc(title_len+2);
	memset(title_string, 0, title_len+2);
	if ( title_len ) { sprintf(title_string, "%s\n", title); }
	free(title);
	return title_string;
}

char *class_read(int wid)
{
	char **classes=get_class(wid);
	size_t class0_len = strlen(classes[0]), class1_len = strlen(classes[1]);
	char *class_string=malloc(class0_len + class1_len + 3);
	if ( class0_len ) {
		sprintf(class_string, "%s\n", classes[0]);
	}
	if ( class1_len ) {
		sprintf(class_string + class0_len + 1, "%s\n", classes[1]);
	}
	free(classes[0]);
	free(classes[1]);
	free(classes);
	return class_string;
}

char *event_read(int wid)
{
	return get_events();
}

char *focused_read(int wid)
{
	(void) wid;
	char *focusedwin;
	int focusedid=focused();
	if(focusedid){
		focusedwin = malloc(WID_STRING_LENGTH+1);
		sprintf(focusedwin, "0x%08x\n", focusedid);
	}else{
		focusedwin = malloc(6);
		sprintf(focusedwin, "root\n");
	}
	return focusedwin;
}

void focused_write(int wid, const char *buf)
{
	(void) wid;
	focus(strtol(buf, NULL, 16));
}
