#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define FILE_END    0xFFFFFFFF
#define FILE_FREE   0x00000000

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

int read_sector(FILE *file, uint32_t sector_start, uint32_t count, buffer_t buf){
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
    int result = fseek(file, sector_start * 512, SEEK_SET);
    if(result){
        return result;
    }
    result = fwrite(buf, 512, count, file);
    return result;
}

int detect_repair_fs(FILE *file, fat_info *info){
    buffer_t boot_sector = malloc(512);
    read_sector(file, 0, 1, boot_sector);
    memcpy(&(info->bpb), boot_sector, 90);
    buffer_t fat = malloc(info->bpb.sectors_per_fat * info->bpb.fat_count * info->bpb.bytes_per_sector);
    printf("allocated 0x%x bytes for fat\n", (info->bpb.sectors_per_fat * info->bpb.fat_count * info->bpb.bytes_per_sector));
    read_sector(file, info->bpb.reserved_sectors, info->bpb.fat_count * info->bpb.sectors_per_fat, fat);
    info->file_table = fat;
    if(!fat[info->bpb.root_dir_cluster]){
        write_cluster_value(info, FILE_END, info->bpb.root_dir_cluster);
    }
}

void write_cluster_value(fat_info *info, uint32_t value, uint32_t cluster){
    for(int i = 0; i < info->bpb.fat_count; i++){
        info->file_table[cluster + i * info->bpb.sectors_per_fat * info->bpb.bytes_per_sector] = value;
    }
}

int fat_write_back(FILE *file, fat_info *info){
    
}

int main(int argc, char **argv){
    char **files = malloc(512);
    int filec = 0;
    FILE *output_file = 0;
    
    fat_info info = {};
    
    printf("write files to virtual disk\n");
    if(argc == 1 || !strcmp(argv[1], "help")){
        printf("Usage:\ndiskwrite <input files> -o <output file>");
        printf("\n-h: Help");
        printf("\n-o: Specify output file\n");
    }
    for(int i = 1; i < argc; i++){
        if(!strcmp(argv[i], "-o")){
            i++;
            if(i >= argc){
                printf("Error: No output file specified!\n");
            }
            printf("OF: %s\n", argv[i]);
            output_file = fopen(argv[i], "r+");
            continue;
        }
        files[filec++] = argv[i];
        printf("IF: %s\n", argv[i]);
    }
    if(filec == 0){
        printf("Nothing to do, no files to write\n");
        return 0;
    }
    detect_repair_fs(output_file, &info);
    printf("Finished. Exiting...\n");
    return 0;
}