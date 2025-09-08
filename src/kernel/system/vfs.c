#include "vfs.h"
#include "../shared/kstdlib.h"

vfile_t root_dir = {"/", VFILE_DIRECTORY};

void vfs_init(){
    // mlog()
    root_dir.access.data.ptr = kmalloc(1);
    root_dir.access.data.size_pgs = 1;
}

void fcreate();
void fdelete();
uint32_t fwrite();
uint32_t fread();
void fopen();