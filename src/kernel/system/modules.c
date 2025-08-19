#include "modules.h"
#include "../shared/kstdlib.h"
#include "elf.h"
#include "../shared/memory.h"
#include <stdarg.h>


#define MODULE_NAME "MLOADER"

vector_t *modules;
vector_t *module_deps;

uint32_t module_api(uint32_t func, ...){
    
}

void init_modules(kernel_info_t *kernel_info){
    mlog(MODULE_NAME, "Initializing Boot-Time Modules\n", MLOG_PRINT);
    kernel_module_t *module = kernel_info->loaded_modules;
    // uint32_t i = 0;
    modules = init_vector(0, sizeof(module_t), 0, 0);
    while(module){
        if(!module->ptr){
            continue;
        }
        void (*entry)(void* api) = load_elf(module->ptr, PT_SYS);
        mlog(MODULE_NAME, "Loaded module with ID %x\n", MLOG_PRINT, module->id);
        // module->type |= 0x8000;//set present flag in type
        module->flags |= MODULE_PRESENT;
        module = module->link;
        (*entry)(0);
    }
}