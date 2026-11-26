# MOS 6502 Emulator

A cycle-accurate MOS 6502 CPU emulator written in C. The implementation covers all 151 official instructions, addressing modes, hardware interrupts, and stack operations.

## Overview

The project is organized into three main components:

- **CPU Core (`cpu.h`, `cpu.c`):** Handles instruction decoding, register state, status flags, addressing mode resolution, and instruction cycle counting.
- **Memory Bus (`bus.h`, `bus.c`):** Provides a decoupled 64 KB address space with `bus_read` and `bus_write` interfaces, along with a binary loader for ROMs and test suites.
- **Test Runner (`main.c`):** Loads a 6502 binary into memory and executes instructions step by step until completion.
