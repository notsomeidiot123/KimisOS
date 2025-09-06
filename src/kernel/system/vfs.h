#pragma once
#include <stdint.h>

typedef enum vfile_type{
    VFILE_POINTER,
    VFILE_DEVICE,
    VFILE_MOUNT,
    VFILE_DIRECTORY,
    VFILE_FILE,
}VFILE_TYPE; 

//this code is gonna be ***really*** unsafe
typedef struct virtual_file_t{
    char name[20];
    VFILE_TYPE type;
    union{
        struct{
            void (*read)(void *data, uint32_t offset, uint32_t count);
            void (*write)(void *data, uint32_t offset, uint32_t count);
        }funcs;
        struct{
            void *ptr;
            uint16_t size_pgs;
            uint16_t free_bytes;
        }__attribute__((packed))data;
    }access;
}vfile_t;

void vfs_init();
void fcreate();
void fdelete();
uint32_t fwrite();
uint32_t fread();
void fopen();