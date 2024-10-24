bits 32

extern kmain
section .text
global _start
_start:
    mov esp, stack_top
    mov ebp, stack_top
    push esi
    ; jmp $
    call kmain
    jmp $
    
section .bss
stack_bottom: resb 0x4000
stack_top: