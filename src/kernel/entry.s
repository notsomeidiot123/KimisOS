bits 32

global _start

section .text
_start:
    mov ebx, 0xb8000
    mov esi, string
    mov ecx, 0
    mov ah, 0x8f
    ; mov ax, 0x55aaaa55
    .lp:
        lodsb
        cmp al, 0
        je .end
        cmp al, 0xa
        je .nl
        cmp al, 0xd
        je .cr
        mov [ebx + ecx], ax
        add ecx, 2
        jmp .lp
    .nl:
        add ebx, 160
        ; mov word [ebx + ecx], 0xf41
        jmp .lp
    .cr:
        xor ecx, ecx
        jmp .lp
    .end:
    jmp $
    
section .data
string: db "Why even use grub at this point?", 0xa, 0xd, "https://github.com/notsomeidiot123 | someidiot332 on Reddit :3", 0xa, 0xd
db "Check comments for more info ^^", 0x0