#include "header.h"
#include <unistd.h>
#include <sys/types.h>
#include "sys_utils.h"
#include <string.h>

static void write_magic_number(int fd);
static void write_file_size(int fd, int64_t new_size);
static void write_metadata_offset(int fd, off_t new_offset);

off_t header_get_magic_number_offset(void)
{
    return HEADER_MAGIC_NUMBER_OFFSET;
}

off_t header_get_metadata_offset_offset(void)
{
    return HEADER_METADATA_OFFSET_OFFSET;
}

off_t header_get_file_size_offset(void)
{
    return HEADER_FILE_SIZE_OFFSET;
}

off_t header_get_data_offset(void)
{
    return HEADER_DATA_OFFSET;
}


Header header_get(int fd)
{
    char buffer[64];

    Header header = safe_malloc(sizeof(*header));

    safe_sys(lseek(fd, 0, SEEK_SET));

    guaranteed_read(fd, buffer, sizeof(MAGIC_NUMBER));
    if(strcmp(buffer, MAGIC_NUMBER) != 0)
    {
        perror("Magic number is not correct\n");
        exit(EXIT_FAILURE);
    }
    
    guaranteed_read(fd, buffer, sizeof(off_t));
    memcpy(&(header->metadata_offset), buffer, sizeof(off_t));

    guaranteed_read(fd, buffer, sizeof(int64_t));
    memcpy(&(header->file_size), buffer, sizeof(int64_t));

    return header;
}

void header_write(Header header, int fd)
{
    write_magic_number(fd);
    write_file_size(fd, header->file_size);
    write_metadata_offset(fd, header->metadata_offset);
}

static void write_magic_number(int fd)
{
    safe_sys(lseek(fd, header_get_magic_number_offset(), SEEK_SET));
    char buffer[MAGIC_NUMBER_SIZE];
    memcpy(buffer, MAGIC_NUMBER, MAGIC_NUMBER_SIZE);
    safe_sys(write(fd, buffer, sizeof(buffer)));
}

static void write_file_size(int fd, int64_t new_size)
{
    safe_sys(lseek(fd, header_get_file_size_offset(), SEEK_SET));
    char buffer[sizeof(new_size)];
    memcpy(buffer, &new_size, sizeof(new_size));
    safe_sys(write(fd, buffer, sizeof(buffer)));
}

static void write_metadata_offset(int fd, off_t new_offset)
{
    safe_sys(lseek(fd, header_get_metadata_offset_offset(), SEEK_SET));
    char buffer[sizeof(new_offset)];
    memcpy(buffer, &new_offset, sizeof(new_offset));
    safe_sys(write(fd, buffer, sizeof(buffer)));
}