# Kimi's Bootloader Documentation

## Basic Information

Kimi's Bootloader is a custom booting solution and the default bootloader for Kimi's OS on legacy BIOS systems. It loads the default x86 GDT to be used by the Kernel, along with any disk or filesystem modules which may be present in the files `ddrv.mod` and `dfs.mod`

The bootloader also sets the video mode to the closest mode present to Standard definition, and obtains the BIOS memory map to be passed to the Kernel. 

### A20 Line

Due to the fact that many x86 PCs POST without enabling the A20 line, the bootloader attempts to enable it through 3 distinct methods, starting with 

1. Enabling through BIOS

Kimi's Bootloader will first attempt to enable the A20 line via the BIOS before attempting any other method of enabling the A20 line. This method is the least likely to have any unintended side effects, which is why it is the first of the three methods to be attempted

2. Enabling via Fast A20 method

The bootloader then attempts to enable the A20 by using the fast A20 method

3. Enabling through keyboard

If all else fails, the bootloader will attempt to enable the a20 line by using the legacy way of enabling the A20 line - by using Intel's 8042 Keyboard controller. This is the most likely to succeed on any chipsets with a PS/2 controller still present, but will fail if one is not. 

### GDT

While any GDT loaded for the Kernel will work, once the Kernel wants to load user processes, it will fail to locate user segments in the GDT if the one loaded is not the one expected. Because of this, the Kernel will need to load it's own if the one it wants is not loaded by this bootloader. To allow for detection, the key "0xaa55" is written at the end of the GDT, marking the last segment invalid, but allowing for the detection of the correct GDT being pre-loaded. This detection will only occurr during boot time, and unexpected writes will cause the Kernel to crash.

### Error Codes

|Error Code|Description|
|----------|-----------|
|1         |A20 Line could not be enabled|