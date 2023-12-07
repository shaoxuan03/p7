#define FUSE_USE_VERSION 30
#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stddef.h>
#include <stdlib.h>
#include "wfs.h"


static struct wfs_inode* inode_finder(const char *path){
    //struct wfs_log_entry *current_log = 0;//this thing here need to be some entry point;
    struct wfs_inode *curr_inode = 0;
    char* copy = strdup(path);
    char* token = strtok(copy, "/");
    while(token != 0){
        int found = 0;
        size_t num_of_entries = 1000000; // this thing should be the number of entries available in the disk
        for(size_t i = 0; i < num_of_entries; i++){
            // if(strcmp(token, current_log -> (wfs_dentry)data[i].name) == 0){ 
            //     *curr_inode = current_log->inode;
            //     found == 1;
            //     break;
            // }
        }
        if (!found || curr_inode == NULL) {
            // Handle path component not found or invalid path
            free(copy);
            return (void*)NULL;
        }
        token = strtok(NULL, "/");
    }

    free(copy);
    return curr_inode;
}

static int wfs_getattr(const char *path, struct stat *stbuf) {
    // Implementation of getattr function to retrieve file attributes
    // Fill stbuf structure with the attributes of the file/directory indicated by path
    // ...
    int res = 0;
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
        //need to implement get size
        // fseek(filename, 0L, SEEK_END);
        // int sz = ftell(filename);
        // stbuf->st_size = sz;
    }
    return 0; // Return 0 on success
}

static int wfs_mknod(const char* path, mode_t mode, dev_t rdev) {
    return 0;
}

static int wfs_mkdir(const char *path, mode_t mode){
    //struct wfs_log_entry newlog;
    int res;
    res = mkdir(path, mode);
    if(res == -1)
        return -errno;

    return 0;
}

static int wfs_read(const char *path, char *buf, size_t size, off_t offset,
			struct fuse_file_info *fi){
    
    inode_finder(path);
    //how to access the file that I want 
    return 0;
}

static int wfs_write(const char *path, const char *buf, size_t size,
             off_t offset, struct fuse_file_info *fi){
    int fd;
    int res;

    (void) fi;
    fd = open(path, O_WRONLY);
    if (fd == -1)
        return -errno;

    res = pwrite(fd, buf, size, offset);
    if (res == -1)
        res = -errno;

    close(fd);
    return res;
}

/**
 * RETURNS
 *  returns 0 on success
*/
static int wfs_readdir(const char* path, void* buf, 
    fuse_fill_dir_t filler, off_t offset, struct fuse_file_info* fi) {
    

    filler(buf, ".", NULL, 0);  // Current Directory
    filler(buf, "..", NULL, 0); // Parent Directory

    if (strcmp(path, "/") == 0) {
        filler(buf, "some placeholder", NULL, 0);
    }


    return 0;
}

static int wfs_unlink(const char *path) {
    int res;
    res = unlink(path);
    if(res == -1)
        return -errno;

    return 0;
}


static struct fuse_operations wfs_operations = {
    // Add other functions (read, write, mkdir, etc.) here as needed
    .getattr    = wfs_getattr,
    .mknod      = wfs_mknod,
    .mkdir      = wfs_mkdir,
    .read       = wfs_read,
    .write      = wfs_write,
    .readdir    = wfs_readdir,
    .unlink     = wfs_unlink,

};

int main(int argc, char *argv[]) {
    // Initialize FUSE with specified operations
    // Filter argc and argv here and then pass it to fuse_main
    char *new_argv[argc - 1];
    for (int i = 0; i < argc; i++) {
        if (i == 3) {
            new_argv[i] = argv[i+1];
            break;
        }
        new_argv[i] = argv[i];
    }
    return fuse_main(argc - 1, new_argv, &wfs_operations, NULL);
}