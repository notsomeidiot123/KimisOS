#include<stdint.h>

typedef struct{
    uint32_t devid;
    enum DeviceType{
        NULL_TYPE,
        DEV_TYPE_DISK,
        DEV_TYPE_AUDIO,
        DEV_TYPE_KBD,
        DEV_TYPE_MOUSE,
        DEV_TYPE_VIDEO,
        DEV_TYPE_OTHER,
        DEV_TYPE_NAT,
    }device_type;
    uint32_t driver_pid;
}device_t;
typedef struct{
    uint64_t start_sector; //Register or sector (depending on command and or device type)
    uint64_t total_sectors;//hard disks only
    volatile void *buffer;
    uint32_t buffer_size;//size of each object in buffer
    uint32_t buffer_count;//Number of objects of size buffer_size in buffer
    enum DeviceCommand{
        NULLOP,
        DEVICE_READ,
        DEVICE_WRITE,
        DEVICE_INIT,
        DEVICE_COMMAND,
        DEVICE_SET_MMAP,
        DEVICE_SET_DMA,
    }command;
    uint32_t requesting_pid;
}device_packet_t;