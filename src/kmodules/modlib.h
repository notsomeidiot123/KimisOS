#pragma once
#include <stdint.h>
typedef void (*KOS_MAPI_FP)(unsigned int function, ...);
typedef struct module{
    void *init_entry;
    uint32_t id;
    char name[16];
    uint8_t flags;
}module_t;