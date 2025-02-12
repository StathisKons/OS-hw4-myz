#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include "myznode.h"
#include <fcntl.h>
#include <wait.h>

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
int readDirectory(char* dirname, Myzdata data, Myznode node, char compress)
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

	while((entries=readdir(directory)) != NULL)
	{		
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

		myznode_insert(data, entries->d_name, fs, nested, compress);
		if(node != NULL)
		{
			myznode_addEntry(node, data->curelements -1);
		}

		if(S_ISREG(fs.st_mode))
		{
			long int fsize;
			char tname[1000];
			memset(tname, 0, 1000);
			strcpy(tname, fname);
			
			if(compress)
			{
				int pid = fork();
				if(!pid)
				{
					char* argv[] = {"gzip", fname, NULL};
					execvp("gzip", argv);
				}
				wait(NULL);
				strcat(tname, ".gz");
			}
			
			char* buff = readData(tname, &fsize);

			if(compress)
			{
				int pid = fork();
				if(!pid)
				{
					char* argv[] = {"gzip", "-d", tname, NULL};
					execvp("gzip", argv);
				}
				wait(NULL);
			}

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
		

			readDirectory(dirpath, data, data->array[data->curelements -1], compress);

		}
	}

	closedir(directory);
	
	return 0;
}

void compressAndInsert(char* dirname, Myzdata data)
{

	readDirectory(dirname, data, NULL, 1);
}



int main()
{
	int entries = 0;
	countFiles("something3", &entries);
	Myzdata data = myz_init(entries);
	compressAndInsert("something3", data);
	myz_print(data);


	writeData(data);
	Myz_destroy(data);


	return 0;
}