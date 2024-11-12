#include "../shared/interrupts.h"
#include "scheduler.h"
#include "../shared/kstdlib.h"

process_t volatile processes[0x10000];
uint32_t volatile process_queue[0x10000];
uint32_t volatile current_pid;

void init_scheduler(){
    for(uint32_t i = 0; i < 0x10000; i++){
        processes[i].flags.present = 0;
        process_queue[i] = 0;
    }
    current_pid = 0;
    install_irq_handler(schedule, 0);
    printf("Installing scheduler");
}

cpu_registers_t *schedule(cpu_registers_t *regs){
    uint32_t i = current_pid;
    // printf("Test");
    while(processes[process_queue[i]].flags.blocked){
        i++;
        if(i == current_pid) return regs;
        if(i >= 0x10000) i = 0;
    }
    current_pid = i;
    return &(processes[i].cpuregs);
    // return regs;
}

void spawn_new_process(cpu_registers_t defaultregs, char **argv, uint32_t argc, void *cr3){
    uint32_t i = current_pid;
    while(processes[i].flags.present){
        i++;
        if(i >= 0x10000) i = 0;
        if(i == current_pid) return;
    }
    process_t new_proc = {0};
    new_proc.argc = argc;
    new_proc.argv = argv;
    new_proc.page_dir = cr3;
    new_proc.cpuregs = defaultregs;
    new_proc.parent = current_pid;
    new_proc.flags.present = 1;
    new_proc.flags.cpu_lvl = 1;
    new_proc.flags.priority = 0;
    new_proc.flags.system = processes[current_pid].flags.system;
    return;
}

void fork(){
    
}
void exec(){
    
}
void exit(){
    
}
void thread_start(void (*function)(), uint32_t *exit_code){
    
}
void thread_join(uint32_t thread_id, uint32_t **exit_code){
    
}
void thread_exit(uint32_t *exit_code){
    
}