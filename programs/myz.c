#include "metadata.h"


int main(/*int argc, char** argv*/){
    Metadata metadata = metadata_create();

    read_Data(metadata, "dir/dir2/dir3", false);
    print_data(metadata);

    return 0;
}