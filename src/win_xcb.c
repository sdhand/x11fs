#include <xcb/xcb.h>
#include <err.h>
#include <stdlib.h>
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
