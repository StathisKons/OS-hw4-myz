#include "metadata.h"
#include "sys_utils.h"
#include "myz.h"
#include <assert.h> 
#include <dirent.h> 
#include <fcntl.h>
#include <wait.h>


static void write_links(Metadata metadata);
static bool is_compressed(char* path);
static char* compress_and_read(const char* path, long int* fsize);

//retrieve file permissions in Unix-like format from a Myznode using the stat structure
static mode_t getPermissions(MyzNode node)
{
	return node->info->mode & 0777;
}

static Entry entry_create(char* name, int myznode_index){
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

// Build filepath entries and recursively read the directory if it ends at one
void read_Data(Metadata metadata, const char* path, bool compressed)
{
	bool fl = false;
	struct stat info;
	char* tpath = strdup(path);
	char relpath[1024] = {0};

	char* token;
	char* delims = "/";
	token = strtok(tpath, delims);
	lstat(".", &info); // the first entry of the metadata is a parent file for easier access
	metadata_insert(metadata, ".", info, compressed, 0, NULL);
	
	MyzNode prev = vector_get_at(metadata->nodes, 0);
	MyzNode cur;
	do {
		strcat(relpath, token);
		safe_sys(lstat(relpath, &info));
		// If the filepath it ends in a non-directory 
		if(!S_ISDIR(info.st_mode))
		{
			fl = true;
			break;
		}

		// Keep track of the previous directory , to update its entries
		strcat(relpath, "/");
		metadata_insert(metadata, token, info, false, 0, NULL);
		cur = vector_get_at(metadata->nodes, vector_size(metadata->nodes) - 1);
		if(prev != NULL)
		{
			entries_insert(prev, token, vector_size(metadata->nodes) - 1);
		}
		prev = cur;
	} while((token = strtok(NULL, delims)));

	
	if(fl)
	{
		char* file_data = NULL;
		long int fsize = 0;
		char name[256] = {0};
		
		// Insert the non-directory file , the filepath ended in
		if(S_ISLNK(info.st_mode))
		{
			file_data = safe_malloc(sizeof(*file_data) * (info.st_size + 1));
			fsize = info.st_size + 1;
			readlink(relpath, file_data, fsize);
			file_data[info.st_size] = '\0';
			strcpy(name, token);
		}
		if(S_ISREG(info.st_mode))
		{
			strcpy(name, token);
			if(compressed && !is_compressed(relpath))
			{
				file_data = compress_and_read(relpath, &fsize);
				strcat(name, ".gz");
			}
			else
			{
				file_data = read_file(relpath, &fsize);
			}
		}
		metadata_insert(metadata, token, info, compressed, fsize, file_data);
		entries_insert(prev, token, vector_size(metadata->nodes) - 1);
	}
	free(tpath);

	// Insert the directory the filepath ended to
	if(!fl)
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
	node->compressed = compressed;

	node->data_offset = -1;	
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

	free(node);
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

 
// Recursively read the data of a directory
void read_data(const char* path, Metadata metadata, bool compressed, int dir_index)
{
	DIR* directory = opendir(path);
	if(directory == NULL)
	{
		perror("Error opening directory\n");
		exit(EXIT_FAILURE);
	}
	
	struct dirent *entries;

	// Check all the entries of the directory
	while((entries = readdir(directory)) != NULL)
	{
		if(strcmp(entries->d_name, ".") == 0 || strcmp(entries->d_name, "..") == 0) continue;
		char fpath[1024] = {0};
		sprintf(fpath, "%s/%s", path, entries->d_name);
		
		struct stat info;
		safe_sys(lstat(fpath, &info));

		char* fdata = NULL;
		long int fsize = 0;
		char fname[259] = {0};
		strcpy(fname, entries->d_name);
		
		// for symlinks
		if((S_ISLNK(info.st_mode)))
		{
			fdata = safe_malloc(sizeof(*fdata) * (info.st_size + 1));
			fsize = info.st_size + 1;
			readlink(fpath, fdata, sizeof(*fdata)* info.st_size);
			fdata[info.st_size] = '\0';
		}		

		// for regular files
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

		// insert it at the parent directory
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


// Check if its compressed
static bool is_compressed(char* path)
{
	int fd = open(path, O_RDONLY);
	unsigned char buff[2] = {0};
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
static void compress(char* path)
{
	int pid = fork();
	if(!pid) 
	{
		char* argv[] = {"gzip", path, NULL};
		safe_sys(execvp("gzip", argv));
	}
	safe_sys(wait(NULL));
}

static void decompress(char* path)
{
	int pid = fork();
	if(!pid)
	{
		char* argv[] = {"gzip", "-d", path, NULL};
		safe_sys(execvp("gzip", argv));
	}
	wait(NULL);

}

// Compresses the file, reads it and decompresses it
static char* compress_and_read(const char* path, long int* fsize)
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

// Write recursively the files and directories
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
		
		// If its a directory , create it and then call the function for its entries
		if(S_ISDIR(tnode->info->mode))
		{
			mkdir(tpath, getPermissions(tnode));
			write_rec(metadata, tnode, tpath, visited);
		}
		else if(S_ISREG(tnode->info->mode))
		{
			// If its a regular file just create it
			int fd;
			safe_sys_assign(fd, open(tpath, O_CREAT | O_TRUNC | O_WRONLY, getPermissions(tnode)));
			if(tnode->file_data != NULL){						
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

		// Same as write_rec , except if you encounter a directory call write_rec
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

	// Write links at the end so all possible references exist
	write_links(metadata);
}

// Same logic as write_rec except it only writes symbolic links
static void write_l(Metadata metadata, MyzNode node, char* path, bool* visited)
{
	if(node->entries == NULL)
		return;

	for(int i = 0; i < vector_size(node->entries); i++)
	{
		char tpath[1300] = {0};
		Entry entry = vector_get_at(node->entries, i);
		if(visited[entry->myznode_index]) continue;
		visited[entry->myznode_index] = true;
		MyzNode tnode = vector_get_at(metadata->nodes, entry->myznode_index);
		if(S_ISDIR(tnode->info->mode))
		{
			strcat(tpath, path);
			strcat(tpath, "/");
			strcat(tpath, tnode->name);
			write_l(metadata, tnode, tpath, visited);
		}
		else if(S_ISLNK(tnode->info->mode))
		{
			strcat(tpath, path);
			strcat(tpath, "/");
			strcat(tpath, tnode->name);
			assert(tnode->file_data != NULL);
			symlink(tnode->file_data, tpath);
		}
	}
}

// Same logic as write_Data except it only writes symlinks
static void write_links(Metadata metadata)
{
	bool* visited = calloc(vector_size(metadata->nodes), sizeof(*visited));
	write_l(metadata, vector_get_at(metadata->nodes, 0), ".", visited);
	free(visited);
}

// Find the nearest node to the filepath given
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
			if(compare_names(node, token)){		// if names are equal return that this node exists
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


// Same as metadata_find_node except it returns the node before the last file in the filepath
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

// Same as write_rec except it only prints the node_names
void print_rec(Metadata metadata, MyzNode node, int rec, bool print_metadata)
{
	if(node->entries == NULL)
		return;

	for(int i = 0; i < vector_size(node->entries); i++)
	{
		Entry entry = vector_get_at(node->entries, i);
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
	MyzNode node = vector_get_at(metadata->nodes, 0);
	if(node->entries == NULL)
		return;

	print_rec(metadata, node, 0, print_metadata);
}
