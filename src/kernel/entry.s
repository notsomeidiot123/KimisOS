bits 32

global _start
extern kmain
section .text
_start:
    call kmain
    jmp $
    
section .data
string: db "Why even use grub at this point?", 0xa, 0xd, "https://github.com/notsomeidiot123 | someidiot332 on Reddit :3", 0xa, 0xd
db "Check comments for more info ^^", 0x0