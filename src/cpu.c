#include "cpu.h"

static inline void stack_push(CPU *cpu, uint8_t value) {
    bus_write(cpu->bus, 0x0100 + cpu->stack_pointer, value);
    cpu->stack_pointer--;
}

static inline uint8_t stack_pop(CPU *cpu){
    cpu->stack_pointer++;
    return bus_read(cpu->bus, 0x0100 + cpu->stack_pointer);
}

void cpu_init(CPU *cpu, Bus *bus){
    cpu->bus = bus;
    cpu->total_cycles = 0;
    cpu_reset(cpu);
}

void cpu_reset(CPU *cpu){
    uint16_t low = bus_read(cpu->bus, 0xFFFC);
    uint16_t high = bus_read(cpu->bus, 0xFFFD);
    cpu->program_counter = (high << 8) | low;
    cpu->accumulator = 0;
    cpu->index_register_X = 0;
    cpu->index_register_Y = 0;
    cpu->stack_pointer = 0xFD;
    cpu->status = FLAG_U | FLAG_I;
    cpu->effective_address = 0;
    cpu->relative_offset = 0;
    cpu->cycles = 8;
}

void cpu_interrupt_masked(CPU *cpu){
    if((cpu->status & FLAG_I) == 0){
        // process interrupted
        bus_write(cpu->bus, 0x0100 | cpu->stack_pointer, (cpu->program_counter >> 8) & 0xFF);
        cpu->stack_pointer -= 1;
        bus_write(cpu->bus, 0x0100 | cpu->stack_pointer, cpu->program_counter & 0xFF);
        cpu->stack_pointer -= 1;

        cpu->status &= ~FLAG_B;
        cpu->status |= (FLAG_U | FLAG_I);
        bus_write(cpu->bus, 0x0100 | cpu->stack_pointer, cpu->status);
        cpu->stack_pointer -= 1;

        uint16_t low = bus_read(cpu->bus, 0xFFFE);
        uint16_t high = bus_read(cpu->bus, 0xFFFF);
        cpu->program_counter =(high << 8) | low;

        cpu->cycles = 7;
        cpu->total_cycles += 7;
    }
}

void cpu_interrupt_unmasked(CPU *cpu){
    bus_write(cpu->bus, 0x0100 | cpu->stack_pointer, (cpu->program_counter >> 8) & 0xFF);
    cpu->stack_pointer--;
    bus_write(cpu->bus, 0x0100 | cpu->stack_pointer, cpu->program_counter & 0xFF);
    cpu->stack_pointer--;

    cpu->status &= ~FLAG_B;
    cpu->status |= (FLAG_U | FLAG_I);
    bus_write(cpu->bus, 0x0100 | cpu->stack_pointer, cpu->status);
    cpu->stack_pointer--;

    uint16_t low = bus_read(cpu->bus, 0xFFFA);
    uint16_t high = bus_read(cpu->bus, 0xFFFB);
    cpu->program_counter = (high << 8) | low;

    cpu->cycles = 8;
    cpu->total_cycles += 8;
}

bool cpu_step(CPU *cpu){
    uint8_t opcode = bus_read(cpu->bus, cpu->program_counter);
    cpu->program_counter++;
    switch(opcode){
        case 0x00: cpu_BRK(cpu, opcode); break;
        case 0x01: case 0x05: case 0x09: case 0x0D:
        case 0x11: case 0x15: case 0x19: case 0x1D: cpu_ORA(cpu, opcode); break;
        case 0x06: case 0x0A: case 0x0E: case 0x16: case 0x1E: cpu_ASL(cpu, opcode); break;
        case 0x08: cpu_PHP(cpu, opcode); break;
        case 0x10: cpu_BPL(cpu, opcode); break;
        case 0x18: cpu_CLC(cpu, opcode); break;
        case 0x20: cpu_JSR(cpu, opcode); break;
        case 0x21: case 0x25: case 0x29: case 0x2D:
        case 0x31: case 0x35: case 0x39: case 0x3D: cpu_AND(cpu, opcode); break;
        case 0x24: case 0x2C: cpu_BIT(cpu, opcode); break;
        case 0x26: case 0x2A: case 0x2E: case 0x36: case 0x3E: cpu_ROL(cpu, opcode); break;
        case 0x28: cpu_PLP(cpu, opcode); break;
        case 0x30: cpu_BMI(cpu, opcode); break;
        case 0x38: cpu_SEC(cpu, opcode); break;
        case 0x40: cpu_RTI(cpu, opcode); break;
        case 0x41: case 0x45: case 0x49: case 0x4D:
        case 0x51: case 0x55: case 0x59: case 0x5D: cpu_EOR(cpu, opcode); break;
        case 0x46: case 0x4A: case 0x4E: case 0x56: case 0x5E: cpu_LSR(cpu, opcode); break;
        case 0x48: cpu_PHA(cpu, opcode); break;
        case 0x4C: case 0x6C: cpu_JMP(cpu, opcode); break;
        case 0x50: cpu_BVC(cpu, opcode); break;
        case 0x58: cpu_CLI(cpu, opcode); break;
        case 0x60: cpu_RTS(cpu, opcode); break;
        case 0x61: case 0x65: case 0x69: case 0x6D:
        case 0x71: case 0x75: case 0x79: case 0x7D: cpu_ADC(cpu, opcode); break;
        case 0x66: case 0x6A: case 0x6E: case 0x76: case 0x7E: cpu_ROR(cpu, opcode); break;
        case 0x68: cpu_PLA(cpu, opcode); break;
        case 0x70: cpu_BVS(cpu, opcode); break;
        case 0x78: cpu_SEI(cpu, opcode); break;
        case 0x81: case 0x85: case 0x8D: case 0x91:
        case 0x95: case 0x99: case 0x9D: cpu_STA(cpu, opcode); break;
        case 0x84: case 0x8C: case 0x94: cpu_STY(cpu, opcode); break;
        case 0x86: case 0x8E: case 0x96: cpu_STX(cpu, opcode); break;
        case 0x88: cpu_DEY(cpu, opcode); break;
        case 0x8A: cpu_TXA(cpu, opcode); break;
        case 0x90: cpu_BCC(cpu, opcode); break;
        case 0x98: cpu_TYA(cpu, opcode); break;
        case 0x9A: cpu_TXS(cpu, opcode); break;
        case 0xA0: case 0xA4: case 0xAC: case 0xB4: case 0xBC: cpu_LDY(cpu, opcode); break;
        case 0xA1: case 0xA5: case 0xA9: case 0xAD:
        case 0xB1: case 0xB5: case 0xB9: case 0xBD: cpu_LDA(cpu, opcode); break;
        case 0xA2: case 0xA6: case 0xAE: case 0xB6: case 0xBE: cpu_LDX(cpu, opcode); break;
        case 0xA8: cpu_TAY(cpu, opcode); break;
        case 0xAA: cpu_TAX(cpu, opcode); break;
        case 0xB0: cpu_BCS(cpu, opcode); break;
        case 0xB8: cpu_CLV(cpu, opcode); break;
        case 0xBA: cpu_TSX(cpu, opcode); break;
        case 0xC0: case 0xC4: case 0xCC: cpu_CPY(cpu, opcode); break;
        case 0xC1: case 0xC5: case 0xC9: case 0xCD:
        case 0xD1: case 0xD5: case 0xD9: case 0xDD: cpu_CMP(cpu, opcode); break;
        case 0xC6: case 0xCE: case 0xD6: case 0xDE: cpu_DEC(cpu, opcode); break;
        case 0xC8: cpu_INY(cpu, opcode); break;
        case 0xCA: cpu_DEX(cpu, opcode); break;
        case 0xD0: cpu_BNE(cpu, opcode); break;
        case 0xD8: cpu_CLD(cpu, opcode); break;
        case 0xE0: case 0xE4: case 0xEC: cpu_CPX(cpu, opcode); break;
        case 0xE1: case 0xE5: case 0xE9: case 0xED:
        case 0xF1: case 0xF5: case 0xF9: case 0xFD: cpu_SBC(cpu, opcode); break;
        case 0xE6: case 0xEE: case 0xF6: case 0xFE: cpu_INC(cpu, opcode); break;
        case 0xE8: cpu_INX(cpu, opcode); break;
        case 0xEA: cpu_NOP(cpu, opcode); break;
        case 0xF0: cpu_BEQ(cpu, opcode); break;
        case 0xF8: cpu_SED(cpu, opcode); break;
        default:
            return false;
    }
    return true;
}

static uint16_t get_effective_adress(CPU *cpu, uint8_t opcode, bool *page_crossed){
    *page_crossed = false;
    if(opcode == 0x09 || opcode == 0x29 || opcode == 0x49 || opcode == 0x69 ||
       opcode == 0xA0 || opcode == 0xA2 || opcode == 0xA9 || opcode == 0xC0 ||
       opcode == 0xC9 || opcode == 0xE0 || opcode == 0xE9){
        // immediate addressing mode
        uint16_t old_program_counter = cpu->program_counter;
        cpu->program_counter++;
        return old_program_counter;
    }
    if(opcode == 0x05 || opcode == 0x06 || opcode == 0x24 || opcode == 0x25 ||
       opcode == 0x26 || opcode == 0x45 || opcode == 0x46 || opcode == 0x65 ||
       opcode == 0x66 || opcode == 0x84 || opcode == 0x85 || opcode == 0x86 ||
       opcode == 0xA4 || opcode == 0xA5 || opcode == 0xA6 || opcode == 0xC4 ||
       opcode == 0xC5 || opcode == 0xC6 || opcode == 0xE4 || opcode == 0xE5 ||
       opcode == 0xE6){
        // zero page addressing mode
        uint16_t old_program_counter = cpu->program_counter;
        cpu->program_counter++;
        return bus_read(cpu->bus, old_program_counter);
    }
    if(opcode == 0x15 || opcode == 0x16 || opcode == 0x35 || opcode == 0x36 ||
       opcode == 0x55 || opcode == 0x56 || opcode == 0x75 || opcode == 0x76 ||
       opcode == 0x94 || opcode == 0x95 || opcode == 0xB4 || opcode == 0xB5 ||
       opcode == 0xD5 || opcode == 0xD6 || opcode == 0xF5 || opcode == 0xF6){
        // zero page X addressing mode
        uint16_t old_program_counter = cpu->program_counter;
        cpu->program_counter++;
        uint8_t base = bus_read(cpu->bus, old_program_counter);
        return (base + cpu->index_register_X) & 0xFF;
    }
    if(opcode == 0x96 || opcode == 0xB6){
        // zero page Y addressing mode
        uint16_t old_program_counter = cpu->program_counter;
        cpu->program_counter++;
        uint8_t base = bus_read(cpu->bus, old_program_counter);
        return (base + cpu->index_register_Y) & 0xFF;
    }
    if(opcode == 0x0D || opcode == 0x0E || opcode == 0x2C || opcode == 0x2D ||
       opcode == 0x2E || opcode == 0x4D || opcode == 0x4E || opcode == 0x6D ||
       opcode == 0x6E || opcode == 0x8C || opcode == 0x8D || opcode == 0x8E ||
       opcode == 0xAC || opcode == 0xAD || opcode == 0xAE || opcode == 0xCC ||
       opcode == 0xCD || opcode == 0xCE || opcode == 0xEC || opcode == 0xED ||
       opcode == 0xEE){
        // absolute addressing mode
        uint16_t low = bus_read(cpu->bus, cpu->program_counter);
        cpu->program_counter++;
        uint16_t high = bus_read(cpu->bus, cpu->program_counter);
        cpu->program_counter++;
        return (high << 8) | low;
    }
    if(opcode == 0x1D || opcode == 0x1E || opcode == 0x3D || opcode == 0x3E ||
       opcode == 0x5D || opcode == 0x5E || opcode == 0x7D || opcode == 0x7E ||
       opcode == 0x9D || opcode == 0xBC || opcode == 0xBD || opcode == 0xDD ||
       opcode == 0xDE || opcode == 0xFD || opcode == 0xFE){
        // absolute X addressing mode
        uint16_t low = bus_read(cpu->bus, cpu->program_counter);
        cpu->program_counter++;
        uint16_t high = bus_read(cpu->bus, cpu->program_counter);
        cpu->program_counter++;
        uint16_t base = (high << 8) | low;
        uint16_t final_address = base + cpu->index_register_X;
        if((final_address & 0xFF00) != (base & 0xFF00)){
            *page_crossed = true;
        }
        return final_address;
    }
    if(opcode == 0x19 || opcode == 0x39 || opcode == 0x59 || opcode == 0x79 ||
       opcode == 0x99 || opcode == 0xB9 || opcode == 0xBE || opcode == 0xD9 ||
       opcode == 0xF9){
        // absolute Y addressing mode
        uint16_t low = bus_read(cpu->bus, cpu->program_counter);
        cpu->program_counter++;
        uint16_t high = bus_read(cpu->bus, cpu->program_counter);
        cpu->program_counter++;
        uint16_t base = (high << 8) | low;
        uint16_t final_address = base + cpu->index_register_Y;
        if((final_address & 0xFF00) != (base & 0xFF00)){
            *page_crossed = true;
        }
        return final_address;
    }
    if(opcode == 0x01 || opcode == 0x21 || opcode == 0x41 || opcode == 0x61 ||
       opcode == 0x81 || opcode == 0xA1 || opcode == 0xC1 || opcode == 0xE1){
        // indirect X addressing mode
        uint16_t old_program_counter = cpu->program_counter;
        cpu->program_counter++;
        uint8_t zp_pointer = bus_read(cpu->bus, old_program_counter);
        uint8_t pointer = (zp_pointer + cpu->index_register_X) & 0xFF;
        uint16_t low = bus_read(cpu->bus, pointer);
        uint16_t high = bus_read(cpu->bus, (pointer + 1) & 0xFF);
        return (high << 8) | low;
    }
    if(opcode == 0x11 || opcode == 0x31 || opcode == 0x51 || opcode == 0x71 ||
       opcode == 0x91 || opcode == 0xB1 || opcode == 0xD1 || opcode == 0xF1){
        // indirect Y addressing mode
        uint16_t old_program_counter = cpu->program_counter;
        cpu->program_counter++;
        uint8_t zp_pointer = bus_read(cpu->bus, old_program_counter);
        uint16_t low = bus_read(cpu->bus, zp_pointer);
        uint16_t high = bus_read(cpu->bus, (zp_pointer + 1) & 0xFF);
        uint16_t base = (high << 8) | low;
        uint16_t final_address = base + cpu->index_register_Y;
        if((final_address & 0xFF00) != (base & 0xFF00)){
            *page_crossed = true;
        }
        return final_address;
    }
    return 0;
}

void cpu_ADC(CPU *cpu, uint8_t opcode){
    bool page_crossed = false;
    uint16_t address = get_effective_adress(cpu, opcode, &page_crossed);
    cpu->effective_address = address;

    uint8_t value = bus_read(cpu->bus, cpu->effective_address);
    uint8_t carry = (cpu->status & FLAG_C) ? 1 : 0;
    uint16_t sum = (uint16_t)cpu->accumulator + (uint16_t)value + (uint16_t)carry;

    if((sum & 0xFF) == 0){
        cpu->status |= FLAG_Z;
    }
    else{
        cpu->status &= ~FLAG_Z;
    }

    if((cpu->status & FLAG_D) != 0){
        if(((cpu->accumulator & 0x0F) + (value & 0x0F) + carry) > 0x09){
            sum += 0x06;
        }
        if((sum & 0x80) != 0){
            cpu->status |= FLAG_N;
        }
        else{
            cpu->status &= ~FLAG_N;
        }
        if((~(cpu->accumulator ^ value) & (cpu->accumulator ^ sum) & 0x80) != 0){
            cpu->status |= FLAG_V;
        }
        else{
            cpu->status &= ~FLAG_V;
        }
        if(sum > 0x99){
            sum += 0x60;
        }
    }
    else{
        if((sum & 0x80) != 0){
            cpu->status |= FLAG_N;
        }
        else{
            cpu->status &= ~FLAG_N;
        }
        if((~(cpu->accumulator ^ value) & (cpu->accumulator ^ sum) & 0x80) != 0){
            cpu->status |= FLAG_V;
        }
        else{
            cpu->status &= ~FLAG_V;
        }
    }

    if(sum > 0xFF){
        cpu->status |= FLAG_C;
    }
    else{
        cpu->status &= ~FLAG_C;
    }

    cpu->accumulator = sum & 0xFF;

    if(opcode == 0x69) cpu->cycles = 2;
    if(opcode == 0x65) cpu->cycles = 3;
    if(opcode == 0x75 || opcode == 0x6D) cpu->cycles = 4;
    if(opcode == 0x7D || opcode == 0x79) cpu->cycles = 4 + (page_crossed ? 1 : 0);
    if(opcode == 0x61) cpu->cycles = 6;
    if(opcode == 0x71) cpu->cycles = 5 + (page_crossed ? 1 : 0);
    cpu->total_cycles += cpu->cycles;
}

void cpu_AND(CPU *cpu, uint8_t opcode){
    bool page_crossed = false;
    uint16_t address = get_effective_adress(cpu, opcode, &page_crossed);
    cpu->effective_address = address;

    uint8_t value = bus_read(cpu->bus, cpu->effective_address);
    uint16_t operation_result = (uint16_t)cpu->accumulator & (uint16_t)value;
    operation_result &= 0xFF;
    cpu->accumulator = operation_result;

    if(operation_result == 0){
        cpu->status |= FLAG_Z;
    }
    else{
        cpu->status &= ~FLAG_Z;
    }

    if(operation_result & 0x80){
        cpu->status |= FLAG_N;
    }
    else{
        cpu->status &= ~FLAG_N;
    }

    if(opcode == 0x29) cpu->cycles = 2;
    if(opcode == 0x25) cpu->cycles = 3;
    if(opcode == 0x35 || opcode == 0x2D) cpu->cycles = 4;
    if(opcode == 0x3D || opcode == 0x39) cpu->cycles = 4 + (page_crossed ? 1 : 0);
    if(opcode == 0x21) cpu->cycles = 6;
    if(opcode == 0x31) cpu->cycles = 5 + (page_crossed ? 1 : 0);
    cpu->total_cycles += cpu->cycles;
}

// trebuie facuta functie separata pentru modificarea flag urilor

void cpu_ASL(CPU *cpu, uint8_t opcode){
    uint8_t value;
    uint16_t address;
    if(opcode == 0x0A){
        value = cpu->accumulator;
    }
    else{
        bool page_crossed = false;
        address = get_effective_adress(cpu, opcode, &page_crossed);
        cpu->effective_address = address;
        value = bus_read(cpu->bus, address);
    }
    // got the value
    if(value & 0x80){
        cpu->status |= FLAG_C;
    }
    else{
        cpu->status &= ~FLAG_C;
    }

    uint8_t result = (value << 1);
    if(opcode == 0x0A){
        cpu->accumulator = result;
    }
    else{
        bus_write(cpu->bus, cpu->effective_address, (uint8_t)result);
    }

    if(result == 0){
        cpu->status |= FLAG_Z;
    }
    else{
        cpu->status &= ~FLAG_Z;
    }

    if(result & 0x80){
        cpu->status |= FLAG_N;
    }
    else{
        cpu->status &= ~FLAG_N;
    }

    if(opcode == 0x0A) cpu->cycles = 2;
    else if(opcode == 0x06) cpu->cycles = 5;
    else if(opcode == 0x16 || opcode == 0x0E) cpu->cycles = 6;
    else if(opcode == 0x1E) cpu->cycles = 7;
    cpu->total_cycles += cpu->cycles;
}

void cpu_BCC(CPU *cpu, uint8_t opcode){
    cpu->relative_offset = bus_read(cpu->bus, cpu->program_counter);
    cpu->program_counter++;
    cpu->cycles = 2;

    if((cpu->status & FLAG_C) == 0){
        cpu->cycles++;
        uint16_t new_program_counter = cpu->program_counter + (int8_t)cpu->relative_offset;
        if((new_program_counter & 0xFF00) != (cpu->program_counter & 0xFF00)){
            cpu->cycles += 1;
        }
        cpu->program_counter = new_program_counter;
    }
    cpu->total_cycles += cpu->cycles;
}

void cpu_BCS(CPU *cpu, uint8_t opcode){
    cpu->relative_offset = bus_read(cpu->bus, cpu->program_counter);
    cpu->program_counter++;
    cpu->cycles = 2;

    if((cpu->status & FLAG_C) != 0){
        cpu->cycles++;
        uint16_t new_program_counter = cpu->program_counter + (int8_t)cpu->relative_offset;
        if((new_program_counter & 0xFF00) != (cpu->program_counter & 0xFF00)){
            cpu->cycles += 1;
        }
        cpu->program_counter = new_program_counter;
    }
    cpu->total_cycles += cpu->cycles;
}

void cpu_BEQ(CPU *cpu, uint8_t opcode){
    cpu->relative_offset = bus_read(cpu->bus, cpu->program_counter);
    cpu->program_counter++;
    cpu->cycles = 2;

    if((cpu->status & FLAG_Z) != 0){
        cpu->cycles++;
        uint16_t new_program_counter = cpu->program_counter + (int8_t)cpu->relative_offset;
        if((new_program_counter & 0xFF00) != (cpu->program_counter & 0xFF00)){
            cpu->cycles += 1;
        }
        cpu->program_counter = new_program_counter;
    }
    cpu->total_cycles += cpu->cycles;
}

void cpu_BIT(CPU *cpu, uint8_t opcode){
    bool page_crossed = false;
    uint16_t address = get_effective_adress(cpu, opcode, &page_crossed);
    cpu->effective_address = address;
    uint8_t value = bus_read(cpu->bus, cpu->effective_address);
    if((value & cpu->accumulator) == 0){
        cpu->status |= FLAG_Z;
    }
    else{
        cpu->status &= ~FLAG_Z;
    }
    if((value & 0x80) != 0){
        cpu->status |= FLAG_N;
    }
    else{
        cpu->status &= ~FLAG_N;
    }
    if((value & 0x40) != 0){
        cpu->status |= FLAG_V;
    }
    else{
        cpu->status &= ~FLAG_V;
    }
    if(opcode == 0x24) cpu->cycles = 3;
    if(opcode == 0x2C) cpu->cycles = 4;
    cpu->total_cycles += cpu->cycles;
}

void cpu_BMI(CPU *cpu, uint8_t opcode){
    cpu->relative_offset = bus_read(cpu->bus, cpu->program_counter);
    cpu->program_counter++;
    cpu->cycles = 2;

    if((cpu->status & FLAG_N) != 0){
        cpu->cycles++;
        uint16_t new_program_counter = cpu->program_counter + (int8_t)cpu->relative_offset;
        if((new_program_counter & 0xFF00) != (cpu->program_counter & 0xFF00)){
            cpu->cycles += 1;
        }
        cpu->program_counter = new_program_counter;
    }
    cpu->total_cycles += cpu->cycles;
}

void cpu_BNE(CPU *cpu, uint8_t opcode){
    cpu->relative_offset = bus_read(cpu->bus, cpu->program_counter);
    cpu->program_counter++;
    cpu->cycles = 2;

    if((cpu->status & FLAG_Z) == 0){
        cpu->cycles++;
        uint16_t new_program_counter = cpu->program_counter + (int8_t)cpu->relative_offset;
        if((new_program_counter & 0xFF00) != (cpu->program_counter & 0xFF00)){
            cpu->cycles += 1;
        }
        cpu->program_counter = new_program_counter;
    }
    cpu->total_cycles += cpu->cycles;
}

void cpu_BPL(CPU *cpu, uint8_t opcode){
    cpu->relative_offset = bus_read(cpu->bus, cpu->program_counter);
    cpu->program_counter++;
    cpu->cycles = 2;

    if((cpu->status & FLAG_N) == 0){
        cpu->cycles++;
        uint16_t new_program_counter = cpu->program_counter + (int8_t)cpu->relative_offset;
        if((new_program_counter & 0xFF00) != (cpu->program_counter & 0xFF00)){
            cpu->cycles += 1;
        }
        cpu->program_counter = new_program_counter;
    }
    cpu->total_cycles += cpu->cycles;
}

void cpu_BRK(CPU *cpu, uint8_t opcode) {
    (void)opcode;
    cpu->program_counter++;
    stack_push(cpu, (cpu->program_counter >> 8) & 0xFF);
    stack_push(cpu, cpu->program_counter & 0xFF);
    stack_push(cpu, cpu->status | FLAG_B | 0x20);
    cpu->status |= FLAG_I;
    uint16_t low = bus_read(cpu->bus, 0xFFFE);
    uint16_t high = bus_read(cpu->bus, 0xFFFF);
    cpu->program_counter = (high << 8) | low;
    cpu->cycles = 7;
    cpu->total_cycles += cpu->cycles;
}

void cpu_BVC(CPU *cpu, uint8_t opcode){
    cpu->relative_offset = bus_read(cpu->bus, cpu->program_counter);
    cpu->program_counter++;
    cpu->cycles = 2;

    if((cpu->status & FLAG_V) == 0){
        cpu->cycles++;
        uint16_t new_program_counter = cpu->program_counter + (int8_t)cpu->relative_offset;
        if((new_program_counter & 0xFF00) != (cpu->program_counter & 0xFF00)){
            cpu->cycles += 1;
        }
        cpu->program_counter = new_program_counter;
    }
    cpu->total_cycles += cpu->cycles;
}

void cpu_BVS(CPU *cpu, uint8_t opcode){
    cpu->relative_offset = bus_read(cpu->bus, cpu->program_counter);
    cpu->program_counter++;
    cpu->cycles = 2;

    if((cpu->status & FLAG_V) != 0){
        cpu->cycles++;
        uint16_t new_program_counter = cpu->program_counter + (int8_t)cpu->relative_offset;
        if((new_program_counter & 0xFF00) != (cpu->program_counter & 0xFF00)){
            cpu->cycles += 1;
        }
        cpu->program_counter = new_program_counter;
    }
    cpu->total_cycles += cpu->cycles;
}

void cpu_CLC(CPU *cpu, uint8_t opcode){
    if(opcode == 0x18){
        cpu->status &= ~FLAG_C;
        cpu->cycles = 2;
        cpu->total_cycles += cpu->cycles;
    }
}

void cpu_CLD(CPU *cpu, uint8_t opcode){
    if(opcode == 0xD8){
        cpu->status &= ~FLAG_D;
        cpu->cycles = 2;
        cpu->total_cycles += cpu->cycles;
    }
}

void cpu_CLI(CPU *cpu, uint8_t opcode) {
    if (opcode == 0x58) {
        cpu->status &= ~FLAG_I;
        cpu->cycles = 2;
        cpu->total_cycles += cpu->cycles;
    }
}

void cpu_CLV(CPU *cpu, uint8_t opcode) {
    if (opcode == 0xB8) {
        cpu->status &= ~FLAG_V;
        cpu->cycles = 2;
        cpu->total_cycles += cpu->cycles;
    }
}

void cpu_CMP(CPU *cpu, uint8_t opcode){
    bool page_crossed = false;
    uint16_t address = get_effective_adress(cpu, opcode, &page_crossed);
    cpu->effective_address = address;
    uint8_t value = bus_read(cpu->bus, cpu->effective_address);
    uint8_t diff = cpu->accumulator - value;
    if(cpu->accumulator >= value){
        cpu->status |= FLAG_C;
    }
    else{
        cpu->status &= ~FLAG_C;
    }
    if(cpu->accumulator == value){
        cpu->status |= FLAG_Z;
    }
    else{
        cpu->status &= ~FLAG_Z;
    }
    if((diff & 0x80) != 0){
        cpu->status |= FLAG_N;
    }
    else{
        cpu->status &= ~FLAG_N;
    }
    if(opcode == 0xC9) cpu->cycles = 2;
    if(opcode == 0xC5) cpu->cycles = 3;
    if(opcode == 0xD5 || opcode == 0xCD) cpu->cycles = 4;
    if(opcode == 0xDD || opcode == 0xD9){
        cpu->cycles = 4;
        if(page_crossed) cpu->cycles++;
    }
    if(opcode == 0xC1) cpu->cycles = 6;
    if(opcode == 0xD1){
        cpu->cycles = 5;
        if(page_crossed) cpu->cycles++;
    } 
    cpu->total_cycles += cpu->cycles;
}

void cpu_CPX(CPU *cpu, uint8_t opcode){
    bool page_crossed = false;
    uint16_t address = get_effective_adress(cpu, opcode, &page_crossed);
    cpu->effective_address = address;
    uint8_t value = bus_read(cpu->bus, cpu->effective_address);
    uint8_t diff = cpu->index_register_X - value;
    if(cpu->index_register_X >= value){
        cpu->status |= FLAG_C;
    }
    else{
        cpu->status &= ~FLAG_C;
    }
    if(cpu->index_register_X == value){
        cpu->status |= FLAG_Z;
    }
    else{
        cpu->status &= ~FLAG_Z;
    }
    if((diff & 0x80) != 0){
        cpu->status |= FLAG_N;
    }
    else{
        cpu->status &= ~FLAG_N;
    }
    if(opcode == 0xE0) cpu->cycles = 2;
    if(opcode == 0xE4) cpu->cycles = 3;
    if(opcode == 0xEC) cpu->cycles = 4; 
    cpu->total_cycles += cpu->cycles;
}

void cpu_CPY(CPU *cpu, uint8_t opcode){
    bool page_crossed = false;
    uint16_t address = get_effective_adress(cpu, opcode, &page_crossed);
    cpu->effective_address = address;
    uint8_t value = bus_read(cpu->bus, cpu->effective_address);
    uint8_t diff = cpu->index_register_Y - value;
    if(cpu->index_register_Y >= value){
        cpu->status |= FLAG_C;
    }
    else{
        cpu->status &= ~FLAG_C;
    }
    if(cpu->index_register_Y == value){
        cpu->status |= FLAG_Z;
    }
    else{
        cpu->status &= ~FLAG_Z;
    }
    if((diff & 0x80) != 0){
        cpu->status |= FLAG_N;
    }
    else{
        cpu->status &= ~FLAG_N;
    }
    if(opcode == 0xC0) cpu->cycles = 2;
    if(opcode == 0xC4) cpu->cycles = 3;
    if(opcode == 0xCC) cpu->cycles = 4; 
    cpu->total_cycles += cpu->cycles;
}

void cpu_DEC(CPU *cpu, uint8_t opcode){
    bool page_crossed = false;
    uint16_t address = get_effective_adress(cpu, opcode, &page_crossed);
    cpu->effective_address = address;
    uint8_t value = bus_read(cpu->bus, cpu->effective_address);
    uint8_t result = value - (uint8_t)1;
    if(result == 0){
        cpu->status |= FLAG_Z;
    }
    else{
        cpu->status &= ~FLAG_Z;
    }
    if((result & 0x80) != 0){
        cpu->status |= FLAG_N;
    }
    else{
        cpu->status &= ~FLAG_N;
    }
    bus_write(cpu->bus, cpu->effective_address, result);
    if(opcode == 0xC6) cpu->cycles = 5;
    if(opcode == 0xD6 || opcode == 0xCE) cpu->cycles = 6;
    if(opcode == 0xDE) cpu->cycles = 7;
    cpu->total_cycles += cpu->cycles;
}

void cpu_DEX(CPU *cpu, uint8_t opcode){
    cpu->index_register_X -= (uint8_t)1;
    if(cpu->index_register_X == 0){
        cpu->status |= FLAG_Z;
    }
    else{
        cpu->status &= ~FLAG_Z;
    }
    if((cpu->index_register_X & 0x80) != 0){
        cpu->status |= FLAG_N;
    }
    else{
        cpu->status &= ~FLAG_N;
    }
    if(opcode == 0xCA) cpu->cycles = 2;
    cpu->total_cycles += cpu->cycles;
}

void cpu_DEY(CPU *cpu, uint8_t opcode){
    cpu->index_register_Y -= (uint8_t)1;
    if(cpu->index_register_Y == 0){
        cpu->status |= FLAG_Z;
    }
    else{
        cpu->status &= ~FLAG_Z;
    }
    if((cpu->index_register_Y & 0x80) != 0){
        cpu->status |= FLAG_N;
    }
    else{
        cpu->status &= ~FLAG_N;
    }
    if(opcode == 0x88) cpu->cycles = 2;
    cpu->total_cycles += cpu->cycles;
}

void cpu_EOR(CPU *cpu, uint8_t opcode){
    bool page_crossed = false;
    uint16_t address = get_effective_adress(cpu, opcode, &page_crossed);
    cpu->effective_address = address;
    uint8_t value = bus_read(cpu->bus, cpu->effective_address);
    cpu->accumulator ^= value;
    if(cpu->accumulator == 0){
        cpu->status |= FLAG_Z;
    }
    else{
        cpu->status &= ~FLAG_Z;
    }
    if((cpu->accumulator & 0x80) != 0){
        cpu->status |= FLAG_N;
    }
    else{
        cpu->status &= ~FLAG_N;
    }
    if(opcode == 0x49) cpu->cycles = 2;
    if(opcode == 0x45) cpu->cycles = 3;
    if(opcode == 0x55 || opcode == 0x4D) cpu->cycles = 4;
    if(opcode == 0x5D || opcode == 0x59) cpu->cycles = 4 + (page_crossed ? 1 : 0);
    if(opcode == 0x41) cpu->cycles = 6;
    if(opcode == 0x51) cpu->cycles = 5 + (page_crossed ? 1 : 0);
    cpu->total_cycles += cpu->cycles;
}

void cpu_INC(CPU *cpu, uint8_t opcode){
    bool page_crossed = false;
    uint16_t address = get_effective_adress(cpu, opcode, &page_crossed);
    cpu->effective_address = address;
    uint8_t value = bus_read(cpu->bus, cpu->effective_address);
    uint8_t result = value + 1;
    bus_write(cpu->bus, cpu->effective_address, result);
    if(result == 0){
        cpu->status |= FLAG_Z;
    }
    else{
        cpu->status &= ~FLAG_Z;
    }
    if((result & 0x80) != 0){
        cpu->status |= FLAG_N;
    }
    else{
        cpu->status &= ~FLAG_N;
    }
    if(opcode == 0xE6) cpu->cycles = 5;
    if(opcode == 0xF6) cpu->cycles = 6;
    if(opcode == 0xEE) cpu->cycles = 6;
    if(opcode == 0xFE) cpu->cycles = 7;
    cpu->total_cycles += cpu->cycles;
}

void cpu_INX(CPU *cpu, uint8_t opcode){
    cpu->index_register_X++;
    if(cpu->index_register_X == 0){
        cpu->status |= FLAG_Z;
    }
    else{
        cpu->status &= ~FLAG_Z;
    }
    if((cpu->index_register_X & 0x80) != 0){
        cpu->status |= FLAG_N;
    }
    else{
        cpu->status &= ~FLAG_N;
    }
    if(opcode == 0xE8) cpu->cycles = 2;
    cpu->total_cycles += cpu->cycles;
}

void cpu_INY(CPU *cpu, uint8_t opcode){
    cpu->index_register_Y++;
    if(cpu->index_register_Y == 0){
        cpu->status |= FLAG_Z;
    }
    else{
        cpu->status &= ~FLAG_Z;
    }
    if((cpu->index_register_Y & 0x80) != 0){
        cpu->status |= FLAG_N;
    }
    else{
        cpu->status &= ~FLAG_N;
    }
    if(opcode == 0xC8) cpu->cycles = 2;
    cpu->total_cycles += cpu->cycles;
}

void cpu_JMP(CPU *cpu, uint8_t opcode){
    uint16_t low = bus_read(cpu->bus, cpu->program_counter);
    cpu->program_counter++;
    uint16_t high = bus_read(cpu->bus, cpu->program_counter);
    cpu->program_counter++;
    uint16_t address = (high << 8) | low;
    if(opcode == 0x4C){
        cpu->effective_address = address;
        cpu->program_counter = address;
        cpu->cycles = 3;
    }
    if(opcode == 0x6C){
        uint16_t target_low = bus_read(cpu->bus, address);
        uint16_t target_high_address = ((address & 0x00FF) == 0x00FF) ? (address & 0xFF00) : (address + 1);
        uint16_t target_high = bus_read(cpu->bus, target_high_address);
        cpu->effective_address = (target_high << 8) | target_low;
        cpu->program_counter = cpu->effective_address;
        cpu->cycles = 5;
    }
    cpu->total_cycles += cpu->cycles;
}

void cpu_JSR(CPU *cpu, uint8_t opcode){
    if(opcode == 0x20){
        uint16_t low = bus_read(cpu->bus, cpu->program_counter);
        cpu->program_counter++;
        uint16_t high = bus_read(cpu->bus, cpu->program_counter);
        uint16_t return_address = cpu->program_counter;
        stack_push(cpu, (return_address >> 8) & 0xFF);
        stack_push(cpu, return_address & 0xFF);
        cpu->effective_address = (high << 8) | low;
        cpu->program_counter = cpu->effective_address;
        cpu->cycles = 6;
        cpu->total_cycles += cpu->cycles;
    }
}

void cpu_LDA(CPU *cpu, uint8_t opcode){
    bool page_crossed = false;
    uint16_t address = get_effective_adress(cpu, opcode, &page_crossed);
    cpu->effective_address = address;
    uint8_t value = bus_read(cpu->bus, cpu->effective_address);
    cpu->accumulator = value;
    if(cpu->accumulator == 0){
        cpu->status |= FLAG_Z;
    }
    else{
        cpu->status &= ~FLAG_Z;
    }
    if((cpu->accumulator & 0x80) != 0){
        cpu->status |= FLAG_N;
    }
    else{
        cpu->status &= ~FLAG_N;
    }
    if(opcode == 0xA9) cpu->cycles = 2;
    if(opcode == 0xA5) cpu->cycles = 3;
    if(opcode == 0xB5 || opcode == 0xAD) cpu->cycles = 4;
    if(opcode == 0xBD || opcode == 0xB9) cpu->cycles = 4 + (page_crossed ? 1 : 0);
    if(opcode == 0xA1) cpu->cycles = 6;
    if(opcode == 0xB1) cpu->cycles = 5 + (page_crossed ? 1 : 0);
    cpu->total_cycles += cpu->cycles;
}

void cpu_LDX(CPU *cpu, uint8_t opcode){
    bool page_crossed = false;
    uint16_t address = get_effective_adress(cpu, opcode, &page_crossed);
    cpu->effective_address = address;
    uint8_t value = bus_read(cpu->bus, cpu->effective_address);
    cpu->index_register_X = value;
    if(cpu->index_register_X == 0){
        cpu->status |= FLAG_Z;
    }
    else{
        cpu->status &= ~FLAG_Z;
    }
    if((cpu->index_register_X & 0x80) != 0){
        cpu->status |= FLAG_N;
    }
    else{
        cpu->status &= ~FLAG_N;
    }
    if(opcode == 0xA2) cpu->cycles = 2;
    if(opcode == 0xA6) cpu->cycles = 3;
    if(opcode == 0xB6 || opcode == 0xAE) cpu->cycles = 4;
    if(opcode == 0xBE) cpu->cycles = 4 + (page_crossed ? 1 : 0);
    cpu->total_cycles += cpu->cycles;
}

void cpu_LDY(CPU *cpu, uint8_t opcode){
    bool page_crossed = false;
    uint16_t address = get_effective_adress(cpu, opcode, &page_crossed);
    cpu->effective_address = address;
    uint8_t value = bus_read(cpu->bus, cpu->effective_address);
    cpu->index_register_Y = value;
    if(cpu->index_register_Y == 0){
        cpu->status |= FLAG_Z;
    }
    else{
        cpu->status &= ~FLAG_Z;
    }
    if((cpu->index_register_Y & 0x80) != 0){
        cpu->status |= FLAG_N;
    }
    else{
        cpu->status &= ~FLAG_N;
    }
    if(opcode == 0xA0) cpu->cycles = 2;
    if(opcode == 0xA4) cpu->cycles = 3;
    if(opcode == 0xB4 || opcode == 0xAC) cpu->cycles = 4;
    if(opcode == 0xBC) cpu->cycles = 4 + (page_crossed ? 1 : 0);
    cpu->total_cycles += cpu->cycles;
}

void cpu_LSR(CPU *cpu, uint8_t opcode){
    uint8_t value;
    if(opcode == 0x4A){
        value = cpu->accumulator;
    }
    else{
        bool page_crossed = false;
        uint16_t address = get_effective_adress(cpu, opcode, &page_crossed);
        cpu->effective_address = address;
        value = bus_read(cpu->bus, cpu->effective_address);
    }
    if((value & 0x01) != 0){
        cpu->status |= FLAG_C;
    }
    else{
        cpu->status &= ~FLAG_C;
    }
    uint8_t result = value >> 1;
    if(opcode == 0x4A){
        cpu->accumulator = result;
    }
    else{
        bus_write(cpu->bus, cpu->effective_address, result);
    }
    if(result == 0){
        cpu->status |= FLAG_Z;
    }
    else{
        cpu->status &= ~FLAG_Z;
    }
    cpu->status &= ~FLAG_N;
    if(opcode == 0x4A) cpu->cycles = 2;
    else if(opcode == 0x46) cpu->cycles = 5;
    else if(opcode == 0x56 || opcode == 0x4E) cpu->cycles = 6;
    else if(opcode == 0x5E) cpu->cycles = 7;
    cpu->total_cycles += cpu->cycles;
}

void cpu_NOP(CPU *cpu, uint8_t opcode){
    if(opcode == 0xEA){
        cpu->cycles = 2;
        cpu->total_cycles += cpu->cycles;
    }
}

void cpu_ORA(CPU *cpu, uint8_t opcode){
    bool page_crossed = false;
    uint16_t address = get_effective_adress(cpu, opcode, &page_crossed);
    cpu->effective_address = address;
    uint8_t value = bus_read(cpu->bus, cpu->effective_address);
    cpu->accumulator |= value;
    if(cpu->accumulator == 0){
        cpu->status |= FLAG_Z;
    }
    else{
        cpu->status &= ~FLAG_Z;
    }
    if((cpu->accumulator & 0x80) != 0){
        cpu->status |= FLAG_N;
    }
    else{
        cpu->status &= ~FLAG_N;
    }
    if(opcode == 0x09) cpu->cycles = 2;
    if(opcode == 0x05) cpu->cycles = 3;
    if(opcode == 0x15 || opcode == 0x0D) cpu->cycles = 4;
    if(opcode == 0x1D || opcode == 0x19) cpu->cycles = 4 + (page_crossed ? 1 : 0);
    if(opcode == 0x01) cpu->cycles = 6;
    if(opcode == 0x11) cpu->cycles = 5 + (page_crossed ? 1 : 0);
    cpu->total_cycles += cpu->cycles;
}

void cpu_PHA(CPU *cpu, uint8_t opcode){
    if(opcode == 0x48){
        stack_push(cpu, cpu->accumulator);
        cpu->cycles = 3;
        cpu->total_cycles += cpu->cycles;
    }
}

void cpu_PHP(CPU *cpu, uint8_t opcode){
    if(opcode == 0x08){
        stack_push(cpu, cpu->status | FLAG_B | FLAG_U);
        cpu->cycles = 3;
        cpu->total_cycles += cpu->cycles;
    }
}

void cpu_PLA(CPU *cpu, uint8_t opcode){
    if(opcode == 0x68){
        cpu->accumulator = stack_pop(cpu);
        if(cpu->accumulator == 0){
            cpu->status |= FLAG_Z;
        }
        else{
            cpu->status &= ~FLAG_Z;
        }
        if((cpu->accumulator & 0x80) != 0){
            cpu->status |= FLAG_N;
        }
        else{
            cpu->status &= ~FLAG_N;
        }
        cpu->cycles = 4;
        cpu->total_cycles += cpu->cycles;
    }
}

void cpu_PLP(CPU *cpu, uint8_t opcode){
    if(opcode == 0x28){
        cpu->status = (stack_pop(cpu) & ~FLAG_B) | FLAG_U;
        cpu->cycles = 4;
        cpu->total_cycles += cpu->cycles;
    }
}

void cpu_ROL(CPU *cpu, uint8_t opcode){
    uint8_t value;
    if(opcode == 0x2A){
        value = cpu->accumulator;
    }
    else{
        bool page_crossed = false;
        uint16_t address = get_effective_adress(cpu, opcode, &page_crossed);
        cpu->effective_address = address;
        value = bus_read(cpu->bus, cpu->effective_address);
    }
    uint8_t old_carry = (cpu->status & FLAG_C) ? 1 : 0;
    if((value & 0x80) != 0){
        cpu->status |= FLAG_C;
    }
    else{
        cpu->status &= ~FLAG_C;
    }
    uint8_t result = (value << 1) | old_carry;
    if(opcode == 0x2A){
        cpu->accumulator = result;
    }
    else{
        bus_write(cpu->bus, cpu->effective_address, result);
    }
    if(result == 0){
        cpu->status |= FLAG_Z;
    }
    else{
        cpu->status &= ~FLAG_Z;
    }
    if((result & 0x80) != 0){
        cpu->status |= FLAG_N;
    }
    else{
        cpu->status &= ~FLAG_N;
    }
    if(opcode == 0x2A) cpu->cycles = 2;
    else if(opcode == 0x26) cpu->cycles = 5;
    else if(opcode == 0x36 || opcode == 0x2E) cpu->cycles = 6;
    else if(opcode == 0x3E) cpu->cycles = 7;
    cpu->total_cycles += cpu->cycles;
}

void cpu_ROR(CPU *cpu, uint8_t opcode){
    uint8_t value;
    if(opcode == 0x6A){
        value = cpu->accumulator;
    }
    else{
        bool page_crossed = false;
        uint16_t address = get_effective_adress(cpu, opcode, &page_crossed);
        cpu->effective_address = address;
        value = bus_read(cpu->bus, cpu->effective_address);
    }
    uint8_t old_carry = (cpu->status & FLAG_C) ? 0x80 : 0;
    if((value & 0x01) != 0){
        cpu->status |= FLAG_C;
    }
    else{
        cpu->status &= ~FLAG_C;
    }
    uint8_t result = (value >> 1) | old_carry;
    if(opcode == 0x6A){
        cpu->accumulator = result;
    }
    else{
        bus_write(cpu->bus, cpu->effective_address, result);
    }
    if(result == 0){
        cpu->status |= FLAG_Z;
    }
    else{
        cpu->status &= ~FLAG_Z;
    }
    if((result & 0x80) != 0){
        cpu->status |= FLAG_N;
    }
    else{
        cpu->status &= ~FLAG_N;
    }
    if(opcode == 0x6A) cpu->cycles = 2;
    else if(opcode == 0x66) cpu->cycles = 5;
    else if(opcode == 0x76 || opcode == 0x6E) cpu->cycles = 6;
    else if(opcode == 0x7E) cpu->cycles = 7;
    cpu->total_cycles += cpu->cycles;
}

void cpu_RTI(CPU *cpu, uint8_t opcode){
    if(opcode == 0x40){
        cpu->status = (stack_pop(cpu) & ~FLAG_B) | FLAG_U;
        uint16_t low = stack_pop(cpu);
        uint16_t high = stack_pop(cpu);
        cpu->program_counter = (high << 8) | low;
        cpu->cycles = 6;
        cpu->total_cycles += cpu->cycles;
    }
}

void cpu_RTS(CPU *cpu, uint8_t opcode){
    if(opcode == 0x60){
        uint16_t low = stack_pop(cpu);
        uint16_t high = stack_pop(cpu);
        cpu->program_counter = ((high << 8) | low) + 1;
        cpu->cycles = 6;
        cpu->total_cycles += cpu->cycles;
    }
}

void cpu_SBC(CPU *cpu, uint8_t opcode){
    bool page_crossed = false;
    uint16_t address = get_effective_adress(cpu, opcode, &page_crossed);
    cpu->effective_address = address;

    uint8_t value = bus_read(cpu->bus, cpu->effective_address);
    uint8_t inverted_value = value ^ 0xFF;
    uint8_t carry = (cpu->status & FLAG_C) ? 1 : 0;
    uint16_t sum = (uint16_t)cpu->accumulator + (uint16_t)inverted_value + (uint16_t)carry;
    uint8_t result = sum & 0xFF;

    if(sum > 0xFF){
        cpu->status |= FLAG_C;
    }
    else{
        cpu->status &= ~FLAG_C;
    }
    if(result == 0){
        cpu->status |= FLAG_Z;
    }
    else{
        cpu->status &= ~FLAG_Z;
    }
    if((result & 0x80) != 0){
        cpu->status |= FLAG_N;
    }
    else{
        cpu->status &= ~FLAG_N;
    }
    if((~(cpu->accumulator ^ inverted_value) & (cpu->accumulator ^ result) & 0x80) != 0){
        cpu->status |= FLAG_V;
    }
    else{
        cpu->status &= ~FLAG_V;
    }

    if((cpu->status & FLAG_D) != 0){
        int16_t low_nibble = (cpu->accumulator & 0x0F) - (value & 0x0F) - (1 - carry);
        if(low_nibble < 0){
            low_nibble = ((low_nibble - 0x06) & 0x0F) - 0x10;
        }
        int16_t high_nibble = (cpu->accumulator & 0xF0) - (value & 0xF0) + low_nibble;
        if(high_nibble < 0){
            high_nibble -= 0x60;
        }
        result = high_nibble & 0xFF;
    }

    cpu->accumulator = result;

    if(opcode == 0xE9) cpu->cycles = 2;
    if(opcode == 0xE5) cpu->cycles = 3;
    if(opcode == 0xF5 || opcode == 0xED) cpu->cycles = 4;
    if(opcode == 0xFD || opcode == 0xF9) cpu->cycles = 4 + (page_crossed ? 1 : 0);
    if(opcode == 0xE1) cpu->cycles = 6;
    if(opcode == 0xF1) cpu->cycles = 5 + (page_crossed ? 1 : 0);
    cpu->total_cycles += cpu->cycles;
}

void cpu_SEC(CPU *cpu, uint8_t opcode){
    if(opcode == 0x38){
        cpu->status |= FLAG_C;
        cpu->cycles = 2;
        cpu->total_cycles += cpu->cycles;
    }
}

void cpu_SED(CPU *cpu, uint8_t opcode){
    if(opcode == 0xF8){
        cpu->status |= FLAG_D;
        cpu->cycles = 2;
        cpu->total_cycles += cpu->cycles;
    }
}

void cpu_SEI(CPU *cpu, uint8_t opcode){
    if(opcode == 0x78){
        cpu->status |= FLAG_I;
        cpu->cycles = 2;
        cpu->total_cycles += cpu->cycles;
    }
}

void cpu_STA(CPU *cpu, uint8_t opcode){
    bool page_crossed = false;
    uint16_t address = get_effective_adress(cpu, opcode, &page_crossed);
    cpu->effective_address = address;
    bus_write(cpu->bus, cpu->effective_address, cpu->accumulator);
    if(opcode == 0x85) cpu->cycles = 3;
    if(opcode == 0x95 || opcode == 0x8D) cpu->cycles = 4;
    if(opcode == 0x9D || opcode == 0x99) cpu->cycles = 5;
    if(opcode == 0x81 || opcode == 0x91) cpu->cycles = 6;
    cpu->total_cycles += cpu->cycles;
}

void cpu_STX(CPU *cpu, uint8_t opcode){
    bool page_crossed = false;
    uint16_t address = get_effective_adress(cpu, opcode, &page_crossed);
    cpu->effective_address = address;
    bus_write(cpu->bus, cpu->effective_address, cpu->index_register_X);
    if(opcode == 0x86) cpu->cycles = 3;
    if(opcode == 0x96 || opcode == 0x8E) cpu->cycles = 4;
    cpu->total_cycles += cpu->cycles;
}

void cpu_STY(CPU *cpu, uint8_t opcode){
    bool page_crossed = false;
    uint16_t address = get_effective_adress(cpu, opcode, &page_crossed);
    cpu->effective_address = address;
    bus_write(cpu->bus, cpu->effective_address, cpu->index_register_Y);
    if(opcode == 0x84) cpu->cycles = 3;
    if(opcode == 0x94 || opcode == 0x8C) cpu->cycles = 4;
    cpu->total_cycles += cpu->cycles;
}

void cpu_TAX(CPU *cpu, uint8_t opcode){
    if(opcode == 0xAA){
        cpu->index_register_X = cpu->accumulator;
        if(cpu->index_register_X == 0){
            cpu->status |= FLAG_Z;
        }
        else{
            cpu->status &= ~FLAG_Z;
        }
        if((cpu->index_register_X & 0x80) != 0){
            cpu->status |= FLAG_N;
        }
        else{
            cpu->status &= ~FLAG_N;
        }
        cpu->cycles = 2;
        cpu->total_cycles += cpu->cycles;
    }
}

void cpu_TAY(CPU *cpu, uint8_t opcode){
    if(opcode == 0xA8){
        cpu->index_register_Y = cpu->accumulator;
        if(cpu->index_register_Y == 0){
            cpu->status |= FLAG_Z;
        }
        else{
            cpu->status &= ~FLAG_Z;
        }
        if((cpu->index_register_Y & 0x80) != 0){
            cpu->status |= FLAG_N;
        }
        else{
            cpu->status &= ~FLAG_N;
        }
        cpu->cycles = 2;
        cpu->total_cycles += cpu->cycles;
    }
}

void cpu_TSX(CPU *cpu, uint8_t opcode){
    if(opcode == 0xBA){
        cpu->index_register_X = cpu->stack_pointer;
        if(cpu->index_register_X == 0){
            cpu->status |= FLAG_Z;
        }
        else{
            cpu->status &= ~FLAG_Z;
        }
        if((cpu->index_register_X & 0x80) != 0){
            cpu->status |= FLAG_N;
        }
        else{
            cpu->status &= ~FLAG_N;
        }
        cpu->cycles = 2;
        cpu->total_cycles += cpu->cycles;
    }
}

void cpu_TXA(CPU *cpu, uint8_t opcode){
    if(opcode == 0x8A){
        cpu->accumulator = cpu->index_register_X;
        if(cpu->accumulator == 0){
            cpu->status |= FLAG_Z;
        }
        else{
            cpu->status &= ~FLAG_Z;
        }
        if((cpu->accumulator & 0x80) != 0){
            cpu->status |= FLAG_N;
        }
        else{
            cpu->status &= ~FLAG_N;
        }
        cpu->cycles = 2;
        cpu->total_cycles += cpu->cycles;
    }
}

void cpu_TXS(CPU *cpu, uint8_t opcode){
    if(opcode == 0x9A){
        cpu->stack_pointer = cpu->index_register_X;
        cpu->cycles = 2;
        cpu->total_cycles += cpu->cycles;
    }
}

void cpu_TYA(CPU *cpu, uint8_t opcode){
    if(opcode == 0x98){
        cpu->accumulator = cpu->index_register_Y;
        if(cpu->accumulator == 0){
            cpu->status |= FLAG_Z;
        }
        else{
            cpu->status &= ~FLAG_Z;
        }
        if((cpu->accumulator & 0x80) != 0){
            cpu->status |= FLAG_N;
        }
        else{
            cpu->status &= ~FLAG_N;
        }
        cpu->cycles = 2;
        cpu->total_cycles += cpu->cycles;
    }
}