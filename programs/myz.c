// #include "metadata.h"
// #include "myz.h"
// #include "sys_utils.h"
#include <unistd.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

typedef enum {
    NONE,
    CREATE,
    APPEND,
    EXTRACT,
    DELETE,
    PRINT_METADATA,
    QUERY,
    PRINT_HIERARCHY
} Operation;

typedef struct {
    Operation operation;
    bool use_compression;
    char* archive_file;
    char** files;
    int num_files;
} Arguments;


static Arguments get_arguments(int argc, char** argv);

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

    return 0;
}

static void print_usage(char* program_name){
    fprintf(stderr, "Usage: %s {-c|-a|-x|-d|-m|-q|-p} [-j] <archive-file> [list-of-files/dirs...]\n", program_name);
}

static void multiple_operations_error(char* program_name){
    fprintf(stderr, "Error: Only one operation can be specified\n");
    print_usage(program_name);
    exit(EXIT_FAILURE);
}

static Arguments get_arguments(int argc, char** argv){
    Arguments args = {.operation = NONE,
                      .use_compression = false,
                      .archive_file = NULL,
                      .files = NULL,
                      .num_files = 0};

    int opt;
    while((opt = getopt(argc, argv, "caxdmqpj")) != -1)
    {
        switch(opt){
            case 'c':
                if(args.operation != NONE){
                    multiple_operations_error(argv[0]);
                }
                args.operation = CREATE;
                break;
            case 'a':
                if(args.operation != NONE){
                    multiple_operations_error(argv[0]);
                }
                args.operation = APPEND;
                break;
            case 'x':
                if(args.operation != NONE){
                    multiple_operations_error(argv[0]);
                }
                args.operation = EXTRACT;
                break;
            case 'd':
                if(args.operation != NONE){
                    multiple_operations_error(argv[0]);
                }
                args.operation = DELETE;
                break;
            case 'm':
                if(args.operation != NONE){
                    multiple_operations_error(argv[0]);
                }
                args.operation = PRINT_METADATA;
                break;
            case 'q':
                if(args.operation != NONE){
                    multiple_operations_error(argv[0]);
                }
                args.operation = QUERY;
                break;
            case 'p':
                if(args.operation != NONE){
                    multiple_operations_error(argv[0]);
                }
                args.operation = PRINT_HIERARCHY;
                break;
            case 'j':
                args.use_compression = true;
                break;
            default:
                print_usage(argv[0]);
                exit(EXIT_FAILURE);
                break;
        }
    }

    if(args.operation == NONE){
        print_usage(argv[0]);
        exit(EXIT_FAILURE);
    }

    if(optind >= argc){
        fprintf(stderr, "Error: No archive file specified\n");
        exit(EXIT_FAILURE);
    }

    args.archive_file = argv[optind++];
    args.num_files = argc - optind;
    args.files = &argv[optind];         // like a boss

    if(args.use_compression && args.operation != CREATE && args.operation != APPEND){
        fprintf(stderr, "Error: Compression can only be used with -c or -a\n");
        exit(EXIT_FAILURE);
    }
    
    return args;
}