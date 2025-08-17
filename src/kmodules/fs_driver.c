#include <stdint.h>
#include "modlib.h"

KOS_MAPI_FP api;

void init(KOS_MAPI_FP module_api){
    uint16_t *t = (void *)0xb8002;
    *t = 0x0f41;
    // for(int i = 0; i < 1000; i++){
    //     (*t)++;
    //     // asm volatile( " " );
    // }
    // t = 0x50000000;
    // *t = 1;
    return;
}