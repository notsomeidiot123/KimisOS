org 0x7c00

bits 16

%macro debug 0
    mov ah, 0xe
    mov al, 0x41
    int 0x10
%endmacro

;TODO:
;Enable A20 line
;Enable unreal mode
;Load GDT
;Load part2
;Set video mode
;get memory map
;enable paging
;read filesystem
;load kernel
;parse ELF format
;pass memory and video information to kernel
;relocate and jump to kernel

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
        mov ax, 100000
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
load_part2:
    jmp $
no_a20:
    mov ah, 0xe
    mov al, 'E'
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
load_part3:
get_mmap:
set_video_mode:
times 4096-($-$$) db 0