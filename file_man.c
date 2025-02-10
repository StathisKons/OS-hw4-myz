#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include "myznode.h"
#include <fcntl.h>

char* readData(char* filepath, long int* size)
{
	int fd = open(filepath, O_RDONLY);
	if(fd == -1)
	{
		printf("Opening was unsuccesful\n");
		return NULL;
	}
	long int sz =  lseek(fd, 0, SEEK_END);
	char* buffer = malloc(sizeof(char)*sz);
	
	lseek(fd, 0, SEEK_SET);

	read(fd, buffer, sz);

	*size = sz;
	close(fd);
	return buffer;
}



// Function for reading a directory's subdirectories and files recursively
static int countFiles(char* dirname, int* entriesn)
{
	DIR* directory;
	struct dirent *entries;
	int files = 0;
	struct stat fs;

	directory = opendir(dirname);
	if(!directory)
	{
		printf("HI2\n");
		perror("Error opening directory\n");
		return 1;
	}

	while((entries=readdir(directory)))
	{
		files++;
		(*entriesn)++;
		int r = stat(entries->d_name, &fs);
		if(entries->d_type == 4 && strcmp(entries->d_name, "..") && strcmp(entries->d_name, "."))
		{
			char dirpath[1000];
			memset(dirpath, 0, 1000);
			strcpy(dirpath, dirname);
			strcat(dirpath, "/");
			strcat(dirpath, entries->d_name);
		

			countFiles(dirpath, entriesn);

		}
	}

	closedir(directory);
	
	return 0;
}



// Function for reading a directory's subdirectories and files recursively
int readDirectory(char* dirname, Myzdata data, Myznode node)
{
	int nested;
	DIR* directory;
	struct dirent *entries;
	int files = 0;
	struct stat fs;

	directory = opendir(dirname);
	if(!directory)
	{
		perror("Error opening directory\n");
		return 1;
	}

	while((entries=readdir(directory)))
	{		
		if(entries == NULL) continue;
		char fname[1000];
		memset(fname, 0, 1000);
		strcpy(fname, dirname);
		strcat(fname, "/");
		strcat(fname, entries->d_name);


		nested = 0;
		files++;
		int r = lstat(fname, &fs);
		if(r == -1)
			continue;

		if(S_ISDIR(fs.st_mode))
			nested = 1;

		myznode_insert(data, entries->d_name, fs, nested);
		if(node != NULL)
		{
			myznode_addEntry(node, data->curelements -1);
		}

		if(S_ISREG(fs.st_mode))
		{
			long int fsize;
			char* buff = readData(fname, &fsize);

			Myznode nd = data->array[data->curelements - 1];
			nd->fsize = fsize;
			nd->filedata = buff;

		}
		
		if( S_ISDIR(fs.st_mode) && strcmp(entries->d_name, "..") && strcmp(entries->d_name, "."))
		{
			char dirpath[1000];
			memset(dirpath, 0, 1000);
			strcpy(dirpath, dirname);
			strcat(dirpath, "/");
			strcat(dirpath, entries->d_name);
		

			readDirectory(dirpath, data, data->array[data->curelements -1]);

		}
	}

	closedir(directory);
	
	return 0;
}




int main()
{
	int entries = 0;
	countFiles("./something", &entries);
	Myzdata data = myz_init(entries);
	readDirectory("./something", data, NULL);
	myz_print(data);



	writeData(data);
	Myz_destroy(data);





	return 0;
}



