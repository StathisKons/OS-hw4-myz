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


void myznode_destroy(Pointer myz_node);
static bool is_compressed(char* path);
char* compress_and_read(const char* path, long int* fsize);
void read_data(char* path, Metadata metadata, bool compressed, int dir_index);

//retrieve file permissions in Unix-like format from a Myznode using the stat structure
static mode_t getPermissions(MyzNode node)
	{
 		return node->info->mode & 0777;
	}

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

void read_Data(Metadata metadata, char* path, bool compressed)
{
	struct stat info;
	char* tpath = strdup(path);
	char relpath[1024] = {0};

	char* token;
	char* delims = "/";
	token = strtok(tpath, delims);
	info.st_mode =  __S_IFDIR;
	metadata_insert(metadata, ".", info, compressed, 0, NULL);
	
	MyzNode prev = vector_get_at(metadata->nodes, 0);
	MyzNode cur;
	do {
		strcat(relpath, token);
		strcat(relpath, "/");
		safe_sys(lstat(relpath, &info));
		if(!S_ISDIR(info.st_mode))
		{
			break;
		}
		metadata_insert(metadata, token, info, false, 0, NULL);
		cur = vector_get_at(metadata->nodes, vector_size(metadata->nodes) - 1);
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
	node->info = safe_malloc(sizeof(*node->info));
	node->info->gid = info.st_gid;
	node->info->uid = info.st_uid;
	node->info->mode = info.st_mode;
	// node->type = find_type(info->mode);
	node->compressed = compressed;

	node->data_offset = -1;	// TODO Define
	node->file_size = file_size;
	node->file_data = file_data;
	node->entries = NULL;

	metadata_insert_node(metadata, node);
}

void myznode_destroy(Pointer myz_node)
{
	assert(myz_node != NULL);

	MyzNode node = myz_node;
	if(node->file_data != NULL){
		free(node->file_data);
	}

	if(node->entries != NULL){
		vector_destroy(node->entries);
	}

	if(node->info != NULL){
		free(node->info);
	}
}


char* read_file(char* path, long int* fsize){
	int fd = open(path, O_RDONLY);


	*fsize = lseek(fd, 0, SEEK_END);
	if(*fsize == 0) 
	{
		close(fd);
		return NULL;
	}
	lseek(fd, 0, SEEK_SET);

	char* fdata = safe_malloc(sizeof(*fdata) * (*fsize));
	guaranteed_read(fd, fdata, sizeof(*fdata) * (*fsize));
	close(fd);
	return fdata;
}

 
void read_data(char* path, Metadata metadata, bool compressed, int dir_index)
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
		//printf("Entries: %s\n", entries->d_name);
		sprintf(fpath, "%s/%s", path, entries->d_name);
		
		struct stat info;
		printf("fpath: %s\n", fpath);
		safe_sys(lstat(fpath, &info));

		char* fdata = NULL;
		long int fsize = 0;
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

char* compress_and_read(const char* path, long int* fsize)
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
		
		if(S_ISDIR(tnode->info->mode))
		{
			mkdir(tpath, getPermissions(tnode));
			write_rec(metadata, tnode, tpath, visited);
		}
		else if(S_ISREG(tnode->info->mode))
		{
			printf("%s\n", tnode->name);
			int fd;
			printf("PATH: %s\n", tpath);
			safe_sys_assign(fd, open(tpath, O_CREAT | O_TRUNC | O_WRONLY, getPermissions(tnode)));
			if(tnode->file_data != NULL){						// !!! CHECK GIANNH, to ebala twra
				safe_sys(write(fd, tnode->file_data, tnode->file_size));
			}
			close(fd);
			if(tnode->compressed)
			{
				int pid = fork();
				if(pid == 0)
				{
					char* argv[] = {"gzip", "-d", "-f", tpath, NULL};
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

		if(S_ISDIR(node->info->mode))
		{
			mkdir(node->name, getPermissions(node));
			write_rec(metadata, node, node->name, visited);
		}
		else if(S_ISREG(node->info->mode))
		{
			int fd = open(node->name, O_CREAT | O_TRUNC | O_WRONLY, getPermissions(node));
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

MyzNode metadata_find_node(Metadata metadata, const char* path_to_find, bool* exists){
	assert(metadata != NULL && path_to_find != NULL);

	*exists = true;

	char* path = strdup(path_to_find);	// strtok modifies the string so we create a copy
	char* token = strtok(path, "/");

	MyzNode current = vector_get_at(metadata->nodes, 0);

	while(token != NULL){
		if(!S_ISDIR(current->info->mode)){
			break;
		}

		bool found = false;
		for(int size = vector_size(current->entries), i = 0 ; i < size ; i++){
			Entry entry = vector_get_at(current->entries, i);
			MyzNode node = vector_get_at(metadata->nodes, entry->myznode_index);
			if(compare_names(node, token)){		// if names are equal 
				current = node;
				found = true;
				break;
			}
		}

		if(!found){
			*exists = false;
			break;
		}

		token = strtok(NULL, "/");
	}

	free(path);
	return current;
}


MyzNode metadata_find_parent(Metadata metadata, const char* path_to_find, bool* exists){
	assert(metadata != NULL && path_to_find != NULL);

	*exists = false;

	for(int i = strlen(path_to_find) - 2 ; i >= 0 ; i--){
		if(path_to_find[i] == '/'){
			char* parent_path = strndup(path_to_find, i);
			MyzNode parent = metadata_find_node(metadata, parent_path, exists);
			free(parent_path);
			return parent;
		}
	}

	return vector_get_at(metadata->nodes, 0);
}

static void print_file_permissions(mode_t mode){
	printf( (S_ISDIR(mode))  ? "d" : "-");
    printf( (mode & S_IRUSR) ? "r" : "-");
    printf( (mode & S_IWUSR) ? "w" : "-");
    printf( (mode & S_IXUSR) ? "x" : "-");
    printf( (mode & S_IRGRP) ? "r" : "-");
    printf( (mode & S_IWGRP) ? "w" : "-");
    printf( (mode & S_IXGRP) ? "x" : "-");
    printf( (mode & S_IROTH) ? "r" : "-");
    printf( (mode & S_IWOTH) ? "w" : "-");
    printf( (mode & S_IXOTH) ? "x" : "-");
}

void print_rec(Metadata metadata, MyzNode node, int rec, bool print_metadata)
{
	if(node->entries == NULL)
		return;

	for(int i = 0; i < vector_size(node->entries); i++)
	{
		Entry entry = vector_get_at(node->entries, i);
		//printf("NODE: %s, INDEX GIVEN: %d  METADATA SIZE: %d \n", node->name, entry->myznode_index, vector_size(metadata->nodes));
		MyzNode tnode = vector_get_at(metadata->nodes, entry->myznode_index);
		for(int j = 0; j < rec; j++)
		{
			printf(" ");
		}
		printf("|");


		
		for(int j = 0; j < rec; j++)
		{
			printf("-");
		}
		printf(" %s\t", tnode->name);
		if(print_metadata)
		{
			print_file_permissions(tnode->info->mode);
			printf(" Uid(%u), Gid(%u)", tnode->info->uid, tnode->info->gid);
		}
		putchar('\n');
		if(S_ISDIR(tnode->info->mode))
		{
			print_rec(metadata, tnode, rec + 1, print_metadata);
		}
	}

}

void print(Metadata metadata, bool print_metadata)
{
	bool* visited = calloc(vector_size(metadata->nodes), sizeof(bool));
	MyzNode node = vector_get_at(metadata->nodes, 0);
	if(node->entries == NULL)
		return;

	print_rec(metadata, node, 0, print_metadata);
}

void print_s(Metadata metadata)
{
	for(int i = 0; i < vector_size(metadata->nodes); i++)
	{
		MyzNode node = vector_get_at(metadata->nodes, i);
		printf("INDEX: %d\tNAME: %s", i, node->name);
		if(!S_ISDIR(node->info->mode))
		{
			printf("\t%ld", node->data_offset);
		}
		printf("\n");
	}
}