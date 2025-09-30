#pragma once
#include <stdint.h>

typedef struct partition{
    uint8_t attributes;
    uint8_t chs_start[3];
    uint8_t type;
    uint8_t chs_end[3];
    uint32_t lba_start;
    uint32_t lba_size;
}__attribute__((packed))partition_t;

#define MBR_MAGIC 0xaa55

typedef struct mbr{
    char code[440];
    char id[4];
    char res[2];
    partition_t partitions[4];
    uint16_t magic;
}__attribute__((packed)) mbr_t;