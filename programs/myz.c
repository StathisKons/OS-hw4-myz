#include "metadata.h"
#include "myz.h"
#include "sys_utils.h"
#include "args.h"
#include <unistd.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>


// read .myz
int main(/*int argc, char** argv*/){
    Myz myz = read_myz_file("temp.myz");
    int old_entries = vector_size(myz->metadata->nodes);
    bool exists;
    printf("%d old entries\n", old_entries);
    bool file_exists;
    MyzNode node = findPath("temp/dir1/dir2", myz->metadata, &file_exists, &exists);
    if(node != NULL && exists == true && file_exists == false)
    {
        printf("EXISTS: %d\tFILE_EXISTS: %d\tNAME: %s\n", exists, file_exists, node->name);
    }
    else if(node == NULL && exists == true)
    {
        printf("Conflicting types, entry already exists as a file or directory\n");
    }
    else if(file_exists == true)
    {
        printf("File or directory exists\n");
    }
    
    append(myz, "temp/dir1/dir2/", false);
    print_data(myz->metadata);

    //write_after_append(myz, old_entries, "temp.myz");
    printf("NEW SIZE: %d\n", vector_size(myz->metadata->nodes));
    return 0;
}


// //write .myz
// int main(/*int argc, char** argv*/){
//     // // Myz myz = read_from_file("temp.myz");
//     Metadata metadata = metadata_create();

//     read_Data(metadata, "temp", true);
//     print_data(metadata);
//     write_Data(metadata);   // extract

//     // temp
//     Myz myz = safe_malloc(sizeof(*myz));
//     myz->metadata = metadata;
//     myz->header = safe_malloc(sizeof(*myz->header));

//     create_myz_file(myz, "temp.myz");

//     return 0;
// }

// int main(int argc, char** argv){
//     Arguments args = get_arguments(argc, argv);

//     switch(args.operation){
//         case CREATE:
//             myz_create(args.archive_file, args.files, args.num_files, args.use_compression); // unfinished
//             break;
//         case APPEND:

//             break;
//         case EXTRACT:

//             break;
//         case DELETE:
        
//             break;
//         case PRINT_METADATA:
        
//             break;
//         case QUERY:
        
//             break;
//         case PRINT_HIERARCHY:
        
//             break;
//         default:
//             fprintf(stderr, "Wtf\n");
//             exit(EXIT_FAILURE);
//     }

//     return 0;
// }