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

#define ATA_ALT_STATUS + 0x206
#define ATA_DEVICE_CONTROL + 0x206
#define ATA_DRIVE_ADDRESS + 0x207

#define ATA_IDENTIFY 0xEC

#define ATA_MASTER_DRIVE 0x1F0

#define ATA_IS_ATA(a) a[0] & 0x8000
#define ATA_GET_SZ28(a) (a[60] | (a[61] << 16))
#define ATA_IS_LBA48(a) (a[83] & (1 << 10))
#define ATA_GET_SZ48L(a) (a[100] | (a[101] << 16))
#define ATA_GET_SZ48H(a) (a[102] | (a[103] << 16))

//is drive busy?
#define ATA_BSY(a) a & 0x80
//Is drive ready?
#define ATA_DRDY(a) a & 0x40
//was there an error during write? 
#define ATA_DWF(a) a & 0x20
//is drive seek complete?
#define ATA_DSC(a) a & 0x10
//is data ready to be transferred?
#define ATA_DRQ(a) a & 0x8
//was data corrected?
#define ATA_CORR(a) a & 0x4
//was there an error?
#define ATA_ERR(a) a & 0x1
//was there a bad block?
#define ATA_BBK(a) a & 0x80
//was there uncorrectable data?
#define ATA_UNC(a) a & 0x40
//mc, whatever that means
#define ATA_MC(a) a & 0x20
//was the sector id not found?
#define ATA_IDNF(a) a & 0x10
//my chemical romance??? in my drive????
#define ATA_MCR(a) a & 0x8
//comman aborted?
#define ATA_ABRT(a) a & 0x4
//track 0 not found
#define ATA_TK0NF(a) a & 0x2
//data address mark not found
#define ATA_AMNF(a) a & 0x1;

void fini();
void ata_identify();
//return 1 if ready, 0 if not
int ata_ready(uint32_t drive){
    uint16_t status = inb(drive ATA_ALT_STATUS);
    if(!ATA_BSY(status) || ATA_DRQ(status)) return 1;
    return 0;
}

void init(KOS_MAPI_FP module_api, uint32_t api_version){
    api = module_api;
    api(MODULE_API_PRINT, MODULE_NAME, "KIDM Storage Driver Module v0.1.0\nSupported interfaces: \n");
    module_t module_data = {
        init,
        0xfae00000,
        "KIDM OFFICIAL 01",
        0
    };
    int status = api(MODULE_API_REGISTER, &module_data);
    if(status){
        api(MODULE_API_PRINT, MODULE_NAME, "Failed to register module, exiting\n");
        return;
    }
    ata_identify();
    return;
}

void ata_identify(){
    
}

void fini(){
    //destroy all objects, free memory, and 
}