#include "modules.h"
#include "../shared/kstdlib.h"
#include "elf.h"

void init_modules(kernel_info_t *kernel_info){
    mlog("Module Loader", "Initializing Boot-Time Modules", MLOG_PRINT);
    kernel_module_t *module = kernel_info->loaded_modules;
    while(module){
        // printf("Module Type: %x, Module Size (Bytes): %x, Ptr: %x\n", module->type, module->size, module->ptr);
        load_elf(module->ptr);
        module = module->link;
    }
}