#include "shared/memory.h"
#include "shared/string.h"
#include "shared/kstdlib.h"
#include "shared/interrupts.h"
#include "drivers/serial.h"
#include "drivers/pic.h"
#include "drivers/cpuio.h"
#include "system/scheduler.h"
#include "system/modules.h"
#include "system/elf.h"

kernel_info_t *boot_info = 0;

void pid0(){
    for(;;);
}
void sysinit(){
    mlog("KERNEL", "PID 1 Started\n", MLOG_PRINT);
    init_modules(boot_info);
    for(;;);
}
extern void kmain(kernel_info_t *kernel_info){
    init_serial();
    pm_init(kernel_info);
    mlog("KERNEL", "Initializing IDT\n", MLOG_PRINT);
    init_idt();
    init_pic(32);
    pic_disable();
    pic_setmask(0xfe, PIC1_DATA);
    //TODO: Map and load filesystem and disk modules
    //TODO: Start Initial Process
    mlog("KERNEL", "Initializing Scheduler & starting PID 1\n", MLOG_PRINT);
    boot_info = kernel_info;
    init_scheduler();
    thread_start(pid0);
    thread_start(sysinit);
    enable_interrupts();
    for(;;);
    return;
}