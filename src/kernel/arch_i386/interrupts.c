#include "../shared/interrupts.h"
#include "../drivers/serial.h"
#include <stdint.h>

extern void _isr0();
extern void _isr1();

extern void _irq0();
extern void _irq1();
extern void _syscall();

idt_t idt_desc;
uint64_t idt_table[256];

inline void set_idt_entry(uint32_t index, void *ptr, uint8_t flags, uint16_t segment){
    uint32_t offset = (uint32_t) ptr;
    idt_table[index] = ((uint64_t)(offset >> 16) << 48) | ((uint64_t)(flags | 0x80) << 40) | (segment << 16) | (offset & 0xffff);
}

cpu_registers_t *(*interrupt_handlers[16])(cpu_registers_t *regs) = {0};

void _irq_handler(cpu_registers_t *regs){
    if(regs->int_no == 0x80){
        regs = syscall(regs);
    }
    else if(interrupt_handlers[regs->int_no - 32])
    {
        regs = interrupt_handlers[regs->int_no - 32](regs);
    }
}
void _isr_handler(cpu_registers_t *regs){
    printf("Error!!\n");
    for(;;);
}

cpu_registers_t *syscall(cpu_registers_t *regs){
    
}

void init_idt(){
    for(uint32_t i = 0; i < 32; i++){
        set_idt_entry(i, (_isr0 + (_isr1 - _isr0) * i), IDT_GATE_TRAP, 0x10);
    }
    for(uint32_t i = 0; i < 16; i++){
        set_idt_entry(i + 32, (_irq0 + (_irq1 - _irq0) * i), IDT_GATE_INT, 0x10);
    }
    
    idt_desc.pointer = (uint32_t)idt_table;
    idt_desc.size = sizeof(idt_table);
    asm volatile("lidt (%0)" : : "m"(idt_desc));
}
