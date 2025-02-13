#ifndef METADATA_H
#define METADATA_H


#include <sys/stat.h>
#include <sys/types.h>
#include <stdbool.h>
#include "vector.h"


typedef enum {
	REGULAR_FILE,
	DIRECTORY,
	SYMBOLIC_LINK
} FileType;

#define MAX_NAME 256

struct myznode {
	char name[MAX_NAME];
	struct stat info; 
	// FileType type;
	bool compressed;

	// file specific
	off_t data_offset;
	long int file_size;
	char* file_data;

	// directory specific
	Vector entries;	// Vector<Entries>
};

typedef struct myznode myznode;
typedef struct myznode* MyzNode;


typedef struct {
	char* name;
	int myznode_index;
} *Entry;


typedef struct {
	Vector nodes;	// vector of myznodes
} *Metadata;




Metadata metadata_create(void);

void metadata_destroy(Metadata metadata);

void metadata_insert_node(Metadata metadata, MyzNode node);

void metadata_insert(Metadata metadata, char* name, struct stat info, bool compressed, long int file_size, char* file_data);
// MyzNode myznode_create();

// void myznode_destroy();


void print_data(Metadata metadata);


void write_Data(Metadata metadata);

void read_Data(Metadata metadata, char* path, bool compressed);
    
void metadata_write_node(MyzNode node, int fd);

MyzNode metadata_read_node(int fd);

#endif // METADATA_H 