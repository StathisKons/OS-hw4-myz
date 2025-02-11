#ifndef HEADER_H
#define HEADER_H

#define MAGIC_NUMBER "MYZ"
#define MAGIC_NUMBER_SIZE (sizeof(MAGIC_NUMBER))

#include <stdint.h>
#include <sys/types.h>

struct header {
    char magic_number[MAGIC_NUMBER_SIZE];
    off_t metadata_offset;
    int64_t file_size;
};

typedef struct header* Header;


off_t get_magic_number_offset(void);

off_t get_metadata_offset_offset(void);

off_t get_file_size_offset(void);

off_t get_data_offset(void);

Header get_header(int fd);

void write_header(Header header, int fd);

#endif // HEADER_H