#include "shared/memory.h"
#include "shared/string.h"
#include "shared/kstdlib.h"
#include "shared/interrupts.h"
#include "drivers/serial.h"
#include "drivers/pic.h"
#include "drivers/cpuio.h"
#include "drivers/pci.h"
#include "system/scheduler.h"
#include "system/modules.h"
#include "system/elf.h"
#include "system/vfs.h"
#include "drivers/ustar.h"
#include "system/initrc.h"

kernel_info_t *boot_info = 0;

void pid0(){
    for(;;);
}
void sysinit(){
    mlog("KERNEL", "PID 1 Started\n", MLOG_PRINT);
    pci_init();
    // modules_init(boot_info, 0);
    read_initrd(boot_info->initrd);
    vfile_t *initrc = fopen("/boot/initrc.conf");
    if(initrc){
        mlog("KERNEL", "Found initrc at: %x\n", MLOG_PRINT, initrc->access.data.ptr);
    }
    modules_init();
    initrc_read(initrc);
    
    printf("Bleh\n");
    for(;;);
}
extern void kmain(kernel_info_t *kernel_info){
    serial_init();
    pm_init(kernel_info);
    vfs_init();
    mlog("KERNEL", "Initializing IDT\n", MLOG_PRINT);
    idt_load();
    pic_init(0x20);
    pic_disable();
    pic_setmask(0x0, PIC1_DATA);
    pic_setmask(0x0, PIC2_DATA);
    
    fcreate("/dev", VFILE_DIRECTORY, kmalloc(1), 1);
    fcreate("/dev/disk", VFILE_DIRECTORY, kmalloc(1), 1);
    fcreate("/tmp", VFILE_DIRECTORY, kmalloc(1), 1);
    fcreate("/boot", VFILE_DIRECTORY, kmalloc(1), 1);
    mlog("KERNEL", "Initializing Scheduler & starting PID 1\n", MLOG_PRINT);
    boot_info = kernel_info;
    scheduler_init();
    //scheduler doesn't work if there is no PID0, and I don't know why.
    thread_start(pid0);
    thread_start(sysinit);
    enable_interrupts();
    for(;;);//we actually **shouldn't** return, like, ever. That's bad.
    return;
}