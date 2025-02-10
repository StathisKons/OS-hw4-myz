#ifndef HEADER_H
#define HEADER_H

#define MAGIC_NUMBER "MYZ"

#include <stdint.h>
#include <sys/types.h>

struct header {
    char magic_number[sizeof(MAGIC_NUMBER)];
    off_t metadata_offset;
    int64_t file_size;
};

typedef struct header* Header;


off_t get_magic_number_offset(void);

off_t get_metadata_offset_offset(void);

off_t get_file_size_offset(void);

off_t get_data_offset(void);

void set_file_size(Header header, int fd, int64_t new_size);

Header get_header(int fd);

#endif // HEADER_H