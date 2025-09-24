#include<stdint.h>
#include "modlib.h"
#include "../kernel/drivers/cpuio.h"
#include "../kernel/shared/string.h"
#include "disk_driver.h"

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

#define ATA_DATA          0
#define ATA_ERROR         1
#define ATA_FEATURES      1
#define ATA_SECTOR_COUNT  2
#define ATA_SECTOR_NUMBER 3
#define ATA_CYLINDER_LOW  4
#define ATA_CYLINDER_HIGH 5
#define ATA_DRIVE_HEAD    6
#define ATA_STATUS        7
#define ATA_COMMAND       7

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
#define ATA_MASTER 0xa0
#define ATA_SLAVE 0xb0
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

module_t module_data = {
    init,
    0xfae00000,
    MODULE_NAME,
    0,
    0,
    0,
    fini
};

uint8_t native_ide_present = 0;

void fini();

enum DRIVE_TYPE{
    TYPE_NULL,
    TYPE_TMP,
    TYPE_IDE,
    TYPE_AHCI,
    TYPE_NVME,
    TYPE_USB,
    TYPE_FLOPPY, //Here, Navya, just for you
};
uint32_t volatile transferring_disk_index = -1;
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

//return 1 if ready, 0 if not
int ata_ready(uint32_t BAR, uint32_t BAR2, uint8_t drive){
    static uint16_t last_disk = 0;
    static uint16_t last_bar = 0;
    if(last_disk != drive && last_bar != BAR){
        last_disk = drive;
        last_bar == BAR;
        outb(BAR + ATA_DRIVE_HEAD, drive);
    }
    for(uint32_t i = 0; i < 4; i++){
        uint8_t _ = inb(BAR2 + ATA_STATUS);
    }
    uint8_t status = inb(BAR2 ATA_ALT_STATUS);
    return (status >> 7) == 0;
}

void ata_reset(uint16_t bar1){
    outw(bar1 ATA_DEVICE_CONTROL, 0x4);
    for(uint32_t i = 0; i < 4096; i++){
        asm volatile("" ::: "memory");
    }
    outw(bar1 ATA_DEVICE_CONTROL, 0x0);
}
//return index of first free drive descriptor
uint32_t find_free_drive(){
    for(uint32_t i = 0; i < 32; i++){
        if(drives[i].type == TYPE_NULL){
            return i;
        }
    }
}

int ata_read(vfile_t *file, uint8_t *ptr, uint32_t offset, uint32_t count){
    if(count == 0){
        return -1;//prevent from accidentally reading 65536 * 512 bytes
    }
    
    drive_t drive = drives[file->mount_id];
    
    uint16_t dmabar = drive.BARs[4] & (uint32_t)(~3);
    
    uint32_t status = inb(dmabar + 2);
    api(MODULE_API_PRINT, MODULE_NAME, "status: %x\n", status);
    PRD_T *prdt = drive.PRDT;
    if(count == 0) return 0;
    uint32_t count_pgs = ((count + 4096) / 4096);
    
    for(int i = 0; i < count_pgs; i++){
        prdt[i].address = api(MODULE_API_PADDR, ptr + (i * (1 << 12)));
        prdt[i].byte_count = (i == (count_pgs - 1)) ? count % (1 << 12) : 1 << 12;
        prdt[i].reserved = (1 << 31) * (i == (count_pgs - 1));
    }
    outb(dmabar + 2, 0x6);
    outl(dmabar + 4, api(MODULE_API_PADDR, prdt));
    outb(dmabar, 0x8);
    
    while(!ata_ready(drive.BARs[0] >> 2, drive.BARs[1] >> 2, drive.flags.slave));
    
    outb(drive.BARs[1], 0x00);
    
    uint64_t lba = (offset >> 9);
    uint8_t *lba_arr = &lba;
    outb(drive.BARs[0] + ATA_DRIVE_HEAD, 0x40 | (drive.flags.slave << 4) | ((lba >> 24) * !drive.flags.huge));
    
    if(count >> 9 == 0){
        return 0;
    }
    
    if(drive.flags.huge){
        outb(drive.BARs[0] + ATA_SECTOR_COUNT, (count >> 9) >> 8);
    }else{
        outb(drive.BARs[0] + ATA_SECTOR_COUNT, (count >> 9));
    }
    for(uint32_t i = 0; i < 3; i++){
        outb(drive.BARs[0] + ATA_LBA_LOW + i, lba_arr[i + 3 * drive.flags.huge]);
    }
    if(drive.flags.huge){
        outb(drive.BARs[0] + ATA_SECTOR_COUNT, (count >> 9) & 0xff);
        for(uint32_t i = 0; i < 3; i++){
            outb(drive.BARs[0] + ATA_LBA_LOW + i, lba_arr[i]);
        }
        puts(api, MODULE_NAME, "bleh\n");
        outb(drive.BARs[0] + ATA_COMMAND, ATA_CMD_READ_DMA_EXT);
    }else{
        outb(drive.BARs[0] + ATA_COMMAND, ATA_CMD_READ_DMA);
    }
    //issue read command to drive
    
    transferring_disk_index = file->mount_id;
    
    outb(dmabar, 0x9);
    // while(!ata_ready(drive.BARs[0] >> 2, drive.BARs[1] >> 2, drive.flags.slave));
    while(transferring_disk_index != -1);
    // while (!(inb(dmabar + 2) & 0x01));
    outb(dmabar, 0x0);
    return 0;
}

cpu_registers_t *int_handler(cpu_registers_t * regs){
    puts(api, MODULE_NAME, "Interrupt called");
    drive_t drive = drives[transferring_disk_index];
    uint16_t dmabar = drive.BARs[4] & (uint32_t)(~3);
    uint32_t status = inb(dmabar + 2);
    transferring_disk_index = -1;
    if(status & 0x4) return regs;
    return regs;
}


int ata_write(vfile_t *file, void *ptr, uint32_t offset, uint32_t count){
    
}



//return 255 if err/ does not exist
//return 0 if is ATA drive
//return 1 if is ATAPI
//return 2 if is SATA
//return 3 if is SATAPI
uint8_t ata_identify(uint32_t index, uint16_t disk){
    uint16_t bar0 = drives[index].BARs[0] >> 2;
    uint16_t bar1 = drives[index].BARs[1] >> 2;
    if(!ata_ready(bar0, bar1, disk)){
        return -1;
    }
    // outb(bar0 ATA_DRIVE_HEAD, disk);
    outb(bar0 + ATA_SECTOR_COUNT, 0);
    outb(bar0 + ATA_LBA_LOW, 0);
    outb(bar0 + ATA_LBA_MID, 0);
    outb(bar0 + ATA_LBA_HIH, 0);
    outb(bar0 + ATA_COMMAND, ATA_CMD_IDENTIFY);
    
    uint8_t status = inb(bar0 + ATA_STATUS);
    if (status == 0x00) return -1; // No device

    while (ATA_BSY(status)) {
        status = inb(bar0 + ATA_STATUS);
        if (ATA_ERR(status)) return -1;
    }

    // Check device signature for non-ATA devices
    uint8_t lba_mid  = inb(bar0 + ATA_LBA_MID);
    uint8_t lba_high = inb(bar0 + ATA_LBA_HIH);
    if (lba_mid != 0 || lba_high != 0) {
        return -1; // Not an ATA device or no device
    }

    // Check for DRQ
    status = inb(bar0 + ATA_STATUS);
    if (!(ATA_DRQ(status))) return -1;
    
    uint16_t identify[256] = {0};
    for(uint16_t i = 0; i < 256; i++){
        identify[i] = inw(bar0 + ATA_DATA);
    }
    // drives[index].size_sectors
    uint32_t lba48 = (identify[83] & (1 << 10));
    // drives[index].flags.huge = lba48 ? 1 : 0;
    drives[index].size_sectors = !lba48 ? (identify[60] | (identify[61] << 16)) : ((uint64_t)(identify[100]) | (uint64_t)(identify[101] << 16) | ((uint64_t)identify[102] << 32));
    api(MODULE_API_PRINT, MODULE_NAME, "Sector Count: %x\n", drives[index].size_sectors);
    char fname[32];
    strcpy("/dev/disk/ide", fname);
    uint8_t ata_drives = 0;
    for(uint32_t i  = 0; i < 32; i++){
        if(drives[i].type == TYPE_IDE) ata_drives++;
    }
    drives[index].type = TYPE_IDE;
    itoa(ata_drives, fname + strlen(fname), 10);
    vfile_t *new_file = fcreate(api, fname, VFILE_DEVICE, ata_write, ata_read);
    new_file->mount_id = index;
    free(api, fname);
    return 1;
}

void ide_init(uint32_t BARS[5]){
    uint32_t primary_index = find_free_drive();
    drives[primary_index].type = TYPE_TMP;
    uint32_t primary_slave_index = find_free_drive();
    drives[primary_slave_index].type = TYPE_TMP;
    uint32_t secondary_index = find_free_drive();
    drives[secondary_index].type = TYPE_TMP;
    uint32_t secondary_slave_index = find_free_drive();
    drives[secondary_slave_index].type = TYPE_TMP;
    for(int i = 0; i < 2; i++){
        drives[primary_index].BARs[i] = BARS[i];
        drives[primary_slave_index].BARs[i] = BARS[i];
        drives[secondary_index].BARs[i] = BARS[i+2];
        drives[secondary_slave_index].BARs[i] = BARS[i+2];
    }
    drives[primary_index].BARs[4] = BARS[4];
    drives[primary_slave_index].BARs[4] = BARS[4];
    drives[secondary_index].BARs[4] = BARS[4];
    drives[secondary_slave_index].BARs[4] = BARS[4];
    
    uint32_t prdt_primary_paddr = api(MODULE_API_PMALLOC64K);
    uint32_t prdt_secondary_paddr = api(MODULE_API_PMALLOC64K);
    drives[primary_index].PRDT = (void *)api(MODULE_API_KMALLOC_PADDR, prdt_primary_paddr, 16);
    drives[secondary_index].PRDT = (void *)api(MODULE_API_KMALLOC_PADDR, prdt_secondary_paddr, 16);
    //now call ATA IDENTIFY
    uint8_t master_status = ata_identify(primary_index, ATA_MASTER);
    uint8_t slave_status = ata_identify(primary_slave_index, ATA_SLAVE);
    if(master_status){
        drives[primary_index].type = TYPE_NULL;
    }
    if(slave_status){
        drives[primary_slave_index].type = TYPE_NULL;
    }
    master_status = ata_identify(secondary_index, ATA_MASTER);
    slave_status = ata_identify(secondary_slave_index, ATA_SLAVE);
    if(master_status){
        drives[secondary_index].type = TYPE_NULL;
    }
    if(slave_status){
        drives[secondary_slave_index].type = TYPE_NULL;
    }
    // outb(BARS[4] >> 1, 0x0);
    // int volatile tt = 0;
    // while(tt < 50000){
    //     tt++;
    // }
    
}

void init(KOS_MAPI_FP module_api, uint32_t api_version){
    api = module_api;
    api(MODULE_API_PRINT, MODULE_NAME, "KIDM Storage Driver Module v0.1.0\nSupported interfaces: \n- PATA\n");
    
    int status = api(MODULE_API_REGISTER, &module_data);
    if(status){
        api(MODULE_API_PRINT, MODULE_NAME, "Failed to register module, exiting\n");
        return;
    }
    api(MODULE_API_ADDINT, 15, module_data.key, int_handler);
    api(MODULE_API_ADDINT, 14, module_data.key, int_handler);
    // api(MODULE_API_ADDINT, 0, module_data.key, int_handler);
    vfile_t *pci_drive_dir = fopen(api, "/dev/pci/disk/");
    vfile_t **dir_data = (pci_drive_dir->access.data.ptr);
    for(uint32_t i = 0; dir_data[i]; i++){
        vfile_t *current_file = dir_data[i];
        uint32_t class = 0;
        fread(api, current_file, &class, 0x8, 1);
        uint32_t progif = class >> 8 & 0xff;
        class >>= 16;
        api(MODULE_API_PRINT, MODULE_NAME, "Class: %x, %x\n", class, progif);
        if(class == 0x101 && PCI_IDE_SUPPORT_DMA(progif) && !native_ide_present){ //in this house, we only support DMA.
            uint32_t BARs[5] = {0};
            fread(api, current_file, BARs, 0x10, 5);
            if(PCI_IDE_NATIVE(~progif)){
                native_ide_present = 1;
                BARs[0] = (ATA_PRIMARY_BUS) << 2 | 1;
                BARs[1] = (ATA_PRIMARY_BUS + 0x206) << 2 | 1;
            }
            if(PCI_IDE_SECONDARY_NATIVE(~progif)){
                BARs[2] = (ATA_SECONDARY_BUS << 2) | 1;
                BARs[3] = (ATA_SECONDARY_BUS + 0x206) << 2 | 1;
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