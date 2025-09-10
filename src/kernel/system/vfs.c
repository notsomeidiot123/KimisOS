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

vfile_t **find_free_dirent(vfile_t *current_dir){
    
}

void fcreate(char *name, VFILE_TYPE type, ...){
    char *last = strtok(name + (name[0] == '/'), '/');
    char *tmp = last;
    uint32_t filename_offset = 0;
    vfile_t *current_dir = &root_dir; 
    vfile_t *tmpfile;
    
    va_list args;
    va_start(args, type);
    while(tmp != 0){
        // dir = tmp;
        tmpfile = search_dir(tmp, *current_dir);
        if(!tmpfile){
            //create file in folder;
            vfile_t *vfile = kmalloc(1);
            vfile->type = type;
            
            switch(type){
                case VFILE_POINTER:
                case VFILE_DIRECTORY:
                    void *ptr = va_arg(args, void*);
                    vfile->access.data.ptr = ptr;
                    vfile->access.data.size_pgs = va_arg(args, uint32_t);
                    strcpy(tmp, vfile->name);
                    vfile_t **dir_data = current_dir->access.data.ptr;
                    int i = 0;
                    while(dir_data[i] && (i * sizeof(vfile_t *))/4096 < current_dir->access.data.size_pgs){
                        i++;
                    }
                    dir_data[i] = vfile;
                    // printf("Created File: %s\n", vfile->name);
                    return;
            }
            return;
        }
        switch(tmpfile->type){
            case VFILE_DIRECTORY:
                current_dir = tmpfile;
                break;
            case VFILE_MOUNT:
                ((mount_t*)(tmpfile->access.data.ptr))->create(name + filename_offset, type == VFILE_DIRECTORY ? FS_FILE_IS_DIR : 0);
                return;
            default:
                mlog(MODULE_NAME, "Error: Cannot create file with same name as another file! %d\n", MLOG_ERR, tmpfile->type);
                return;
                break;
        }
        filename_offset += strlen(tmp) + 1;
        tmp = strtok(0, '/');
    }
    return;
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
            file_entry->access.funcs.write(file_entry, byte_array, offset, count);
    }
}

uint32_t fread();

vfile_t *search_dir(char *name, vfile_t dir){
    vfile_t **dir_data = (vfile_t **)dir.access.data.ptr;
    uint32_t i = 0;
    while(dir_data[i]){
        if(!strcmp(dir_data[i]->name, name)){
            return dir_data[i];
        }
        i++;
    }
    return 0;
}

int fopen(char *name, vfile_t *file){
    char *dir = strtok(name, '/');
    char *tmp = dir;
    uint32_t filename_offset = 0;
    vfile_t *current_dir = &root_dir; 
    vfile_t *tmpfile;
    while(tmp != 0){
        dir = tmp;
        tmpfile = search_dir(tmp, *current_dir);
        if(!tmpfile){
            return -1;
        }
        switch(tmpfile->type){
            case VFILE_DIRECTORY:
                current_dir = tmpfile;
                break;
            case VFILE_MOUNT:
                return ((mount_t*)(tmpfile->access.data.ptr))->open(name + filename_offset, file);
            default:
                *file = *tmpfile;
                return 0;
        }
        filename_offset += strlen(tmp) + 1;
        tmp = strtok(0, '/');
    }
    *file = *tmpfile;
    return 0;
}