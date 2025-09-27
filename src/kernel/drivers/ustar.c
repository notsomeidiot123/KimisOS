#include "ustar.h"
#include "../system/vfs.h"
#include "../shared/kstdlib.h"
#include "../shared/string.h"
#include "../shared/memory.h"

uint32_t ustar_getsize(const char *in){
    uint32_t size = 0;
    uint32_t j;
    uint32_t count = 1;
    
    for (j = 11; j > 0; j--, count *= 8){
        size += ((in[j - 1] - '0') * count);
    }
    return size;
    
}

int read_initrd(initrd_t *initrd){
    // printf("Name: %s", ((USTAR_file_t * )initrd->ptr)->filename);
    char *archive = initrd->ptr;
    uint32_t offset = 0;
    while (!memcmp(((USTAR_file_t *)(archive + offset))->ustar, "ustar", 5)) {
        USTAR_file_t *file = (USTAR_file_t *)(archive + offset);
        int filesize = ustar_getsize(file->fsize);
        // if (!memcmp(file->filename, filename, strlen(filename) + 1)) {
        //     *out = ptr + 512;
        //     return filesize;
        // }
        uint32_t fsize_pgs = (((filesize + 4095)/4096) + 1);
        // uint32_t *ptr = kmalloc(fsize_pgs);
        
        // for(uint32_t i = 0; i < (filesize/4) + 1; i++){
        //     ptr[i] = ((uint32_t *)(archive + offset))[i];
        // }
        char final_fname[80] = "/boot/";
        strcat(file->filename, final_fname);
        fcreate(final_fname, VFILE_POINTER, archive + offset + sizeof(USTAR_file_t), fsize_pgs);
        printf("%s, %d, %d\n", final_fname, filesize, fsize_pgs);
        offset += (((filesize + 511) / 512) + 1) * 512;
    }
}