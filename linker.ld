ENTRY(_start)
OUTPUT_FORMAT(elf32-i386)
/* OUTPUT_FORMAT(binary) */
/* OUTPUT(kernel.elf) */
/* SEARCH_DIR(bin/kernel) */
SECTIONS{
    . = 0xc0000000;
    .text : AT(ADDR(.text) - 0xc0000000){
        _code = .;
        *(.text)
        *(.rodata)
        . = ALIGN(4096);
    }
    .data : AT(ADDR(.data) - 0xc0000000){
        _data = .;
        *(.data)
    }
    .eh_frame : AT(ADDR(.eh_frame) - 0xc0000000){
        _eh_frame = .;
        *(.eh_frame)
        . = ALIGN(4096);
    }
    
    .bss : AT(ADDR(.bss) - 0xc0000000){
        _bss = .;
        *(.bss)
        *(.COMMON)
        . = ALIGN(4096);
    }
    _end = .;
    /DISCARD/ : 
    {
        *(.comment)
    }
}