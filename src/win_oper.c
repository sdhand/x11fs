#include "win_oper.h"
#include "win_xcb.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "x11fs.h"

//Specific read and write functions for each file

void border_color_write(int wid, const char *buf)
{
	set_border_color(wid, strtol(buf, NULL, 16));
}

char *border_width_read(int wid)
{
	int border_width=get_border_width(wid);
	if(border_width==-1){
		errno = -EIO;
		return NULL;
	}
	//Work out the size needed to malloc by calling snprintf with a size of 0
	char *border_width_string=malloc(snprintf(NULL, 0, "%d\n", border_width)+1);
	sprintf(border_width_string, "%d\n", border_width);
	return border_width_string;
}

void border_width_write(int wid, const char *buf)
{
	set_border_width(wid, atoi(buf));
}

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

char *geometry_width_read(int wid)
{
	int width=get_width(wid);
	if(width==-1){
		errno = -EIO;
		return NULL;
	}

	char *width_string=malloc(snprintf(NULL, 0, "%d\n", width)+1);
	sprintf(width_string, "%d\n", width);
	return width_string;
}

void geometry_width_write(int wid, const char *buf)
{
	set_width(wid, atoi(buf));
}

char *geometry_height_read(int wid)
{
	int height=get_height(wid);
	if(height==-1){
		errno = -EIO;
		return NULL;
	}

	char *height_string=malloc(snprintf(NULL, 0, "%d\n", height)+1);
	sprintf(height_string, "%d\n", height);
	return height_string;
}

void geometry_height_write(int wid, const char *buf)
{
	set_height(wid, atoi(buf));
}

char *geometry_x_read(int wid)
{
    int x=get_x(wid);
    if(x==-1){
        errno = -EIO;
        return NULL;
    }

    char *x_string=malloc(snprintf(NULL, 0, "%d\n", x)+1);
    sprintf(x_string, "%d\n", x);
    return x_string;
}

void geometry_x_write(int wid, const char *buf)
{
	set_x(wid, atoi(buf));
}

char *geometry_y_read(int wid)
{
    int y=get_y(wid);
    if(y==-1){
        errno = -EIO;
        return NULL;
    }

    char *y_string=malloc(snprintf(NULL, 0, "%d\n", y)+1);
    sprintf(y_string, "%d\n", y);
    return y_string;
}

void geometry_y_write(int wid, const char *buf)
{
	set_y(wid, atoi(buf));
}

char *mapped_read(int wid)
{
	return strdup(get_mapped(wid) ? "true\n" : "false\n");
}

void mapped_write(int wid, const char *buf)
{
	set_mapped(wid, !strcmp(buf, "true\n"));
}

char *ignored_read(int wid)
{
	return strdup(get_ignored(wid) ? "true\n" : "false\n");
}

void ignored_write(int wid, const char *buf)
{
	set_ignored(wid, !strcmp(buf, "true\n"));
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
