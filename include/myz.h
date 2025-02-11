#ifndef MYZ_H
#define MYZ_H

#include "myznode.h"
#include "header.h"
#include "sys_utils.h"

typedef struct {
    int fd;
    Header header;
    // files
    Myzdata data;
} Myz;

Myz myz_read(const char *pathname);

void myz_store(Myz myz);


#endif // MYZ_H