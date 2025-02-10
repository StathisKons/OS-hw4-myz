#ifndef HEADER_H
#define HEADER_H

#define MAGIC_NUMBER "MYZ"

#include <stdint.h>

typedef struct {
    char magic_number[sizeof(MAGIC_NUMBER)];
    int64_t metadata_offset;
    int64_t file_size;
} Header;



#endif // HEADER_H