#include <stdint.h>
#include "../shared/memory.h"
#include "../shared/kstdlib.h"
uint8_t pm_map[0x20000];

int pm_alloc(){
    
}
int pm_init(kernel_info_t *kernel_info){
    
}
void map(void *vaddr, void *paddr, uint32_t flags){
    uint32_t pd_index = (uint32_t)vaddr >> 22;
    uint32_t pt_index = (uint32_t)vaddr >> 12 & 0x3ff;
    
    uint32_t *pd = (uint32_t*)0xfffff000;
    if(!pd[pd_index]){
        pd[pd_index] = pm_alloc();
    }
}
