#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define FILE_END    0xFFFFFFFF
#define FILE_FREE   0x00000000

#define FAT_ATTR_READONLY   0x01
#define FAT_ATTR_HIDDEN     0x02
#define FAT_ATTR_SYSTEM     0x04
#define FAT_ATTR_VOLID      0x08
#define FAT_ATTR_DIR        0x10
#define FAT_ATTR_ARCHIVE    0x20
#define FAT_LFN             FAT_ATTR_READONLY | FAT_ATTR_HIDDEN | FAT_ATTR_SYSTEM | FAT_ATTR_VOLID

typedef uint8_t *buffer_t;

typedef struct{
    uint8_t jmp[3];
    char oem[8];
    uint16_t bytes_per_sector;
    uint8_t sectors_per_cluster;
    uint16_t reserved_sectors;
    uint8_t fat_count;
    uint16_t root_dir_entries;
    uint16_t sectors_in_volume_small;
    uint8_t media_descriptor;
    uint16_t sectors_per_fat_small;
    uint16_t sectors_per_track;
    uint16_t heads_per_volume;
    uint32_t partition_start;
    uint32_t large_sector_count;
    
    uint32_t sectors_per_fat;
    uint16_t flags;
    uint16_t fat_verno;
    uint32_t root_dir_cluster;
    uint16_t fsinfo_sector;
    uint16_t backup_boot_sector;
    uint8_t reserved[12];
    uint8_t drive_no;
    uint8_t nt_flags;
    uint8_t sig;
    uint32_t vol_uuid;
    char volid[11];
    char sysid[8];
}__attribute__((packed))fat_bpb_t;

typedef struct{
    uint32_t lead_sig;
    uint8_t reserved[480];
    uint32_t sig0;
    uint32_t last_free_cluster;
    uint32_t cluster_search_start;
    uint8_t reserved0[12];
    uint32_t trail_sig;
}fsinfo_t;

typedef struct{
    fat_bpb_t bpb;
    fsinfo_t fsinfo;
    uint32_t *file_table;
}fat_info;

typedef struct{
    char filename[8];
    char ext[3];
    uint8_t attributes;
    uint8_t ntflags;
    uint8_t creation_time;
    uint16_t create_time;
    uint16_t create_date;
    uint16_t last_access;
    uint16_t start_cluster_high;
    uint16_t last_modification_time;
    uint16_t last_modification_date;
    uint16_t start_cluster_low;
    uint32_t size_bytes;
}__attribute__((packed)) file_t;

int verbose = 0;

int read_sector(FILE *file, uint32_t sector_start, uint32_t count, buffer_t buf){
    verbose && printf("Attempting to read bytes 0x%x - 0x%x\n", sector_start * 512, sector_start * 512  + count * 512);
    // rewind(file);
    int result = fseek(file, sector_start * 512, SEEK_SET);
    if(result){
        return result;
    }
    fflush(stdout);
    memset(buf, 0, count * 512);
    result = fread(buf, 512, count, file);
    return result;
}

int write_sector(FILE *file, uint32_t sector_start, uint32_t count, buffer_t buf){
    // printf("pre seek\n");
    // rewind(file);
    int result = fseek(file, sector_start * 512, SEEK_SET);
    if(result){
        printf("Error during seek\n");
        return result;
    }
    // printf("pre write\n");
    result = fwrite(buf, 512, count, file);
    return result;
}

uint32_t find_free_cluster(fat_info *info){
    verbose && printf("finding free cluster\n");
    uint32_t start = info->fsinfo.cluster_search_start;
    if(start < 2){
        start = 2;
    }
    for(int i = start; i < (info->bpb.sectors_per_fat * info->bpb.bytes_per_sector)/4; i++){
        // printf("%d, %p\n", i);
        if(info->file_table[i]) continue;
        return i;
    }
}

void write_cluster_value(fat_info *info, uint32_t value, uint32_t cluster){
    printf("writing cluster :3\n");
    for(int i = 0; i < info->bpb.fat_count; i++){
        info->file_table[cluster + i * info->bpb.sectors_per_fat * (info->bpb.bytes_per_sector/4)] = value;
    }
}

int read_file(fat_info *info, file_t *file, buffer_t *buffer, size_t size, FILE *disk){
    if(!file){
        uint32_t first_cluster = info->bpb.root_dir_cluster;
        uint32_t next_cluster = first_cluster;
        // while
        return 1;
    }
    uint32_t first_cluster = file->start_cluster_low | (file->start_cluster_high << 16);
    uint32_t clusters  = (size / (info->bpb.bytes_per_sector * info->bpb.sectors_per_cluster )) + 1;
    uint8_t *new_buffer = calloc(1, clusters * (info->bpb.bytes_per_sector * info->bpb.sectors_per_cluster));
    
}
//write size bytes to file from buffer. no value may be zero or null.
int write_file(fat_info *info, file_t *file, buffer_t *buffer, size_t size, FILE *disk){
    if(!file){
        return 1;
    }
    uint32_t first_cluster = file->start_cluster_low | (file->start_cluster_high << 16);
    uint32_t clusters  = (size / (info->bpb.bytes_per_sector * info->bpb.sectors_per_cluster )) + 1;
    uint8_t *new_buffer = calloc(1, clusters * (info->bpb.bytes_per_sector * info->bpb.sectors_per_cluster ));
    memcpy(new_buffer, buffer, size);
    uint32_t cluster = first_cluster;
    for(uint32_t i = 0; i < clusters; i++){
        write_sector(disk, info->bpb.reserved_sectors + info->bpb.partition_start + (cluster * info->bpb.sectors_per_cluster), info->bpb.sectors_per_cluster, new_buffer + i * info->bpb.sectors_per_cluster * info->bpb.bytes_per_sector);
        uint32_t next_cluster = info->file_table[cluster];
        if(next_cluster == -1){
            next_cluster = find_free_cluster(info);
            write_cluster_value(info, next_cluster, cluster);
        }
        cluster = next_cluster;
    }
    write_cluster_value(info, -1, cluster);
    return size * info->bpb.bytes_per_sector * info->bpb.sectors_per_cluster;
}

int create_file(char *path, fat_info *info, FILE *file){
    
}

int fat_write_back(FILE *file, fat_info *info){
    int res = 0;
    if(res = write_sector(file, info->bpb.reserved_sectors, info->bpb.sectors_per_fat * info->bpb.fat_count, (buffer_t)info->file_table) == 0){
        verbose && printf("Error during write: %d\n", res);
    }
}

int detect_repair_fs(FILE *file, fat_info *info){
    //perform sanity checks and set data if wrong
    buffer_t boot_sector = malloc(512);
    read_sector(file, 0, 1, boot_sector);
    info->bpb = *(fat_bpb_t *)boot_sector;
    verbose && printf("\033[0;33mBytes per sector: %d\n", info->bpb.bytes_per_sector);
    verbose && printf("Sectors per cluster: %x\n", info->bpb.sectors_per_cluster);
    info->bpb.bytes_per_sector += 512 * info->bpb.bytes_per_sector == 0;
    info->bpb.sectors_per_cluster += 8 * info->bpb.sectors_per_cluster == 0;
    verbose && printf("Reserved sectors %d\n", info->bpb.reserved_sectors);
    info->bpb.reserved_sectors += 32 * info->bpb.reserved_sectors == 0;
    verbose && printf("Root directory cluster: %d\n", info->bpb.root_dir_cluster);
    info->bpb.root_dir_cluster += 2 * info->bpb.root_dir_cluster == 0;
    memcpy(boot_sector, &info->bpb, sizeof(fat_bpb_t));
    write_sector(file, 0, 1, boot_sector);
    

    verbose && printf("\033[1;31mAllocating File Table: 0x%x bytes\033[0m\n",  info->bpb.sectors_per_fat * info->bpb.fat_count * info->bpb.bytes_per_sector);
    info->file_table = malloc(info->bpb.sectors_per_fat * info->bpb.fat_count * info->bpb.bytes_per_sector);
    read_sector(file, info->bpb.reserved_sectors, info->bpb.sectors_per_fat * info->bpb.fat_count, (buffer_t)info->file_table);
}

int main(int argc, char **argv){
    char **files = malloc(512);
    int filec = 0;
    FILE *output_file = 0;
    
    fat_info info = {};
    
    if(argc == 1 || !strcmp(argv[1], "-h")){
        printf("write files to virtual disk\n");
        printf("Usage:\ndiskwrite <input files> -o <output file>");
        printf("\n-h: Help");
        printf("\n-o: Specify output file\n");
        printf("-v: Verbose\n");
    }
    for(int i = 1; i < argc; i++){
        if(!strcmp(argv[i], "-o")){
            i++;
            if(i >= argc){
                printf("Error: No output file specified!\n");
            }
            verbose && printf("OF: %s\n", argv[i]);
            output_file = fopen(argv[i], "r+");
            continue;
        }
        if(!strcmp(argv[i], "-v")){
            verbose = 1;
            continue;
        }
        files[filec++] = argv[i];
        verbose && printf("IF: %s\n", argv[i]);
    }
    if(filec == 0){
        printf("Nothing to do, no files to write\n");
        return 0;
    }
    detect_repair_fs(output_file, &info);
    for(int i = 0; i < filec; i++){
        printf("\033[1;34mWriting file %d in list: %s\033[0m\n", i, files[i]);
        create_file(files[i], &info, output_file);
    }
    fat_write_back(output_file, &info);
    verbose && printf("Finished. Exiting...\n");
    return 0;
}