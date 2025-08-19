#include <stdint.h>
#include "modlib.h"

KOS_MAPI_FP api;

void init(KOS_MAPI_FP module_api){
    uint16_t *t = (void *)0xb8002;
    *t = 0xf000;
    return;
}