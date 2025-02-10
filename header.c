#include "header.h"
#include <unistd.h>
#include <sys/types.h>
#include "sys_utils.h"
#include <string.h>

off_t get_magic_number_offset(void)
{
    return 0;
}

off_t get_metadata_offset_offset(void)
{
    return get_magic_number_offset() + sizeof(MAGIC_NUMBER);
}

off_t get_file_size_offset(void)
{
    return get_metadata_offset_offset() + sizeof(off_t);  // sizeof metadata_offset
}

off_t get_data_offset(void)
{
    return get_file_size_offset() + sizeof(int64_t);
}


void set_file_size(Header header, int fd, int64_t new_size)
{
    header->file_size = new_size;
    safe_sys(lseek(fd, get_file_size_offset(), SEEK_SET));
    char buffer[sizeof(new_size)];
    memcpy(buffer, &new_size, sizeof(new_size));
    safe_sys(write(fd, buffer, sizeof(buffer)));
}

void set_metadata_offset(Header header, int fd, off_t new_offset)
{
    header->metadata_offset = new_offset;
    safe_sys(lseek(fd, get_metadata_offset_offset(), SEEK_SET));
    char buffer[sizeof(new_offset)];
    memcpy(buffer, &new_offset, sizeof(new_offset));
    safe_sys(write(fd, buffer, sizeof(buffer)));
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