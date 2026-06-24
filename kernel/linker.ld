ENTRY(kernel_main)

SECTIONS {
    . = 0x10000;

    .text : {
        kernel/kernel.o(.text)
        *(.text)
    }

    .rodata : { *(.rodata*) }

    .data : { *(.data) }

    .bss : {
        *(.bss)
        *(COMMON)
    }
}