#define FUSE_USE_VERSION 30
#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stddef.h>
#include <stdlib.h>

//parse the path into filename
char* get_filename(const char* path){
    char* filename = 0;
    strcpy(filename, path);

    while(*filename != '\0'){
        filename++;
    }

    while(*filename != '/'){
        filename--;
    }
    //at this point, filename pointing to the '/'
    filename++;
    return filename;
}

static int wfs_getattr(const char *path, struct stat *stbuf) {
    // Implementation of getattr function to retrieve file attributes
    // Fill stbuf structure with the attributes of the file/directory indicated by path
    // ...
    int res = 0;
    //char* filename = get_filename(path);
    memset(stbuf, 0, sizeof(struct stat));
    if(strcmp(path, "/") == 0){
        stbuf->st_uid = (uid_t)getuid();
        stbuf->st_gid = (uid_t)getgid();
        stbuf->st_mtime = time(NULL);
        stbuf->st_mode =  __S_IFDIR | 0755;
        stbuf->st_nlink = 2;
        return res;
    }
    else{
        stbuf->st_uid = (uid_t)getuid();
        stbuf->st_gid = (uid_t)getgid();
        stbuf->st_mtime = time(NULL);
        stbuf->st_mode =  __S_IFREG | 0755;
        stbuf->st_nlink = 1;
        // fseek(filename, 0L, SEEK_END);
        // int sz = ftell(filename);
        // stbuf->st_size = sz;
    }
    return 0; // Return 0 on success
}

static int wfs_mknod(){
    return 0;
}

static int wfs_mkdir(const char *path, mode_t mode){

    int res;
    res = mkdir(path, mode);
    if(res == -1)
        return -errno;

    return 0;
}
static int wfs_read(){
    return 0;
}
static int wfs_write(){
    return 0;
}
static int wfs_readdir(){
    return 0;
}

static int wfs_unlink(const char *path){
    int res;
    res = unlink(path);
    if(res == -1)
        return -errno;

    return 0;
}


static struct fuse_operations wfs_operations = {
    .getattr    = wfs_getattr,
    .mknod      = wfs_mknod,
    .mkdir      = wfs_mkdir,
    .read	    = wfs_read,
    .write      = wfs_write,
    .readdir	= wfs_readdir,
    .unlink    	= wfs_unlink,

    // Add other functions (read, write, mkdir, etc.) here as needed
};

int main(int argc, char *argv[]) {
    // Initialize FUSE with specified operations
    // Filter argc and argv here and then pass it to fuse_main
    return fuse_main(argc, argv, &wfs_operations, NULL);
}