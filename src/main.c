#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "bus.h"
#include "cpu.h"

#define TEST_BINARY_PATH "6502_functional_test.bin"
#define START_ADDRESS 0x0400
#define SUCCESS_ADDRESS 0x3469

int main(void){
    Bus bus;
    bus_init(&bus);

    if(bus_load_binary(&bus, TEST_BINARY_PATH, 0x0000) != 0){
        return 1;
    }

    CPU cpu;
    cpu_init(&cpu, &bus);
    cpu.program_counter = START_ADDRESS;

    while(true){
        uint16_t previous_pc = cpu.program_counter;

        if(!cpu_step(&cpu)){
            uint8_t opcode = bus_read(&bus, previous_pc);
            printf("Unknown opcode 0x%02X at 0x%04X\n", opcode, previous_pc);
            return 1;
        }

        if(cpu.program_counter == previous_pc){
            if(cpu.program_counter == SUCCESS_ADDRESS){
                printf("All tests passed! PC: 0x%04X | Cycles: %llu\n",
                       cpu.program_counter, cpu.total_cycles);
            }
            else{
                printf("Test failed at PC: 0x%04X (A=0x%02X X=0x%02X Y=0x%02X SP=0x%02X P=0x%02X)\n",
                       cpu.program_counter,
                       cpu.accumulator,
                       cpu.index_register_X,
                       cpu.index_register_Y,
                       cpu.stack_pointer,
                       cpu.status);
            }
            break;
        }
    }

    return 0;
}