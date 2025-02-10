#ifndef MYZNODE
#define MYZNODE
#include <sys/stat.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdio.h>
#include "vector.h"

struct myznode{
	char fname[1000];
	struct stat info; 
	int nested;


	long int fsize;
	char* filedata;
	Vector entries;
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


void myznode_insert(Myzdata data, char* fname, struct stat info, int nested);

Myzdata myz_init(int capacity);

void Myz_destroy(Myzdata data);

// Retrieve file permissions in Unix-like format from a Myznode using the stat structure
int getPermissions(Myznode node);

void getChangeTime(Myznode node, char* timestamp);

void getAccessTime(Myznode node, char* timestamp);

void getModTime(Myznode node, char* timestamp);

void myz_print(Myzdata data);

void myznode_addEntry(Myznode node, int index);

void writeData(Myzdata data);
#endif
