#include "pci.h"
#include "../shared/kstdlib.h"
#include "cpuio.h"
#include "../system/vfs.h"
#include "../shared/string.h"
#include "../shared/memory.h"
#define MODULE_NAME "PCI"

char *classes[][20] = {
    {},
    {"scsi", "ide", "floppy", "ipi", "raid", "ata", "sata", "sascsi", "nvm"},
    {"eth", "tring", "fddi", "atm", "isdn", "wordfip", "picmg", "infband", "fabric"},
    {"vga", "xga", "nvga3d"},
    {},
    {},
    {"host", "isa", "eisa", "mca", "pci", "pcmcia", "nubus", "cardbus"},
    {},
    {"pic", "dma", "timer", "rtc"}
    
};

uint16_t pci_read_config(uint32_t bus, uint32_t slot, uint32_t func, uint8_t offset){
    uint32_t address = (uint32_t)((bus << 16) | (slot << 11) | (func << 8) | (offset & 0xfc) | ((uint32_t)0x80000000));
    outl(0xcf8, address);
    uint16_t tmp = (uint16_t)((inl(0xcfc) >> ((offset & 2) * 8)) & 0xffff);
    return tmp;
}

void pci_read_file(vfile_t *file, uint32_t *buffer, uint32_t offset, uint32_t count){
    uint8_t bus = file->id >> 16;
    uint8_t func = file->id>> 8 & 0xff;
    uint8_t slot = file->id & 0xff;
    for(uint32_t i = 0; i < count/2; i++){
        buffer[i] = pci_read_config(bus, slot, func, offset + i*2) | (pci_read_config(bus, slot, func, offset + i * 2 + 2) << 16);
    }
    return;
}
void pci_write_file(vfile_t *file, uint32_t *buffer, uint32_t offset, uint32_t count){
    return;
}
void pci_make_file(uint32_t class, uint8_t bus, uint8_t slot, uint8_t func){
    char *str = kmalloc(1);
    memcpy("/dev/pci/", str, 9);
    char *dev = classes[class >> 8][class & 0xf];
    switch(class >> 8){
        case 0x1://PCI class 0x1 is disks
        strcpy("disk/", str + strlen(str));
        break;
        case 0x2://PCI class 0x2 is network controllers
        strcpy("net/", str + strlen(str));
        break;
        case 0x3://PCI class 0x3 is video controllers
        strcpy("video/", str + strlen(str));
        break;
        case 0x6://PCI class 0x6 is bridge controllers
        strcpy("bridge/", str + strlen(str));
        break;
    }
    strcpy(dev,str+strlen(str));
    // vfile_t *file = fcreate(str, VFILE_DEVICE, pci_write_file, pci_read_file);
    char num[10];
    char cpy[256];
    vfile_t *file = 0;
    uint8_t tries = 0;
    while(file == 0){
        itoa(tries++, num, 10);
        strcpy(str, cpy);
        strcpy(num, cpy + strlen(cpy));
        file = fcreate(cpy, VFILE_DEVICE, pci_write_file, pci_read_file);
    }
    file->id = (uint32_t)slot | ((uint32_t)func << 8) | ((uint32_t)bus << 16);
}

void pci_read_funcs(uint8_t bus, uint8_t slot){
    for(uint16_t i = 1; i < 256; i++){
        //read each function of device
        uint32_t vendor = pci_read_config(bus, slot, i, 0);
        if(vendor != 0xffff){
            uint16_t device = pci_read_config(bus, slot, i, 2);
            uint16_t header = pci_read_config(bus, slot, i, 0xe);
            uint16_t class = pci_read_config(bus, slot, i, 0xa);
            mlog(MODULE_NAME, "Device found! Func: %d, Vendor: %x, Device: %x, Class: %x, Header: %x\n", MLOG_PRINT, i, vendor, device, class, header);
            pci_make_file(class, bus, slot, i);
        }
        
    }
}

uint16_t pci_get_vendor(uint8_t bus, uint8_t slot){
    uint16_t vendor = 0, device = 0;
    vendor = pci_read_config(bus, slot, 0, 0);
    if(vendor != 0xffff){
        device = pci_read_config(bus, slot, 0, 2);
        uint16_t header = pci_read_config(bus, slot, 0, 0xe);
        uint16_t class = pci_read_config(bus, slot, 0, 0xa);
        mlog(MODULE_NAME, "Device found! Vendor: %x, Device: %x, Class: %x, Header: %x\n", MLOG_PRINT, vendor, device, class, header);
        if(header & 0x80){
            pci_read_funcs(bus, slot);
        }
        pci_make_file(class, bus, slot, 0);
    }
    return vendor;
}

void pci_enumerate_bus(uint8_t bus){
    for(uint32_t i = 0; i < 32; i++){
        uint32_t vendor = pci_get_vendor(bus, i);
    }
}

void pci_init(){
    mlog(MODULE_NAME, "Enumerating PCI Buses\n", MLOG_PRINT);
    vfile_t *file = fcreate("dev/pci/", VFILE_DIRECTORY, kmalloc(1), 1);
    fcreate("dev/pci/disk", VFILE_DIRECTORY, kmalloc(1), 1);
    fcreate("dev/pci/net", VFILE_DIRECTORY, kmalloc(1), 1);
    fcreate("dev/pci/video", VFILE_DIRECTORY, kmalloc(1), 1);
    fcreate("dev/pci/bridge", VFILE_DIRECTORY, kmalloc(1), 1);
    for(uint32_t i = 0 ; i < 256; i++){
        pci_enumerate_bus(i);
    }
}