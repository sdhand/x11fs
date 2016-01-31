#define FUSE_USE_VERSION 26

#include <stdio.h>
#include <fuse.h>
#include <stdbool.h>
#include <errno.h>
#include <string.h>
#include <fnmatch.h>
#include "x11fs.h"
#include "win_xcb.h"


//Represents a single file, contains pointers to the functions to call to read and write for that file
struct x11fs_file{
	const char *path;
	int mode;
	bool dir;
	char *(*read)(int wid);
	void (*write)(int wid, const char *buf);	
};


//Our file layout
static const struct x11fs_file x11fs_files[] = {
	{"/", S_IFDIR | 0700, true, NULL, NULL},
		{"/0x*", S_IFDIR | 0700, true, NULL, NULL},
			{"/0x*/border", S_IFDIR | 0700, true, NULL, NULL},
				{"/0x*/border/color", S_IFREG | 0200, false, NULL, NULL},
				{"/0x*/border/width", S_IFREG | 0600, false, NULL, NULL},
			{"/0x*/geometry", S_IFDIR | 0700, true, NULL, NULL},
				{"/0x*/geometry/width", S_IFREG | 0600, false, NULL, NULL},
				{"/0x*/geometry/height", S_IFREG | 0600, false, NULL, NULL},
				{"/0x*/geometry/x", S_IFREG | 0600, false, NULL, NULL},
				{"/0x*/geometry/y", S_IFREG | 0600, false, NULL, NULL},
			{"/0x*/mapped", S_IFREG | 0600, false, NULL, NULL},
			{"/0x*/ignored", S_IFREG | 0600, false, NULL, NULL},
			{"/0x*/stack", S_IFREG | 0600, false, NULL, NULL},
			{"/0x*/title", S_IFREG | 0600, false, NULL, NULL},
			{"/0x*/class", S_IFREG | 0600, false, NULL, NULL},
			{"/0x*/event", S_IFREG | 0400, false, NULL, NULL},
		{"/focused", S_IFREG | 0600, false, NULL, NULL},
		{"/event", S_IFREG | 0400, false, NULL, NULL},
};

//Pull out the id of a window from a path
static int get_winid(const char *path)
{
	int wid = 0;
	//Check if the path is to a window directory or it's contents
	if(strncmp(path, "/0x", 3) == 0)
		//Get the id
		sscanf(path, "/0x%08x", &wid);
	else
		return -1;

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
	for(int i=0; i<sizeof(x11fs_files)/sizeof(struct x11fs_file); i++){
		if(fnmatch(x11fs_files[i].path, path, FNM_PATHNAME) == 0){
			//if the path is to a window file, check that the window exists
			int wid;
			if((wid=get_winid(path)) != -1){
				if(!exists(wid))
					return -ENOENT;
			}

			//if a path matches just use the information about the file from the layout
			if(x11fs_files[i].dir)
				stbuf->st_nlink = 2;
			else
				stbuf->st_nlink = 1;
			stbuf->st_mode = x11fs_files[i].mode;
			return 0;
		}
	}

	//No such file or directory
	return -ENOENT;
}

static int x11fs_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi)
{
	(void) offset;
	(void) fi;

	for(int i=0; i<sizeof(x11fs_files)/sizeof(struct x11fs_file); i++){
		
	}
}

static int x11fs_open(const char *path, struct fuse_file_info *fi)
{
	return 0;
}

static int x11fs_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
	return 0;
}

static int x11fs_write(const char *path, const char *buf, size_t size, off_t offset,  struct fuse_file_info *fi)
{
	return 0;
}

static int x11fs_rmdir(const char *path)
{
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
		fprintf(stderr, "Failed to setup xcb. Quiting...\n");
		return 1;
	}
	return fuse_main(argc, argv, &x11fs_operations, NULL);
}
