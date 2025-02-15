#include "myz.h"
#include "header.h"
#include "metadata.h"
#include "sys_utils.h"
#include <fcntl.h>
#include <assert.h>

static void write_stat(int fd, MyzNode node);

// void read_metadata(Myz myz, int fd)
// {
// 	lseek(fd, myz->header->metadata_offset, SEEK_SET);

// 	Metadata metadata = safe_malloc(sizeof(*metadata));
//     metadata->nodes = vector_create(0, NULL);   // TODO: destroy function
// 	MyzNode node = NULL;
// 	while((node = metadata_read_node(fd)) != NULL)
// 	{
// 		vector_insert_last(metadata->nodes, node);
// 	}

// 	myz->metadata = metadata;
// }

// Myz read_from_file(const char* path)
// {
//     int fd;
//     safe_sys_assign(fd, open(path, O_RDONLY));
//     Myz myz = safe_malloc(sizeof(*myz));

//     myz->header = header_get(fd);

//     read_metadata(myz, fd);
    
//     for(int i = 0; i < vector_size(myz->metadata->nodes); i++)
//     {
//         MyzNode node = vector_get_at(myz->metadata->nodes, i);
        
//         if(S_ISREG(node->info->mode))
//         {
//             safe_sys(lseek(fd, node->data_offset, SEEK_SET));
//             node->file_data = safe_malloc(node->file_size * sizeof(*node->file_data));
//             guaranteed_read(fd, node->file_data, node->file_size * sizeof(*node->file_data));
//         }
//     }


//     return myz;
// }

static void write_name(int fd, const char* name)
{
    size_t name_length = strlen(name) + 1;    // +1 for \0
    safe_sys(write(fd, &name_length, sizeof(name_length)));
    safe_sys(write(fd, name, name_length));
}


static void write_entries(Vector entries, int fd)
{
    if(entries == NULL)
    {
        int size = 0;
        safe_sys(write(fd, &size, sizeof(size)));
        return;
    }
    int size = vector_size(entries);
    safe_sys(write(fd, &size, sizeof(size)));
    for(int i = 0 ; i < size ; i++)
    {
        Entry entry = vector_get_at(entries, i);
        write_name(fd, entry->name);
        safe_sys(write(fd, &entry->myznode_index, sizeof(entry->myznode_index)));
    }
}

static void write_metadata_node(MyzNode node, int fd)
{
    write_name(fd, node->name);
    write_stat(fd, node);
    safe_sys(write(fd, &node->compressed, sizeof(node->compressed)));
    if(S_ISREG(node->info->mode))
    {
        safe_sys(write(fd, &node->data_offset, sizeof(node->data_offset)));
        safe_sys(write(fd, &node->file_size, sizeof(node->file_size)));
    }
    if(S_ISDIR(node->info->mode))
    {
        write_entries(node->entries, fd);
    }
}


void create_myz_file(Myz myz, const char* file_name)
{
    int fd;
    safe_sys_assign(fd, open(file_name, O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR));


    header_write(myz->header, fd);
    safe_sys(lseek(fd, header_get_data_offset(), SEEK_SET));        // for safety, remove

    Metadata metadata = myz->metadata;
    
    Vector nodes = metadata->nodes;
    int size = vector_size(nodes);

    // write data
    for(int i = 0 ; i < size ; i++)
    {
        MyzNode node = vector_get_at(nodes, i);
        //assert((node->file_data != NULL && S_ISREG(node->info->mode)) || (node->file_data == NULL && !S_ISREG(node->info->mode)));
        // printf("writing %s, data: %s\n", node->name, node->file_data);
        
        if(node->file_data != NULL)
        {
            safe_sys_assign(node->data_offset, lseek(fd, 0, SEEK_CUR));
            safe_sys(write(fd, node->file_data, node->file_size));
        }
    }


    off_t metadata_offset;
    safe_sys_assign(metadata_offset, lseek(fd, 0, SEEK_CUR));

    // write metadata
    safe_sys(write(fd, &size, sizeof(size)));
    for(int i = 0 ; i < size ; i++)
    {
        MyzNode node = vector_get_at(nodes, i);
        write_metadata_node(node, fd);
    }

    myz->header->metadata_offset = metadata_offset;
    safe_sys_assign(myz->header->file_size, lseek(fd, 0, SEEK_END));
    header_write(myz->header, fd);

    safe_sys(close(fd));
}

static void read_stat(int fd, MyzNode node)
{
    guaranteed_read(fd, &node->info->mode, sizeof(node->info->mode));
    guaranteed_read(fd, &node->info->uid, sizeof(node->info->uid));
    guaranteed_read(fd, &node->info->gid, sizeof(node->info->gid));
}

static void write_stat(int fd, MyzNode node)
{
    safe_sys(write(fd, &node->info->mode, sizeof(node->info->mode)));
    safe_sys(write(fd, &node->info->uid, sizeof(node->info->uid)));
    safe_sys(write(fd, &node->info->gid, sizeof(node->info->gid)));
}

static Vector read_entries(int fd)
{
    int size;
    guaranteed_read(fd, &size, sizeof(size));
    if(size == 0) return NULL;
    Vector entries = vector_create(size, free);

    for(int i = 0; i < size; i++)
    {
        Entry entry = safe_malloc(sizeof(*entry));
        size_t name_length;
        guaranteed_read(fd, &name_length, sizeof(name_length));
        guaranteed_read(fd, entry->name, sizeof(*entry->name) * name_length);

        guaranteed_read(fd, &entry->myznode_index, sizeof(entry->myznode_index));
        vector_insert_last(entries, entry);
    }
    return entries;
}

static MyzNode read_node(int fd)
{
    MyzNode node = safe_malloc(sizeof(*node));
    node->info = safe_malloc(sizeof(*node->info));
    size_t name_length;
    guaranteed_read(fd, &name_length, sizeof(name_length));
    guaranteed_read(fd, node->name, name_length);
    read_stat(fd, node);
    guaranteed_read(fd, &node->compressed, sizeof(node->compressed));
    if(S_ISREG(node->info->mode))
    {
        guaranteed_read(fd, &node->data_offset, sizeof(node->data_offset));
        guaranteed_read(fd, &node->file_size, sizeof(node->file_size));
    }
    if(S_ISDIR(node->info->mode))
    {
        node->entries = read_entries(fd);
    }

    return node;
}


static Header read_header(int fd)
{
    Header header = safe_malloc(sizeof(*header));

    safe_sys(lseek(fd, 0, SEEK_SET));

    guaranteed_read(fd, &header->magic_number, MAGIC_NUMBER_SIZE);
    if(strcmp(header->magic_number, MAGIC_NUMBER) != 0)
    {
        fprintf(stderr, "Invalid Magic Number, make sure its a .myz file\n");
        exit(EXIT_FAILURE);
    }
    
    guaranteed_read(fd, &header->metadata_offset, sizeof(header->metadata_offset));

    guaranteed_read(fd, &header->file_size, sizeof(header->file_size));

    return header;
}

Myz read_myz_file(char* name)
{
    Myz myz = safe_malloc(sizeof(*myz));
    int fd;
    safe_sys_assign(fd, open(name, O_RDONLY));
    
    myz->header = read_header(fd);

    safe_sys(lseek(fd, myz->header->metadata_offset, SEEK_SET));

    int metadata_entries;
    myz->metadata = safe_malloc(sizeof(*myz->metadata));
    guaranteed_read(fd, &metadata_entries, sizeof(metadata_entries));
    myz->metadata->nodes = vector_create(metadata_entries, myznode_destroy);
    
    for(int i = 0; i < metadata_entries; i++)
    {
       MyzNode node = read_node(fd);
       vector_insert_last(myz->metadata->nodes, node);
    }

    for(int i = 0; i < metadata_entries; i++)
    {
        MyzNode node = vector_get_at(myz->metadata->nodes, i);
        if(S_ISREG(node->info->mode))
        {
            if(node->data_offset == -1)
            {
                node->file_data = NULL;
                continue;
            }

            safe_sys(lseek(fd, node->data_offset, SEEK_SET));
            node->file_data = safe_malloc(sizeof(*node->file_data) * node->file_size);
            guaranteed_read(fd, node->file_data, node->file_size * sizeof(*node->file_data));
            // printf("read %s, data: %s\n", node->name, node->file_data);
        }
    }
    
    safe_sys(close(fd));
    return myz;
}

static MyzNode searchNode(Metadata metadata, char* name)
{
    for(int i = 0; i < vector_size(metadata->nodes); i++)
    {
        MyzNode node = vector_get_at(metadata->nodes, i);
        if(strcmp(node->name, name) == 0)
            return node;
    }

    return NULL;
}

static MyzNode searchEntries(Metadata metadata, MyzNode node,  char* name)
{
    if(node->entries != NULL)
    {
        for(int i = 0; i < vector_size(node->entries); i++)
        {
            Entry entry = vector_get_at(node->entries, i);
            MyzNode node = vector_get_at(metadata->nodes, entry->myznode_index);

            if(strcmp(node->name, name) == 0)
            {
                return node;
            }
            else if(node->compressed)
            {
                char temp[100];
                memset(temp, 0, 100);
                strncpy(temp, node->name, strlen(node->name) - 3);
                if(strcmp(temp, name) == 0)
                    return node;
            }
        }
    }

    return NULL;
}

MyzNode findPath(char* path, Metadata metadata, bool* file_exists, bool* exists)
{   
    MyzNode prev = NULL;
    *exists = true;
    *file_exists = false;
    bool fl = false;
    char* tpath = strdup(path);
    char* token = strtok(tpath, "/");
    struct stat info;

    char curpath[1000];
    memset(curpath, 0, 1000);
    strcat(curpath, token);

    MyzNode node = searchNode(metadata, token);
    if(node == NULL)
    {
        free(tpath);
        *exists = false;
        return NULL;
    }
    
    while((token = strtok(NULL, "/")) != NULL)
    {
        printf("TOKEN %s\n", token);
        fl = true;
        MyzNode tnode ;
        if(!fl)
        {
            printf("HERE2\n");
            fl = true;
            tnode = searchEntries(metadata, node, token);
        }
        else
        {
            printf("Here3\n");
            tnode = searchEntries(metadata, prev, token);
        }
        if(tnode == NULL && prev == NULL) 
        {
            free(tpath);
            return node;
        }
        else if(tnode == NULL)
        {
            free(tpath);
            return prev;
        }
        strcat(curpath, "/");
        strcat(curpath, token);
        printf("CURPATH: %s\n", curpath);
        if(strcmp(curpath, path) == 0)
        {
            free(tpath);
            *file_exists = true;
            return NULL;
        }
       
        // Just remove the last / from the path
        strcat(curpath, "/");
        if(strcmp(curpath, path) == 0)
        {
            free(tpath);
            *file_exists = true;
            return NULL;
        }



        lstat(curpath, &info);
        if(tnode->info->mode != info.st_mode) // Conflicting file types
        {
            free(tpath);
            *file_exists = true;
            return NULL;
        }

        prev = tnode;
    }
    if(!fl && node != NULL)
    {
        if(strcmp(curpath, path) == 0)
        {
            *file_exists = true;
            free(tpath);
            return NULL;
        }
        strcat(curpath, "/");
        if(strcmp(curpath, path) == 0)
        {
            *file_exists = true;
            free(tpath);
            return NULL;
        }
        free(tpath);
        return node;
    }

    *exists = false;
    free(tpath);
    return NULL;
}


// void append(Metadata metadata)
// {
//     struct stat info;
//     lstat();
// }

// // TODO read Data from files
// Myz myz_create(const char* file_name, const char* files[], int file_count, bool compress){
//     Myz myz = safe_malloc(sizeof(*myz));
//     myz->header = safe_malloc(sizeof(*myz->header));
//     myz->metadata = metadata_create();

//     // read_Data(myz->metadata, )   // εχει θεματα...

//     create_myz_file(myz, file_name);

//     return myz;
// }


// void myz_extract(const char* myz_name, const char* files[], int file_count){
//     Myz myz = read_myz_file(myz_name);
//     Metadata metadata = myz->metadata;

//     for(int i = 0 ; i < file_count ; i++){
//         bool file_exists = false;
        
//     }
// }


bool compare_names(MyzNode node, char* name)
{
    char tname1[256] = {0};
    if(node->compressed && !S_ISDIR(node->info->mode))
        strncpy(tname1, node->name, strlen(node->name) - 3);
    else
        strcpy(tname1, node->name);

    char tname2[256] = {0};

    strcpy(tname2, name);
    
    if(strcmp(tname1, tname2) == 0) return true;
    else return false;
}


void write_after_append(Myz myz, int old_entries, char* filename)
{
    Metadata metadata = myz->metadata;
    int fd = open(filename, O_WRONLY);
    lseek(fd, myz->header->metadata_offset, SEEK_SET);


    for(int i = old_entries; i < vector_size(metadata->nodes); i++)
    {
        MyzNode node = vector_get_at(metadata->nodes, i);
        if(S_ISREG(node->info->mode))
        {
            if(node->file_data != NULL)
            {
                safe_sys_assign(node->data_offset, lseek(fd, 0, SEEK_CUR));
                safe_sys(write(fd, node->file_data, sizeof(*node->file_data) * node->file_size));
            }
        }
    }

    myz->header->metadata_offset = lseek(fd, 0, SEEK_CUR);
    int size = vector_size(metadata->nodes);
    safe_sys(write(fd, &size, sizeof(size)));
    for(int i = 0; i < vector_size(metadata->nodes); i++)
    {
        MyzNode node = vector_get_at(metadata->nodes, i);
        write_metadata_node(node, fd);
    }

    safe_sys_assign(myz->header->file_size, lseek(fd, 0, SEEK_CUR));
    header_write(myz->header, fd);
    close(fd);
}



 bool append(Myz myz, char* path, bool compressed)
{
    Metadata metadata = myz->metadata;
    char* tpath = strdup(path);
    bool exists;

    MyzNode node = metadata_find_node(metadata, path, &exists);
    printf("NODE: %s\n", node->name);
    if(exists)
    {
        free(tpath);
        return false;
    }
    else if(strcmp(node->name, ".") == 0)
    {
        char* token;
        char curpath[1000];
        memset(curpath, 0, 1000);
        MyzNode prev = vector_get_at(metadata->nodes, 0);
        token = strtok(tpath, "/");
        while(token  != NULL)
        {
            struct stat info;
            strcat(curpath, token);
            lstat(curpath, &info);
            entries_insert(prev, token,  vector_size(metadata->nodes));
            if(S_ISREG(info.st_mode))
            {
                long int file_size;
                char* file_data = read_file(curpath, &file_size);
                metadata_insert(metadata, token, info, compressed, file_size, file_data);
                free(tpath);
                return true;
            }

            metadata_insert(metadata, token, info, compressed, 0, NULL);
            strcat(curpath, "/");
            token = strtok(NULL, "/");
        }

        read_data(path, metadata, compressed, vector_size(metadata->nodes) -1 );
        free(tpath);
        return true;
    }

    char curpath[1000] = {0};
    char* token = strtok(tpath, "/");
    while(token != NULL)
    {
        strcat(curpath, token);
        if(strcmp(node->name, token) == 0)
        {
            printf("BREAK\n");
            break;
        }
        strcat(curpath, "/");
        token = strtok(NULL, "/");
    }

    strcat(curpath, "/");
    while((token = strtok(NULL, "/")))
    {
        struct stat info;
        strcat(curpath, token);
        lstat(curpath, &info);
        entries_insert(node, token, vector_size(metadata->nodes));
        if(S_ISREG(info.st_mode))
        {
            long fsize;
            char* file_data = read_file(curpath, &fsize);
            metadata_insert(metadata, token, info, compressed, fsize, file_data);
            free(tpath);
            return true;
        }

        metadata_insert(metadata, token, info, compressed, 0, NULL);
        strcat(curpath, "/");
        node = vector_get_at(metadata->nodes, vector_size(metadata->nodes) - 1);
    }

    read_data(path, metadata, compressed, vector_size(metadata->nodes) - 1);
    free(tpath);
    return true;
    
}


void myz_query_for_existence(const Myz myz, int file_count, const char* files[]){
    for(int i = 0 ; i < file_count ; i++){
        bool exists;
        metadata_find_node(myz->metadata, files[i], &exists);
        if(exists){
            printf("%s %s\n", files[i], exists ? "exists" : "does not exist");
        }
    }
}


// void myz_append(const char* file_name, int file_count, const char* files[], bool compress){
//     // TODO ελεγχος πρωτα αν υπαρχει το myz, αν ναι το κανεις read, αν οχι το κανεις create

//     for(int i = 0 ; i < file_count ; i++){
//         append(myz, files[i], compress);
//     }
// }


void myz_extract(Myz myz){
    write_Data(myz->metadata);
}