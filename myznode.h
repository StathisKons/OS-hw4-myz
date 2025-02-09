#ifndef MYZNODE
#define MYZNODE
#include <sys/stat.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

struct myznode{
	char fname[1000];
	struct stat info; 
	int nested;

	struct myznode* next;
	struct myznode* contents; // If the node corresponds to a directory this is a list of its contents
};

typedef struct myznode myznode;
typedef struct myznode* Myznode;

struct myzdata
{
	int capacity;
	int curelements;
	
	Myznode* array;
};

typedef struct myzdata myzdata;
typedef myzdata* Myzdata;


static Myznode myznode_init(char* fname, struct stat info, int nested);

void myznode_insert(Myzdata data, char* fname, struct stat info, int nested);

Myzdata myz_init(int capacity);

// Retrieve file permissions in Unix-like format from a Myznode using the stat structure
int getPermissions(Myznode node);

void getChangeTime(Myznode node, char* timestamp);

void getAccessTime(Myznode node, char* timestamp);

void getModTime(Myznode node, char* timestamp);

void myz_print(Myzdata data);

#endif
