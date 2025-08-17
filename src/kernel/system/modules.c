#include "modules.h"
#include "../shared/kstdlib.h"
#include "elf.h"
#include "../shared/memory.h"

#define MODULE_NAME "MLOADER"

void init_modules(kernel_info_t *kernel_info){
    mlog(MODULE_NAME, "Initializing Boot-Time Modules\n", MLOG_PRINT);
    kernel_module_t *module = kernel_info->loaded_modules;
    uint32_t i = 0;
    while(module){
        if(!module->ptr){
            continue;
        }
        // printf("Module Type: %x, Module Size (Bytes): %x, Ptr: %x\n", module->type, module->size, module->ptr);
        load_elf(module->ptr, 0);
        mlog(MODULE_NAME, "Loaded module of type %x\n", MLOG_PRINT, module->type);
        // module->type |= 0x8000;//set present flag in type
        module->type |= MODULE_PRESENT;
        module = module->link;
    }
}