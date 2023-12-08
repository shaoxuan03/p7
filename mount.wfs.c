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
#include <fcntl.h>
#include <sys/mman.h>

//global variable
struct wfs_starting_point* start;

//use inode num to find log entry
static struct wfs_log_entry* inode_num_to_log(unsigned long inode_number){
    struct wfs_log_entry* start_entry =  (struct wfs_log_entry*)(start->disk + sizeof(struct wfs_sb));
    struct wfs_log_entry* log_entry = 0;
    struct wfs_inode *curr_inode = 0;
    while(start_entry <= start->head){
        if(start_entry->inode.inode_number == inode_number){
            if(start_entry->inode.deleted == 0){
                log_entry = start_entry;
                break;
            }
            else{
                start_entry += sizeof(struct wfs_log_entry);
            }
        }
    }
    return log_entry;
}

//use the name to find inode num, then call the helper method to find log entry
static struct wfs_log_entry* inode_finder(const char *path){
    //first thing is to start with root
    int root_inode_num = 0;
    struct wfs_log_entry* root_log = inode_num_to_log(root_inode_num);
    struct wfs_log_entry* curr_log;
    int curr_inode_num = 0;
    //found the root, access the data 
    char* copy = strdup(path);
    char* token = strtok(copy, "/");
    //struct wfs_dentry* arr = (struct wfs_dentry*)root_log -> data;
    while(token != NULL){
        for(int i = 0; i < root_log->inode.size/sizeof(struct wfs_dentry); i++){
            if(token == (struct wfs_dentry) root_log->data.){
                curr_inode_num = arr[i].inode_number;
                curr_log = inode_num_to_log(curr_inode_num);
                break;
            }
        }
        token = strtok(NULL, "/");
    }
    return &curr_log->inode;
}

static int wfs_getattr(const char *path, struct stat *stbuf) {
    // Implementation of getattr function to retrieve file attributes
    // Fill stbuf structure with the attributes of the file/directory indicated by path
    // ...
    printf("hi\n");
    int res = 0;
    memset(stbuf, 0, sizeof(struct stat));
    if(strcmp(path, "/") == 0){
        // Root directory
        stbuf->st_uid = (uid_t)getuid();
        stbuf->st_gid = (uid_t)getgid();
        stbuf->st_mtime = time(NULL);
        stbuf->st_mode =  __S_IFDIR | 0755;
        stbuf->st_nlink = 2;
        return res;
    }else if (strcmp(path, "/hello") == 0) {
        // File "hello" inside the root directory
        stbuf->st_mode = S_IFREG | 0444;
        stbuf->st_nlink = 1;
        stbuf->st_size = strlen("Hello, World!");
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
   // char* copy_path = strdup(path);

    return 0;
}

static int wfs_mkdir(const char *path, mode_t mode){
    //struct wfs_log_entry newlog;
    // this is wrong
    printf("running create dir\n");
    // int res;
    // res = mkdir(path, mode);
    // if(res == -1)
    //     return -errno;
    inode_finder(path);
    return 0;
}

static int wfs_read(const char *path, char *buf, size_t size, off_t offset,
			struct fuse_file_info *fi){
    
    struct wfs_inode *current_inode = inode_finder(path);
    struct wfs_log_entry *log = 0;
    if(current_inode == &log->inode){
        for(int i = 0; i < 1000000; i++)
            printf("%d", log->data[i]);
    }
    //how to access the file that I want 
    return 0;
}

static int wfs_write(const char *path, const char *buf, size_t size,
			 off_t offset, struct fuse_file_info *fi){

	return 0;
}

/**
 * RETURNS
 *  returns 0 on success
*/
static int wfs_readdir(const char* path, void* buf, 
    fuse_fill_dir_t filler, off_t offset, struct fuse_file_info* fi) {
    
    // printf("doing something in readdir\n");

    filler(buf, ".", NULL, 0);  // Current Directory
    filler(buf, "..", NULL, 0); // Parent Directory

    // printf("path %s\n", path);
    if (strcmp(path, "/") == 0) {
        // filler(buf, "some placeholder", NULL, 0);
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
    if (argc < 4) {
        return -1;
    }

    int fd; 
    void* disk;
    struct stat file_stat;
    if((fd = open(argv[argc-2], O_RDWR)) < 0)
        return -1;
    
    if(stat(argv[argc-2], &file_stat) < 0)
        return -1;

    if((disk = mmap((void*)argv[argc-2], file_stat.st_size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0)) == MAP_FAILED)
        return -1;

    start->disk = disk;
    start->head = sizeof(struct wfs_sb);

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