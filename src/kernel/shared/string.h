#pragma once
#include <stdint.h>

void itoa(int64_t data, char *res, uint8_t base);
void strpad(char *string, char padding, uint32_t length);
inline uint32_t strlen(char *str){
    uint32_t c = 0;
    while(*str){
        str++;
        c++;
    }
    return c;
}
void strcpy(char *src, char *dest);
void memcpy(char *src, char *dest, uint32_t c);