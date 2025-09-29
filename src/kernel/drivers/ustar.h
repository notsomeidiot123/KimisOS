#pragma once
#include "stdint.h"
#include "../shared/kstdlib.h"
typedef struct{
    char filename[100];
    char mode[8];
    char owner_uid[8];
    char group_uid[8];
    char fsize[12];
    char last_modified[12];
    char checksum[8];
    uint8_t type;
    char linked_file[100];
    char ustar[6];
    char version [2];
    char owner_name[32];
    char group_name[32];
    char dev_major[8];
    char dev_minor[8];
    char filename_prefix[155];
    char reserved[12];
}__attribute__((packed)) USTAR_file_t;

enum USTAR_TYPES{
    FILE,
    SYMLINK,
    CHARDEV,
    BLOCKDEV, 
    DIR,
    PIPE
};
int read_initrd(initrd_t *ptr);