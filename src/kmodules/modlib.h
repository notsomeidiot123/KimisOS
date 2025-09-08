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
#define MODULE_API_MAP 10 //map physical address to virtual address
#define MODULE_API_UNMAP 11 //unmap physical address to virtual address
#define MODULE_API_PADDR 12 //get physical address of memory
#define MODULE_API_MALLOC 13 //allocate memory in 4kb blocks
#define MODULE_API_FREE 14 //free memory allocated by malloc

typedef uint32_t (*KOS_MAPI_FP)(unsigned int function, ...);
typedef struct module{
    void *init_entry;
    uint32_t id;
    char name[16];
    uint8_t flags;
}module_t;

typedef enum vfile_type{
    VFILE_POINTER,
    VFILE_DEVICE,
    VFILE_MOUNT,
    VFILE_DIRECTORY,
    VFILE_FILE,
}VFILE_TYPE; 