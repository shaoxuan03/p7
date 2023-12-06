#define FUSE_USE_VERSION 30
#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stddef.h>

//parse the path into filename
char* get_filename(char* path){
    char* filename;
    filename = path;

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

    return 0; // Return 0 on success
}


static struct fuse_operations wfs_operations = {
    // Add other functions (read, write, mkdir, etc.) here as needed
    .getattr    = wfs_getattr

};

int main(int argc, char *argv[]) {
    // Initialize FUSE with specified operations
    // Filter argc and argv here and then pass it to fuse_main
    return fuse_main(argc, argv, &wfs_operations, NULL);
}