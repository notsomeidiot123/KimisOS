#include "shared/memory.h"
#include "shared/string.h"
#include "shared/kstdlib.h"
#include "shared/interrupts.h"
#include "drivers/serial.h"
#include "drivers/pic.h"
#include "drivers/cpuio.h"
#include "system/scheduler.h"

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
    init_scheduler();
    thread_start(dead_proc);
    enable_interrupts();
    for(;;);
    return;
}