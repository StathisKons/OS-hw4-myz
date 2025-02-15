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

MyzNode findPath(char* path, Metadata metadata, bool* file_exists, bool* exists);

Myz myz_create(const char* file_name, const char* files[], int file_count, bool compress);

void myz_extract(const char* myz_name, const char* files[], int file_count);

bool append(Metadata metadata, char* path, bool compressed);

#endif // MYZ_H