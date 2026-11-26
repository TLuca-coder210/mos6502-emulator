# MOS 6502 Emulator

A cycle-accurate MOS 6502 CPU emulator written in C99. The core implements all 151 official opcodes, all addressing modes, hardware and software interrupts (`IRQ`, `NMI`, `BRK`), and NMOS Binary-Coded Decimal (BCD) arithmetic.

## Overview

The codebase is split between the CPU execution engine (`cpu.h`, `cpu.c`) and a decoupled 64 KB memory bus (`bus.h`, `bus.c`), with a standalone runner (`main.c`) built to validate the implementation against external test ROMs.

- **Cycle Accuracy:** Every instruction tracks its base cycle count alongside extra cycles incurred on page boundary crossings (`+1` cycle for indexed reads) and taken branches (`+1` cycle when taken, `+2` when crossing a page boundary).
- **Decoupled Bus Interface:** The CPU communicates strictly through `bus_read` and `bus_write`, making it straightforward to map ROM, RAM, or memory-mapped I/O peripherals onto the 16-bit address space.
- **NMOS 6502 Quirks:**
  - **Indirect `JMP` Bug:** `JMP ($xxFF)` wraps around within the same memory page when fetching the high byte of the target address instead of incrementing into the next page.
  - **Decimal Mode (`FLAG_D`):** `ADC` and `SBC` adjust nibble carries and borrows in BCD mode while preserving authentic NMOS flag behavior (`Z`, `N`, `V`, `C`).
  - **Stack & Status Register:** Hardware stack mapped to page `0x0100–0x01FF`, with proper handling of the unused (`U`) and break (`B`) flags during `PHP`, `PLP`, `BRK`, `IRQ`, and `NMI`.
