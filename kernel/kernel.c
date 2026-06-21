void __attribute__((section(".text.entry"))) kernel_main() {
    const char* str = "Kernel Activated Successfully!";
    char* video_memory = (char*) 0xB8000;
    int offset = 80 * 2 * 2;

    for (int i = 0; str[i] != '\0'; i++) {
        video_memory[offset] = str[i];
        video_memory[offset + 1] = 0x0A;
        offset += 2;
    }

    while (1) {
        __asm__ volatile ("hlt");
    }
}
