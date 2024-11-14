#include "../shared/interrupts.h"
#include "scheduler.h"
#include "../shared/string.h"
#include "../shared/kstdlib.h"
#include "../shared/memory.h"

process_t volatile processes[0x10000];
uint32_t volatile process_queue[0x10000];
uint32_t active_processes;
uint32_t volatile current_pid;
uint32_t volatile queue_length;
uint32_t volatile current_queue_index;
void init_scheduler(){
    for(uint32_t i = 0; i < 0x10000; i++){
        processes[i].flags.present = 0;
        process_queue[i] = 0;
    }
    current_pid = 0;
    queue_length = 0;
    current_queue_index = 0;
    active_processes = 0;
    install_irq_handler(schedule, 0);
    // printf("Installing scheduler");
}

cpu_registers_t *schedule(cpu_registers_t *regs){
    if(queue_length == 0){
        return regs;
    }
    uint32_t i = current_queue_index;
    for(; i < queue_length; i++){
        if(processes[process_queue[i%queue_length]].flags.blocked) continue;
        else break;
    }
    current_pid = process_queue[i];
    // printf("\n%d", current_pid);
    // *regs = processes[current_pid].cpuregs;
    cpu_registers_t *newregs = (cpu_registers_t*)processes[current_pid].cpuregs.esp;
    return newregs;
    // return regs;
}

void add_process_queue(uint32_t pid){
    process_queue[queue_length] = pid;
    queue_length++;
}
void remove_process_queue(uint32_t pid){
    uint32_t last_queue_index = queue_length-1;
    uint32_t old_index = 0;
    for(uint32_t i = 0; i < last_queue_index; i++){
        if(process_queue[i] == pid){
            old_index = i;
            process_queue[i] = 0;
        }
    }
    if(old_index == last_queue_index) return;
    process_queue[old_index] = process_queue[last_queue_index];
    process_queue[last_queue_index] = 0;
}

void spawn_new_process(cpu_registers_t defaultregs, char **argv, uint32_t argc, void *cr3){
    uint32_t i = current_pid;
    while(processes[i].flags.present){
        i++;
        if(i >= 0x10000) i = 0;
        if(i == current_pid) return;
    }
    i += !active_processes;
    active_processes++;
    // printf("Found %d", i);
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
    processes[i] = new_proc;
    add_process_queue(i);
    return;
}

void fork(){
    
}
void exec(){
    
}
void exit(){
    
}
void thread_start(void (*function)(), uint32_t *exit_code){
    cpu_registers_t regs = {0};
    regs.cs = 0x10;
    regs.ds = 0x18;
    regs.es = 0x18;
    regs.ss = 0x18;
    // regs.ebp = 
    regs.esp = 0x7000 - sizeof(regs) + 8;
    regs.ebp = regs.esp;
    regs.eip = (uint32_t)function;
    memcpy((char *)&regs, (char*)regs.esp, sizeof(regs));
    spawn_new_process(regs, 0, 0, (void *)0x10000);
}
void thread_join(uint32_t thread_id, uint32_t **exit_code){
    
}
void thread_exit(uint32_t *exit_code){
    
}