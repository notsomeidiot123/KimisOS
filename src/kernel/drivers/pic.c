#include "pic.h"
#include "cpuio.h"

void init_pic(uint8_t idt_index){
    uint8_t a1 = inb(PIC1_DATA);
    uint8_t a2 = inb(PIC2_DATA);
    
    outb(PIC1_COMMAND, PIC_ICW1_INIT | PIC_ICW1_ICW4);
    outb(PIC2_COMMAND, PIC_ICW1_INIT | PIC_ICW1_ICW4);
    
    outb(PIC1_DATA, 32);
    outb(PIC2_DATA, 40);
    
    outb(PIC1_DATA, 4);
    outb(PIC2_DATA, 2);
    
    outb(PIC1_DATA, PIC_ICW4_8086);
    outb(PIC2_DATA, PIC_ICW4_8086);
    
    outb(PIC1_DATA, a1);
    outb(PIC2_DATA, a2);
}
void pic_disable(){
    outb(PIC1_DATA, 0xff);
    outb(PIC2_DATA, 0xff);
}
void pic_setmask(uint8_t mask, uint16_t pic){
    // if(pic != PIC1_DATA || pic != PIC2_DATA) return;
    outb(pic, mask);
}
uint8_t pic_getmask(uint16_t pic){
    // if(pic != PIC1_DATA || pic != PIC2_DATA) return 0;
    return inb(pic);
}