#define FUSE_USE_VERSION 26

#include <stdio.h>
#include <fuse.h>
#include "x11fs.h"
#include "win_xcb.h"

static void x11fs_destroy()
{

}

static int x11fs_truncate(const char *path, off_t size)
{
	return 0;	
}

static int x11fs_getattr(const char *path, struct stat *stbuf)
{
	return 0;
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

//A list of functions for fuse to run on IO events
static struct fuse_operations x11fs_operations = {
	.destroy  = x11fs_destroy,
	.truncate = x11fs_truncate,
	.getattr  = x11fs_getattr,
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
