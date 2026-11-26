#ifndef BUS_H
#define BUS_H

#include <stdint.h>
#include <stddef.h>

#define BUS_RAM_SIZE 65536

typedef struct {
    uint8_t ram[BUS_RAM_SIZE];
}Bus;

void bus_init(Bus *bus);

int bus_load_binary(Bus *bus, const char *filepath, uint16_t load_address);

uint8_t bus_read(const Bus *bus, uint16_t address);

void bus_write(Bus *bus, uint16_t address, uint8_t data);

#endif