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

cpu_registers_t *(*volatile interrupt_handlers[16])(cpu_registers_t *regs) = {0};
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


cpu_registers_t *_irq_handler(cpu_registers_t *regs){
    regs->esp = (uint32_t)regs;
    if(regs->int_no == 0x80){
        regs = syscall(regs);
    }
    else if(interrupt_handlers[regs->int_no - 32])
    {
        // printf("found one for an int");
        regs = interrupt_handlers[regs->int_no - 32](regs);
        // printf("returned;")
        // printf("eip: %x", regs->eip);
    }
    outb(0x20, 0x20);
    if(regs->int_no >= 0x28){
        outb(0xa0, 0x20);
    }
    return regs;
}
void _isr_handler(cpu_registers_t *regs){
    printf("Kernel Panic!\n");
    printf("ISR: %d\n", regs->int_no);
    printf("EAX: %x EBX: %x ECX: %x EDX: %x\n", regs->eax, regs->ebx, regs->ecx, regs->edx);
    printf("ESI: %x EDI: %x ESP: %x EBP: %x\n", regs->esi, regs->edi, regs->esp, regs->ebp);
    printf("EIP: %x CS: %x DS: %x SS: %x\n", regs->eip, regs->cs, regs->ds, regs->ss);
    for(;;);
}

cpu_registers_t *syscall(cpu_registers_t *regs){
    
}

extern void panic_hold(cpu_registers_t *regs);

void kernel_panic(char *message, cpu_registers_t *regs){
    printf("Kernel Panic!\nAn fatal error occured, and could not be recovered.\nError: ");
    printf(message);
    printf("System halting...\n");
    if(regs != 0) panic_hold(regs);
    asm volatile ("jmp .");
}

void install_irq_handler(void (*handler)(), uint8_t irqno){
    interrupt_handlers[irqno] = handler;
}

void init_idt(){
    for(uint32_t i = 0; i < 32; i++){
        set_idt_entry(i, (_isr0 + ((_isr1 - _isr0) * i)), IDT_GATE_INT, 0x10);
        // printf("int %d: address: %x\n", i, _isr0 + (_isr1 - _isr0) * i);
    }
    // printf("last ")
    for(uint32_t i = 0; i < 16; i++){
        set_idt_entry(i + 32, (_irq0 + ((_irq1 - _irq0) * i)), IDT_GATE_INT, 0x10);
        // printf("int %d: address: %x\n", i + 32, _irq0 + (_irq1 - _irq0) * i);
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
