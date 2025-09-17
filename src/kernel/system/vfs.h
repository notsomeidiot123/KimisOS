#pragma once
#include <stdint.h>

typedef enum vfile_type{
    VFILE_POINTER,//virtual files
    VFILE_DEVICE,
    VFILE_MOUNT,//in the pointer passed to the function, must specify a read, write, open, create, and delete function.
    VFILE_DIRECTORY,
    VFILE_PDIR,//used for directories in physical filesystems.
    VFILE_FILE,//physical files
    VFILE_PIPE //yay we have pipes
}VFILE_TYPE; 

//this code is gonna be ***really*** unsafe
typedef struct virtual_file{
    char name[20];
    VFILE_TYPE type;
    uint32_t id;//to be assigned by driver;
    uint32_t mount_id;
    uint8_t lock;
    uint32_t size;
    struct virtual_file *parent;//should point to A: a virtual directory, or B: a mounted filesystem
    union{
        struct{
            int (*read)(struct virtual_file *file, void *data, uint32_t offset, uint32_t count);
            int (*write)(struct virtual_file *file, void *data, uint32_t offset, uint32_t count);
        }funcs;
        struct{
            void *ptr;
            uint32_t size_pgs;
        }__attribute__((packed))data;
    }access;
}vfile_t;

typedef enum fs_flags{
    FS_FILE_READ_ONLY = 1,
    FS_FILE_HIDDEN = 2,
    FS_FILE_SYSTEM = 4,
    FS_FILE_IS_DIR = 0x10,
    FS_FILE_ARCHIVE = 0x20
}FS_FILE_FLAGS;

typedef struct mount_funcs{
    int (*write)(vfile_t *file, void *data, uint32_t offset, uint32_t count);
    int (*read)(vfile_t *file, void *data, uint32_t offset, uint32_t count);
    int (*open)(char *filename, vfile_t *file);
    void (*delete)(vfile_t *file);
    void (*create)(char *filename, FS_FILE_FLAGS flags);
}mount_t;

void vfs_init();
vfile_t *fcreate(char *name, VFILE_TYPE type, ...);
void fdelete();
int fwrite(vfile_t *file_entry, void *byte_array, uint32_t offset, uint32_t count);
int fread(vfile_t *file_entry, void *byte_array, uint32_t offset, uint32_t count);
vfile_t *search_dir(char *name, vfile_t dir);
vfile_t *fopen(char *name);