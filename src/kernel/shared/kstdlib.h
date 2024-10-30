#include "memory.h"
typedef struct kernel_info{
    memory_map_entry *mmap_ptr;
    uint16_t mmap_entry_count;
    uint8_t padding;
    uint8_t pixel_depth;
    uint16_t monitor_x_resolution;
    uint16_t monitor_y_resolution;
    void *framebuffer;
    void *loaded_modues;
    uint32_t loaded_module_count;
}__attribute__((packed)) kernel_info_t;