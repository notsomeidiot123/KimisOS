#include <stdint.h>
#include "modlib.h"

#define MODULE_NAME "KIFSM"

KOS_MAPI_FP api;

void init(KOS_MAPI_FP module_api, uint32_t api_version){
    api = module_api;
    
    api(MODULE_API_PRINT, MODULE_NAME, "KIFSM Filesystem Driver Module v0.1.0\nSupported Filesystems:\n");
    
    return;
}