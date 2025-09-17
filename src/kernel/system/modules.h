#pragma once
#include <stdint.h>
#include "../shared/kstdlib.h"

#define MODULE_PRESENT 0x80
//semver is represented by hexadecimal
//first two digits are the major number
//second two digits are the minor number
//no patch number
#define MODULE_API_VERSION 0x0100

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
#define MODULE_API_OPEN 10
#define MODULE_API_MAP 11 //map physical address to virtual address
#define MODULE_API_UNMAP 12 //unmap physical address to virtual address
#define MODULE_API_PADDR 13 //get physical address of memory
#define MODULE_API_MALLOC 14 //allocate memory in 4kb blocks
#define MODULE_API_FREE 15 //free memory allocated by malloc
#define MODULE_API_PMALLOC64K 16
#define MODULE_API_KMALLOC_PADDR 17

typedef struct module{
    void *init_entry;
    uint32_t id;
    char name[16];
    uint8_t flags;
    void (*fini)(void);
}module_t;

void modules_init(kernel_info_t *kernel_info);