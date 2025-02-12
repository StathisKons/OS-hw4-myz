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


struct myznode {
	char name[256];
	struct stat info; 
	FileType type;
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

void metadata_insert(Metadata metadata, char* name, struct stat info, bool compressed, long int file_size, char* file_data);
// MyzNode myznode_create();

// void myznode_destroy();


void print_data(Metadata metadata);


void read_Data(Metadata metadata, char* path, bool compressed);
#endif // METADATA_H 