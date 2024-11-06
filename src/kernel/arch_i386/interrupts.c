#include "../shared/interrupts.h"
#include "../drivers/serial.h"
#include "../drivers/cpuio.h"
#include <stdint.h>

extern void _isr0();
extern void _isr1();

extern void _irq0();
extern void _irq1();
extern void _syscall();
extern void load_idt(void*);
typedef struct idt_entry{
    uint16_t offset_low;
    uint16_t code_seg;
    uint8_t reserved;
    uint8_t flags;
    uint16_t offset_high;
}__attribute__((packed)) idt_entry_t;

cpu_registers_t *(*interrupt_handlers[16])(cpu_registers_t *regs) = {0};
idt_t idt_desc;

idt_entry_t idt_table[256];

inline void set_idt_entry(uint32_t index, void *ptr, uint8_t flags, uint16_t segment){
    uint32_t offset = (uint32_t) ptr;
    idt_table[index].offset_high = offset >> 16;
    idt_table[index].offset_low = offset & 0xffff;
    idt_table[index].code_seg = segment;
    idt_table[index].flags = flags | 0x80;
    // idt_table[index] = (segment << 16) | (offset & 0xffff);
    // idt_table[index+1] = (offset >> 16) | ((flags | 0x80) << 8);
    // idt_table[index] = ((uint64_t)(offset >> 16) << 48) | ((uint64_t)(flags | 0x80) << 40) | (segment << 16) | (offset & 0xffff);
}


void _irq_handler(cpu_registers_t *regs){
    if(regs->int_no == 0x80){
        regs = syscall(regs);
    }
    else if(interrupt_handlers[regs->int_no - 32])
    {
        if(regs->int_no == 32){
            // printf("Tick\n");
            // printf("%x\n", regs->int_no);
        }
        regs = interrupt_handlers[regs->int_no - 32](regs);
        // printf("eip: %x", regs->eip);
        outb(0x20, 0x20);
        if(regs->int_no >= 0x28){
            outb(0xa0, 0x20);
        }
    }
}
void _isr_handler(cpu_registers_t *regs){
    printf("Error!! %x\n", regs->eip);
    for(;;);
}

cpu_registers_t *syscall(cpu_registers_t *regs){
    
}

void init_idt(){
    for(uint32_t i = 0; i < 32; i++){
        set_idt_entry(i, (_isr0 + ((_isr1 - _isr0) * i)), IDT_GATE_INT, 0x10);
        // printf("int %d: address: %x\n", i, _isr0 + (_isr1 - _isr0) * i);
    }
    // printf("last ")
    for(uint32_t i = 0; i < 16; i++){
        set_idt_entry(i + 32, (_irq0 + ((_irq1 - _irq0) * i)), IDT_GATE_INT, 0x10);
        printf("int %d: address: %x\n", i + 32, _irq0 + (_irq1 - _irq0) * i);
    }
    
    idt_desc.pointer = (uint32_t)&idt_table;
    idt_desc.size = sizeof(idt_entry_t) * 256 - 1;
    for(int i = 0; i < 16; i++){
        interrupt_handlers[i] = 0;
    }
    // asm volatile("lidt (%0)" : : "m"(&idt_desc));
    // outb(0x20, 0x11 );
    // outb(0xa0, 0x11);
    // outb(0x21, 0x20);
    // outb(0xa1, 0x28);
    // outb(0x21, 4);
    // outb(0xa1, 2);
    // outb(0x21, 1);
    // outb(0xa1, 1);
    // outb(0x21, 0);
    // outb(0xa1, 0);
    
    //set pic masks
    // outb(0x21, 0);
    // outb(0xa1, 0);
    load_idt(&idt_desc);
}
