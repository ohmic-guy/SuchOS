Project: SuchOS
Day 9 complete.

State:
- ELF32 loader: parses ELF header, program headers, loads PT_LOAD segments
- User program compiled with i686-elf-gcc 13.2.0
- ELF binary embedded in kernel via objcopy -I binary
- User program runs in ring 3 at 0x040000, stack at 0x078000
- sys_write + sys_exit called from real C code via int 0x80
- paging: CPUID check before EFER.NXE — works on default QEMU CPU
- NX applied conditionally based on CPU support
- stage2 loads 63 sectors (was 32) — handles larger kernel.bin

Files added:
- kernel/elf.h/c
- user/syscall.h
- user/start.asm
- user/hello.c
- user/user.ld

Files updated:
- kernel/usermode.h/c  — usermode_run_elf added
- kernel/paging.c      — CPUID NX check
- kernel/shell.c       — elf command
- kernel/kernel.c      — v0.9
- boot/stage2.asm      — 63 sectors
- Makefile             — cross-compiler, user build, objcopy embed

Next session (Day 10):
- Round-robin process scheduler
- Process control block (PCB)
- Context switch in assembly
- Timer IRQ0 drives scheduling
- sys_exit returns to scheduler instead of halting