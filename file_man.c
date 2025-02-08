#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>


// Function for reading a directory's subdirectories and files recursively
int readDirectory(char* dirname)
{
	DIR* directory;
	struct dirent *entries;
	int files = 0;

	directory = opendir(dirname);
	if(!directory)
	{
		perror("Error opening directory\n");
		return 1;
	}

	while((entries=readdir(directory)))
	{
		files++;
		printf("File %d with type %d: %s\n", files, entries->d_type, entries->d_name);
		if(entries->d_type == 4 && strcmp(entries->d_name, "..") && strcmp(entries->d_name, "."))
		{
			char dirpath[1000];
			memset(dirpath, 0, 1000);
			strcpy(dirpath, dirname);
			strcat(dirpath, "/");
			strcat(dirpath, entries->d_name);

			readDirectory(dirpath);

		}
	}

	closedir(directory);
	
	return 0;
}


int main()
{
	readDirectory("./something");
	return 0;
}



