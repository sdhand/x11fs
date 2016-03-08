#include <xcb/xcb.h>
#include <err.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "x11fs.h"

//Our connection to xcb and our screen
static xcb_connection_t *conn;
static xcb_screen_t *scrn;

//Setup our connection to the X server and get the first screen
//TODO: Check how this works with multimonitor setups
X11FS_STATUS xcb_init()
{
	conn = xcb_connect(NULL, NULL);
	if(xcb_connection_has_error(conn)){
		warnx("Cannot open display: %s", getenv("DISPLAY"));
		return X11FS_FAILURE;
	}

	scrn = xcb_setup_roots_iterator(xcb_get_setup(conn)).data;
	if(scrn == NULL){
		warnx("Cannot retrieve screen information");
		return X11FS_FAILURE;
	}
	return X11FS_SUCCESS;
}

//End our connection
void xcb_cleanup(){
	if(conn != NULL)
		xcb_disconnect(conn);
}

//check if a window exists
bool exists(int wid)
{
	xcb_get_window_attributes_cookie_t attr_c = xcb_get_window_attributes(conn, wid);
	xcb_get_window_attributes_reply_t *attr_r = xcb_get_window_attributes_reply(conn, attr_c, NULL);

	if(attr_r == NULL)
		return false;

	free(attr_r);
	return true;
}

//List every open window
int *list_windows()
{
	//Get the window tree for the root window
	xcb_query_tree_cookie_t tree_c = xcb_query_tree(conn, scrn->root);
	xcb_query_tree_reply_t *tree_r = xcb_query_tree_reply(conn, tree_c, NULL);

	if(tree_r == NULL)
		warnx("Couldn't find the root window's");

	//Get the array of windows
	xcb_window_t *xcb_win_list = xcb_query_tree_children(tree_r);
	if(xcb_win_list == NULL)
		warnx("Couldn't find the root window's children");

	int *win_list = malloc(sizeof(int)*(tree_r->children_len+1));
	int i;
	for (i=0; i<tree_r->children_len; i++) {
		 win_list[i] = xcb_win_list[i];
	}

	free(tree_r);

	//Null terminate our list
	win_list[i]=0;
	return win_list;
}

//This is a terrible way to close windows
//TODO: use EWMH atoms here to get the window to close rather than just killing it
void close_window(int wid)
{
	xcb_kill_client(conn, wid);
	xcb_flush(conn);
}

//Get the focused window
int focused()
{
	//Ask xcb for the focused window
	xcb_get_input_focus_cookie_t focus_c;
	xcb_get_input_focus_reply_t *focus_r;

	focus_c = xcb_get_input_focus(conn);
	focus_r = xcb_get_input_focus_reply(conn, focus_c, NULL);

	//Couldn't find the focused window
	if(focus_r == NULL)
		return -1;

	int focused = focus_r->focus;
	if(focused==scrn->root)
		focused=0;
	free(focus_r);
	return focused;
}

//Change the focus
void focus(int wid)
{
	xcb_set_input_focus(conn, XCB_INPUT_FOCUS_POINTER_ROOT, wid, XCB_CURRENT_TIME);
	xcb_flush(conn);
}

//Get the properties of a window (title, class etc)
static xcb_get_property_reply_t *get_prop(int wid, xcb_atom_t property, xcb_atom_t type)
{
	xcb_get_property_cookie_t prop_c = xcb_get_property(conn, 0, wid, property, type, 0L, 32L);
	return xcb_get_property_reply(conn, prop_c, NULL);
}

//Get the geometry of a window
static xcb_get_geometry_reply_t *get_geom(int wid)
{
	xcb_get_geometry_cookie_t geom_c = xcb_get_geometry(conn, wid);
	return xcb_get_geometry_reply(conn, geom_c, NULL);
}

//Get the attributes of a window (mapped, ignored etc)
static xcb_get_window_attributes_reply_t *get_attr(int wid)
{
	xcb_get_window_attributes_cookie_t attr_c = xcb_get_window_attributes(conn, wid);
	return xcb_get_window_attributes_reply(conn, attr_c, NULL);
}

//Bunch of functions to get and set window properties etc.
//All should be fairly self explanatory

int get_width(int wid)
{
	if(wid==-1)
		wid=scrn->root;
	xcb_get_geometry_reply_t *geom_r = get_geom(wid);
	if(geom_r == NULL)
		return -1;

	int width = geom_r->width;
	free(geom_r);
	return width;
}

void set_width(int wid, int width)
{
	uint32_t values[] = {width};
	xcb_configure_window(conn, wid, XCB_CONFIG_WINDOW_WIDTH, values);
	xcb_flush(conn);
}

int get_height(int wid)
{
	if(wid==-1)
		wid=scrn->root;
	xcb_get_geometry_reply_t *geom_r = get_geom(wid);
	if(geom_r == NULL)
		return -1;

	int height = geom_r->height;
	free(geom_r);
	return height;
}

void set_height(int wid, int height)
{
	uint32_t values[] = {height};
	xcb_configure_window(conn, wid, XCB_CONFIG_WINDOW_HEIGHT, values);
	xcb_flush(conn);
}

int get_x(int wid)
{
	xcb_get_geometry_reply_t *geom_r = get_geom(wid);
	if(geom_r == NULL)
		return -1;

	int x = geom_r->x;
	free(geom_r);
	return x;
}

void set_x(int wid, int x)
{
    uint32_t values[] = {x};
    xcb_configure_window(conn, wid, XCB_CONFIG_WINDOW_X, values);
    xcb_flush(conn);
}

int get_y(int wid)
{
    xcb_get_geometry_reply_t *geom_r = get_geom(wid);
    if(geom_r == NULL)
        return -1; 

    int y = geom_r->y;
    free(geom_r);
    return y;
}

void set_y(int wid, int y)
{
    uint32_t values[] = {y};
    xcb_configure_window(conn, wid, XCB_CONFIG_WINDOW_Y, values);
    xcb_flush(conn);
}

int get_border_width(int wid)
{
    xcb_get_geometry_reply_t *geom_r = get_geom(wid);
    if(geom_r == NULL)
        return -1; 

    int bw = geom_r->border_width;
    free(geom_r);
    return bw; 
}

void set_border_width(int wid, int width)
{
    uint32_t values[] = {width};
    xcb_configure_window(conn, wid, XCB_CONFIG_WINDOW_BORDER_WIDTH, values);
    xcb_flush(conn);
}

void set_border_color(int wid, int color)
{
    uint32_t values[] = {color};
    xcb_change_window_attributes(conn, wid, XCB_CW_BORDER_PIXEL, values);
    xcb_flush(conn);
}

int get_mapped(int wid)
{
    xcb_get_window_attributes_reply_t *attr_r = get_attr(wid);
    if(attr_r == NULL)
        return -1; 

    int map_state = attr_r->map_state;
    free(attr_r);
    if(map_state == XCB_MAP_STATE_VIEWABLE){
        return 1;
    }
    return 0;
}

void set_mapped(int wid, int mapstate)
{
    if(mapstate)
        xcb_map_window(conn, wid);
    else
        xcb_unmap_window(conn, wid);
    xcb_flush(conn);
}

int get_ignored(int wid)
{
    xcb_get_window_attributes_reply_t *attr_r = get_attr(wid);
    if(attr_r == NULL)
        return -1;

    int or = attr_r->override_redirect;
    free(attr_r);
    return or;
}

void set_ignored(int wid, int ignore)
{
    uint32_t values[] = {ignore};
    xcb_change_window_attributes(conn, wid, XCB_CW_OVERRIDE_REDIRECT, values);
}

char *get_title(int wid)
{
    xcb_get_property_reply_t *prop_r = get_prop(wid, XCB_ATOM_WM_NAME, XCB_ATOM_STRING);
    if(prop_r == NULL)
        return NULL;

    char *title = (char *) xcb_get_property_value(prop_r);
    int len = xcb_get_property_value_length(prop_r);
    char *title_string=malloc(len+1);
    sprintf(title_string, "%.*s", len, title);
    free(prop_r);
    return title_string;
}

//Get an array of the classes of the window
char **get_class(int wid)
{
    char **classes = malloc(sizeof(char*)*2);
    xcb_get_property_reply_t *prop_r = get_prop(wid, XCB_ATOM_WM_CLASS, XCB_ATOM_STRING);
    if(prop_r == NULL) {
        free(classes);
        return NULL;
    }

    char *class;
    class=(char *) xcb_get_property_value(prop_r);
    classes[0]=strdup(class);
    classes[1]=strdup(class+strlen(class)+1);

    free(prop_r);
    return classes;
}

void raise(int wid)
{
    uint32_t values[] = {XCB_STACK_MODE_ABOVE};
    xcb_configure_window(conn, wid, XCB_CONFIG_WINDOW_STACK_MODE, values);
    xcb_flush(conn);
}

void lower(int wid)
{
    uint32_t values[] = {XCB_STACK_MODE_BELOW};
    xcb_configure_window(conn, wid, XCB_CONFIG_WINDOW_STACK_MODE, values);
    xcb_flush(conn);
}

static void unsubscribe(int wid){
	uint32_t values[] = {XCB_EVENT_MASK_NO_EVENT};
	xcb_change_window_attributes(conn, wid, XCB_CW_EVENT_MASK, values);
}

static void subscribe(int wid){
	uint32_t values[] = {XCB_EVENT_MASK_ENTER_WINDOW|XCB_EVENT_MASK_LEAVE_WINDOW};
	xcb_change_window_attributes(conn, wid, XCB_CW_EVENT_MASK, values);
}

//Get events for a window
char *get_events(){
	//Subscribe to events from all windows
	uint32_t values[] = {XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY};
	xcb_change_window_attributes(conn, scrn->root, XCB_CW_EVENT_MASK, values);
	int *windows=list_windows();
	int wid;
	while((wid=*(windows++))){
		if(!get_ignored(wid))
			subscribe(wid);
	}
	xcb_flush(conn);

	char *event_string;
	bool done;	
	while(!done){
		xcb_generic_event_t *event = xcb_wait_for_event(conn);
		int wid;
		switch (event->response_type & ~0x80){
			case XCB_CREATE_NOTIFY:
				wid=((xcb_create_notify_event_t*)event)->window;
				event_string=malloc(snprintf(NULL, 0, "CREATE: 0x%08x\n", wid)+1);
				sprintf(event_string, "CREATE: 0x%08x\n", wid);
				done=true;
				break;

			case XCB_DESTROY_NOTIFY:
				wid=((xcb_create_notify_event_t*)event)->window;
				event_string=malloc(snprintf(NULL, 0, "DESTROY: 0x%08x\n", wid)+1);
				sprintf(event_string, "DESTROY: 0x%08x\n", wid);
				done=true;
				break;

			case XCB_ENTER_NOTIFY:
				wid = ((xcb_enter_notify_event_t*)event)->event;
				event_string=malloc(snprintf(NULL, 0, "ENTER: 0x%08x\n", wid)+1);
				sprintf(event_string, "ENTER: 0x%08x\n", wid);
				done=true;
				break;

			case XCB_LEAVE_NOTIFY:
				wid = ((xcb_enter_notify_event_t*)event)->event;
				event_string=malloc(snprintf(NULL, 0, "LEAVE: 0x%08x\n", wid)+1);
				sprintf(event_string, "LEAVE: 0x%08x\n", wid);
				done=true;
				break;

			case XCB_MAP_NOTIFY:
				wid = ((xcb_map_notify_event_t*)event)->window;
				event_string=malloc(snprintf(NULL, 0, "MAP: 0x%08x\n", wid)+1);
				sprintf(event_string, "MAP: 0x%08x\n", wid);
				done=true;
				break;

			case XCB_UNMAP_NOTIFY:
				wid = ((xcb_map_notify_event_t*)event)->window;
				event_string=malloc(snprintf(NULL, 0, "UNMAP: 0x%08x\n", wid)+1);
				sprintf(event_string, "UNMAP: 0x%08x\n", wid);
				done=true;
				break;
		}
	}
	//Unsubscribe from events
	unsubscribe(scrn->root);
	while((wid=*(windows++))){
		unsubscribe(wid);
	}

	return event_string;
}
