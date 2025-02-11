#include "header.h"
#include <unistd.h>
#include <sys/types.h>
#include "sys_utils.h"
#include <string.h>

static void write_magic_number(int fd);
static void write_file_size(int fd, int64_t new_size);
static void write_metadata_offset(int fd, off_t new_offset);

off_t get_magic_number_offset(void)
{
    return 0;
}

off_t get_metadata_offset_offset(void)
{
    return get_magic_number_offset() + MAGIC_NUMBER_SIZE;
}

off_t get_file_size_offset(void)
{
    return get_metadata_offset_offset() + sizeof(off_t);  // sizeof metadata_offset
}

off_t get_data_offset(void)
{
    return get_file_size_offset() + sizeof(int64_t);
}


Header get_header(int fd)
{
    char buffer[64];

    Header header = safe_malloc(sizeof(*header));

    safe_sys(lseek(fd, 0, SEEK_SET));

    safe_sys(read(fd, buffer, sizeof(MAGIC_NUMBER)));
    if(strcmp(buffer, MAGIC_NUMBER) != 0)
    {
        perror("Magic number is not correct\n");
        exit(EXIT_FAILURE);
    }
    
    safe_sys(read(fd, buffer, sizeof(off_t)));
    memcpy(&(header->metadata_offset), buffer, sizeof(off_t));

    safe_sys(read(fd, buffer, sizeof(int64_t)));
    memcpy(&(header->file_size), buffer, sizeof(int64_t));

    return header;
}

void write_header(Header header, int fd)
{
    write_magic_number(fd);
    write_file_size(fd, header->file_size);
    write_metadata_offset(fd, header->metadata_offset);
}

static void write_magic_number(int fd)
{
    safe_sys(lseek(fd, get_magic_number_offset(), SEEK_SET));
    char buffer[MAGIC_NUMBER_SIZE];
    memcpy(buffer, MAGIC_NUMBER, MAGIC_NUMBER_SIZE);
    safe_sys(write(fd, buffer, sizeof(buffer)));
}

static void write_file_size(int fd, int64_t new_size)
{
    safe_sys(lseek(fd, get_file_size_offset(), SEEK_SET));
    char buffer[sizeof(new_size)];
    memcpy(buffer, &new_size, sizeof(new_size));
    safe_sys(write(fd, buffer, sizeof(buffer)));
}

static void write_metadata_offset(int fd, off_t new_offset)
{
    safe_sys(lseek(fd, get_metadata_offset_offset(), SEEK_SET));
    char buffer[sizeof(new_offset)];
    memcpy(buffer, &new_offset, sizeof(new_offset));
    safe_sys(write(fd, buffer, sizeof(buffer)));
}