ENTRY(kernel_main)
OUTPUT_FORMAT("binary")

SECTIONS {
    . = 0x10000;
    
    .text : {
        *(.text.entry)
        *(.text*)
        *(.rodata*)
    }
    
    .data : {
        *(.data)
    }
    
    .bss : {
        *(COMMON)
        *(.bss)
    }
}
