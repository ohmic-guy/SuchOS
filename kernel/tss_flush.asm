[BITS 32]
[global tss_flush]

tss_flush:
    mov ax, 0x28    ; GDT[5] selector = 5*8 = 40 = 0x28
    ltr ax
    ret