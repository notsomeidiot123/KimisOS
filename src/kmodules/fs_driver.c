#include <stdint.h>
#include "modlib.h"
#include "fs_driver.h"
#define MODULE_NAME "KIFSM"

KOS_MAPI_FP api;

void detect_partitions(vfile_t *file){
    char *buffer = malloc(api, 1);
    // buffer[0] = 5;
    fread(api, file, buffer, 0, 512);
    mbr_t *mbr = buffer;
    api(MODULE_API_PRINT, MODULE_NAME, "Magic: %x\n", mbr->magic);
    return;
}

void init(KOS_MAPI_FP module_api, uint32_t api_version){
    api = module_api;
    api(MODULE_API_PRINT, MODULE_NAME, "KIFSM Filesystem Driver Module v0.1.0\nSupported Filesystems:\n");
    
    vfile_t *disk_dir = fopen(api, "/dev/disk");
    if(disk_dir == 0){
        puts(api, MODULE_NAME, "Could not find /dev/disk\n");
    }
    vfile_t **dir_data = disk_dir->access.data.ptr;
    for(uint32_t i = 0; dir_data[i]; i++){
        detect_partitions(dir_data[i]);
    }
    
    return;
}