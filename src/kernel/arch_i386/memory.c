#include <stdint.h>
#include "../shared/memory.h"
#include "../shared/kstdlib.h"

#define mmap_count 0x20000

uint8_t pm_map[mmap_count];

uint32_t total_memory = 0;
uint32_t total_memory_usable = 0;
uint32_t last_allocated = 0;
extern uint32_t _start;

uint32_t pm_alloc(){
    for(uint32_t i = 0; i < mmap_count; i++){
        for(int j = 0; j < 8; j++){
            if(!(pm_map[i] & (1 << j))){
                return (i << 3 | j) << 12;
            }
        }
    }
}
void pm_free(uint32_t address){
    pm_map[address >> 15] &= ~(1 << ((address>>12) & 7));
}
void pm_reserve(uint32_t address){
    pm_map[address >> 15] |= 1 << ((address>>12) & 7);
}

int pm_init(kernel_info_t *kernel_info){
    mmap_entry_t *mmap = (mmap_entry_t*)(kernel_info->mmap_ptr);
    // printf("mmap count: %d\n| start  | length |type|\n|--------|--------|----|\n", kernel_info->mmap_entry_count);
    for(uint32_t i = 0; i < mmap_count; i++){
        pm_map[i] = 0xff;
    }
    for(uint32_t i = 0; i < kernel_info->mmap_entry_count; i++){
        for(uint32_t j = 0; j < (mmap[i].entry_length >> 12); j++){
            if(mmap[i].entry_base + (j << 12) <= mmap[i-1].entry_base + mmap[i-1].entry_length){
                continue;
            }
            // printf("j = %d\nIndex:%x\n", j, (mmap[i].entry_base >> 12) + (j & 3));
            if(mmap[i].type != BIOS_MMAP_USABLE){
                pm_map[(mmap[i].entry_base >> 15) + (j >> 3)] |= 1 << (j & 7) + ((mmap[i].entry_base >> 12) & 7);
            }
            else{
                // uint8_t tmp = pm_map[(mmap[i].entry_base >> 12 ) + (j >> 3)];
                // tmp &= ~(1 << (j & 3));
                pm_map[(mmap[i].entry_base >> 15) + (j >> 3)] &= ~(1 << ((j & 7) + ((mmap[i].entry_base >> 12) & 7)));
            }
            // if(j % 8 == 7){
            //     printf("%d, %x, %d\n", j >> 3, pm_map[(mmap[i].entry_base >> 12 ) + (j >> 3)], mmap[i].type);
            // }
        }
        // printf("|%x|", mmap[i].entry_base);
        // printf("%x",  mmap[i].entry_length);
        // printf("|%d   |\n", mmap[i].type);
    }
    for(uint32_t i = 0; i < 512; i++){
        pm_reserve(i * 4096);
    }
    
    // printf("%x", pm_map);
    void *kernel_addr = (void *)_start;
    while(get_paddr(kernel_addr)){
    
        // printf("%x\n", kernel_addr);
        pm_reserve(get_paddr(kernel_addr));
        kernel_addr += 0x1000;
    }
}
void map(void *vaddr, void *paddr, uint32_t flags){
    uint32_t pd_index = (uint32_t)vaddr >> 22;
    uint32_t pt_index = (uint32_t)vaddr >> 12 & 0x3ff;
    
    uint32_t *pd = (uint32_t*)0xfffff000;
    if(!pd[pd_index]){
        pd[pd_index] = pm_alloc() | 1;
        asm volatile("invlpg (%0)" : : "b"(0xffc00000 + (pd_index * 0x400)) : "memory");
    }
    uint32_t *pt = (uint32_t *)(0xffc00000 + (pd_index * 0x400));
    pt[pt_index] = (uint32_t)paddr | flags;
    asm volatile("invlpg (%0)  " : : "b"(vaddr) : "memory");
}
void unmap(void *vaddr){
    uint32_t pd_index = (uint32_t)vaddr >> 22;
    uint32_t pt_index = (uint32_t)vaddr >> 12 & 0x3ff;
    
    uint32_t *pd = (uint32_t *)0xfffff000;
    if(!pd[pd_index]){
        return;
    }
    uint32_t *pt = (uint32_t *)(0xffc00000 + (pd_index * 0x400));
    uint32_t paddr = pt[pt_index];
    paddr ^= (paddr & 0xfff);
    pm_free(paddr);
    asm volatile("invlpg (%0)  " : : "b"(vaddr) : "memory");
}
void map_4mb(void *vaddr, void *paddr, uint32_t flags){
    //just wanted to get the prototype out
    //!TODO: Finish mapping and unmapping 4mb pages
}
uint32_t get_paddr(void *addr){
    //!TODO: Support 4mb pages
    uint32_t vaddr = (uint32_t)addr;
    uint32_t pd_index = vaddr >> 22;
    uint32_t pt_index = vaddr >> 12 & 0x3ff;
    uint32_t *pd = (uint32_t *)0xfffff000;
    if(!pd[pd_index]){
        return 0;
    }
    uint32_t *pt = (uint32_t *)(0xffc00000 + (0x400 * pd_index));
    return pt[pt_index] & ~(pt[pt_index] & 0xfff);
}