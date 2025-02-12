#include "metadata.h"
#include "sys_utils.h"
#include <sys/stat.h>
#include <assert.h>

#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <wait.h>


static void myznode_destroy(Pointer myz_node);

char* compress_and_read(const char* path, int* fsize);

// Retrieve file permissions in Unix-like format from a Myznode using the stat structure
static mode_t getPermissions(MyzNode node)
{
	return node->info.st_mode & 0777;
}






Metadata metadata_create(void)
{
	Metadata metadata = safe_malloc(sizeof(*metadata));
	metadata->nodes = vector_create(0, myznode_destroy);
	
	return metadata;
}

void metadata_destroy(Metadata metadata)
{
	assert(metadata != NULL);

	vector_destroy(metadata->nodes);
	free(metadata);
}

static FileType find_type(mode_t mode)
{
	if(S_ISDIR(mode)){
		return DIRECTORY;
	} else if(S_ISREG(mode)) {
		return REGULAR_FILE;
	} else if(S_ISLNK(mode)){
		return SYMBOLIC_LINK;
	} else {
		fprintf(stderr, "unsupported file type\n");
		exit(EXIT_FAILURE);
	}
}

void metadata_insert(Metadata metadata, char* name, struct stat info, bool compressed, long int file_size, char* file_data)
{
	assert(metadata != NULL);

	MyzNode node = safe_malloc(sizeof(*node));

	strcpy(node->name, name); 
	node->info = info;
	node->type = find_type(info.st_mode);
	node->compressed = compressed;

	node->data_offset = -1;	// TODO Define
	node->file_size = file_size;
	node->file_data = file_data;
	node->entries = NULL;

	vector_insert_last(metadata->nodes, node);
}

static void myznode_destroy(Pointer myz_node)
{
	assert(myz_node != NULL);

	MyzNode node = myz_node;
	if(node->file_data != NULL){
		free(node->file_data);
	}

	if(node->entries != NULL){
		vector_destroy(node->entries);
	}
}


char* read_file(char* path, int* fsize){
	int fd = open(path, O_RDONLY);

	*fsize = lseek(fd, 0, SEEK_END);
	lseek(fd, 0, SEEK_SET);

	char* fdata = safe_malloc(sizeof(*fdata) * (*fsize));
	safe_sys(read(fd, fdata, sizeof(*fdata) * (*fsize)));

	close(fd);
	return fdata;
	
}

 
void read_data(char* path, Metadata metadata, bool compressed, int dir_index)
{
	bool nested;
	DIR* directory;
	struct dirent *entries;
	struct stat info;


	directory = opendir(path);
	if(!directory)
	{
		perror("Error opening directory\n");
		exit(EXIT_FAILURE);
	}
	
	while((entries = readdir(directory)) != NULL)
	{
		nested = 0;
		char fname[1000];
		memset(fname, 0, 1000);
		sprintf(fname, "%s/%s", path, entries->d_name);

		safe_sys(lstat(fname, &info));
		if(S_ISDIR(info.st_mode))
			nested = 1;


		char* fdata;
		int fsize;
		if(S_ISREG(info.st_mode))
		{
			if(compressed)
			{
				compress_and_read(fname, &fsize);
			}
	lse
			{
				read_file(fname, metad)
			}
		}


	}

}

static bool is_compressed(char* path)
{
	int fd = open(path, O_RDONLY);
	unsigned char buff[2];
	read(fd, buff, sizeof(buff));

	close(fd);
	return buff[0] == 0x1f && buff[1] == 0x8b;
}


// moves fd to start of file
static off_t get_file_size(int fd){
	off_t fsize = lseek(fd, 0, SEEK_END);
	lseek(fd, 0 , SEEK_SET);

	return fsize;
}

// compreses file and static 
void compress(char* path)
{
	int pid = fork();
	if(!pid) 
	{
		char* argv[] = {"gzip", path, NULL};
		safe_sys(execvp("gzip", argv));
	}
	safe_sys(wait(NULL));
}

void decompress(char* path)
{
	int pid = fork();
	if(!pid)
	{
		char* argv[] = {"gzip", "-d", path, NULL};
		safe_sys(execvp("gzip", argv));
	}
	wait(NULL);

}

char* compress_and_read(const char* path, int* fsize)
{
	char tname[1024];
	strcpy(tname, "path");

	compress(tname);
	strcat(tname, ".gz");

	int fd = open(tname, O_RDONLY);
	*fsize = get_file_size(fd);

	char* fdata = safe_malloc(sizeof(*fdata) * (*fsize));
	safe_sys(read(fd, fdata, sizeof(*fdata) * (*fsize)));

	decompress(tname);

	return fdata;
}








// static void write_rec(Myzdata data, Myznode node, int* visited, char* filepath)
// {
// 	Myznode nd; 
// 	for(int i = 0; i < node->entries->curelements; i++)
// 	/ 		int ind = node->entries->entries[i];
// 		nd = data->array[ind];

// 		if(visited[ind] == 1) continue;
// 		else visited[ind] = 1;

// 		if(S_ISDIR(nd->info.st_mode) && strcmp(nd->fname, "..") && strcmp(nd->fname, "."))
// 		{
// 			char npath[300];
// 			memset(npath, 0, 300);
// 			strcpy(npath, filepath);
// 			strcat(npath, "/");
// 			strcat(npath, nd->fname);
			
// 			mkdir(npath, getPermissions(nd));
// 			write_rec(data, nd, visited, npath);
// 		}
// 		else if(S_ISREG(nd->info.st_mode))
// 		{
// 			char npath[300];
// 			memset(npath, 0, 300);
// 			strcpy(npath, filepath);
// 			strcat(npath, "/");
// 			strcat(npath, nd->fname);
			
// 			if(!nd->compressed)
// 			{
// 				int fd = open(npath, O_CREAT | O_WRONLY | O_TRUNC, getPermissions(nd));
// 				write(fd, nd->filedata, nd->fsize);
// 				close(fd);
// 			}
//      			else if(nd->compressed)
// 			{
// 				strcat(npath, ".gz");
// 				int fd = open(npath, O_CREAT | O_WRONLY | O_TRUNC, getPermissions(nd));
// 				write(fd, nd->filedata, nd->fsize);
// 				close(fd);
// 				int pid = fork();
// 				if(!pid)
// 				{
// 					char* argv[] = {"gzip", "-d",  npath, NULL};
// 					execvp("gzip", argv);
// 				}
// 				wait(NULL);
// 			}
     			
// 		}
// 	}

// }


// void writeData(Myzdata data)
// {	
// 	Myznode node;
// 	int* visited = calloc(data->curelements, sizeof(int));
// 	char filepath[] = "something2";
// 	mkdir(filepath, 0777);
// 	Myznode nd;

// 	for(int i = 0; i < data->curelements; i++)
// 	{
// 		nd = data->array[i];
// 		if(visited[i] == 1) continue;
// 		else visited[i] = 1;
		

// 		if(S_ISDIR(nd->info.st_mode) && strcmp(nd->fname, "..") && strcmp(nd->fname, "."))
// 		{
//     			char npath[300];
// 			memset(npath, 0, 300);
// 			strcpy(npath, filepath);
// 			strcat(npath, "/");
// 			strcat(npath, nd->fname);
// 			mkdir(npath, getPermissions(nd));
// 			write_rec(data, nd, visited, npath);
// 		}
//      		else if(S_ISREG(nd->info.st_mode))
//      		{
//     			char npath[300];
// 			memset(npath, 0, 300);
// 			strcpy(npath, filepath);
// 			strcat(npath, "/");
// 			strcat(npath, nd->fname);
// 			if(!nd->compressed)
// 			{
// 				int fd = open(npath, O_CREAT | O_WRONLY | O_TRUNC, getPermissions(nd));
// 				write(fd, nd->filedata, nd->fsize);
// 				close(fd);
// 			}
//      			else if(nd->compressed)
// 			{
// 				strcat(npath, ".gz");
// 				int fd = open(npath, O_CREAT | O_WRONLY | O_TRUNC, getPermissions(nd));
// 				write(fd, nd->filedata, nd->fsize);
// 				close(fd);
// 				int pid = fork();
// 				if(!pid)
// 				{
// 					char* argv[] = {"gzip", "-d",  npath, NULL};
// 					execvp("gzip", argv);
// 				}
// 				wait(NULL);
// 			}
     			

// 		}
		
// 	}

// 	free(visited);

// }

// void getAccessTime(Myznode node, char* timestamp)
// {
// 	strcpy(timestamp, ctime(&node->info.st_atime));
// }

// void getModTime(Myznode node, char* timestamp)
// {
// 	strcpy(timestamp, ctime(&node->info.st_mtime));
// }

// void getChangeTime(Myznode node, char* timestamp)
// {
// 	strcpy(timestamp, ctime(&node->info.st_ctime));
// }




