#pragma once
#include <stdint.h>
#include "../shared/kstdlib.h"

#define MODULE_PRESENT 0x80
//semver is represented by hexadecimal
//first two digits are the major number
//second two digits are the minor number
//no patch number
#define MODULE_API_VERSION 0x0100

#define MODULE_API_REGISTER 0
#define MODULE_API_ADDFUNC 1
#define MODULE_API_DELFUNC 2
#define MODULE_API_ADDINT 3
#define MODULE_API_DELINT 4
#define MODULE_API_PRINT 5
#define MODULE_API_READ 6 //read from virtual file
#define MODULE_API_WRITE 7 //write to virtual file
#define MODULE_API_CREAT 8 //create a virtual file and assigns it to the the proper module (requires having a read and write function passed)
#define MODULE_API_DELET 9
#define MODULE_API_MAP 10
#define MODULE_API_UNMAP 11
#define MODULE_API_MALLOC 12
#define MODULE_API_FREE 13

typedef struct module{
    void *init_entry;
    uint32_t id;
    char name[16];
    uint8_t flags;
    void (*fini)(void);
}module_t;

void init_modules(kernel_info_t *kernel_info);