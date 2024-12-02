#pragma once
// #include "memory.h"
#include "../drivers/serial.h"
#include "interrupts.h"

typedef struct module_linked_list{
    void *ptr;
    uint32_t type;
    struct module_linked_list *link;
}__attribute__((packed)) kernel_module_t;

typedef struct kernel_info{
    void *mmap_ptr;
    uint16_t mmap_entry_count;
    uint8_t padding;
    uint8_t pixel_depth;
    uint16_t monitor_x_resolution;
    uint16_t monitor_y_resolution;
    void *framebuffer;
    kernel_module_t *loaded_modules;
    // uint32_t loaded_module_count;
}__attribute__((packed)) kernel_info_t;


#define MODULE_TYPE_DISK 1
#define MODULE_TYPE_FS 2
#define MODULE_TYPE_EXT 3
#define MODULE_TYPE_VIDEO 4

#define PANIC(m) kernel_panic(m, 0)