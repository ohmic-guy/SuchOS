# SuchOS

SuchOS is a custom 32-bit x86 (i386) operating system kernel built from scratch. It features a custom two-stage bootloader, a basic shell, user-mode execution (Ring 3), paging, physical memory management, an ELF32 binary loader, and a rudimentary process scheduler.

## Features

- **Custom Bootloader**: A two-stage bootloader written in assembly that sets up the environment and loads the kernel into memory (up to 63 sectors).
- **Core Kernel**: GDT, IDT, TSS, and ISR/PIC configurations.
- **Memory Management**: Physical Memory Management (PMM) and Heap allocation, plus Paging with PAE, NX (No-eXecute) bit, and Ring 3 memory support.
- **Drivers**: VGA text mode driver for display and a basic PS/2 keyboard driver.
- **User Mode & System Calls**: Supports executing user-space code in Ring 3 and interacting with the kernel via `int 0x80` system calls (e.g., `sys_write`, `sys_exit`).
- **ELF32 Loader**: Parses ELF headers and program headers, and loads `PT_LOAD` segments for executing `.elf` binaries.
- **Process Scheduler**: Basic round-robin process scheduler driven by Timer IRQ0.
- **Security**: Stack smashing protection with stack canaries (`-fstack-protector-strong`).
- **Interactive Shell**: A basic built-in shell that provides commands to interact with the OS and launch user binaries.

## Prerequisites

### Linux
To build and run SuchOS, you need the following dependencies installed on your Linux system:

- **Make**: For building the project.
- **NASM**: The Netwide Assembler for compiling the bootloader and assembly routines.
- **GCC / GNU Binutils**: Host compiler and linker.
- **QEMU**: Specifically `qemu-system-i386` for running the built floppy image.
- **i686-elf Cross-Compiler**: You need a cross-compiler targeting `i686-elf` installed in your home directory (specifically `~/opt/cross/bin/i686-elf-gcc` and `~/opt/cross/bin/i686-elf-ld`). This is used to build user-space programs (`user/*.c` and `user/*.asm`) without linking against the host OS libraries.

### Windows
For Windows users, the recommended and easiest approach is to use **Windows Subsystem for Linux (WSL)** (preferably WSL 2 with an Ubuntu distribution).

1. **Install WSL**: Open PowerShell as Administrator and run `wsl --install`. Restart if necessary, then set up your Ubuntu username and password.
2. **Install Base Tools**: Open your WSL Ubuntu terminal and install the required packages:
   ```bash
   sudo apt update
   sudo apt install build-essential nasm qemu-system-x86
   ```
3. **i686-elf Cross-Compiler**: Inside WSL, you must build and install the `i686-elf` cross-compiler to `~/opt/cross/bin` exactly like on a native Linux system. This is required for building the user-space applications.

## Project Structure

- `boot/`: Contains the bootloader assembly code (`boot.asm`, `stage2.asm`).
- `kernel/`: Contains the core kernel source files (C and Assembly), including drivers, memory management, and process scheduling.
- `user/`: Contains the source code for user-mode applications and the custom `syscall.h` wrapper for user space.
- `Makefile`: Defines the build rules for the kernel, bootloader, user-mode binaries, and the final `floppy.img`.

## Build & Run Instructions

1. **Build the OS**:
   To compile the kernel, bootloader, user programs, and assemble them into a floppy disk image (`floppy.img`), run:
   ```bash
   make all
   ```
   *(or simply `make`)*

2. **Run the OS in QEMU**:
   To start SuchOS in the QEMU emulator, run:
   ```bash
   make run
   ```
   This will automatically build the floppy image if necessary and launch QEMU. Once it boots, you will be greeted by the `SuchOS` terminal. Type `help` to see available commands or `sched` to test the scheduler.

3. **Clean the Build Space**:
   To remove all compiled object files, binaries, and the floppy image, run:
   ```bash
   make clean
   ```

## Development

Check out `SESSION.md` for a log of development progress, including completed milestones and plans for upcoming features like context switching and process control blocks (PCBs).
