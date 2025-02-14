// #include "metadata.h"
// #include "myz.h"
// #include "sys_utils.h"
#include "args.h"
#include <unistd.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>


// // read .myz
// int main(/*int argc, char** argv*/){
//     Myz myz = read_myz_file("temp.myz");
//     print_data(myz->metadata);
//     write_Data(myz->metadata);
//     return 0;
// }


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

int main(int argc, char** argv){
    Arguments args = get_arguments(argc, argv);

    switch(args.operation){
        case CREATE:

            break;
        case APPEND:

            break;
        case EXTRACT:

            break;
        case DELETE:
        
            break;
        case PRINT_METADATA:
        
            break;
        case QUERY:
        
            break;
        case PRINT_HIERARCHY:
        
            break;
        default:
            fprintf(stderr, "Wtf\n");
            exit(EXIT_FAILURE);
    }

    return 0;
}
