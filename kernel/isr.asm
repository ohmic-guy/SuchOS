[BITS 32]

%macro ISR_NOERR 1
[global isr%1]
isr%1:
    push dword 0
    push dword %1
    jmp isr_common
%endmacro

%macro ISR_ERR 1
[global isr%1]
isr%1:
    push dword %1
    jmp isr_common
%endmacro

%macro IRQ 2
[global irq%1]
irq%1:
    push dword 0
    push dword %2
    jmp irq_common
%endmacro

ISR_NOERR 0
ISR_NOERR 1
ISR_NOERR 2
ISR_NOERR 3
ISR_NOERR 4
ISR_NOERR 5
ISR_NOERR 6
ISR_NOERR 7
ISR_ERR   8
ISR_NOERR 9
ISR_ERR   10
ISR_ERR   11
ISR_ERR   12
ISR_ERR   13
ISR_ERR   14
ISR_NOERR 15
ISR_NOERR 16
ISR_ERR   17
ISR_NOERR 18
ISR_NOERR 19
ISR_NOERR 20
ISR_NOERR 21
ISR_NOERR 22
ISR_NOERR 23
ISR_NOERR 24
ISR_NOERR 25
ISR_NOERR 26
ISR_NOERR 27
ISR_NOERR 28
ISR_NOERR 29
ISR_ERR   30
ISR_NOERR 31

IRQ  0, 32
IRQ  1, 33
IRQ  2, 34
IRQ  3, 35
IRQ  4, 36
IRQ  5, 37
IRQ  6, 38
IRQ  7, 39
IRQ  8, 40
IRQ  9, 41
IRQ 10, 42
IRQ 11, 43
IRQ 12, 44
IRQ 13, 45
IRQ 14, 46
IRQ 15, 47

[global isr128]
isr128:
    push dword 0
    push dword 128
    jmp isr_common

[extern isr_handler]
[extern irq_handler]
[extern sched_esp_ptr]

; ── Shared exit path ─────────────────────────────────────────
; Called by both isr_common and irq_common after handler returns.
; Checks sched_esp_ptr — if set, switches to new process stack.
common_exit:
    mov eax, [sched_esp_ptr]
    test eax, eax
    jz .no_switch
    mov esp, [eax]              ; load new process's saved GS pointer
    mov dword [sched_esp_ptr], 0
.no_switch:
    pop gs
    pop fs
    pop es
    pop ds
    popa
    add esp, 8                  ; discard int_no + err_code
    iret

; ── Exception / syscall path ─────────────────────────────────
isr_common:
    pusha
    push ds
    push es
    push fs
    push gs
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    push esp                    ; registers_t* = &saved_gs
    call isr_handler
    add esp, 4                  ; discard registers_t* arg
    jmp common_exit

; ── Hardware IRQ path ─────────────────────────────────────────
irq_common:
    pusha
    push ds
    push es
    push fs
    push gs
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    push esp
    call irq_handler
    add esp, 4
    jmp common_exit

; ── Scheduler entry point ─────────────────────────────────────
; Called from C (sched_prepare_first sets sched_esp_ptr first).
; Does NOT return — irets into first scheduled process.
[global sched_enter_first]
sched_enter_first:
    mov eax, [sched_esp_ptr]
    mov esp, [eax]
    mov dword [sched_esp_ptr], 0
    pop gs
    pop fs
    pop es
    pop ds
    popa
    add esp, 8
    iret

[global idt_load]
idt_load:
    mov eax, [esp+4]
    lidt [eax]
    ret