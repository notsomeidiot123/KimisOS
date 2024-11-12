#pragma once
#include <stdint.h>
#include "../shared/interrupts.h"
typedef struct{
    char **argv;
    uint32_t argc;
    cpu_registers_t cpuregs;
    void *page_dir;
    struct pflags{
        uint8_t blocked :1;
        uint8_t system  :1;
        uint8_t cpu_lvl :1;
        uint8_t priority:3;
        uint8_t present :1;
        uint8_t unused  :1;
    }flags;
    uint32_t thread_id;
    uint32_t parent;
    uint32_t uid;
    uint32_t gid;
}process_t;

cpu_registers_t* schedule(cpu_registers_t *regs);
void init_scheduler();