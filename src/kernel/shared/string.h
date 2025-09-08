#pragma once
#include <stdint.h>

void itoa(int64_t data, char *res, uint8_t base);
void strpad(char *string, char padding, uint32_t length);
//str cannot be null
inline uint32_t strlen(char *str){
    uint32_t c = 0;
    while(*str){
        str++;
        c++;
    }
    return c;
}
//str1 and str2 both cannot be null
inline uint32_t strcmp(char *str1, char *str2){
    uint32_t i = 0;
    while(str1[i] && str1[i] == str2[i]){
        i++;
    }
    if(str1[i] == str2[i]) return 0;
    else return 1;
}
char *strtok(char *str, char delim);
void strcpy(char *src, char *dest);
void memcpy(char *src, char *dest, uint32_t c);