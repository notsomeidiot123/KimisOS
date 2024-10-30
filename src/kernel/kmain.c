#include "shared/memory.h"
#include "shared/string.h"
#include "shared/kstdlib.h"
#include "drivers/serial.h"

extern void kmain(kernel_info_t *kernel_info){
    init_serial();
    pm_init(kernel_info);
    
    return;
}