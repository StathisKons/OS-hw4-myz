#include "metadata.h"
#include "sys_utils.h"
#include "vector.h"
#include "myz.h"
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
#include "header.h"


static void myznode_destroy(Pointer myz_node);
static bool is_compressed(char* path);
char* compress_and_read(const char* path, int* fsize);
static void read_data(char* path, Metadata metadata, bool compressed, int dir_index);

//Retrieve file permissions in Unix-like format from a Myznode using the stat structure
//static mode_t getPermissions(MyzNode node)
// 	{
//  		return node->info.st_mode & 0777;
// 	}

Entry entry_create(char* name, int myznode_index){
	Entry entry = safe_malloc(sizeof(*entry));
	strcpy(entry->name, name);
	entry->myznode_index = myznode_index;	

	return entry;
}

void entries_insert(MyzNode node, char* name, int myznode_index)
{
	assert(name != NULL && myznode_index >= 0);

	Entry entry = entry_create(name, myznode_index);

	if(node->entries == NULL){
		node->entries = vector_create(0, free);
	}

	vector_insert_last(node->entries, entry);
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

// static FileType find_type(mode_t mode)
// {
// 	if(S_ISDIR(mode)){
// 		return DIRECTORY;
// 	} else if(S_ISREG(mode)) {
// 		return REGULAR_FILE;
// 	} else if(S_ISLNK(mode)){
// 		return SYMBOLIC_LINK;
// 	} else {
// 		fprintf(stderr, "unsupported file type\n");
// 		exit(EXIT_FAILURE);
// 	}
// }

void read_Data(Metadata metadata, char* path, bool compressed)
{
	struct stat info;
	char* tpath = (char*)strdup(path);
	char relpath[1024] = {0};

	char* token;
	char* delims = "/";
	token = strtok(tpath, delims);
	MyzNode prev = NULL;
	MyzNode cur;
	bool fl = true;
	do {
		strcat(relpath, token);
		strcat(relpath, "/");
		safe_sys(lstat(relpath, &info));
		if(!S_ISDIR(info.st_mode))
		{
			break;
		}
		metadata_insert(metadata, token, info, false, 0, NULL);
		if(fl)
		{
			cur = vector_get_at(metadata->nodes, 0); 
		}

		if(prev != NULL)
		{
			entries_insert(prev, token, vector_size(metadata->nodes) - 1);
		}
		prev = cur;
	} while((token = strtok(NULL, delims)));
	free(tpath);
	read_data(path, metadata, compressed, vector_size(metadata->nodes) - 1);
}

void metadata_insert_node(Metadata metadata, MyzNode node){
	assert(metadata != NULL && node != NULL && metadata->nodes != NULL);
	vector_insert_last(metadata->nodes, node);
}


void metadata_insert(Metadata metadata, char* name, struct stat info, bool compressed, long int file_size, char* file_data)
{
	assert(metadata != NULL);

	MyzNode node = safe_malloc(sizeof(*node));

	strcpy(node->name, name); 
	node->info = info;
	// node->type = find_type(info.st_mode);
	node->compressed = compressed;

	node->data_offset = -1;	// TODO Define
	node->file_size = file_size;
	node->file_data = file_data;
	node->entries = NULL;

	metadata_insert_node(metadata, node);
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
	guaranteed_read(fd, fdata, sizeof(*fdata) * (*fsize));

	close(fd);
	return fdata;
}

 
static void read_data(char* path, Metadata metadata, bool compressed, int dir_index)
{
	DIR* directory = opendir(path);
	if(directory == NULL)
	{
		perror("Error opening directory\n");
		exit(EXIT_FAILURE);
	}
	
	struct dirent *entries;
	while((entries = readdir(directory)) != NULL)
	{
		if(strcmp(entries->d_name, ".") == 0 || strcmp(entries->d_name, "..") == 0) continue;
		char fpath[1024] = {0};
		sprintf(fpath, "%s/%s", path, entries->d_name);
		
		struct stat info;
	printf("DEBUG\t %s\n", fpath);
		safe_sys(lstat(fpath, &info));

		char* fdata = NULL;
		int fsize = 0;
		char fname[259] = {0};
		strcpy(fname, entries->d_name);
		
		if(S_ISREG(info.st_mode))
		{
			if(compressed && !is_compressed(fpath))
			{
				fdata = compress_and_read(fpath, &fsize);
				strcat(fname, ".gz");
			}
			else
			{
				fdata = read_file(fpath, &fsize);
			}
		}
		metadata_insert(metadata, fname, info, compressed, fsize, fdata);

		if(dir_index != -1)
		{
			entries_insert(vector_get_at(metadata->nodes, dir_index), entries->d_name, vector_size(metadata->nodes) - 1);
		}

		if(S_ISDIR(info.st_mode) && strcmp(entries->d_name, ".") && strcmp(entries->d_name, ".."))
		{
			read_data(fpath, metadata, compressed, vector_size(metadata->nodes) - 1);
		}

	}

	closedir(directory);
}

static void print_directory(Metadata metadata, MyzNode node, char* path, bool* visited)
{
	if(node->entries == NULL)
	{
		printf("%s/\n", path);
		return;
	}
	for(int i = 0; i < vector_size(node->entries); i++)
	{
		Entry entry = vector_get_at(node->entries, i);
		visited[entry->myznode_index] = 1;

		MyzNode tnode = vector_get_at(metadata->nodes, entry->myznode_index);

		if(S_ISDIR(tnode->info.st_mode))
		{
			char npath[1024] = {0};
			sprintf(npath, "%s/%s", path, tnode->name);
			print_directory(metadata, tnode, npath, visited);
		}
		else if(S_ISREG(tnode->info.st_mode))
		{
			printf("%s/%s\n", path, tnode->name);
		}
	} 
}

void print_data(Metadata metadata)
{
	bool* visited = calloc(vector_size(metadata->nodes), sizeof(bool));

	for(int i = 0; i < vector_size(metadata->nodes); i++)
	{
		MyzNode node = vector_get_at(metadata->nodes, i);
		if(visited[i]) continue;
		visited[i] = true;
		if(S_ISDIR(node->info.st_mode))
		{
			char path[1000] = {0};
			strcpy(path, node->name);
			print_directory(metadata, node, path, visited);
		}
		else if(S_ISREG(node->info.st_mode))
		{
			printf("%s\n", node->name);
		}
	}

	free(visited);
}

static bool is_compressed(char* path)
{
	int fd = open(path, O_RDONLY);
	unsigned char buff[2];
	guaranteed_read(fd, buff, sizeof(buff));

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
	strcpy(tname, path);

	compress(tname);
	strcat(tname, ".gz");

	int fd = open(tname, O_RDONLY);
	*fsize = get_file_size(fd);

	char* fdata = safe_malloc(sizeof(*fdata) * (*fsize));
	guaranteed_read(fd, fdata, sizeof(*fdata) * (*fsize));

	decompress(tname);

	return fdata;
}

static void write_rec(Metadata metadata, MyzNode node, char* path, bool* visited)
{
	if(node->entries == NULL)
		return;
	for(int i = 0; i < vector_size(node->entries); i++)
	{		
		Entry entry = vector_get_at(node->entries, i);
		MyzNode tnode = vector_get_at(metadata->nodes, entry->myznode_index);
		
		char tpath[1056] = {0};
		strcat(tpath, path);
		strcat(tpath, "/");
		strcat(tpath, tnode->name);


		if(visited[entry->myznode_index]) continue;
		visited[entry->myznode_index] = true;
		
		if(S_ISDIR(tnode->info.st_mode))
		{
			mkdir(tpath, node->info.st_mode);
			write_rec(metadata, tnode, tpath, visited);
		}
		else if(S_ISREG(tnode->info.st_mode))
		{
			printf("PATH %s\n", tpath);
			int fd;
			safe_sys_assign(fd, open(tpath, O_CREAT | O_TRUNC | O_WRONLY, tnode->info.st_mode));
			safe_sys(write(fd, tnode->file_data, tnode->file_size));
			close(fd);
			if(tnode->compressed)
			{
				int pid = fork();
				if(pid == 0)
				{
					char* argv[] = {"gzip", "-d", tpath, NULL};
					execvp("gzip", argv);
				}
				wait(NULL);
			}
		}
	}

	

}

void write_Data(Metadata metadata)
{
	bool* visited = calloc(vector_size(metadata->nodes), sizeof(*visited));

	for(int i = 0; i < vector_size(metadata->nodes); i++)
	{
		MyzNode node;
		node = vector_get_at(metadata->nodes, i);
		if(visited[i]) continue;
		visited[i] = true;

		if(S_ISDIR(node->info.st_mode))
		{
			mkdir(node->name, node->info.st_mode);
			write_rec(metadata, node, node->name, visited);
		}
		else if(S_ISREG(node->info.st_mode))
		{
			int fd = open(node->name, O_CREAT | O_TRUNC | O_WRONLY, node->info.st_mode);
			safe_sys(write(fd, node->file_data, node->file_size));
			close(fd);
			if(node->compressed)
			{
				int pid = fork();
				if(pid == 0)
				{
					char* argv[] = {"gzip", "-d", node->name, NULL};
					execvp("gzip", argv);
				}
				wait(NULL);
			}
		}
	}


	free(visited);

}

static void write_entries(Vector entries, int fd){
	if(entries == NULL){
		int size = 0; 
		safe_sys(write(fd, &size, sizeof(size)));
		return;
	}

	int size = vector_size(entries);
	safe_sys(write(fd, &size, sizeof(size)));

	for(int i = 0 ; i < size ; i++){
		Entry entry = vector_get_at(entries, i);
		off_t offset = lseek(fd, 0, SEEK_CUR);
		printf("DEBUG\t Writing entry %s, at offset %ld, index %d\n", entry->name, offset, entry->myznode_index);
		safe_sys(write(fd, entry->name, sizeof(*entry->name) * MAX_NAME));
		safe_sys(write(fd, &(entry->myznode_index), sizeof(entry->myznode_index)));
		printf("DEBUG\t Finished writing entry %s, new offset %ld\n", entry->name, lseek(fd, 0, SEEK_CUR));
	}
}
	


void metadata_write_node(MyzNode node, int fd){
	
	safe_sys(write(fd, node->name, sizeof(node->name)));
	safe_sys(write(fd, &(node->info), sizeof(node->info)));	
	safe_sys(write(fd, &(node->compressed), sizeof(node->compressed)));
	safe_sys(write(fd, &(node->data_offset), sizeof(node->data_offset)));
	safe_sys(write(fd, &(node->file_size), sizeof(node->file_size)));

	write_entries(node->entries, fd);
}

static Vector read_entries(int fd)
{
	int size;
	guaranteed_read(fd, &size, sizeof(size));
	printf("ENTRIES: %d\n", size);
	
	if(size == 0)
		return NULL;

	Vector entries = vector_create(size, free);
	for(int i = 0; i < size; i++)
	{
		Entry entry = safe_malloc(sizeof(*entry));
		// entry->name = safe_malloc(sizeof(*entry->name) * MAX_NAME);		// TODO CHANGE
		int index;
		guaranteed_read(fd, entry->name, /*sizeof(*entry->name)**/256);
		guaranteed_read(fd, &index, sizeof(index));
		entry->myznode_index = index;
		
		vector_insert_last(entries, entry);
	}
	printf("FINISHED\n");
	return entries;
}


MyzNode metadata_read_node(int fd)
{
	MyzNode node = safe_malloc(sizeof(*node));
	int ret;
	safe_sys_assign(ret, (guaranteed_read(fd, node->name, sizeof((node->name)))));
	if (ret >= sizeof(node->name))
	{
        node->name[sizeof(node->name) - 1] = '\0';
	}
    else
	{
        node->name[ret] = '\0';
	}

	printf("metadata read node: %s\n", node->name);
	if(ret == 0)
	{
		free(node);
		return NULL;
	}

	bool var = false;
	guaranteed_read(fd, &(node->info), sizeof(node->info));
	guaranteed_read(fd, &var, sizeof(var));
	node->compressed = var;
	guaranteed_read(fd, &(node->data_offset), sizeof(node->data_offset));
	guaranteed_read(fd, &(node->file_size), sizeof(node->file_size));
	if(S_ISDIR(node->info.st_mode))
		node->entries = read_entries(fd);
	else
		node->entries = NULL;

	return node;
}
