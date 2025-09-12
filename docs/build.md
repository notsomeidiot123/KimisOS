# How to build this project

## To build and run in qemu:

simply run `make` That's it. It'll ask for your password to run `sudo mount image.bin`, to create a few directories (and eventually move in files that will be needed for installation/running). If you don't feel comfortable doing this through make, cause you don't trust the build script or something, you can run

`make tools && make bootloader && make kernel`

followed by `cp bin/bootloader.bin image.bin`, `qemu-img resize -f raw image.bin 64M`, and `diskwrite idm.elf ifsm.elf kernel.elf -o image.bin`, which will then allow you to mount image.bin. Then, run `mk {mount}/mod {mount}/bin {mount}/sys` and copy and files that need to be copied into their respective folders (currently there are none other than `idm.elf`, `ifsm.elf`, and `kernel.elf`)

You MUST run the `diskwrite` tool BEFORE mounting, as it sets up the filesystem for the bootloader, and copies `idm.elf`, `ifsm.elf` and `kernel.elf` in a way that can be read by the bootloader.

To run, just use the command `qemu-system-i386 -hda image.bin {settings}`, with whatever settings you choose. I recommend using at least 32 megabytes of memory, and if you would like to see the kernel logs as they are writen to serial, with the option `-serial mon:stdio`

## To build, but NOT run in qemu

You could always just run `make` and close qemu immediately, or you could run `make mount`. It'll ask for your password again for the above reason, but you can just follow the above steps, minus running qemu if you don't feel comfortable letting the build script handle it. Again, it is imperitive that you run `diskwrite` as specified, or else the bootloader won't be able to find the files (as linux defaults to long file names, and my bootloader does not support them, and the short filenames are in a different style than expected (eg `kernel.elf` becomes `KERNEL  ELF`))
