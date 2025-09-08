#include "vfs.h"
#include "../shared/kstdlib.h"
#include "../shared/memory.h"
#include "../shared/string.h"
#define MODULE_NAME "KVFS"
vfile_t root_dir = {"\0", VFILE_DIRECTORY};

void vfs_init(){
    mlog(MODULE_NAME, "Initializing VFS\n", MLOG_PRINT);
    root_dir.access.data.ptr = kmalloc(1);
    root_dir.access.data.size_pgs = 1;
}

void fcreate(char *name, VFILE_TYPE type, ...){
    printf("Sasdafdasdf\n");
    char *dir = strtok(name, '/');
    char *file = dir;
    char *tmp = dir;
    while(tmp != 0){
        dir = file;
        file = tmp;
        tmp = strtok(0, '/');
    }
}
void fdelete();

// navya was here https://github.com/novabansal
uint32_t fwrite(vfile_t *file_entry, void *byte_array, uint32_t offset, uint32_t count){
    // a: a way to call RW functions from any given module
    // OR
    // b: specify a pointer to data that is written to virtual file in memory
    
    // search through root directory
    switch(file_entry->type){
        case VFILE_DEVICE:
            file_entry->access.funcs.write(byte_array, offset, count);
    }
}
uint32_t fread();
void fopen(char *name){
    char *dir = strtok(name, '/');
    char *tmp = dir;
    vfile_t *current_dir = &root_dir; 
    while(tmp != 0){
        dir = tmp;
        uint32_t i = 0;
        vfile_t *dir = ((vfile_t *)(current_dir->access.data.ptr));
        while(dir[i].name[0]){
            if(!strcmp(dir[i].name, tmp)){
                if(dir[i].type == )
            }
        }
        
        tmp = strtok(0, '/');
    }
    return;
}