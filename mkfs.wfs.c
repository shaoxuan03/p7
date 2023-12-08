//empty filesystem = superblock, root dir
//task: initialize the super block and create a root dir and initialize it.
#include "wfs.h"
#include <errno.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <fcntl.h>
#include <sys/mman.h>


void superblock_init(struct wfs_sb* superblock){
    superblock->magic = WFS_MAGIC;
    superblock->head = sizeof(struct wfs_sb); //size of superblock and root entry
    sb = superblock;
}

int init_root(char* path){
    //first, create a log entry(root)
    struct stat stats;
    struct wfs_log_entry* log = malloc(sizeof(struct wfs_log_entry));
    if (stat(path, &stats) != 0){
        return -1;
    }
    log->inode.inode_number = global_inode;
    log->inode.deleted = 0;
    log->inode.mode = __S_IFDIR;
    log->inode.atime = time(NULL);
    log->inode.ctime = time(NULL);
    log->inode.mtime = time(NULL);
    log->inode.links = 1;
    //from this point onward i dont know what value i should initialize them to
    log->inode.gid = (uid_t)getgid();
    log->inode.uid = (uid_t)getuid();
    log->inode.flags = 1;
    log->inode.size = 0; // size of the directory/file

    log->data[0] = 0;
    return 0;
}


int main(int argc, char *argv[]) {
    int fd;
    void* disk;
    struct stat file_stat;
    
    if(argc != 2)
        return -1;

    if((fd = open(argv[1], O_RDWR)) < 0)
        return -1;

    if(stat(argv[1], &file_stat) < 0)
        return -1;
    
    if((disk = mmap((void*)argv[1], file_stat.st_size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0)) == MAP_FAILED)
        return -1;

    superblock_init((struct wfs_sb*) disk);
    return init_root(argv[1]);
}