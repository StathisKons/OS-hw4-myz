#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include "myznode.h"


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
		nested = 0;
		files++;
		int r = stat(entries->d_name, &fs);
		printf("%s\n", entries->d_name);
		if(entries->d_type == 4)
			nested = 1;

		myznode_insert(data, entries->d_name, fs, nested);
		if(node != NULL)
		{
			myznode_addEntry(node, data->curelements -1);
		}
		
		if(entries->d_type == 4 && strcmp(entries->d_name, "..") && strcmp(entries->d_name, "."))
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
	printf("PRINTING DATA: \n\n");
	myz_print(data);

	Myz_destroy(data);
	return 0;
}



