#pragma once
#include <stdint.h>

typedef enum vfile_type{
    VFILE_POINTER,
    VFILE_DEVICE,
    VFILE_MOUNT,
    VFILE_DIRECTORY,
    VFILE_PDIR,//used for directories in physical filesystems.
    VFILE_FILE
}VFILE_TYPE; 

//this code is gonna be ***really*** unsafe
typedef struct virtual_file{
    char name[20];
    VFILE_TYPE type;
    uint32_t id;//to be assigned by driver;
    uint32_t mount_id;
    union{
        struct{
            void (*read)(struct virtual_file *file, void *data, uint32_t offset, uint32_t count);
            void (*write)(struct virtual_file *file, void *data, uint32_t offset, uint32_t count);
        }funcs;
        struct{
            void *ptr;
            uint32_t size_pgs;
        }__attribute__((packed))data;
    }access;
}vfile_t;

typedef enum fs_flags{
    FS_FILE_IS_DIR = 1,
    FS_FILE_IS_SYSTEM = 2,
    FS_FILE_IS_HIDDEN = 4,
}FS_FILE_FLAGS;

typedef struct mount_funcs{
    void (*write)(vfile_t *file, void *data, uint32_t offset, uint32_t count);
    void (*read)(vfile_t *file, void *data, uint32_t offset, uint32_t count);
    int (*open)(char *filename, vfile_t *file);
    void (*delete)(vfile_t *file);
    void (*create)(char *filename, FS_FILE_FLAGS flags);
}mount_t;

void vfs_init();
void fcreate(char *name, VFILE_TYPE type, ...);
void fdelete();
uint32_t fwrite();
uint32_t fread();
vfile_t *search_dir(char *name, vfile_t dir);
int fopen(char *name, vfile_t *file);