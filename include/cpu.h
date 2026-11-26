#ifndef CPU_H
#define CPU_H

#include <stdint.h>
#include <stdbool.h>
#include "bus.h"

typedef enum {
    FLAG_C = (1 << 0), /* carry flag */
    FLAG_Z = (1 << 1), /* zero */
    FLAG_I = (1 << 2), /* interrupt disable */
    FLAG_D = (1 << 3), /* decimal mode */
    FLAG_B = (1 << 4), /* break */
    FLAG_U = (1 << 5), /* unused */
    FLAG_V = (1 << 6), /* overflow */
    FLAG_N = (1 << 7) /* negative */
}CPUFlag;

typedef struct {
    uint8_t accumulator;
    uint8_t index_register_X;
    uint8_t index_register_Y;
    uint8_t stack_pointer;
    uint16_t program_counter;
    uint8_t status;

    Bus *bus;
    uint16_t effective_address;
    uint16_t relative_offset;
    uint8_t opcode;
    uint8_t cycles;
    uint64_t total_cycles;
}CPU;

void cpu_init(CPU *cpu, Bus *bus);

void cpu_reset(CPU *cpu);

void cpu_interrupt_masked(CPU *cpu);

void cpu_interrupt_unmasked(CPU *cpu);

bool cpu_step(CPU *cpu);

void cpu_ADC(CPU *cpu, uint8_t opcode);

void cpu_AND(CPU *cpu, uint8_t opcode);

void cpu_ASL(CPU *cpu, uint8_t opcode);

void cpu_BCC(CPU *cpu, uint8_t opcode);

void cpu_BCS(CPU *cpu, uint8_t opcode);

void cpu_BEQ(CPU *cpu, uint8_t opcode);

void cpu_BIT(CPU *cpu, uint8_t opcode);

void cpu_BMI(CPU *cpu, uint8_t opcode);

void cpu_BNE(CPU *cpu, uint8_t opcode);

void cpu_BPL(CPU *cpu, uint8_t opcode);

void cpu_BRK(CPU *cpu, uint8_t opcode);

void cpu_BVC(CPU *cpu, uint8_t opcode);

void cpu_BVS(CPU *cpu, uint8_t opcode);

void cpu_CLC(CPU *cpu, uint8_t opcode);

void cpu_CLD(CPU *cpu, uint8_t opcode);

void cpu_CLI(CPU *cpu, uint8_t opcode);

void cpu_CLV(CPU *cpu, uint8_t opcode);

void cpu_CMP(CPU *cpu, uint8_t opcode);

void cpu_CPX(CPU *cpu, uint8_t opcode);

void cpu_CPY(CPU *cpu, uint8_t opcode);

void cpu_DEC(CPU *cpu, uint8_t opcode);

void cpu_DEX(CPU *cpu, uint8_t opcode);

void cpu_DEY(CPU *cpu, uint8_t opcode);

void cpu_EOR(CPU *cpu, uint8_t opcode);

void cpu_INC(CPU *cpu, uint8_t opcode);

void cpu_INX(CPU *cpu, uint8_t opcode);

void cpu_INY(CPU *cpu, uint8_t opcode);

void cpu_JMP(CPU *cpu, uint8_t opcode);

void cpu_JSR(CPU *cpu, uint8_t opcode);

void cpu_LDA(CPU *cpu, uint8_t opcode);

void cpu_LDX(CPU *cpu, uint8_t opcode);

void cpu_LDY(CPU *cpu, uint8_t opcode);

void cpu_LSR(CPU *cpu, uint8_t opcode);

void cpu_NOP(CPU *cpu, uint8_t opcode);

void cpu_ORA(CPU *cpu, uint8_t opcode);

void cpu_PHA(CPU *cpu, uint8_t opcode);

void cpu_PHP(CPU *cpu, uint8_t opcode);

void cpu_PLA(CPU *cpu, uint8_t opcode);

void cpu_PLP(CPU *cpu, uint8_t opcode);

void cpu_ROL(CPU *cpu, uint8_t opcode);

void cpu_ROR(CPU *cpu, uint8_t opcode);

void cpu_RTI(CPU *cpu, uint8_t opcode);

void cpu_RTS(CPU *cpu, uint8_t opcode);

void cpu_SBC(CPU *cpu, uint8_t opcode);

void cpu_SEC(CPU *cpu, uint8_t opcode);

void cpu_SED(CPU *cpu, uint8_t opcode);

void cpu_SEI(CPU *cpu, uint8_t opcode);

void cpu_STA(CPU *cpu, uint8_t opcode);

void cpu_STX(CPU *cpu, uint8_t opcode);

void cpu_STY(CPU *cpu, uint8_t opcode);

void cpu_TAX(CPU *cpu, uint8_t opcode);

void cpu_TAY(CPU *cpu, uint8_t opcode);

void cpu_TSX(CPU *cpu, uint8_t opcode);

void cpu_TXA(CPU *cpu, uint8_t opcode);

void cpu_TXS(CPU *cpu, uint8_t opcode);

void cpu_TYA(CPU *cpu, uint8_t opcode);

#endif