BITS 16
ORG 0x7E00

    cli

    ; Enable A20
    in  al, 0x92
    or  al, 2
    out 0x92, al

    ; Load kernel: 32 sectors from sector 3 → 0x10000
    mov ax, 0x1000      ; segment 0x1000 × 16 = physical 0x10000
    mov es, ax
    xor bx, bx          ; offset 0 → ES:BX = 0x10000

    mov ah, 0x02
    mov al, 32          ; 32 sectors (16KB, more than enough)
    mov ch, 0
    mov cl, 3           ; BIOS sector 3 (1-indexed)
    mov dh, 0
    int 0x13
    jc .disk_err

    xor ax, ax
    mov es, ax

    lgdt [gdt_desc]

    mov eax, cr0
    or  eax, 1
    mov cr0, eax

    jmp 0x08:pm_entry

.disk_err:
    mov ah, 0x0E
    mov al, 'E'
    int 0x10
    cli
    hlt

BITS 32
pm_entry:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, 0x90000

    mov eax, 0x10000
    call eax

    cli
    hlt

gdt_start:
    dq 0
    dw 0xFFFF, 0x0000, 0x9A00, 0x00CF
    dw 0xFFFF, 0x0000, 0x9200, 0x00CF
gdt_end:

gdt_desc:
    dw gdt_end - gdt_start - 1
    dd gdt_start