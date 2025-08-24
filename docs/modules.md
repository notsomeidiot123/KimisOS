#define MODULE_API_REGISTER 0
#define MODULE_API_ADDFUNC 1
#define MODULE_API_DELFUNC 2
#define MODULE_API_ADDINT 3
#define MODULE_API_DELINT 4
#define MODULE_API_PRINT 5
#define MODULE_API_READ 6 //read from virtual file
#define MODULE_API_WRITE 7 //write to virtual file
#define MODULE_API_CREAT 8 //create a virtual file and assigns it to the the proper module (requires having a read and write function passed)
#define MODULE_API_DELET 9

# Kimi's OS Kernel Module API

On boot time, the bootloader will search the filesystem for the files `idm.elf` and `ifsm.elf` in the root directory. These are the only two modules loaded at boot time. Afterwards, the kernel will search the folder 