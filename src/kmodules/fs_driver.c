#include <stdint.h>

void init(){
    uint16_t *t = (void *)0xb8002;
    *t = 0x0f41;
    // for(int i = 0; i < 1000; i++){
    //     (*t)++;
    //     // asm volatile( " " );
    // }
    for(;;);
}