#include "string.h"

#pragma warning

char res[256];

char* strtok(char* string, char d)
{
    static char* input = 0;

    if (string){
        input = string;
    }
    if (!input){
        return 0;
    }
    int i = 0;
    for (; input[i] != 0; i++) {
        if (input[i] != d){
            res[i] = input[i];
        }
        else {
            res[i] = '\0';
            input = input + i+1;
            return res;
        }
    }
    res[i] = '\0';
    input = 0;
    
    return res;
}