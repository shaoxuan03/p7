//empty filesystem = superblock, root dir
//task: initialize the super block and create a root dir and initialize it.
#include "wfs.h"
#include <errno.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>

//global variable
struct wfs_sb *superblock;

void superblock_init(){
    superblock = malloc(sizeof(struct wfs_sb));
    superblock->magic = WFS_MAGIC;
    superblock->head = 0;
}

int init_root(char* path){
    //first, create a log entry(root)
    struct stat stats;
    struct wfs_log_entry* log = malloc(sizeof(struct wfs_log_entry));
    if (stat(path, &stats) != 0){
        return -1;
    }
    log->inode.inode_number = 0;
    log->inode.deleted = 0;
    log->inode.mode = __S_IFDIR;
    log->inode.atime = 0;
    log->inode.ctime = 0;
    log->inode.mtime = 0;
    log->inode.links = 1;
    //from this point onward i dont know what value i should initialize them to
    log->inode.gid = (unsigned int) getgid();
    log->inode.uid = (unsigned int) getuid();
    log->inode.flags = 1;
    log->inode.size = 0; // size of the directory/file

    log->data[0] = 0;
    return 0;
}