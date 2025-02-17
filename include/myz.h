#ifndef MYZ_H
#define MYZ_H

#include "header.h"
#include "metadata.h"

typedef struct {
    Header header;
    Metadata metadata;
} *Myz;


Myz read_from_file(const char* path);

void write_to_file(Myz myz);

void create_myz_file(Myz myz, const char* file_name);

void read_metadata(Myz myz, int fd);

Myz read_myz_file(const char* name);

MyzNode findPath(char* path, Metadata metadata, bool* file_exists, bool* exists);

void myz_create(const char* file_name, int file_count, const char* files[], bool compress);
// void myz_extract(const char* myz_name, const char* files[], int file_count);

void myz_append(const char* file_name, int file_count, const char* files[], bool compress);

void write_after_append(Myz myz, int old_entries, char* filename);

bool compare_names(MyzNode node, char* name);

void myz_query_for_existence(const Myz myz, int file_count, char* files[]);

void myz_extract(Myz myz);

bool append(Myz myz, const char* path, bool compressed);

void myz_delete(Myz myz, char* file_name, int file_number, char* files[]);

#endif // MYZ_H