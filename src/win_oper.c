#include "win_oper.h"
#include "win_xcb.h"
#include <stdio.h>
#include <stdlib.h>
#include "x11fs.h"

void border_color_write(int wid, const char *buf)
{

}

char *border_width_read(int wid)
{
	return 0;
}

void border_width_write(int wid, const char *buf)
{

}

char *geometry_width_read(int wid)
{
	return 0;
}

void geometry_width_write(int wid, const char *buf)
{

}

char *geometry_height_read(int wid)
{
	return 0;
}

void geometry_height_write(int wid, const char *buf)
{

}

char *geometry_x_read(int wid)
{
	return 0;
}

void geometry_x_write(int wid, const char *buf)
{

}

char *geometry_y_read(int wid)
{
	return 0;
}

void geometry_y_write(int wid, const char *buf)
{

}

char *mapped_read(int wid)
{
	return 0;
}

void mapped_write(int wid, const char *buf)
{

}

char *ignored_read(int wid)
{
	return 0;
}

void ignored_write(int wid, const char *buf)
{

}

void stack_write(int wid, const char *buf)
{

}

char *title_read(int wid)
{
	return 0;
}

char *class_read(int wid)
{
	return 0;
}

char *event_read(int wid)
{
	return get_events(wid);
}

char *focused_read(int wid)
{
	(void) wid;
	char *focusedwin = malloc(WID_STRING_LENGTH+1);
	sprintf(focusedwin, "0x%08x\n", focused());
	return focusedwin;
}

void focused_write(int wid, const char *buf)
{

}

char *generic_event_read(int wid)
{
	return 0;
}
