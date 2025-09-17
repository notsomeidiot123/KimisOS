#include<stdint.h>
#include "modlib.h"
#include "../kernel/drivers/cpuio.h"
#include "../kernel/shared/string.h"

#define MODULE_NAME "KIDM"

KOS_MAPI_FP api;

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

#define ATA_PRIMARY_BUS 0x1F0
#define ATA_SECONDARY_BUS 0x170

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

//commands
#define ATA_CMD_READ_PIO          0x20
#define ATA_CMD_READ_PIO_EXT      0x24
#define ATA_CMD_READ_DMA          0xC8
#define ATA_CMD_READ_DMA_EXT      0x25
#define ATA_CMD_WRITE_PIO         0x30
#define ATA_CMD_WRITE_PIO_EXT     0x34
#define ATA_CMD_WRITE_DMA         0xCA
#define ATA_CMD_WRITE_DMA_EXT     0x35
#define ATA_CMD_CACHE_FLUSH       0xE7
#define ATA_CMD_CACHE_FLUSH_EXT   0xEA
#define ATA_CMD_PACKET            0xA0
#define ATA_CMD_IDENTIFY_PACKET   0xA1
#define ATA_CMD_IDENTIFY          0xEC
#define ATAPI_CMD_READ            0xA8
#define ATAPI_CMD_EJECT           0x1B

#define IDE_ATA        0x00
#define IDE_ATAPI      0x01

#define PCI_IDE_NATIVE(a) a & 1
#define PCI_IDE_CAN_SET_NATIVE(a) a & 2
#define PCI_IDE_SECONDARY_NATIVE(a) a & 4
#define PCI_IDE_SECONDARY_CAN_SET_NATIVE(a) a & 8
#define PCI_IDE_SUPPORT_DMA(a) (a & 0x80)
#define PCI_CLASS_IS_IDE(a) a == 0x0101;

typedef struct PRD{
    uint32_t address;
    uint16_t byte_count;
    uint16_t reserved;//set msb when last;
}__attribute__((packed)) PRD_T;



void fini();

enum DRIVE_TYPE{
    TYPE_NULL,
    TYPE_IDE,
    TYPE_AHCI,
    TYPE_NVME,
    TYPE_USB,
    TYPE_FLOPPY, //Here, Navya, just for you
};

typedef struct drive_desc{
    uint32_t BARs[8];
    enum DRIVE_TYPE type;
    uint64_t size_sectors;
    uint32_t sector_size_bytes;
    struct{
        uint8_t irq_dispatched:1;
        uint8_t huge:1;//more than 2^28 sectors
        uint8_t locked:1;//semaphores!!!!!
        uint8_t slave:1;//is this drive a slave drive to another drive?
    }__attribute__((packed))flags;
    PRD_T *PRDT;
}drive_t;

drive_t drives[32] = {0};

uint16_t get_bus_from_drive(uint32_t drive){
    return drive & 2 ? ATA_SECONDARY_BUS : ATA_PRIMARY_BUS;
}

//return 1 if ready, 0 if not
int ata_ready(uint32_t BAR, uint32_t BAR2, uint8_t drive){
    if(drive){
        outb(BAR ATA_DRIVE_HEAD, drive ? 0xB0 : 0xA0);
    }
    for(uint32_t i = 0; i < 15; i++){
        uint8_t _ = inb(BAR2 ATA_ALT_STATUS);
    }
    uint8_t status = inb(BAR2 ATA_ALT_STATUS);
    if(!ATA_BSY(status) || ATA_DRQ(status)) return 1;
    return 0;
}
//return index of first free drive descriptor
uint32_t find_free_drive(){
    for(uint32_t i = 0; i < 32; i++){
        if(drives[i].type == TYPE_NULL){
            return i;
        }
    }
}

void ide_init(uint32_t BARS[5]){
    uint32_t index = find_free_drive();
    drives[index].type = TYPE_IDE;
    for(int i = 0; i < 5; i++){
        drives[index].BARs[i] = BARS[i];
    }
    uint32_t paddr = api(MODULE_API_PMALLOC64K);
    drives[index].PRDT = api(MODULE_API_KMALLOC_PADDR, paddr, 16);
    api(MODULE_API_PRINT, MODULE_NAME, "Vaddr: %x, Paddr: %x", drives[index].PRDT, paddr);
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
    
    vfile_t *pci_drive_dir = fopen(api, "/dev/pci/disk/");
    vfile_t **dir_data = (pci_drive_dir->access.data.ptr);
    for(uint32_t i = 0; dir_data[i]; i++){
        vfile_t *current_file = dir_data[i];
        uint32_t class = 0;
        fread(api, current_file, &class, 0x8, 1);
        uint32_t progif = class >> 8 & 0xff;
        class >>= 16;
        api(MODULE_API_PRINT, MODULE_NAME, "Class: %x, %x\n", class, progif);
        if(class == 0x101 && PCI_IDE_SUPPORT_DMA(progif)){ //in this house, we only support DMA.
            uint32_t BARs[5] = {0};
            fread(api, current_file, BARs, 0x10, 5);
            if(PCI_IDE_NATIVE(progif)){
                BARs[0] = ATA_PRIMARY_BUS;
                BARs[1] = ATA_PRIMARY_BUS + 0x206;
            }
            if(PCI_IDE_SECONDARY_NATIVE(progif)){
                BARs[2] = ATA_SECONDARY_BUS;
                BARs[3] = ATA_SECONDARY_BUS + 0x206;
            }
            ide_init(BARs);
        }
    }
    return;
}


void fini(){
    //destroy all objects, free memory, and exit
    return; //nothing to do (yet)
}