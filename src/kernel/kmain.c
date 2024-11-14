#include "shared/memory.h"
#include "shared/string.h"
#include "shared/kstdlib.h"
#include "shared/interrupts.h"
#include "drivers/serial.h"
#include "drivers/pic.h"
#include "drivers/cpuio.h"
#include "system/scheduler.h"

void test(){
    printf("Hi from another thread!\n");
    // *((uint16_t*)0xb8000) = 0xf41;
    for(;;);
}

extern void kmain(kernel_info_t *kernel_info){
    init_serial();
    pm_init(kernel_info);
    init_idt();
    init_pic(32);
    pic_disable();
    init_scheduler();
    pic_setmask(0xfe, PIC1_DATA);
    enable_interrupts();
    // printf("done\n");
    thread_start(test, 0);
    // printf("%x", test);
    // void test();
    for(;;);
    return;
}