Project: SuchOS
Day 7 complete.

State:
- TSS: loaded at GDT[5], kernel stack at esp0
- GDT: expanded to 6 descriptors including TSS slot
- Ring 3: user pages at 0x040000-0x07FFFF with PAGE_USER
- PD entry 0 has PAGE_USER — entire pt0 walk user-accessible
- usermode_enter: NASM iret frame, CS=0x1B SS=0x23
- GPF from ring 3 caught by ring 0 IDT handler
- Privilege separation confirmed working
- shell: ring3 command triggers usermode test

Files added:
- kernel/tss.h/c
- kernel/tss_flush.asm
- kernel/usermode.h/c
- kernel/usermode.asm → kernel/usermode_asm.o

Files updated:
- kernel/gdt.h/c — 6 entries, gdt_set_gate public
- kernel/paging.c — PAGE_USER on pd[0] + user PTEs
- kernel/isr.c — ring level printed in exceptions
- kernel/shell.c — ring3 command
- kernel/kernel.c
- Makefile

Next session (Day 8):
- Syscall interface via int 0x80
- register eax = syscall number
- sys_write, sys_read, sys_exit
- User program calls kernel via syscall instead of crashing