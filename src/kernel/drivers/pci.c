#include "pci.h"
#include "../shared/kstdlib.h"
#include "cpuio.h"

#define MODULE_NAME "PCI"

uint16_t pci_read_config(uint32_t bus, uint32_t slot, uint32_t func, uint8_t offset){
    uint32_t address = (uint32_t)((bus << 16) | (slot << 11) | (func << 8) | (offset & 0xfc) | ((uint32_t)0x80000000));
    outl(0xcf8, address);
    uint16_t tmp = (uint16_t)((inl(0xcfc) >> ((offset & 2) * 8)) & 0xffff);
    return tmp;
}

uint16_t pci_get_vendor(uint8_t bus, uint8_t slot){
    uint16_t vendor = 0, device = 0;
    vendor = pci_read_config(bus, slot, 0, 0);
    if(vendor != 0xffff){
        device = pci_read_config(bus, slot, 0, 2);
        uint16_t class = pci_read_config(bus, slot, 0, 0xa);
        mlog(MODULE_NAME, "Device found! Vendor: %x, Device: %x, Class: %x\n", MLOG_PRINT, vendor, device, class);
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
    for(uint32_t i = 0 ; i < 256; i++){
        pci_enumerate_bus(i);
    }
}