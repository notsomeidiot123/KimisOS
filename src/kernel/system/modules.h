#pragma once
#include <stdint.h>
#include "../shared/kstdlib.h"

#define MODULE_PRESENT 0x80

typedef struct module{
    void *init_entry;
    uint32_t id;
    char name[16];
    uint8_t flags;
}module_t;

void init_modules(kernel_info_t *kernel_info);