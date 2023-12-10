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
#include <assert.h>

//global variable
void* disk_map;
uint32_t head;
int length;
int inode_number;


char** tokenize(char str[]) {
    int i = 0;
    char *p = strtok(str, "/");
    char **array = malloc(100 * sizeof(char*));  // Allocate memory for an array of pointers

    while (p != NULL && i < 100) {
        array[i++] = strdup(p);  // Duplicate the token and store its pointer
        p = strtok(NULL, "/");
    }

    array[i] = NULL;  // Null-terminate the array

    return array;
} 
char* removeLastToken(char str[]) {
    char** tokens = tokenize(str);

    if (tokens[0] == NULL) {
        char* result = malloc(1);
        result[0] = '\0';
        return result;
    }

    int lastTokenIndex = 0;
    while (tokens[lastTokenIndex + 1] != NULL) {
        lastTokenIndex++;
    }

    int lengthWithoutLastToken = 0;
    for (int i = 0; i < lastTokenIndex; i++) {
        lengthWithoutLastToken += strlen(tokens[i]) + 1; // Add 1 for the '/'
    }

    char* result = malloc(lengthWithoutLastToken + 1); // Add 1 for the null terminator

    result[0] = '\0';
    for (int i = 0; i < lastTokenIndex; i++) {
        strcat(result, tokens[i]);
        strcat(result, "/");
    }

    if (lengthWithoutLastToken > 0) {
        result[lengthWithoutLastToken - 1] = '\0';
    }

    for (int i = 0; tokens[i] != NULL; i++) {
        free(tokens[i]);
    }
    free(tokens);

    return result;
}
//use inode num to find LATEST log entry
// static struct wfs_log_entry* inode_num_to_log(unsigned long target_inode) {
//     printf("%ld, targetInode\n", target_inode);
//     // start entry is the first log entry
//     struct wfs_log_entry* curr_entry =  (struct wfs_log_entry*)((char *)start->disk + sizeof(struct wfs_sb));

//     // looking for the target entry and its inode
//     struct wfs_log_entry* target_entry = 0;

//     // Stopping condition might be right
//     while((char *)curr_entry <= ((char *)start->disk + start->head)){
//         // Update when desired target inode is found
//         if(curr_entry->inode.inode_number == target_inode){
//             target_entry = curr_entry;
//         }

//         // Iterating through log entry sizes
//         curr_entry += sizeof(struct wfs_log_entry) + curr_entry->inode.size;
//     }
//     return target_entry;
// }

//use the name to find inode num, then call the helper method to find log entry
// in other words
// find the inode from path string

/**
 * Takes path string and convert it to log entry
*/
// static struct wfs_log_entry* inode_finder(const char *path){
//     // Assumes path is correct
//     printf("%s, path\n", path);
//     // first thing is to start with latest root log
//     int root_inode_num = 0;
//     struct wfs_log_entry* root_log = inode_num_to_log(root_inode_num);

//     struct wfs_log_entry* curr_log = root_log;

//     // Looping statement for the path
//     char* copy = strdup(path);
//     char* token = strtok(copy, "/");
    
//     while(token != NULL) {
//         printf("inside while\n");
//         // Read the dentries in the curr_log
//         int dentry_num = curr_log->inode.size / sizeof(struct wfs_dentry);

//         int found = 0;
//         for(int i = 0; i < dentry_num; i++){
//             // Check if the token exist in the dentries
//             struct wfs_dentry *dentry = (struct wfs_dentry *)&curr_log->data[i];
//             printf("%s, this is inode finder\n", dentry->name);
//             if(strcmp(token, dentry->name) == 0){
//                 // Update the curr_log
//                 curr_log = inode_num_to_log(curr_log->inode.inode_number);
//                 found = 1;
//                 break;
//             }
//         }
//         if (found == 0) {
//             // Throw error
//             return NULL;
//         }

//         // Looping condition on path string
//         token = strtok(NULL, "/");
//     }
//     return curr_log;
// }
struct wfs_log_entry *getLatestLogEntryFromNum(int num) {
    char *curr = ((char*)disk_map + sizeof(struct wfs_sb));
    struct wfs_log_entry *latest = NULL;
    while(curr < (char*)disk_map+head) {
        struct wfs_log_entry * temp_curr = (struct wfs_log_entry*)curr;
        if(num == (temp_curr->inode.inode_number)) {
            latest = (struct wfs_log_entry *)curr;
        }
        curr += sizeof(struct wfs_inode) + temp_curr->inode.size;
        temp_curr = (struct wfs_log_entry *)curr;
    }
    return latest;
}


struct wfs_inode *inode_finder(const char *path) {

    int flag=0;
    printf("get_inode path %s\n",path);

    char input[100];
    for(int i = 0; i < 100; i++) {
        input[i] = path[i];
    }
    char**tokens=tokenize(input);

    struct wfs_log_entry *curr = getLatestLogEntryFromNum(0);
    printf("curr %p\n",(void*)curr);
    printf("size: %ld\n", curr->inode.size/sizeof(struct wfs_dentry));

    int i = 0;
    if(tokens[i] == NULL)    return (struct wfs_inode*)curr;
    while(tokens[i] != NULL) {
        printf("Trying to find: %s\n", tokens[i]);
        flag=0;
        struct wfs_dentry*e=((void*)curr->data);
        for(int j = 0; j < curr->inode.size/sizeof(struct wfs_dentry); j++) {
            printf("DIR entry: %s\n", e->name);
            if(strcmp(tokens[i],e->name)==0) {
                curr = getLatestLogEntryFromNum(e->inode_number);
                flag=1;
            }
            e++;
        }
        i++;
    }

    if(flag==0){
        return NULL;
    }else{
        return (struct wfs_inode*)curr; 
    }
}

static int wfs_getattr(const char *path, struct stat *stbuf) {
    // Implementation of getattr function to retrieve file attributes
    // Fill stbuf structure with the attributes of the file/directory indicated by path
    // ...
    printf("hi\n");
    struct wfs_log_entry *log = (struct wfs_log_entry*)inode_finder(path);
    struct wfs_inode *i = &log->inode;
    if (!i)
        return -ENOENT;

    memset(stbuf, 0, sizeof(*stbuf));
    stbuf->st_uid = i->uid;
    stbuf->st_gid = i->gid;
    stbuf->st_atime = i->atime;
    stbuf->st_mtime = i->mtime;
    stbuf->st_mode = i->mode;
    stbuf->st_nlink = i->links;
    stbuf->st_size = i->size;

return 0;
}

static int wfs_mknod(const char *path, mode_t mode,dev_t rdev)
{
    char x[100];
    strcpy(x,path);

    char y[100];
    strcpy(y,removeLastToken(x));
    if(strlen(y)==0){
        strcpy(y,"/");
    }

    struct wfs_inode *i=inode_finder(y);
    struct wfs_log_entry *e=(void*)i;

    size_t size=sizeof(struct wfs_log_entry)+sizeof(struct wfs_dentry)+i->size;
    struct wfs_log_entry *new_entry=malloc(size);
    memcpy(new_entry,i,sizeof(*i));
    memcpy(new_entry->data,e->data,i->size);
    new_entry->inode.size+=sizeof(struct wfs_dentry);

    struct wfs_dentry *d=(void*)(new_entry->data+i->size);

    char input[25];
    for(int i = 0; i < 100; i++) {
        input[i] = path[i];
    }
    char**tokens=tokenize(input);
    int k=0;
    char z[25];
    while(tokens[k]!=NULL){
        strcpy(z,tokens[k]);
        k++;
    }
    strcpy(d->name,z);

    inode_number++;
    d->inode_number=inode_number;

    memcpy((char*)disk_map+head,new_entry,size);

    head+=size;
    free(new_entry);

    struct wfs_inode inode={
        .inode_number=inode_number,
        .deleted=0,
        .mode=S_IFREG|mode,
        .uid=getuid(),
        .gid=getgid(),
        .size=0,
        .atime=time(NULL),
        .mtime=time(NULL),
        .ctime=time(NULL),
        .links=1,
    };

    memcpy((char*)disk_map+head, &inode,sizeof(inode));

    head+=sizeof(inode);

    return 0;
}

static int wfs_mkdir(const char *path, mode_t mode){
    
    printf("mkdir called\n");
    char x[100];
    strcpy(x,path);

    char y[100];
    strcpy(y,removeLastToken(x));
    if(strlen(y)==0){
        strcpy(y,"/");
    }

    struct wfs_inode *i=inode_finder(y);
    struct wfs_log_entry *e=(void*)i;

    //update the parent dir size
    size_t size=sizeof(struct wfs_log_entry)+sizeof(struct wfs_dentry)+i->size;
    struct wfs_log_entry *new_entry=malloc(size);

    //copy a new parent dir
    memcpy(new_entry,i,sizeof(*i));
    memcpy(new_entry->data,e->data,i->size);
    new_entry->inode.size+=sizeof(struct wfs_dentry);

    // //put the new entry at disk_map + head
    // memcpy((char*)disk_map+head,new_entry,size); 
    // //move the head
    // head+=size;

    //create the new dir
    struct wfs_dentry *d=(void*)(new_entry->data+i->size);

    char input[25];
    for(int i = 0; i < 100; i++) {
        input[i] = path[i];
    }
    char**tokens=tokenize(input);
    int k=0;
    char z[25];
    while(tokens[k]!=NULL){
        strcpy(z,tokens[k]);
        k++;
    }
    strcpy(d->name,z);

    inode_number++;
    d->inode_number=inode_number;

    memcpy((char*)disk_map+head,new_entry,size);

    head+=size;
    free(new_entry);

    struct wfs_inode inode={
        .inode_number=inode_number,
        .deleted=0,
        .mode=S_IFDIR|mode,
        .uid=getuid(),
        .gid=getgid(),
        .size=0,
        .atime=time(NULL),
        .mtime=time(NULL),
        .ctime=time(NULL),
        .links=1,
    };

    memcpy((char*)disk_map+head, &inode,sizeof(inode));

    head+=sizeof(inode);

    return 0;
}

static int wfs_read(const char *path, char *buffer, size_t size, 
             off_t offset, struct fuse_file_info *fi ){

    struct wfs_inode *i = inode_finder(path);
    if (i == NULL) {
        // Handle case where the log entry is not found
        return -ENOENT;
    }
    struct wfs_log_entry *e = (void *)i;

    size_t new_size;
    if(i->size > size){
        new_size = i->size;
    }else{
        new_size = size;
    }

    memcpy(buffer, e->data + offset, new_size);

    return new_size;
}

static int wfs_write(const char *path, const char *buffer, size_t size,
			 off_t offset, struct fuse_file_info *fi){

    struct wfs_inode *i = inode_finder(path);
    if (i == NULL) {
        // Handle case where the log entry is not found
        return -ENOENT;
    }
    struct wfs_log_entry *e = (void *)i;

    size_t new_size;
    if(i->size > size){
        new_size = i->size;
    }else{
        new_size = size;
    }

    memcpy(e->data + offset, buffer, new_size);

    // update inode
    i->size = new_size;
    i->atime = time(NULL);
    i->mtime = time(NULL);
    i->ctime = time(NULL);

    head += new_size;

    return new_size;
}

/**
 * RETURNS
 *  returns 0 on success
*/
static int wfs_readdir(const char* path, void* buf, 
    fuse_fill_dir_t filler, off_t offset, struct fuse_file_info* fi) {
    
    printf("doing something in readdir\n");

    filler(buf, ".", NULL, 0);  // Current Directory
    filler(buf, "..", NULL, 0); // Parent Directory

    struct wfs_log_entry * log = (struct wfs_log_entry*)inode_finder(path);
    struct wfs_dentry* dentry = (void*) log->data;

    size_t entries = log->inode.size/sizeof(struct wfs_dentry);
    assert(log->inode.size % sizeof(struct wfs_dentry) == 0);

    for(size_t i = 0; i < entries; i++){
        filler(buf, (dentry + i) -> name, NULL, 0);
    }
    printf("finish something in readdir\n");
    return 0;
}

static int wfs_unlink(const char *path) { 
    // printf("im here1\n");
    // unsigned int dlt_inode_num;
    // unsigned int dlt_size;
    // unsigned int new_size;
    // struct wfs_inode *i = inode_finder(path);
    // if (i == NULL) {
    //     // Handle case where the log entry is not found
    //     return -ENOENT;
    // }
    // struct wfs_log_entry *e = (void *)i;
    // e->inode.deleted = 1;
    // dlt_inode_num = e->inode.inode_number;
    // dlt_size = e->inode.size;
    // printf("im here2\n");
    
    // //create a new log entry without the deleted inode
    // char* copy = strdup(path);
    // char** arr = tokenize(copy);
    // //char* removed;
    // char* parent_dir;
    // for(int i = 0; i < 32; i++){
    //     if(arr[i] != 0){
    //         //removed = arr[i];
    //         parent_dir = arr[i-1];
    //     }
    //     break;
    // }
    // printf("im here3\n");
    // struct wfs_inode *parent_i = inode_finder(parent_dir);
    // struct wfs_log_entry *parent_e = (struct wfs_log_entry*)parent_i;
    // //update the new size of new log entry
    // new_size = parent_e->inode.size - dlt_size + sizeof(struct wfs_log_entry) + sizeof(struct wfs_dentry);
    // struct wfs_log_entry *new_log_entry = malloc(new_size);

    // //copy inode to the new log entry
    // printf("im here4\n");
    // memcpy(new_log_entry, parent_i, sizeof(*parent_i));
    // new_log_entry->inode.size -= dlt_size; //adjust the size of the data[]

    // //copy data to the new log entry
    // memcpy(new_log_entry->data, parent_e->data, parent_i->size);
    
    // //remove delete dentry from the data
    // struct wfs_dentry* dentry = (struct wfs_dentry*)new_log_entry->data;
    // while(dentry != NULL){
    //     printf("im here5\n");
    //     if(dentry->inode_number == dlt_inode_num){
    //         dentry = NULL;
    //     }
    //     dentry++;
    // }
    // memcpy((char*)disk_map + head, new_log_entry, new_size);
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

int main(int argc, char *argv[] ){
	char *disk_object = argv[argc-2];
	argv[argc-2] = argv[argc-1];
	argv[argc-1] = NULL;
	argc--;

	int fd = open(disk_object,O_RDWR);

	struct stat st_obj;
	stat(disk_object,&st_obj);

	disk_map = mmap(0,st_obj.st_size,PROT_READ|PROT_WRITE,MAP_SHARED,fd,0);
	struct wfs_sb*sb = (void*)disk_map;
	if(sb->magic != WFS_MAGIC){
		printf("not superblock\n");
	}

	length = st_obj.st_size;
	head = sb->head;

	fuse_main( argc, argv, &wfs_operations, NULL );
	printf("after fuse\n");

	sb->head = head;

	munmap(disk_map,st_obj.st_size);
	close(fd);

	return 0;
}

// int main(int argc, char *argv[]) {
//     // Initialize FUSE with specified operations
//     // Filter argc and argv here and then pass it to fuse_main

//     int fd; 
//     struct stat file_stat;

//     if((fd = open(argv[argc-2], O_RDWR)) < 0)
//         return -1;
    
//     if(stat(argv[argc-2], &file_stat) < 0)
//         return -1;

//     if((disk_map = mmap(0, file_stat.st_size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0)) == MAP_FAILED)
//         return -1;

//     struct wfs_sb* superblock = (struct wfs_sb*) disk_map;
//     head = superblock->head;

//     char *new_argv[argc - 1];
//     for (int i = 0; i < argc; i++) {
//         if (i == 3) {
//             new_argv[i] = argv[i+1];
//             break;
//         }
//         new_argv[i] = argv[i];
//     }
//     printf("running here final\n");
    
//     fuse_main(argc - 1, new_argv, &wfs_operations, NULL);
//     sb->head = head;
//     munmap(disk_map, sizeof(struct wfs_sb)+sizeof(struct wfs_inode));
//     close(fd);
//     return 0;
// }
//     munmap(disk, sizeof(struct wfs_sb)+sizeof(struct wfs_inode));
//     return fuse_main(argc - 1, new_argv, &wfs_operations, NULL);
// }

