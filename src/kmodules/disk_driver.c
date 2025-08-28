#include<stdint.h>
#include "modlib.h"

#define MODULE_NAME "KIDM"

KOS_MAPI_FP api;

inline void outb(uint16_t port, uint8_t data){
    asm volatile ("outb %0, %1" :: "a"(data), "Nd"(port));
}

inline void outw(uint16_t port, uint16_t data){
    asm volatile ("outw %0, %1" :: "a"(data), "Nd"(port));
}

inline uint8_t inb(uint16_t port){
    uint8_t byte = 0;
    asm volatile ("inb %1, %0" : "=a"(byte) : "Nd"(port));
    return byte;
}

inline uint16_t inw(uint16_t port){
    uint16_t word = 0;
    asm volatile ("inw %1, %0" : "=a"(word) : "Nd"(port));
    return word;
}
/*
TODO:
Register module 
IDENTIFY
CREATE ((virtual)) FILES
CREATE interface when reading and writing from virtual files
make fini function to destroy global object, and free any remaining resources.
*/

#define ATA_DATA + 0
#define ATA_ERROR + 1
#define ATA_FEATURES + 1
#define ATA_SECTOR_COUNT + 2
#define ATA_SECTOR_NUMBER + 3
#define ATA_CYLINDER_LOW + 4
#define ATA_CYLINDER_HIGH + 5
#define ATA_DRIVE_HEAD + 6
#define ATA_STATUS + 7
#define ATA_COMMAND + 7

#define ATA_LBA_LOW ATA_SECTOR_NUMBER
#define ATA_LBA_MID ATA_CYLINDER_LOW
#define ATA_LBA_HIH ATA_CYLINDER_HIGH

#define ATA_ALT_STATUS + 0
#define ATA_DEVICE_CONTROL + 0
#define ATA_DRIVE_ADDRESS + 1

#define ATA_IDENTIFY 0xEC

#define ATA_SLAVE_DRIVE 0xF0
#define ATA_MASTER_DRIVE 0xE0

#define ATA_IS_ATA(a) a[0] & 0x8000
#define ATA_GET_SZ28(a) (a[60] | (a[61] << 16))
#define ATA_IS_LBA48(a) (a[83] & (1 << 10))
#define ATA_GET_SZ48L(a) (a[100] | (a[101] << 16))
#define ATA_GET_SZ48H(a) (a[102] | (a[103] << 16))

void fini();
void ata_identify();

void init(KOS_MAPI_FP module_api, uint32_t api_version){
    api = module_api;
    api(MODULE_API_PRINT, MODULE_NAME, "KIDM Storage Driver Module v0.1.0\nSupported interfaces: \n");
    return;
}

void ata_identify(){
    
}

void fini(){
    //destroy all objects, free memory, and 
}