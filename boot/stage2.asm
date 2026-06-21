BITS 16
ORG 0x7E00

    cli

    in  al, 0x92
    or  al, 2
    out 0x92, al

    lgdt [gdt_desc]

    mov eax, cr0
    or  eax, 1
    mov cr0, eax

    jmp 0x08:pm_entry

BITS 32
pm_entry:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, 0x90000

    ; Clear screen — stosw writes 2 bytes, exactly 80*25*2 = 4000 bytes total
    mov edi, 0xB8000
    mov ecx, 80*25
    mov ax, 0x0720
    rep stosw

    ; Print banner at absolute top-left
    mov esi, banner
    mov edi, 0xB8000
    mov bl, 0x0B
.print:
    lodsb
    test al, al
    jz .done
    mov [edi], al
    mov [edi+1], bl
    add edi, 2
    jmp .print
.done:
    cli
    hlt

banner db '[ SuchOS v0.1 ] Protected mode active.', 0

gdt_start:
    dq 0
    dw 0xFFFF, 0x0000, 0x9A00, 0x00CF
    dw 0xFFFF, 0x0000, 0x9200, 0x00CF
gdt_end:

gdt_desc:
    dw gdt_end - gdt_start - 1
    dd gdt_start
