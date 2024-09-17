;   Kimi's Bootloader - Custom Booting solution for Kimi's OS
;   (c) 2024 Notsomeidiot123 on Github
;   Documentation available at https://github.com/notsomeidiot123/KimisOS


org 0x7c00

bits 16

%macro debug 0
    mov ah, 0xe
    mov al, 0x41
    int 0x10
    jmp $
%endmacro

;TODO:
;Enable A20 line                            |x|
;Enable unreal mode                         |x|
;Load GDT                                   |x|
;Load part2                                 |x|
;Get video mode information                 |0|
;Set video mode                             |0|
;get memory map                             |0|
;enable paging                              |0|
;read filesystem                            |0|
;load kernel                                |0|
;parse ELF format                           |0|
;pass memory and video information to kernel|0|
;relocate and jump to kernel                |0|

jump:
    jmp short start
    times 3-($-$$) db 0x90
fat_bpb:
    .oem:
        db "KIMI'SOS"
    .bytes_per_sector:
        dw 512
    .sectors_per_cluster:
        db 8
    .reserved_sectors:
        dw 32
    .fat_count:
        db 2
    .root_dir_entries:
        dw 0
    .sectors_in_volume:
        dw 0
    .media_type:
        db 0xf8
    .sectors_per_fat:
        dw 0
    .sectors_per_track:
        dw 63
    .heads:
        dw 8
    .part_start:
        dd 0
    .large_sector_count:
        dd 1048576
fat_ebpb32:
    .sectors_per_fat:
        dd 1024
    .flags:
        dw 0
    .version:
        db 0, 1
    .root_dir_cluster:
        dd 2 
    .fs_info_sector:
        dw 1
    .backup_boot:
        dw 0
    .reserved:
        times 12 db 0
    .drive_num:
        db 0x80
    .ntflags:
        db 0
    .sig:
        db 0x29
    .vid_Serial:
        dd 0
    .label:
        db " Kimi's OS "
    .sysid:
        db "FAT32   "
    
start:
    mov sp, 0x7000
    mov bp, 0x7000
    mov ax, ds
    xor bx, bx
    mov ds, bx
    mov [data.part_seg], ax
    mov [data.part_off], si
    mov [data.boot_disc], dl
    jmp 0:set_cs
set_cs:
    xor cx, cx
check_a20:
    inc cx
    mov ax, 0xffff
    mov es, ax
    mov ax, [es:0x7e0e]
    cmp ax, [ds:0x7dfe]
    jne load_gdt
enable_a20:
    cmp cx, 4
    je no_a20
    cmp cx, 3
    je .fast_a20
    cmp cx, 2
    je .kb_en
    .bios_en:
        mov ax, 0x2401
        int 0x15
        jmp check_a20
    .fast_a20:
        in al, 0x92
        or al, 2
        out 0x92, al
        mov ax, 10000
        .lp:
            dec ax
            cmp ax, 0
            jge .lp
        jmp check_a20
    .kb_en:
        cli
        mov al, 0xad
        out 0x64, al

        call .wait
        mov al, 0xd0
        out 0x64, al

        call .wait2
        in al, 0x60
        push eax

        call .wait
        mov al, 0xd1
        out 0x64, al

        call .wait
        pop eax
        or al, 2
        out 0x60, al

        call .wait
        mov al, 0xae
        out 0x64, al

        call .wait
        sti
        jmp check_a20

    .wait:
        in al, 0x64
        test al,2 
        jnz .wait
        ret
    .wait2:
        in al, 0x64
        test al, 1
        jnz .wait2
        ret
load_gdt:
    lgdt [GDT_desc]
    push ds
    mov eax, cr0
    or al, 1
    mov cr0, eax
    jmp 0x8:.unreal
    .unreal:
        mov bx, 0x18
        mov ds, bx
        and al, 0xfe
        mov cr0, eax
        jmp 0x0:.real
    .real:;no actual code here, just label for readability and organization
    pop ds
load_part2:
    mov ah, 0x42
    xor edx, edx
    mov dx, word [data.part_seg]
    shl edx, 4
    xor ecx, ecx
    mov cx, word [data.part_off]
    add edx, ecx
    
    mov al, byte [edx]
    and al, 0x80
    jz .read
    mov ecx, [edx + 0x8]
    
    add [DAP.sector_start], ecx
    .read:
        mov dl, byte [data.boot_disc]
        push 0
        pop ds
        mov si, DAP
        jmp $
        int 0x13
        jc load_fail
        jmp part_2
    
    jmp $
no_a20:
    mov ah, 0xe
    mov al, 'E'
    int 0x10
    mov al, '1'
    int 0x10
    jmp $
load_fail:
    mov ah, 0xe
    mov al, 'E'
    int 0x10
    mov al, '2'
    int 0x10
    jmp $
DAP:
    .sz: db 0x10
    .unused: db 0
    .read_count: dw 7
    .offset: dw 0x7e00
    .segment: dw 0
    .sector_start:
        dd 1
        dd 0
GDT_desc:
    .size: dw GD_TABLE-GD_END - 1
    .offset: dd GD_TABLE
GD_TABLE:
    .null:
        dd 0
        dd 0
    .kcode16:
        dw 0xffff
        dw 0x0000
        db 0x00
        db 0b10011010
        db 0b00001111
        db 0x00
    .kcode32:
        dw 0xffff
        dw 0x0000
        db 0x00
        db 0b10011010
        db 0b11001111
        db 0x00
    .kdata32:
        dw 0xffff
        dw 0x0000
        db 0x00
        db 0b10010010
        db 0b11001111
        db 0x00
    .ucode32:
        dw 0xffff
        dw 0x0000
        db 0x00
        db 0b11111010
        db 0b11001111
        db 0x00
    .udata32:
        dw 0xffff
        dw 0x0000
        db 0x00
        db 0b11110010
        db 0b11001111
        db 0x00
    .tss:
        dd 0
        dd 0
    .phony:
        dw 0xaa55
        dd 0
        dw 0x55aa
GD_END:
data:
    .boot_disc: db 0
    .part_seg: dw 0
    .part_off: dw 0

times 510-($-$$) db 0
db 0x55, 0xaa
fs_info:
    .id: dd 0x41615252 
    .reserved0: times 480 db 0
    .sig: dd 0x61417272
    .last_known_free: dd 0xFFFFFFFF
    .free_search_start: dd 3
    .reserved: times 12 db 0
    .trail: dd 0xAA550000 
times 1024-($-$$) db 0
part_2:
    call get_mmap
    call paging_en
    call open_file
    jmp $
load_kernel:
get_mmap:
    pushad
    push ds
    push es
    
    push 0x200
    pop es
    mov di, 0
    
    mov edx, 0x534D4150
    xor ebx, ebx
    xor esi, esi
    .get_map:
        mov eax, 0xe820
        mov ecx, 24
        int 0x15
        jc .ret
        mov eax, [es:di + 8]
        or eax, [es:di + 16]
        jz .get_map
        inc si
        or dword [es:di + 20], 1
        add di, 24
        cmp ebx, 0
        je .ret
        jmp .get_map
    
    .ret:
        cmp si, 0
        je .ret_err
        mov [k_info.memory_map_count], si
        pop es
        pop ds
        popad
        mov ax, 0
        ret
    .ret_err:
        pop es
        pop ds
        popad
        mov ax, 1
        ret
set_video_mode:
paging_en:
    mov eax, cr4
    or eax, 0x00000010 ;enable 4mb pages
    mov cr4, eax
    
    mov eax, 0x4000;page table base
    mov [eax], dword 0
    or [eax], dword 0b0_0001_1000_1001;present, Write-through, page size 4mb, global, present
    mov cr3, eax
    
    ;enable recursive paging
    mov [eax + 1023], eax
    or [eax + 1023], dword 0b0_0001_0000_1001
    
    ret
open_file:
    ;only looks in the root directory
    xor esi, esi
    xor ecx, ecx
    xor edx, edx
    mov esi, [fat_bpb.part_start];
    add si, [fat_bpb.reserved_sectors]
    mov cl, [fat_bpb.fat_count]
    mov dx, [fat_ebpb32.sectors_per_fat]
    .addlp:
        add esi, edx
        dec ecx
        or ecx, ecx
        jnz .addlp
    xor dx, dx
    mov ecx, [fat_ebpb32.root_dir_cluster]
    mov dl, [fat_bpb.sectors_per_cluster]
    .addlp1:
        add esi, edx
        dec ecx
        or ecx, ecx
        jnz .addlp1
    mov [DAP.sector_start], esi
    mov [DAP.read_count],   dx
    mov [DAP.offset],       word 0xd000
    mov [DAP.segment],      word 0x0000
    
    mov eax, [DAP.sector_start]
    mov ebx, [DAP.sector_start + 4]
    mov ecx, [DAP.offset]
    mov dx, [DAP.read_count]
    push 0
    pop ds
    push 0
    pop es
    ;sector start now contains the sector number of the first data sector of the root directory
    mov ah, 0x42
    mov dl, byte [data.boot_disc]
    mov si, DAP
    int 0x13
    jc load_fail
    debug
read_file:
    ;reads all data from file
load_elf:
    ;parse elf and relocate each section to it's appropriate location
;kernel info
k_info:
    .memory_map_ptr:    dd 0x2000
    .memory_map_count:  dw 0
    .padding:           db 0
    .bpp:               db 0;if depth == 0 then video is text mode
    .xres:              dw 0
    .yres:              dw 0
    .framebuffer_ptr:   dd 0
    .loaded_modules:    dd 0
;loaded_modules points to a struct array containing a ptr to the entry point of each pre-loaded module to be executed during startup
;module_struct:
    ;.entry_point: dd 0
    ; cr3_load: dd 0
    ;.type:  db 0

times 4096-($-$$) db 0