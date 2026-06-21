BITS 16
ORG 0x7C00

start:
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00

    mov ah, 0x02
    mov al, 1
    mov ch, 0
    mov cl, 2
    mov dh, 0
    mov bx, 0x7E00
    int 0x13
    jc .error
    jmp 0x7E00

.error:
    mov ah, 0x0E
    mov al, '!'
    int 0x10
    cli
    hlt

times 510-($-$$) db 0
dw 0xAA55
