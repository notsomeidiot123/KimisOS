#pragma once
#include <stdint.h>
#define PIC1            0x20
#define PIC2            0xA0
#define PIC1_COMMAND    0x20
#define PIC1_DATA       0x21
#define PIC2_COMMAND    0xA0
#define PIC2_DATA       0xA1

#define PIC_EOI         0x20
#define PIC_ICW1_ICW4   0x01
#define PIC_ICW1_SINGLE 0x02
#define PIC_ICW1_INT4   0x04
#define PIC_ICW1_LEVEL  0x08
#define PIC_ICW1_INIT   0x10

#define PIC_ICW4_8086   0x01
#define P
