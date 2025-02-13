#include "myz.h"
#include "header.h"
#include "metadata.h"
#include "sys_utils.h"
#include <fcntl.h>


void write_to_file_create(Myz myz, const char* file_name) {
    int fd;
    safe_sys_assign(fd, open(file_name, O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR));

    header_write(myz->header, fd);

    Vector nodes = myz->metadata->nodes;

    // write data
    int size = vector_size(nodes);
    for(int i = 0 ; i < size ; i++){
        MyzNode node = vector_get_at(nodes, i);
        if(node->file_data != NULL)
        {
                        if(!S_ISREG(node->info.st_mode)){
                            fprintf(stderr, "WTFFFF???!!?!?!??\n\n");
                            exit(EXIT_FAILURE);
                        }
            safe_sys(write(fd, node->file_data, node->file_size));
            node->data_offset = lseek(fd, 0, SEEK_CUR);
        }
    }
    
    off_t metadata_offset = lseek(fd, 0, SEEK_CUR);

    // write metadata
    for(int i = 0 ; i < size ; i++){
        MyzNode node = vector_get_at(nodes, i);
        metadata_write_node(node, fd);
    }

    off_t file_size = lseek(fd, 0, SEEK_CUR);

    // Update header with final offsets and sizes
    myz->header->metadata_offset = metadata_offset;
    myz->header->file_size = file_size;
    header_write(myz->header, fd);

    safe_sys(close(fd));
}

void read_metadata(Myz myz, int fd)
{
	lseek(fd, myz->header->metadata_offset, SEEK_SET);

	Metadata metadata = safe_malloc(sizeof(*metadata));
    metadata->nodes = vector_create(0, NULL);   // TODO: destroy function
	MyzNode node = NULL;
	while((node = metadata_read_node(fd)) != NULL)
	{
		vector_insert_last(metadata->nodes, node);
	}

	myz->metadata = metadata;
}

Myz read_from_file(const char* path)
{
    int fd;
    safe_sys_assign(fd, open(path, O_RDONLY));

    Myz myz = safe_malloc(sizeof(*myz));

    myz->header = header_get(fd);

    read_metadata(myz, fd);
    
    for(int i = 0; i < vector_size(myz->metadata->nodes); i++)
    {
        MyzNode node = vector_get_at(myz->metadata->nodes, i);
        
        if(S_ISREG(node->info.st_mode))
        {
            safe_sys(lseek(fd, node->data_offset, SEEK_SET));
            node->file_data = safe_malloc(node->file_size * sizeof(*node->file_data));
            guaranteed_read(fd, node->file_data, node->file_size * sizeof(*node->file_data));
        }
    }


    return myz;
}


