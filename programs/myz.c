#include "metadata.h"
#include "myz.h"
#include "sys_utils.h"
#include <unistd.h>

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


int main(/*int argc, char** argv*/){
    // // Myz myz = read_from_file("temp.myz");
    // Metadata metadata = metadata_create();

    // read_Data(metadata, "temp", false);
    // print_data(metadata);
    // write_Data(metadata);   // extract

    // // temp
    // Myz myz = safe_malloc(sizeof(*myz));
    // myz->metadata = metadata;
    // myz->header = safe_malloc(sizeof(*myz->header));

    // write_to_file_create(myz, "temp.myz");


    Myz myz = read_from_file("temp.myz");
    print_data(myz->metadata);
    write_Data(myz->metadata);
    return 0;
}

// Operation get_arguments(int argc, char** argv, bool* use_compression){
//     *use_compression = false;
//     Operation op = NONE;
//     int opt;
//     while((opt = getopt(argc, argc, "cax")))
// }