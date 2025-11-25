# N12OS - Operating System Kernel & Terminal Calculator - Technical Documentation

## 1. Project Overview & Architectural Vision

This Documentation Provides A Detailed Technical Analysis Of N12OS, A Custom 32-Bit Operating System Kernel Developed From Scratch For The X86 (I386) Architecture. The Project Is An Educational Implementation Of Low-Level System Programming, Demonstrating The Fundamentals Of Kernel Bootstrapping, Hardware Interaction (Keyboard/VGA), And Standard Library Implementation.

The Core Feature Of This OS Version Is A Terminal Calculator, Which Runs Directly In Kernel Mode (Ring 0) Immediately After Boot. It Bypasses The Need For A User Space Or Shell, Interacting Directly With Hardware Ports To Accept User Input And Perform Arithmetic Operations.

## 2. Technology Stack & Toolchain

The Project Relies On A Minimal, Cross-Platform Toolchain Designed For Bare-Metal Development.

- **Kernel Language**: C (GNU C11)
    - **Usage**: Used For The Core Logic, Hardware Drivers, And The Calculator Application.
    - **Configuration**: Compiled With `-ffreestanding` To Ensure No Dependency On The Host Operating System's Standard Library.

- **Assembly**: GNU Assembler (GAS)
    - **Usage**: Used For The Bootstrap Code (`boot.S`) And C Runtime Initialization (`crti.S`, `crtn.S`).
    - **Function**: Sets Up The Multiboot Headers And Stack Before Handing Control To C Code.

- **Build System**: GNU Make & Shell Scripts
    - **Automation**: A Set Of Shell Scripts (`config.sh`, `make.sh`, `clean.sh`) Automates The Cross-Compilation Environment Configuration, Build Process, And ISO Generation.

- **Bootloader**: GRUB (Multiboot)
    - **Compliance**: The Kernel Is Compliant With The Multiboot Specification (0x1BADB002 Magic Number), Allowing It To Be Loaded By GRUB.

- **Emulation**: QEMU
    - **Testing**: Used For Testing The Generated ISO Image (`HDH.iso`).

- **Standard Library**: Custom Libc (Libk)
    - **Implementation**: A Freestanding Implementation Of Standard C Functions (`printf`, `memcpy`, `strlen`) Integrated Directly Into The Kernel.

## 3. System Architecture

### 3.1 Boot Process

- **BIOS/UEFI**: Loads The Bootloader (GRUB).
- **Multiboot**: GRUB Reads The Multiboot Header In `boot.S`, Ensuring The Kernel Is Loaded Into Memory At The 1MB Mark (0x100000).
- **Assembly Entry**: Execution Starts At `_start`. The Stack Pointer (`esp`) Is Initialized To The Top Of A 16KiB Stack Reserved In The `.bss` Section.
- **Runtime Init**: Calls `_init` (Defined In `crti.S`) To Handle Global Constructors (Though C++ Isn't Used, This Is Standard Practice).
- **Kernel Main**: Control Is Transferred To `kernel_main()` In `kernel.c`.
- **Halt**: If `kernel_main` Returns, The CPU Enters An Infinite Loop (`hlt`) With Interrupts Disabled (`cli`).

### 3.2 Memory Management

- **Linker Script**: The `linker.ld` File Dictates The Memory Layout.
    - `.text`: Code Section, Aligned To 4KB Blocks, Starting At 1MB.
    - `.rodata`: Read-Only Data.
    - `.data`: Initialized Read-Write Data.
    - `.bss`: Uninitialized Data (And Stack), Zeroed Out By The Bootloader.

### 3.3 Hardware Abstraction

#### VGA Text Mode Driver

The OS Uses Memory-Mapped I/O (MMIO) To Display Text.

- **Address**: 0xB8000.
- **Format**: Each Character Takes 2 Bytes (1 Byte For ASCII Character, 1 Byte For Color Attributes).
- **Implementation**: The `printf` Function In `kernel.c` Writes Directly To This Memory Address, Managing Cursor Position (`cursor_x`, `cursor_y`) And Handling Newlines.

#### PS/2 Keyboard Driver

Input Is Handled Via Port-Mapped I/O (PMIO).

- **Port**: 0x60 (Data Port).
- **Mechanism**: The `inb` Function Uses Inline Assembly To Read A Byte From The Hardware Port.
- **Keymap**: A Static Lookup Table Converts Scan Codes (E.g., 0x1C For Enter) Into ASCII Characters.
- **Polling**: The `keyboard_getchar` Function Spin-Waits Until A Valid Scan Code Is Received.

## 4. Core Feature: Terminal Calculator

The Primary Application Of N12OS Is A Mathematical Calculator That Operates In The Kernel Main Loop.

### 4.1 Logic Flow (`kernel_main`)

- **Menu Display**: Prints Options For Addition, Subtraction, Multiplication, And Division.
- **Input Handling**: Waits For A Single Keystroke To Select An Operation.
- **Operand Parsing**:
    - Calls `get_number()`, Which Reads Characters Buffer Until A Newline Is Detected.
    - Converts The String Buffer To A 64-Bit Integer (`int64_t`).
    - Supports Negative Numbers.
- **Computation**: Performs The Selected Arithmetic Operation.
    - Includes A Safety Check For Division By Zero.
- **Output**: Uses `print_number()` To Convert The Result Back To A String And Display It.
- **Loop**: Pauses For User Acknowledgment And Then Clears The Screen To Restart.

### 4.2 Functions

- **get_number()**: Robust Integer Parsing From The Raw Keyboard Buffer.
- **print_number(int64_t num)**: Implementation Of `itoa` (Integer To ASCII) To Render Results.
- **clear_screen()**: Resets The VGA Buffer To Spaces With The Default Color Attribute.

## 5. Custom Standard Library (Libc)

To Maintain Independence, N12OS Implements Essential C Library Functions.

### 5.1 String Manipulation (`libc/string/`)

- **memcpy**: Copies Memory Blocks.
- **memset**: Fills Memory With A Constant Byte.
- **strlen**: Calculates String Length.
- **memmove**: Safely Copies Overlapping Memory Regions.

### 5.2 Input/Output (`libc/stdio/`)

- **printf**: A Custom Implementation Of The Variadic Print Function. It Supports Format Specifiers Like `%s` (String) And `%c` (Char). It Relies On `putchar` For The Actual Output.
- **putchar**: Delegates Character Printing To The Kernel's Terminal Driver (`terminal_write`) When Compiled As `libk`.

### 5.3 System Definitions

- **sys/cdefs.h**: Defines The `__myos_libc` Macro To Identify The Environment.
- **stdlib.h**: Defines `abort()`, Which Prints A Panic Message And Hangs The CPU.

## 6. Build & Deployment

### 6.1 Configuration

The `config.sh` Script Establishes Environment Variables For The Cross-Compiler:

- **Target Architecture**: i686-elf (32-Bit X86).
- **Compiler Flags**: `-O2 -g` (Optimized With Debug Symbols).
- **Sysroot**: Defines A `sysroot/` Directory To Simulate The OS File System Structure During The Build.

### 6.2 Build Process (`make.sh`)

This Script Orchestrates The Entire Build Pipeline:

- **Header Installation**: Installs System Headers To The Sysroot.
- **Compilation**: Calls `make install` For Both `libc` And `kernel` Projects.
- **ISO Creation**:
    - Creates A Directory Structure `isodir/boot/grub`.
    - Copies The Compiled `os.kernel` To The Boot Directory.
    - Generates A `grub.cfg` File Defining The Boot Entry "HDH".
    - Uses `grub-mkrescue` To Bundle Everything Into `HDH.iso`.
- **Emulation**: Automatically Launches `qemu-system-i386` To Boot The Generated ISO.

## 7. Project Structure

```plaintext
N12OS/
├── kernel/                 # The Operating System Kernel
│   ├── arch/i386/          # Architecture-Specific Code (X86)
│   │   ├── boot.S          # Multiboot Entry Point
│   │   ├── linker.ld       # Linker Configuration
│   │   ├── tty.c           # VGA Text Mode Driver
│   │   └── vga.h           # VGA Color Definitions
│   ├── include/kernel/     # Kernel Headers
│   ├── kernel/             # Kernel Source
│   │   └── kernel.c        # Main Entry & Calculator Logic
│   └── Makefile            # Kernel Build Rules
├── libc/                   # Custom C Standard Library
│   ├── include/            # Standard Headers (stdio.h, string.h)
│   ├── stdio/              # I/O Implementation (printf, putchar)
│   ├── stdlib/             # Standard Library (abort)
│   ├── string/             # String Operations (memcpy, strlen)
│   └── Makefile            # Libc Build Rules
├── sysroot/                # Virtual Root File System For Build Artifacts
├── isodir/                 # Staging Area For ISO Creation
├── config.sh               # Environment Variables & Toolchain Setup
├── make.sh                 # Main Build & Run Script
├── clean.sh                # Cleanup Script
└── arch.sh                 # Architecture Detection Helper
```


## 8. License

The Project Appears To Be A Custom Educational Implementation. No Specific License File Was Provided In The Uploaded Documents.
