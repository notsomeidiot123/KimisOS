#include<stdint.h>
#include "modlib.h"

KOS_MAPI_FP api;

uint32_t init(KOS_MAPI_FP module_api){
    uint16_t *t = (void *)0xb8000;
    *t = 0x0f41;
    api = module_api;
    // for(int i = 0; i < 1000; i++){
    //     (*t)++;
    //     // asm volatile( " " );
    // }
    return &api;
}