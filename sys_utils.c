#include "sys_utils.h"
#include <assert.h>

void *safe_malloc(size_t size){
    assert(size > 0);
    void *mem = malloc(size);
    if(mem == NULL){
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    return mem;
}