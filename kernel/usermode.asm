[BITS 32]
[global usermode_enter]

; void usermode_enter(uint32_t eip, uint32_t esp)
usermode_enter:
    mov eax, [esp+4]    ; eip
    mov ecx, [esp+8]    ; user stack top

    ; User data segment: GDT[4] | RPL=3 = 0x23
    mov bx, 0x23
    mov ds, bx
    mov es, bx
    mov fs, bx
    mov gs, bx

    ; iret frame: ss, esp, eflags, cs, eip
    push 0x23           ; ss  — user data
    push ecx            ; esp — user stack
    pushf
    pop  edx
    or   edx, 0x200     ; ensure IF=1
    push edx            ; eflags
    push 0x1B           ; cs  — GDT[3] | RPL=3
    push eax            ; eip

    iret