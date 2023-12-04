//empty filesystem = superblock, root dir
//task: initialize the super block and create a root dir and initialize it.
#include "wfs.h"

void superblock_init(){
    struct wfs_sb *superblock = malloc(sizeof(struct wfs_sb));
    superblock->magic = WFS_MAGIC;
}