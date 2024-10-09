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
    // printf("finding free cluster\n");
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

int read_file(fat_info *info, uint32_t first_cluster, file_t *file, buffer_t *buffer, FILE *disk);

int create_file(char *path, fat_info *info, FILE *file){
    FILE *ifile = fopen(path, "r");
    if(!ifile){
        printf("Failed to open file: %s\nAborting...\n", path);
        exit(-1);
    }
    fseek(ifile, 0, SEEK_END);
    int ifsize = ftell(ifile);
    if(ifsize == 0){
        verbose && printf("File has length of 0, skipping %s\n", path);
        return 0;
    }
    buffer_t ifile_data = malloc((ifsize/(info->bpb.bytes_per_sector * info->bpb.sectors_per_cluster) + 1) * info->bpb.bytes_per_sector * info->bpb.sectors_per_cluster);
    rewind(ifile);
    if(!fread(ifile_data, 1, ifsize, ifile)){
        verbose && printf("Failed to read from file %s, skippping\n", path);
        return 0;
    }
    verbose && printf("Read %d bytes from ifile\n", ifsize);
    //create file in root dir
    file_t dirent = {0};
    int extoffset = 0;
    while(*(path + extoffset) != '.' && *(path + extoffset)){
        extoffset++;
    }
    memcpy(dirent.filename, path, 8 > extoffset ? extoffset : 8);
    verbose && printf("File extension at %d\n", extoffset);
    verbose && printf("file ext: %s\n", path + extoffset);
    memmove(dirent.ext, path + extoffset + 1, 3);
    dirent.size_bytes = ifsize;
    verbose && printf("Writing %u clusters\n", ifsize/(info->bpb.bytes_per_sector * info->bpb.sectors_per_cluster) + 1);
    uint32_t last_cluster = 0;
    for(int i = 0; i <= ifsize/(info->bpb.bytes_per_sector * info->bpb.sectors_per_cluster); i++){
        uint32_t cluster = find_free_cluster(info);
        if(i == 0){
            dirent.start_cluster_high = cluster >> 16;
            dirent.start_cluster_low = cluster & 0xFFFF;
        }
        // printf("found free cluster\n");
        write_sector(file, (info->bpb.reserved_sectors + info->bpb.fat_count * info->bpb.sectors_per_fat) + cluster * info->bpb.sectors_per_cluster, info->bpb.sectors_per_cluster, ifile_data + (info->bpb.bytes_per_sector * info->bpb.sectors_per_cluster) * i);
        printf("cluster %d @ %p\n", i, ifile_data + (info->bpb.bytes_per_sector * info->bpb.sectors_per_fat * info->bpb.sectors_per_cluster) * i);
        
        verbose && printf("wrote to sector %x %x sectors of data from buffer\n", (info->bpb.reserved_sectors + info->bpb.fat_count * info->bpb.sectors_per_fat) + cluster * info->bpb.sectors_per_cluster, info->bpb.sectors_per_cluster);
        if(last_cluster != 0) write_cluster_value(info, cluster, last_cluster);
        write_cluster_value(info, FILE_END, cluster);
        last_cluster = cluster;
    }
    verbose && printf("last cluster: %d\n", last_cluster);
    // write_cluster_value(info, FILE_END, last_cluster);
    
    buffer_t root_dir_buffer = 0;
    uint32_t dirent_count = read_file(info, info->bpb.root_dir_cluster, 0, &root_dir_buffer, file)/sizeof(file_t);
    
    //searching for a free dirent
    file_t *root_dir = (file_t *)root_dir_buffer;
    uint32_t free_file_index = 0;
    while(root_dir[free_file_index].filename[0] && free_file_index < dirent_count){
        free_file_index++;
    }
    if(free_file_index >= dirent_count){
        //!FIXME
        verbose && printf("could not find free dirent; this will be fixed in a later version\n");
        exit(-1);
    }
    root_dir[free_file_index] = dirent;
    verbose && printf("Found a free dirent at index %u\n", free_file_index);
    uint32_t cluster = info->bpb.root_dir_cluster;
    uint32_t cluster_count = (dirent_count * sizeof(file_t)) / info->bpb.bytes_per_sector /info->bpb.sectors_per_cluster;
    for(uint32_t i = 0; i < cluster_count; i++){
        write_sector(file, (info->bpb.reserved_sectors + (info->bpb.fat_count * info->bpb.sectors_per_fat) + cluster * info->bpb.sectors_per_cluster), info->bpb.sectors_per_cluster, (buffer_t)(root_dir + i * info->bpb.sectors_per_cluster * info->bpb.bytes_per_sector));
        uint32_t lcluster = cluster;
        cluster = info->file_table[cluster];
        if(cluster == FILE_END && i < cluster_count - 1){
            cluster = find_free_cluster(info);
            if(cluster != FILE_END){
                write_cluster_value(info, cluster, lcluster);
                write_cluster_value(info, cluster, FILE_END);
            }
        }
    }
}

int read_file(fat_info *info, uint32_t first_cluster, file_t *file, buffer_t *buffer, FILE *disk){
    uint32_t cluster_size_bytes = info->bpb.bytes_per_sector * info->bpb.sectors_per_cluster;
    if(file == 0){
        verbose && printf("Reading starting from cluster %d\n", first_cluster);
        buffer_t buf = malloc(1);
        uint32_t allocated = 0;
        uint32_t cluster = first_cluster;
        do {
            allocated++;
            buf = realloc(buf, cluster_size_bytes * allocated);
            verbose && printf("Reallocated to %u, ptr: %p\n", allocated * cluster_size_bytes, buf);
            read_sector(disk, info->bpb.reserved_sectors + (info->bpb.fat_count * info->bpb.sectors_per_fat) + cluster * info->bpb.sectors_per_cluster, info->bpb.sectors_per_cluster, buf + (allocated * cluster_size_bytes));
            cluster = info->file_table[cluster];
        }while(cluster != FILE_END);
        if(*buffer != 0){
            free(buffer);
        }
        verbose && printf("Read %d cluster starting from cluster %d\n", allocated, first_cluster);
        *buffer = buf;
        return allocated * cluster_size_bytes;
    }
}


int fat_write_back(FILE *file, fat_info *info){
    int res = 0;
    if(res = write_sector(file, info->bpb.reserved_sectors, info->bpb.sectors_per_fat * info->bpb.fat_count, (buffer_t)info->file_table) == 0){
        verbose && printf("Error during write: %d\n", res);
    }
}

int detect_repair_fs(FILE *file, fat_info *info){
    buffer_t boot_sector = malloc(512);
    read_sector(file, 0, 1, boot_sector);
    memcpy(&(info->bpb), boot_sector, 90);
    buffer_t fat = malloc(info->bpb.sectors_per_fat * info->bpb.fat_count * info->bpb.bytes_per_sector);
    verbose && printf("allocated 0x%x bytes for fat\n", (info->bpb.sectors_per_fat * info->bpb.fat_count * info->bpb.bytes_per_sector));
    int result = read_sector(file, info->bpb.reserved_sectors, info->bpb.fat_count * info->bpb.sectors_per_fat, fat);
    if(result == 0){
        printf("Error: Could not read from disk %p\n", file);
        return 1;
    }
    info->file_table = (uint32_t *)fat;
    verbose && printf("FAT assigned\n");
    if(!info->file_table[info->bpb.root_dir_cluster]){
        verbose && printf("Root directory not assigned, assigning cluster at cluster %d\n", info->bpb.root_dir_cluster);
        write_cluster_value(info, FILE_END, info->bpb.root_dir_cluster);
    }
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
        create_file(files[i], &info, output_file);
    }
    fat_write_back(output_file, &info);
    verbose && printf("Finished. Exiting...\n");
    return 0;
}