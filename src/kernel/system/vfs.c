#include "vfs.h"
#include "../shared/kstdlib.h"
#include "../shared/memory.h"
#include "../shared/string.h"
#define MODULE_NAME "KVFS"
vfile_t root_dir = {"/", VFILE_DIRECTORY};

void vfs_init(){
    mlog(MODULE_NAME, "Initializing VFS\n", MLOG_PRINT);
    root_dir.access.data.ptr = kmalloc(1);
    root_dir.access.data.size_pgs = 1;
}

void fcreate(char *name, VFILE_TYPE type, ...){
    char *dir = strtok(name, '/');
    char *tmp = dir;
    uint32_t filename_offset = 0;
    vfile_t *current_dir = &root_dir; 
    vfile_t *tmpfile;
    while(tmp != 0){
        dir = tmp;
        tmpfile = search_dir(tmp, *current_dir);
        if(!tmpfile){
            //create file in folder;
            
        }
        switch(tmpfile->type){
            case VFILE_DIRECTORY:
                current_dir = tmpfile;
                break;
            case VFILE_MOUNT:
                return ((mount_t*)(tmpfile->access.data.ptr))->create(name + filename_offset, type == VFILE_DIRECTORY ? FS_FILE_IS_DIR : 0);
            default:
                break;
        }
        filename_offset += strlen(tmp) + 1;
        kfree(tmp);
        tmp = strtok(0, '/');
    }
    return 0;
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

vfile_t *search_dir(char *name, vfile_t dir){
    vfile_t *dir_data = (vfile_t *)dir.access.data.ptr;
    uint32_t i = 0;
    while(dir_data[i].name[0]){
        if(!strcmp(dir_data[i].name, name)) return &dir_data[i];
        i++;
    }
    mlog(MODULE_NAME, "File not found\n", MLOG_ERR);
    return 0;
}

file_t vfile_to_file(vfile_t *file){
    file_t out = {-1, &file};
}

int fopen(char *name, file_t *file){
    char *dir = strtok(name, '/');
    char *tmp = dir;
    uint32_t filename_offset = 0;
    vfile_t *current_dir = &root_dir; 
    vfile_t *tmpfile;
    while(tmp != 0){
        dir = tmp;
        tmpfile = search_dir(tmp, *current_dir);
        if(!tmpfile){
            kfree(tmp);
            return -1;
        }
        switch(tmpfile->type){
            case VFILE_DIRECTORY:
                current_dir = tmpfile;
                break;
            case VFILE_MOUNT:
                return ((mount_t*)(tmpfile->access.data.ptr))->open(name + filename_offset, file);
            default:
                *file = vfile_to_file(tmpfile);
                kfree(tmp);
                return 0;
        }
        filename_offset += strlen(tmp) + 1;
        kfree(tmp);
        tmp = strtok(0, '/');
    }
    *file = vfile_to_file(tmpfile);
    kfree(tmp);
    return 0;
}