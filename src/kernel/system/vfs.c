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

void add_file(vfile_t *vfile, vfile_t *current_dir){
    vfile_t **dir_data = current_dir->access.data.ptr;
    int i = 0;
    while(dir_data[i] && (i * sizeof(vfile_t *))/4096 < current_dir->access.data.size_pgs){
        i++;
    }
    dir_data[i] = vfile;
}
/*
Long comment cause this one's a complicated one.
    Create a file of the specified path in name
    if the path leads to a mounted filesystem then
        use the mounted filesystem's specified create() function to create the file.
    else, create a pointer in the virtual path that points to a file entry for the new file.
    If this new file is a Directory or Pointer, specify a pointer to be used as a buffer to store data for this file, as well as the size of the buffer, in pages.
    If this new file is a Mounted filesystem, this pointer must be an array of functions as defined below:
        int fwrite(vfile_t *file, void *data, uint32_t offset, uint32_t count);
        //must write count bytes from data to file offset by offset bytes.
        int fread(vfile_t *file, void *data, uint32_t offset, uint32_t count);
        //must read count bytes from data to file offset by offset bytes.
        int fopen(char *filename, vfile_t *file);
        //must return a vfile_t struct written to the address specified in file, to represent a file named name (from the mount's filename)
        (eg. /mount/root/test is passed as /root/test)
        fcreate(char *filename, FS_FILE_FLAGS flags);
        FS_FILE_FLAGS is in the same format as FAT's file attribute byte of dirents.
        filenames are truncated, as with the previous example.
        fdelete(vfile_t *file);
        //the file specified in the file pointer must be deleted.
    else, if the file is a virtual representation of a device, of of a file (any miscellaneous file object that does not fit the other types) then
        specify a pointer to be used as the read function, as defined below:
                void read(struct virtual_file *file, void *data, uint32_t offset, uint32_t count);
        specify a pointer to be used as the write function, with the same function prototype as the function above
                void write(struct virtual_file *file, void *data, uint32_t offset, uint32_t count);
*/
void fcreate(char *name, VFILE_TYPE type, ...){
    name += name[0]== '/';
    char *last = strtok(name, '/');
    char *tmp = last;
    uint32_t filename_offset = 0;
    vfile_t *current_dir = &root_dir; 
    vfile_t *tmpfile;
    
    va_list args;
    va_start(args, type);
    while(tmp != 0){
        // dir = tmp;
        tmpfile = search_dir(tmp, *current_dir);
        // printf("%s\n", name + filename_offset);
        if(!tmpfile){
            //create file in folder;
            vfile_t *vfile = kmalloc(1);
            vfile->type = type;
            uint32_t tmpnsize = strlen(tmp);
            if(tmp[tmpnsize - 1] == '/'){
                tmp[tmpnsize - 1] = 0;
            }
            strcpy(tmp, vfile->name);
            
            switch(type){
                case VFILE_POINTER:
                case VFILE_DIRECTORY:
                case VFILE_MOUNT:
                    void *ptr = va_arg(args, void*);
                    vfile->access.data.ptr = ptr;
                    vfile->access.data.size_pgs = va_arg(args, uint32_t);
                    add_file(vfile, current_dir);
                    // printf("Created File: %s\n", vfile->name);
                    return;
                case VFILE_FILE:
                case VFILE_DEVICE:
                    void *write = va_arg(args, void *);
                    void *read = va_arg(args, void *);
                    vfile->access.funcs.write = write;
                    vfile->access.funcs.read = read;
                    // strcpy
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
        case VFILE_POINTER:
            memcpy(byte_array, file_entry->access.data.ptr + offset, count);
            break;
        case VFILE_PDIR:
        case VFILE_FILE:
        case VFILE_DEVICE:
            file_entry->access.funcs.write(file_entry, byte_array, offset, count);
            break;
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