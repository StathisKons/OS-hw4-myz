#include "sys_utils.h"
#include <assert.h>
#include <stdlib.h>

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