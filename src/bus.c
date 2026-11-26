#include "bus.h"
#include <stdio.h>
#include <string.h>

void bus_init(Bus *bus){
    memset(bus->ram, 0, BUS_RAM_SIZE);
}

int bus_load_binary(Bus *bus, const char *filepath, uint16_t load_adress){
    FILE *file = fopen(filepath, "rb");
    if(file == NULL){
        perror("Invalid file!");
        return -1;
    }
    size_t max_bytes = BUS_RAM_SIZE - load_adress;
    size_t bytes_read = fread(&bus->ram[load_adress], 1, max_bytes, file);
    fclose(file);
    if(bytes_read == 0){
        perror("No bytes read!");
        return -1;
    }
    return 0;
}

uint8_t bus_read(const Bus *bus, uint16_t address){
    return bus->ram[address];
}

void bus_write(Bus *bus, uint16_t address, uint8_t data){
    bus->ram[address] = data;
}