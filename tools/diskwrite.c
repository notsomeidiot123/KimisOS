#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv){
    char **files = malloc(512);
    int filec = 0;
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
        }
        files[filec++] = argv[i];
    }
    return 0;
}