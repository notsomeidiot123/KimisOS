#include "modules.h"
#include "../shared/kstdlib.h"
#include "elf.h"
#include "vfs.h"
#include "../shared/memory.h"
#include <stdarg.h>

#define MODULE_NAME "MLOADER"

vector_t *modules;
vector_t *module_deps;

uint32_t module_api(uint32_t func, ...){
    va_list vars;
    va_start(vars, func);
    uint32_t return_value = 0;
    switch(func){
        case MODULE_API_REGISTER:
            //do something
            void *structure = va_arg(vars, void *);
            if(structure == 0){
                return -1;
            }
            vector_push(modules, structure);
            return 0;
            break;
        case MODULE_API_ADDFUNC:
            //do something
            return_value = -1;
            break;
        case MODULE_API_DELFUNC:
            //do something
            return_value = -1;
            break;
        case MODULE_API_ADDINT:
            return_value = -1;
            //do something
            break;
        case MODULE_API_DELINT:
            //do something
            return_value = -1;
            break;
        case MODULE_API_PRINT:
            // vmlog(MODULE_NAME, "Testing modules calling kernel functions\n", MLOG_PRINT, vars);
            char *name = va_arg(vars, char *);
            char *string = va_arg(vars, char *);
            vmlog(name, string, MLOG_PRINT, vars);
            break;
        case MODULE_API_READ:
            vfile_t *file = va_arg(vars, vfile_t *);
            char *buffer = va_arg(vars, char *);
            uint32_t offset = va_arg(vars, uint32_t), count = va_arg(vars, uint32_t);
            return_value = fread(file, buffer, offset, count);
            break;
        case MODULE_API_WRITE:
            file = va_arg(vars, vfile_t *);
            buffer = va_arg(vars, char *);
            offset = va_arg(vars, uint32_t), count = va_arg(vars, uint32_t);
            return_value = fwrite(file, buffer, offset, count);
            break;
        case MODULE_API_CREAT:
            name = va_arg(vars, char *);
            VFILE_TYPE ftype = va_arg(vars, VFILE_TYPE);
            void *arg1, *arg2;
            arg1 = va_arg(vars, void *);
            arg2 = va_arg(vars, void *);
            return_value = (uint32_t)fcreate(name, ftype, arg1, arg2);
            break;
        case MODULE_API_DELET:
            return_value = -1;
            break;
        case MODULE_API_OPEN:
            name = va_arg(vars, char *);
            return_value = (uint32_t)fopen(name);
            break;
        case MODULE_API_MAP:
            return_value = 0;
            void *vaddr = va_arg(vars, void *);
            void *paddr = va_arg(vars, void *);
            uint32_t flags = va_arg(vars, uint32_t);
            map(vaddr, paddr, flags);
            break;
        case MODULE_API_UNMAP:
            return_value = -1;
            break;
        case MODULE_API_PADDR:
            void *addr = va_arg(vars, void *);
            return_value = get_paddr(addr);
            break;
        case MODULE_API_MALLOC: 
            uint32_t size_pgs = va_arg(vars, uint32_t);
            return_value = (uint32_t)kmalloc(size_pgs);
            break;
        case MODULE_API_FREE:
            void *ptr = va_arg(vars, void *);
            kfree(ptr);
            return_value = 0;
            break;
        case MODULE_API_PMALLOC64K:
            return_value = pm_alloc_64kaligned();
            break;
        case MODULE_API_KMALLOC_PADDR:
            paddr = (void *)va_arg(vars, uint32_t);
            uint32_t size = va_arg(vars, uint32_t);
            return_value = (uint32_t)kmalloc_page_paddr((uint32_t)paddr, size);
            break;
        
    }
    va_end(vars);
    return return_value;
}

void modules_init(kernel_info_t *kernel_info){
    mlog(MODULE_NAME, "Initializing Boot-Time Modules\n", MLOG_PRINT);
    kernel_module_t *module = kernel_info->loaded_modules;
    // uint32_t i = 0;
    modules = init_vector(0, sizeof(module_t), 0, 0);
    while(module){
        if(!module->ptr){
            continue;
        }
        void (*entry)(void* api, uint32_t version) = load_elf(module->ptr, PT_SYS);
        mlog(MODULE_NAME, "Loaded module with ID %x\n", MLOG_PRINT, module->id);
        // module->type |= 0x8000;//set present flag in type
        module->flags |= MODULE_PRESENT;
        module = module->link;
        (*entry)(module_api, 0);
    }
}