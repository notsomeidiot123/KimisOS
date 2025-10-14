#include <stdint.h>
#include "modlib.h"
#include "fs_driver.h"
#define MODULE_NAME "KIFSM"

KOS_MAPI_FP api;

void mount_fat32(vfile_t *dev_file, char *destination){
    
}

void detect_partitions(vfile_t *file){
    char *buffer = malloc(api, 1);
    fread(api, file, buffer, 0, 512);
    mbr_t *mbr = buffer;
    if(mbr->magic != MBR_MAGIC){
        puts(api, MODULE_NAME, "No MBR!\n");
        api(MODULE_API_PRINT, MODULE_NAME, "Magic: %x\n", mbr->magic);
        return;
    }
    //first: check for filesystem headers (if they exist, register entire drive as fs and return)
    //then, check partitions.
    for(int i = 0; i < 4; i++){
        partition_t part = mbr->partitions[i];
        if(part.type == 0xee){
            puts(api, MODULE_NAME, "Found gpt\n");
            return;
        }
        if(part.attributes != 0 || part.attributes != 0x80){
            //check disk for filesystem
            api(MODULE_API_PRINT, MODULE_NAME, "No partitions!\n");
            return;
        }
        //now, re-check for filesystems
    }
    api(MODULE_API_PRINT, MODULE_NAME, "Magic: %x\n", mbr->magic);
    return;
}
//buffer is a pointer to a string that contains the filename, followed by a comma, followed by the mount destination. If there is no destination [i.e. no comma], command will be treated as unmount
int mount(vfile_t *file, char *buffer, uint32_t offset, uint32_t count){
    char filename_buffer[256];
    char destination_buffer[256];
    uint32_t comma_offset = 0;
    int i = 0;
    for(; i < 256; i++){
        if(buffer[i] == ',' || !buffer[i]){
            break;
        }
        filename_buffer[i] = buffer[i];
    }
    filename_buffer[i] = 0;
    comma_offset = i + 1;
    i = 0;
    for(; i < 256; i++){
        if(buffer[i] == 0){
            break;
        }
        destination_buffer[i] = buffer[i];
    }
    destination_buffer[i] = 0;
    vfile_t *src = fopen(api, filename_buffer);
    if(!src){
        puts(api, MODULE_NAME, "src does not exist\n");
    }
    return 0;
}
int phony_read(vfile_t *file, char*buffer, uint32_t offset, uint32_t count){
    return 0;
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
        api(MODULE_API_PRINT, MODULE_NAME, "Filename: %s\n", dir_data[i]->name);
        detect_partitions(dir_data[i]);
    }
    fcreate(api, "/dev/fat32mnt", VFILE_DEVICE, mount, phony_read);
    // mount(0, "bleh,test", 0, 15);
    return;
}