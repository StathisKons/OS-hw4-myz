#ifndef MYZ_H
#define MYZ_H

#include "header.h"
#include "metadata.h"

typedef struct {
    Header header;
    // ? data
    Metadata metadata;
} *Myz;


Myz read_from_file(const char* path);

void write_to_file(Myz myz);

void create_myz_file(Myz myz, const char* file_name);

void read_metadata(Myz myz, int fd);


Myz read_myz_file(char* name);

#endif // MYZ_H