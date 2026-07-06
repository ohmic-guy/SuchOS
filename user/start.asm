[BITS 32]
[global _start]
[extern main]

_start:
    call main
    mov eax, 60
    mov ebx, 0
    int 0x80