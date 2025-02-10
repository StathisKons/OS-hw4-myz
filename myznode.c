#include "myznode.h"
#include <sys/stat.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <wait.h>

int isDirectory(struct stat st)
{
	return S_ISDIR(st.st_mode);
}

static Myznode myznode_init(char* fname, struct stat info, int nested)
{
	Myznode node = (Myznode)malloc(sizeof(myznode));

	node->info = info;
	memset(node->fname , 0, 1000);
	strcpy(node->fname, fname);

	node->nested = nested;
	if(nested)
	{
		node->entries = (Vector)malloc(sizeof(vector));
		vec_init(node->entries);
	}

	return node;

}

// Retrieve file permissions in Unix-like format from a Myznode using the stat structure
mode_t getPermissions(Myznode node)
{
	return node->info.st_mode & 0777;
}


static void expand(Myzdata data)
{
	data->capacity *= 2;
	data->array = (Myznode*)realloc(data->array, data->capacity * sizeof(Myznode));
}


void myznode_addEntry(Myznode node, int index)
{
	vec_insert(node->entries, index);
}


void myznode_insert(Myzdata data, char* fname, struct stat info, int nested, int compressed)
{
	
	Myznode node = myznode_init(fname, info, nested);
	node->compressed = compressed;
	data->array[data->curelements] = node;
	data->curelements++;
	if(data->curelements == data->capacity)
	{
		expand(data);
	}
}

Myzdata myz_init(int capacity)
{
	Myzdata data = (Myzdata)malloc(sizeof(myzdata));

	data->capacity = capacity;
	data->array = (Myznode*)malloc(sizeof(Myznode)*capacity);

	for(int i = 0; i < capacity; i++)
		data->array[i] = NULL;

	data->curelements = 0;

	return data;
}

void myz_print(Myzdata data)
{
	Myznode node;
	int* printed = malloc(data->curelements * sizeof(int));
	for(int i = 0; i < data->curelements; i++)
	{
		node = data->array[i];
		if(printed[i] != 1)
			printf("%s\n", node->fname);
		printed[i] = 1;
		if(node->nested)
		{
			for(int j = 0; j < node->entries->curelements; j++)
			{
       				int ind = node->entries->entries[j];
				printed[node->entries->entries[j]] = 1;
				printf("  %s\n", data->array[ind]->fname);
			}
		}
	}
	free(printed);

}

void Myz_destroy(Myzdata data)
{
	Myznode node;
	for(int i = 0; i < data->curelements; i++)
	{
		node = data->array[i];
		if(node->nested)
		{
			vec_destroy(node->entries);
			free(node->entries);
		}
		else  if(S_ISREG(node->info.st_mode))
			free(node->filedata);
		free(node);
	}

	free(data->array);
	free(data);
}


static void write_rec(Myzdata data, Myznode node, int* visited, char* filepath)
{
	Myznode nd; 
	for(int i = 0; i < node->entries->curelements; i++)
	{
		int ind = node->entries->entries[i];
		nd = data->array[ind];

		if(visited[ind] == 1) continue;
		else visited[ind] = 1;

		if(S_ISDIR(nd->info.st_mode) && strcmp(nd->fname, "..") && strcmp(nd->fname, "."))
		{
			char npath[300];
			memset(npath, 0, 300);
			strcpy(npath, filepath);
			strcat(npath, "/");
			strcat(npath, nd->fname);
			
			mkdir(npath, getPermissions(nd));
			write_rec(data, nd, visited, npath);
		}
		else if(S_ISREG(nd->info.st_mode))
		{
			char npath[300];
			memset(npath, 0, 300);
			strcpy(npath, filepath);
			strcat(npath, "/");
			strcat(npath, nd->fname);
			
			if(!nd->compressed)
			{
				int fd = open(npath, O_CREAT | O_WRONLY | O_TRUNC, getPermissions(nd));
				write(fd, nd->filedata, nd->fsize);
				close(fd);
			}
     			else if(nd->compressed)
			{
				strcat(npath, ".gz");
				int fd = open(npath, O_CREAT | O_WRONLY | O_TRUNC, getPermissions(nd));
				write(fd, nd->filedata, nd->fsize);
				close(fd);
				int pid = fork();
				if(!pid)
				{
					char* argv[] = {"gzip", "-d",  npath, NULL};
					execvp("gzip", argv);
				}
				wait(NULL);
			}
     			
		}
	}

}


void writeData(Myzdata data)
{	
	Myznode node;
	int* visited = calloc(data->curelements, sizeof(int));
	char filepath[] = "something2";
	mkdir(filepath, 0777);
	Myznode nd;

	for(int i = 0; i < data->curelements; i++)
	{
		nd = data->array[i];
		if(visited[i] == 1) continue;
		else visited[i] = 1;
		

		if(S_ISDIR(nd->info.st_mode) && strcmp(nd->fname, "..") && strcmp(nd->fname, "."))
		{
    			char npath[300];
			memset(npath, 0, 300);
			strcpy(npath, filepath);
			strcat(npath, "/");
			strcat(npath, nd->fname);
			mkdir(npath, getPermissions(nd));
			write_rec(data, nd, visited, npath);
		}
     		else if(S_ISREG(nd->info.st_mode))
     		{
    			char npath[300];
			memset(npath, 0, 300);
			strcpy(npath, filepath);
			strcat(npath, "/");
			strcat(npath, nd->fname);
			if(!nd->compressed)
			{
				int fd = open(npath, O_CREAT | O_WRONLY | O_TRUNC, getPermissions(nd));
				write(fd, nd->filedata, nd->fsize);
				close(fd);
			}
     			else if(nd->compressed)
			{
				strcat(npath, ".gz");
				int fd = open(npath, O_CREAT | O_WRONLY | O_TRUNC, getPermissions(nd));
				write(fd, nd->filedata, nd->fsize);
				close(fd);
				int pid = fork();
				if(!pid)
				{
					char* argv[] = {"gzip", "-d",  npath, NULL};
					execvp("gzip", argv);
				}
				wait(NULL);
			}
     			

		}
		
	}

	free(visited);

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




