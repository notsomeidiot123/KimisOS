# Kimi's OS

A totally FR Operating System made by yours truly!

## Details

Kimi's OS is an x86 32-bit Modular operating system created as a hobby project. It uses a custom-made bootloader also made by me. 

My goal for this project is to make something modular in design that can be swapped out as it's running with other or updated modules without a full reboot. Sadly updating the kernel will still require a full restart.

I'm focused on making this as stable as possible, and (eventually) secure. Right now there's not many security features, but it's a work in progress. If anyone that knows anything about cybersecurity has any suggestions on how to implement security features, please open an issue!

## Features

- Custom legacy bootloader, which loads the Kernel (`/kernel.elf`), and the Initial boot and FS drivers (`/idm.elf` and `/ifsm.elf`)
- Support for PCI (not PCIe)
- Custom VFS, with built-in explicit support for mount points, devices, directories, and pipes.
- Support for ELF executable files
- Loading Kernel modules
- Serial debugging (Logging messages over COM0)

## Instructions

So far, I only officially support using an emulator.

To build and run, first you must run 

`make init`

to create necesary folders and files, followed by 

`make`

to build and run in qemu. Running will ask you for your `sudo` password, this is because of this line in the makefile: `sudo mount image.bin mount`. This line will simply mount the kernel image to a LOCAL directory made by `make init` called `mount`. It will not override anything outside of the root directory OF THIS PROJECT. When this project is more mature and supports bare metal better (Tested with a machine I can test on), I'll offer pre-compiled images on the github releases tab.