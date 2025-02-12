#ifndef HEADER_H
#define HEADER_H

#include <stdint.h>
#include <sys/types.h>


#define MAGIC_NUMBER "MYZ"
#define MAGIC_NUMBER_SIZE (sizeof(MAGIC_NUMBER))

#define HEADER_MAGIC_NUMBER_OFFSET      0
#define HEADER_METADATA_OFFSET_OFFSET   (HEADER_MAGIC_NUMBER_OFFSET + MAGIC_NUMBER_SIZE)
#define HEADER_FILE_SIZE_OFFSET         (HEADER_METADATA_OFFSET_OFFSET + sizeof(off_t))     /*sizeof(metadata_offset)*/
#define HEADER_DATA_OFFSET              (HEADER_FILE_SIZE_OFFSET + sizeof(int64_t))         /*sizeof(file_size)*/


struct header {
    char magic_number[MAGIC_NUMBER_SIZE];
    off_t metadata_offset;
    int64_t file_size;
};

typedef struct header* Header;


off_t header_get_magic_number_offset(void);

off_t header_get_metadata_offset_offset(void);

off_t header_get_file_size_offset(void);

off_t header_get_data_offset(void);

Header header_get(int fd);

void header_write(Header header, int fd);

#endif // HEADER_H