#include "elf.h"
#include "../shared/kstdlib.h"
#include "../shared/memory.h"

#define MODULE_NAME "ELFLOAD"

char *type[] = {"", "reloc", "exec", "shared", "core"};
char *flags[] = {"---", "X--", "-W-", "XW-", "--R", "X-R", "-WR", "XWR"};

void *load_segment(program_entry_t entry, void *file_data, void *base_segment, uint32_t map_flags){
    // mlog("ELFLOAD", "p_addr: %d\n", MLOG_DEBUG, entry.paddr);
    printf("Type: %x, Flags: %s, Vaddr: %x, Offset: %x, Size: %x, Align: %x\n", entry.type, flags[entry.flags], entry.vaddr, entry.data_offset, entry.msize, entry.alignment);
    uint32_t *segment;
    uint32_t data_offset = ((elf_header_t *)file_data)->entry_offset;
    printf("Base: %x\n", base_segment);
    if(entry.type == 1){
        if(((elf_header_t *)(file_data))->type == ELF_TYPE_SHARED){
            if(base_segment == 0){
                segment = kmalloc(entry.msize/4096 + 1);
                base_segment = segment;
            }
            else{
                segment = base_segment + (entry.vaddr & 0xfffff000);
                map(segment, (void *)pm_alloc(), PT_PRESENT | map_flags);
            }
            for(uint32_t i = 0; i < entry.fsize/sizeof(uint32_t) + 1; i++){
                (segment)[i] = ((uint32_t *)(file_data))[i + entry.data_offset];
                // printf("Writing to %x\n", (uint32_t)base_segment+i+entry.vaddr);
            }
        }
    }
    printf("dSegment: %x\n", data_offset);
    return base_segment;
}

void *load_elf(void *file_data, uint32_t map_flags){
    elf_header_t *header = file_data;
    if(header->magic != 0x464c457f || header->type == ELF_TYPE_CORE){
        return 0;
    }
    program_entry_t *program_header = file_data + header->program_header_offset;
    uint32_t program_header_count = header->program_entry_count;
    mlog(MODULE_NAME, "PROGRAM ENTRIES: %d\n", MLOG_PRINT, program_header_count);
    void *base_segment = 0;
    void *entry_offet = 0;
    for(uint32_t i = 0; i < program_header_count; i++){
        // mlog("ELFLOAD", "Type: %x, Flags: %s, ALIGN: %x\n", MLOG_DEBUG, program_header[i].type, flags[program_header[i].flags], program_header[i].alignment);
        // void *segment_ptr = 0;
        // if(header->type == ELF_TYPE_SHARED){
        //     segment_ptr = kmalloc(1 + (program_header[i].msize/4096));
        // }
        // else{
        //     for(uint32_t j = 0; j < program_header[i].msize/4096; j++){
        //         map(program_header[i].vaddr, get_paddr(file_data + program_header[i].data_offset), );
        //     }
        // }
        base_segment = load_segment(program_header[i], file_data, base_segment, map_flags);
        
    }
    // void (*vt)(void) = base_segment;
    uint32_t* vt = base_segment;
    printf("%x", *vt);
    // (*vt)();
}