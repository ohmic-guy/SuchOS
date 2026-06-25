Project: SuchOS
Day 4 complete.

State:
- GDT: moved to C (gdt.c + gdt_flush.asm), 5 gates — null, kernel code/data, user code/data
- PMM: bitmap allocator, 32MB flat memory, marks kernel frames used at init
- Heap: bump allocator, 4MB heap above kernel_end, 4-byte aligned kmalloc
- kfree: stub — proper free list Day 5
- kernel_end exposed via linker symbol, heap base page-aligned above it
- kmalloc test runs at boot, prints ptr address
- Build: nasm + gcc -m32 + ld + objcopy + make + qemu

Files added:
- kernel/gdt.h/c
- kernel/gdt_flush.asm
- kernel/pmm.h/c
- kernel/heap.h/c

Files updated:
- kernel/kernel.c
- kernel/linker.ld
- Makefile

Next session (Day 5):
- Paging: enable CR0 paging bit, identity map first 4MB
- Page directory + page tables
- kfree proper free list (optional, can defer)
- Terminal command buffer — parse what user types, execute basic commands