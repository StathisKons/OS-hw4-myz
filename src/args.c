#include "args.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


static void print_usage(char* program_name){
    fprintf(stderr, "Usage: %s {-c|-a|-x|-d|-m|-q|-p} [-j] <archive-file> [list-of-files/dirs...]\n", program_name);
}

static void multiple_operations_error(char* program_name){
    fprintf(stderr, "Error: Only one operation can be specified\n");
    print_usage(program_name);
    exit(EXIT_FAILURE);
}

Arguments get_arguments(int argc, char** argv){
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