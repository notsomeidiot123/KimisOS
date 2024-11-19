#include "shared/memory.h"
#include "shared/string.h"
#include "shared/kstdlib.h"
#include "shared/interrupts.h"
#include "drivers/serial.h"
#include "drivers/pic.h"
#include "drivers/cpuio.h"
#include "system/scheduler.h"

void test(){
    printf("Hello from thread!\n");
    // thread_exit(1);
    // enable_interrupts();
    // asm volatile ("jmp .");
    for(;;);
}
void test1(){
    // printf("Hello from thread1!\n");
    uint16_t *test = (void *)0xb8000;
    *test = 0x0f41;
    for(;;);
    // asm 
}
void dead_proc(){
    for(;;);
}
extern void kmain(kernel_info_t *kernel_info){
    init_serial();
    pm_init(kernel_info);
    init_idt();
    init_pic(32);
    pic_disable();
    pic_setmask(0xfe, PIC1_DATA);
    //TODO: Map and load filesystem and disk modules
    //TODO: Start Initial Process
    uint32_t exit_code = -1;
    init_scheduler();
    thread_start(dead_proc);
    thread_start(test);
    thread_start(test1);
    enable_interrupts();
    for(;;);
    return;
}