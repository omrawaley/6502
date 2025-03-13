#include "emulator.h"

#include <stdio.h>

static inline u8 mem_read_byte(u8* mem, const u16 addr) {
    return mem[addr];
}

static inline void mem_write_byte(u8* mem, const u16 addr, const u8 val) {
    mem[addr] = val;
}

static u8 emulator_read_bus(void* ctx, const u16 addr) {
    emulator_t* emulator = (emulator_t*)ctx;
    return mem_read_byte(emulator->mem, addr);
}

static void emulator_write_bus(void* ctx, const u16 addr, const u8 val) {
    emulator_t* emulator = (emulator_t*)ctx;
    mem_write_byte(emulator->mem, addr, val);
}

void emulator_init(emulator_t* emulator) {
    emulator->cpu.bus = emulator;
    emulator->cpu.read_bus = &emulator_read_bus;
    emulator->cpu.write_bus = &emulator_write_bus;
}

void emulator_load(emulator_t* emulator, const char* path) {
    FILE* file = fopen(path, "rb");

    size_t size = ftell(file);
    rewind(file);

    if(fread(emulator->mem, sizeof(u8), MEM_SIZE, file) != size) {
        fprintf(stderr, "Error reading program");
    }
}