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


static Myznode myznode_init(char* fname, struct stat info, int nested)
{
	Myznode node = (Myznode)malloc(sizeof(myznode));

	node->info = info;
	memset(node->fname , 0, 1000);
	strcpy(node->fname, fname);

	node->nested = nested;
	if(!nested)
	{
		node->contents = NULL;
	}

	return node;

}

static void expand(Myzdata data)
{
	data->capacity *= 2;
	data->array = (Myznode*)realloc(data->array, data->capacity * sizeof(Myznode));
}



void myznode_insert(Myzdata data, char* fname, struct stat info, int nested)
{
	if(data->curelements == data->capacity)
	{
		expand(data);
	}
	Myznode node = myznode_init(fname, info, nested);
	data->array[data->curelements] = node;
	data->curelements++;
}

Myzdata myz_init(int capacity)
{
	Myzdata data = (Myzdata)malloc(sizeof(myzdata));

	data->capacity = capacity;
	data->array = (Myznode*)malloc(sizeof(Myznode)*capacity);

	for(int i = 0; i < capacity; i++)
		data->array[i] = NULL;

	return data;
}




// Retrieve file permissions in Unix-like format from a Myznode using the stat structure
int getPermissions(Myznode node)
{
	int user = 0;
	struct stat* info = &node->info;
	if(info->st_mode & S_IRUSR)
		user += 4;
	if(info->st_mode & S_IWUSR)
		user += 2;
	if(info->st_mode & S_IXUSR)	
		user += 1;

	int group = 0;
	if(info->st_mode & S_IRGRP)
		group += 4;
	if(info->st_mode & S_IWGRP)
		group += 2;
	if(info->st_mode & S_IXGRP)
		group += 1;

	int others = 0;
	if(info->st_mode & S_IROTH)
		others += 4;
	if(info->st_mode & S_IWOTH)
		others += 2;
	if(info->st_mode & S_IXOTH)
		others += 1;

	return user*100 + group*10 + others;
}

void getAccessTime(Myznode node, char* timestamp)
{
	strcpy(timestamp, ctime(&node->info.st_atime));
}

void getModTime(Myznode node, char* timestamp)
{
	strcpy(timestamp, ctime(&node->info.st_mtime));
}

void getChangeTime(Myznode node, char* timestamp)
{
	strcpy(timestamp, ctime(&node->info.st_ctime));
}



