#ifndef ARGS_H
#define ARGS_H

#include <stdbool.h>

typedef enum {
    NONE,
    CREATE,
    APPEND,
    EXTRACT,
    DELETE,
    PRINT_METADATA,
    QUERY,
    PRINT_HIERARCHY
} Operation;

typedef struct {
    Operation operation;
    bool use_compression;
    char* archive_file;
    char** files;
    int num_files;
} Arguments;


Arguments get_arguments(int argc, char** argv);

#endif // ARGS_H