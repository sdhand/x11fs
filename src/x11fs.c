#define FUSE_USE_VERSION 26

#include <stdio.h>
#include <fuse.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>
#include <string.h>
#include <fnmatch.h>
#include "x11fs.h"
#include "win_xcb.h"
#include "win_oper.h"


//Represents a single file, contains pointers to the functions to call to read and write for that file
struct x11fs_file{
	const char *path;
	int mode;
	bool direct_io;
	bool dir;
	char *(*read)(int wid);
	void (*write)(int wid, const char *buf);
};


//Our file layout
static const struct x11fs_file x11fs_files[] = {
	{"/",                     S_IFDIR | 0700, false, true,  NULL,                 NULL},
	{"/root",                 S_IFDIR | 0700, false, true,  NULL,                 NULL},
	{"/root/geometry",        S_IFDIR | 0700, false, true,  NULL,                 NULL},
	{"/root/geometry/width",  S_IFREG | 0400, false, false, root_width_read,      NULL},
	{"/root/geometry/height", S_IFREG | 0400, false, false, root_height_read,     NULL},
	{"/0x*",                  S_IFDIR | 0700, false, true,  NULL,                 NULL},
	{"/0x*/border",           S_IFDIR | 0700, false, true,  NULL,                 NULL},
	{"/0x*/border/color",     S_IFREG | 0200, false, false, NULL,                 border_color_write},
	{"/0x*/border/width",     S_IFREG | 0600, false, false, border_width_read,    border_width_write},
	{"/0x*/geometry",         S_IFDIR | 0700, false, true,  NULL,                 NULL},
	{"/0x*/geometry/width",   S_IFREG | 0600, false, false, geometry_width_read,  geometry_width_write},
	{"/0x*/geometry/height",  S_IFREG | 0600, false, false, geometry_height_read, geometry_height_write},
	{"/0x*/geometry/x",       S_IFREG | 0600, false, false, geometry_x_read,      geometry_x_write},
	{"/0x*/geometry/y",       S_IFREG | 0600, false, false, geometry_y_read,      geometry_y_write},
	{"/0x*/mapped",           S_IFREG | 0600, false, false, mapped_read,          mapped_write},
	{"/0x*/ignored",          S_IFREG | 0600, false, false, ignored_read,         ignored_write},
	{"/0x*/stack",            S_IFREG | 0200, false, false, NULL,                 stack_write},
	{"/0x*/title",            S_IFREG | 0400, false, false, title_read,           NULL},
	{"/0x*/class",            S_IFREG | 0400, false, false, class_read,           NULL},
	{"/focused",              S_IFREG | 0600, false, false, focused_read,         focused_write},
	{"/event",                S_IFREG | 0400, true,  false, event_read,           NULL},
};

//Pull out the id of a window from a path
static int get_winid(const char *path)
{
	int wid = -1;
	//Check if the path is to a window directory or it's contents
	if(strncmp(path, "/0x", 3) == 0)
		//Get the id
		sscanf(path, "/0x%08x", &wid);

	return wid;
}

//Runs when our filesystem is unmounted
static void x11fs_destroy()
{
	xcb_cleanup();
}

//Doesn't actually do anything but it's required we implement this as it will get called when writing a value to a file that's shorter than the current contents
static int x11fs_truncate(const char *path, off_t size)
{
	return 0;
}


//Gives information about a file
static int x11fs_getattr(const char *path, struct stat *stbuf)
{
	//zero the information about the file
	memset(stbuf, 0, sizeof(struct stat));

	//loop through our filesystem layout and check if the path matches one in our layout
	size_t files_length = sizeof(x11fs_files)/sizeof(struct x11fs_file);
	for(size_t i=0; i<files_length; i++){
		if(fnmatch(x11fs_files[i].path, path, FNM_PATHNAME) == 0){
			//if the path is to a window file, check that the window exists
			int wid;
			if((wid=get_winid(path)) != -1 && !exists(wid)){
				return -ENOENT;
			}

			//if a path matches just use the information about the file from the layout
			stbuf->st_nlink = 2 - !x11fs_files[i].dir;
			stbuf->st_mode = x11fs_files[i].mode;

			//Set the size of a file by getting its contents
			//If the file uses direct IO (it acts like a stream, just set size to 0)
			stbuf->st_size = 0;
			if((x11fs_files[i].read != NULL) && !(x11fs_files[i].direct_io))
			{
				char *read_string=x11fs_files[i].read(wid);
				if(read_string==NULL)
					return errno;

				stbuf->st_size=strlen(read_string);
				free(read_string);
			}

			return 0;
		}
	}

	//No such file or directory
	return -ENOENT;
}

//Gives the contents of a directory
static int x11fs_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi)
{
	(void) offset;
	(void) fi;

	//If the path is to a non existant window says so
	int wid;
	if((wid = get_winid(path)) != -1){
		if(!exists(wid))
			return -ENOENT;
	}

	bool exists = false;
	bool dir = false;


	//Iterate through our filesystem layout
	for(int i=0; i<sizeof(x11fs_files)/sizeof(struct x11fs_file); i++){

		//If the path was to a window replace the wildcard in the layout with the actual window we're looking at
		char *matchpath;
		if((wid != -1) && (get_winid(x11fs_files[i].path) != -1)){
			matchpath=malloc(strlen(x11fs_files[i].path)+8);
			sprintf(matchpath, "/0x%08x", wid);
			sprintf(matchpath+11, x11fs_files[i].path+4);
		}
		else
			matchpath=strdup(x11fs_files[i].path);


		//As the path for the root directory is just a / with no text we need to treat it as being 0 length
		//This is for when we check if something in our layout is in the folder we're looking at, but not in a subfolder
		int len;
		if(strcmp(path, "/") == 0)
			len = 0;
		else
			len = strlen(path);

		//If the file exists in our layout
		if(strncmp(path, matchpath, strlen(path)) == 0){
			exists = true;

			//Check that to see if an element in our layout is directly below the folder we're looking at in the heirarchy
			//If so add it to the directory listing
			if((strlen(matchpath) > strlen(path))
					&& ((matchpath+len)[0] == '/')
					&& (strchr(matchpath+len+1, '/') == NULL)){
				dir = true;

				//If it's a wildcarded window in our layout with the list of actual windows
				if(strcmp(matchpath, "/0x*") == 0){
					//Get the list of windows
					int *wins = list_windows();

					//Add each window to our directory listing
					for(int j=0; wins[j]; j++){
						int win = wins[j];
						char *win_string;

						win_string = malloc(sizeof(char)*(WID_STRING_LENGTH));
						sprintf(win_string, "0x%08x", win);

						filler(buf, win_string, NULL, 0);

						free(win_string);
					}

					free(wins);
				}
				//Otherwise just add the file to our directory listing
				else
					filler(buf, matchpath+len+1, NULL, 0);
			}
		}
		free(matchpath);
	}

	if(!exists)
		return -ENOENT;

	//Add any extra needed elements to the directory list
	if(dir){
		filler(buf, ".", NULL, 0);
		filler(buf, "..", NULL, 0);
	}else
		return -ENOTDIR;

	return 0;
}


//Open a file, just check if it exists and set non seekable
static int x11fs_open(const char *path, struct fuse_file_info *fi)
{
	//Iterate through our layout
	for(int i=0; i<sizeof(x11fs_files)/sizeof(struct x11fs_file); i++){
		//If our file is in the layout
		 if(fnmatch(x11fs_files[i].path, path, FNM_PATHNAME) == 0){
			//If the path is to a window check it exists
			int wid;
			if((wid=get_winid(path)) != -1){
				if(!exists(wid))
					return -ENOENT;
			}

			//Check if open makes sense
			if(x11fs_files[i].dir)
				return -EISDIR;

			fi->nonseekable=1;
			fi->direct_io=x11fs_files[i].direct_io;
			return 0;
		 }
	}
	return -ENOENT;
}

//Read a file
static int x11fs_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
	//Iterate through our layout
	for(int i=0; i<sizeof(x11fs_files)/sizeof(struct x11fs_file); i++){
		//If our file is in the layout
		if(fnmatch(x11fs_files[i].path, path, FNM_PATHNAME) == 0){
			//If the path is to a window check it exists
			int wid;
			if((wid=get_winid(path)) != -1){
				if(!exists(wid))
					return -ENOENT;
			}

			//Check we can actually read
			if(x11fs_files[i].dir)
				return -EISDIR;

			if(x11fs_files[i].read == NULL)
				return -EACCES;

			//Call the read function and stick the results in the buffer
			char *result= x11fs_files[i].read(wid);
			if(result == NULL)
				return errno;

			size_t len = strlen(result);
			if(size > len)
				size = len;
			memcpy(buf, result, size);
			free(result);
			return size;
		}
	}
	return -ENOENT;
}

//Write to a file
static int x11fs_write(const char *path, const char *buf, size_t size, off_t offset,  struct fuse_file_info *fi)
{
	//Iterate through our layout
	for(int i=0; i<sizeof(x11fs_files)/sizeof(struct x11fs_file); i++){
		//If our file is in the layout
		if(fnmatch(x11fs_files[i].path, path, FNM_PATHNAME) == 0){
			//If the path is to a window check it exists
			int wid;
			if((wid=get_winid(path)) != -1){
				if(!exists(wid))
					return -ENOENT;
			}

			//Check we can actually read
			if(x11fs_files[i].dir)
				return -EISDIR;

			if(x11fs_files[i].write == NULL)
				return -EACCES;

			//Call the write function
			x11fs_files[i].write(wid, buf);
		}
	}
	return size;
}

//Delete a folder (closes a window)
static int x11fs_rmdir(const char *path)
{
	//Check the folder is one representing a window
	//Returning ENOSYS because sometimes this will be on a dir, just not one that represents a window
	//TODO: Probably return more meaningful errors
	int wid;
	if((wid=get_winid(path)) == -1)
		return -ENOSYS;

	if(strlen(path)>11)
		return -ENOSYS;

	//Close the window
	close_window(wid);
	return 0;
}

//Functions for fuse to run on IO events
static struct fuse_operations x11fs_operations = {
	.destroy  = x11fs_destroy,
	.truncate = x11fs_truncate,
	.getattr  = x11fs_getattr,
	.readdir  = x11fs_readdir,
	.open     = x11fs_open,
	.read     = x11fs_read,
	.write    = x11fs_write,
	.rmdir    = x11fs_rmdir,
};

//Just setup our connection to X then let fuse handle the rest
int main(int argc, char **argv)
{
	if(xcb_init()!=X11FS_SUCCESS){
		fputs("Failed to setup xcb. Quiting...\n", stderr);
		return 1;
	}
	return fuse_main(argc, argv, &x11fs_operations, NULL);
}
