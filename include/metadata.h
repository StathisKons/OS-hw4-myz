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


typedef struct {
	mode_t mode;
	uid_t uid;
	gid_t gid;
} *Info;


struct myznode {
	char name[MAX_NAME];
	
	// FileType type;
	bool compressed;
	Info info;
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
	char name[256];
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

void write_Data(Metadata metadata);

void read_Data(Metadata metadata, char* path, bool compressed);
    
void myznode_destroy(Pointer myz_node);

void read_data(char* path, Metadata metadata, bool compressed, int dir_index);

char* read_file(char* path, long int* fsize);

void entries_insert(MyzNode node, char* name, int myznode_index);

// if exists = false, returns the "closest" parent node (may return . eg the root directory in metadata aka the first node)
MyzNode metadata_find_node(Metadata metadata, const char* path_to_find, bool* exists);

MyzNode metadata_find_parent(Metadata metadata, const char* path_to_find, bool* exists);

void print(Metadata metadata, bool print_metadata);

void print_s(Metadata metadata);

#endif // METADATA_H 