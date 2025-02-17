#include "myz.h"
#include "args.h"
#include <stdio.h>
#include <stdlib.h>


int main(int argc, char** argv){
    Arguments args = get_arguments(argc, argv);
    switch(args.operation){
        case CREATE:
            if(args.num_files < 1){
                fprintf(stderr, "No files to archive\n");
                exit(EXIT_FAILURE);
            }
            myz_create(args.archive_file, args.num_files, (const char**)args.files, args.use_compression);
            break;
        case APPEND:
            if(args.num_files < 1){
                fprintf(stderr, "No files to archive\n");
                exit(EXIT_FAILURE);
            }
            myz_append(args.archive_file, args.num_files, (const char**)args.files, args.use_compression);
            break;
        case EXTRACT:{      // create an inner scope to avoid redefinition of variable myz
            Myz myz = read_myz_file(args.archive_file);
            myz_extract(myz);
            myz_destroy(myz);
            break;
        }
        case DELETE:{
            Myz myz = read_myz_file(args.archive_file);
            myz_delete(myz, args.archive_file, args.num_files, args.files);
            myz_destroy(myz);
            break;
        }
        case PRINT_METADATA:{
            Myz myz = read_myz_file(args.archive_file);
            print(myz->metadata, true);
            myz_destroy(myz);
            break;
        }
        case QUERY:{
            Myz myz = read_myz_file(args.archive_file);
            myz_query_for_existence(myz, args.num_files, args.files);
            myz_destroy(myz);
            break;
        }
        case PRINT_HIERARCHY:{
            Myz myz = read_myz_file(args.archive_file);
            print(myz->metadata, false);
            myz_destroy(myz);
            break;
        }
        default:
            fprintf(stderr, "Wtf, should have exited already\n");       // TODO ......
            exit(EXIT_FAILURE);
    }
}