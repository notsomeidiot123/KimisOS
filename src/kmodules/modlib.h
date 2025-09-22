#pragma once
#include <stdint.h>

#define MODULE_API_REGISTER 0 //registers the module as active using information provided from the module
#define MODULE_API_ADDFUNC 1 //adds function to kernel API handler
#define MODULE_API_DELFUNC 2 //delete function from kernel API handler
#define MODULE_API_ADDINT 3 //set interrupt handler
#define MODULE_API_DELINT 4 //delete interrupt handler
#define MODULE_API_PRINT 5 //print to terminal
#define MODULE_API_READ 6 //read from virtual file
#define MODULE_API_WRITE 7 //write to virtual file
#define MODULE_API_CREAT 8 //create a virtual file and assigns it to the the proper module (requires having a read and write function passed)
#define MODULE_API_DELET 9 //delete a virtual file
#define MODULE_API_OPEN 10 //open a virtual file
#define MODULE_API_MAP 11 //map physical address to virtual address
#define MODULE_API_UNMAP 12 //unmap physical address to virtual address
#define MODULE_API_PADDR 13 //get physical address of memory
#define MODULE_API_MALLOC 14 //allocate memory in 4kb blocks
#define MODULE_API_FREE 15 //free memory allocated by malloc
#define MODULE_API_PMALLOC64K 16 //get 64k-aligned pages
#define MODULE_API_KMALLOC_PADDR 17

typedef uint32_t (*KOS_MAPI_FP)(unsigned int function, ...);
typedef struct module{
    void *init_entry;
    uint32_t id;
    char name[16];
    uint8_t flags;
    uint16_t interrupts;
    uint32_t key;
    void (*fini)(void);
}module_t;

typedef enum vfile_type{
    VFILE_POINTER,
    VFILE_DEVICE,
    VFILE_MOUNT,
    VFILE_DIRECTORY,
    VFILE_PDIR,//used for directories in physical filesystems.
    VFILE_FILE,
}VFILE_TYPE;

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

inline void *malloc(KOS_MAPI_FP api, uint32_t size_pages){
    return (void *)api(MODULE_API_MALLOC, size_pages);
}
inline void *free(KOS_MAPI_FP api, void *ptr){
    api(MODULE_API_FREE, ptr);
    return 0;
}
inline vfile_t *fopen(KOS_MAPI_FP api, char *filename){
    // vfile_t *file = malloc(api, 1);
    return (void *)api(MODULE_API_OPEN, filename);
}
inline int fread(KOS_MAPI_FP api, vfile_t *file, char *buffer, uint32_t offset, uint32_t count){
    return api(MODULE_API_READ, file, buffer, offset, count);
}
inline int fwrite(KOS_MAPI_FP api, vfile_t *file, char *buffer, uint32_t offset, uint32_t count){
    return api(MODULE_API_WRITE, file, buffer, offset, count);
}
//read documenation for this one
inline vfile_t *fcreate(KOS_MAPI_FP api, char *filename, VFILE_TYPE type, char *pointer_write, char *size_read){
    return (void *)api(MODULE_API_CREAT, filename, type, pointer_write, size_read);
}
inline void puts(KOS_MAPI_FP api, char *mname, char *str){
    api(MODULE_API_PRINT, mname, str);
};
