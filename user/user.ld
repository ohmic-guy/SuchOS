ENTRY(_start)

SECTIONS {
    . = 0x040000;

    .text : {
        user/start.o(.text)
        *(.text*)
    }

    .rodata : { *(.rodata*) }
    .data   : { *(.data)   }

    .bss : {
        *(.bss)
        *(COMMON)
    }
}