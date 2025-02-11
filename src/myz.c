#include "myz.h"
#include <fcntl.h>

// static void 

void myz_store(Myz myz)
{
    // write header size σκουπιδια
    // write_data
    // write_metadata
    header_write(myz.header, myz.fd);
}