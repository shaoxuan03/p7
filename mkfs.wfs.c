// //empty filesystem = superblock, root dir
// //task: initialize the super block and create a root dir and initialize it.
// #include "wfs.h"
// #include <errno.h>
// #include <sys/stat.h>
// #include <unistd.h>
// #include <stdlib.h>
// #include <time.h>
// #include <fcntl.h>
// #include <sys/mman.h>


// void superblock_init(struct wfs_sb* superblock){
//     superblock->magic = WFS_MAGIC;
//     superblock->head = sizeof(struct wfs_sb) + sizeof(struct wfs_log_entry); //size of superblock and root entry
//     sb = superblock;
// }

// int init_root(char* path){
//     //first, create a log entry(root)
//     struct stat stats;
//     struct wfs_log_entry* log = malloc(sizeof(struct wfs_log_entry));
//     if (stat(path, &stats) != 0){
//         return -1;
//     }
//     log->inode.inode_number = 0;
//     log->inode.deleted = 0;
//     log->inode.mode = __S_IFDIR;
//     log->inode.atime = time(NULL);
//     log->inode.ctime = time(NULL);
//     log->inode.mtime = time(NULL);
//     log->inode.links = 1;
//     //from this point onward i dont know what value i should initialize the:qm to
//     log->inode.gid = (uid_t)getgid();
//     log->inode.uid = (uid_t)getuid();
//     log->inode.flags = 1;
//     log->inode.size = 0; // size of the directory/file

//     return 0;
// }


// int main(int argc, char *argv[]) {
//     int fd;
//     void* disk;
//     struct stat file_stat;
    
//     if(argc != 2)
//         return -1;

//     if((fd = open(argv[1], O_RDWR)) < 0)
//         return -1;

//     if(stat(argv[1], &file_stat) < 0)
//         return -1;
    
//     if((disk = mmap(0, file_stat.st_size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0)) == MAP_FAILED)
//         return -1;

//     superblock_init((struct wfs_sb*) disk);
//     init_root(argv[1]);
//     memcpy((char*) disk + sizeof(struct wfs_sb *), &)
//     munmap(disk, sizeof(struct wfs_sb)+sizeof(struct wfs_inode));
//     fsync(fd);
//     close(fd);
//     return 0;
// }
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include "wfs.h"

void *disk_map;
#define DISK_SIZE 1000000 

int main(int argc, char *argv[]) {
	if (argc != 2) {
		fprintf(stderr, "Usage: %s <path to disk>\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	const char *disk_path = argv[1];

	int fd = open(disk_path, O_RDWR);
	if (fd<0) {
		perror("Error creating disk file");
		exit(EXIT_FAILURE);
	}

	struct stat st;
	stat(disk_path,&st);
	disk_map=mmap(0,st.st_size,PROT_READ|PROT_WRITE,MAP_SHARED,fd,0);
	if(disk_map<(void*)0){
		printf("map error\n");
		return errno;
	}

	struct wfs_sb *superblock=disk_map;
	superblock->magic=WFS_MAGIC;
	superblock->head=sizeof(struct wfs_sb);
	printf("head %u\n",superblock->head);

	struct wfs_inode i={
		.inode_number=0,
		.deleted=0,       
		.mode=S_IFDIR,   
		.uid=getuid(),  
		.gid=getgid(), 
		.size=0,      
		.links=2,      
		.atime=time(NULL),
		.mtime=time(NULL), 
		.ctime=time(NULL), 
	};

	struct wfs_log_entry root={
		.inode=i,
	};

	memcpy((char*)disk_map+sizeof(*superblock), &root,sizeof(root));

	superblock->head+=sizeof(root);
	printf("head %u\n",superblock->head);
	munmap(disk_map,st.st_size);
	close(fd);

	printf("File system created successfully at %s\n", disk_path);

	return 0;
}
