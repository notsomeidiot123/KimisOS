bits 32

; extern kmain
section .text
global _start
_start:
    mov ebx, 0xb8002
    mov word [ebx], 0xf41
    ; call kmain
    jmp $
    
section .data
string: db "Why even use grub at this point?", 0xa, 0xd, "https://github.com/notsomeidiot123 | someidiot332 on Reddit :3", 0xa, 0xd
db "Check comments for more info ^^", 0x0