#include<stdint.h>

void main(){
    uint16_t *t = (void *)0xb8000;
    *t = 0x0f41;
}