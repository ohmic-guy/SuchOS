Project: SuchOS
Day 10 complete.

State:
- Scheduler: round-robin, preemptive via IRQ0 timer
- PCB: process_t with pid, state, esp, kstack (4KB aligned)
- Context switch: fake iret frame on kernel stack, common_exit in isr.asm
- sched_enter_first: cli + iret into first process
- sched_tick: saves current esp, finds next READY proc, sets TSS esp0
- sched_exit: marks DEAD, switches to next or halts
- Task A + B: raw x86 bytes, ESI counter, print A/B x5 via int 0x80
- IRQ0 unmasked in PIC (0xFC)
- sys_exit routes to sched_exit when scheduler active
- sched_esp_ptr: shared between C and NASM for context switch

Files added:
- kernel/sched.h/c

Files updated:
- kernel/isr.asm     — common_exit, sched_enter_first, sched_esp_ptr
- kernel/isr.c       — IRQ0 routes to sched_tick
- kernel/pic.c       — IRQ0 unmasked
- kernel/syscall.c   — sys_exit routes to sched_exit
- kernel/usermode.h/c — sched_run_tasks, task_a/b raw bytes
- kernel/shell.c     — sched + ps commands
- kernel/kernel.c    — v0.10

Next session (Day 11):
- SMEP + SMAP (CR4 bits 20 + 21)
- Separate kernel/user address spaces
- ASLR: randomise user load base via RDTSC seed
- Audit log: ring buffer, logs syscalls + exceptions