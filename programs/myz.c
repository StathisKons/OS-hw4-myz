#include "metadata.h"
#include "myz.h"
#include "sys_utils.h"
#include "args.h"
#include <unistd.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>


// read .myz
int main(/*int argc, char** argv*/){
    Myz myz = read_myz_file("temp.myz");
    int old_entries = vector_size(myz->metadata->nodes);
    bool exists;
    printf("%d old entries\n", old_entries);
    metadata_find_node(myz->metadata, "temp/dir2/file4", &exists);
    append(myz, "temp/dir3", false);
    print_data(myz->metadata);

    //write_after_append(myz, old_entries, "temp.myz");
    //printf("NEW SIZE: %d\n", vector_size(myz->metadata->nodes));
    return 0;
}


// //write .myz
// int main(/*int argc, char** argv*/){
//     // // Myz myz = read_from_file("temp.myz");
//     Metadata metadata = metadata_create();

//     read_Data(metadata, "temp", true);
//     print_data(metadata);

//     // temp
//     Myz myz = safe_malloc(sizeof(*myz));
//     myz->metadata = metadata;
//     myz->header = safe_malloc(sizeof(*myz->header));

//     create_myz_file(myz, "temp.myz");

//     return 0;
// }


// int main(int argc, char** argv){
//     Arguments args = get_arguments(argc, argv);
//     switch(args->operation){
//         case CREATE:

//             break;
//         case APPEND:
//             myz_append(args.archive_file, args.num_files, args->files, args.use_compression);
//             break;
//         case EXTRACT:
//             Myz myz = read_myz_file(args->filename);
//             myz_extract(myz);
//             break;
//         case DELETE:

//             break;
//         case PRINT_METADATA:

//             break;
//         case QUERY:
//             Myz myz = read_myz_file(args->filename);
//             myz_query_for_existence(myz, args.num_files, args->files);
//             break;
//         case PRINT_HIERARCHY:

//             break;
//         default:
//             fprintf(stderr, "Wtf, should have exited already\n");
//             exit(EXIT_FAILURE);
//     }
// }