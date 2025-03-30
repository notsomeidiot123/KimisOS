#include "shared/memory.h"
#include "shared/string.h"
#include "shared/kstdlib.h"
#include "shared/interrupts.h"
#include "drivers/serial.h"
#include "drivers/pic.h"
#include "drivers/cpuio.h"
#include "system/scheduler.h"

void pid0(){
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
    
    printf("%x\n", *(uint32_t *)kernel_info->loaded_modules->ptr);
    printf("%x", kernel_info->loaded_modules->size);
    
    init_scheduler();
    thread_start(pid0);
    enable_interrupts();
    for(;;);
    return;
}