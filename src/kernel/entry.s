bits 16

global _start

_start:
    mov ebx, 0xb8000
    mov word [ds:ebx], 0x0f41
    cli
    hlt
    jmp $