#include "sys_utils.h"
#include <assert.h>
#include <stdlib.h>
#include <errno.h>

void* safe_malloc(size_t size){
    // assert(size > 0);
    void *mem = malloc(size);
    if(mem == NULL){
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    return mem;
}

void* safe_realloc(void *ptr, size_t size){
    assert(size > 0);
    void *mem = realloc(ptr, size);
    if(mem == NULL){
        perror("realloc");
        exit(EXIT_FAILURE);
    }
    return mem;
}


ssize_t guaranteed_read(int fd, void *buffer, size_t bytes_to_read_){
    ssize_t bytes_read = 0, r;
    ssize_t bytes_to_read = bytes_to_read_; // suppress -Wsign-compare warning
    char *buf = buffer;     // char* to allow pointer arithmetic
    while(bytes_read != bytes_to_read){
        r = read(fd, buf + bytes_read, bytes_to_read - bytes_read);
        if(bytes_read == -1){
            if(errno == EINTR /* || errno == EAGAIN*/){
                continue;
            }
            perror("Read failed");
            exit(EXIT_FAILURE); // ? αντι για exit μπορω return -1 και να αφησω τον parent να κανει handle
        }
        if(r == 0){
            return bytes_read;
        }
        bytes_read += r;
    }
    if(bytes_read != bytes_to_read){
        char buffer[256];
        sprintf(buffer, "Read failed, read %ld bytes instead of %ld", bytes_read, bytes_to_read);
        perror(buffer);
        exit(EXIT_FAILURE);
    }
    return bytes_read;
}