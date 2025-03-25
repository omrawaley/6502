#ifndef M6502_EMULATOR_H
#define M6502_EMULATOR_H

#define MEM_SIZE 0x10000

#include <stdio.h>
#include <memory.h>

#include "../lib/cpu.h"

typedef struct emulator {
    cpu_t cpu;
    u8 mem[MEM_SIZE];
} emulator_t;

void emulator_reset(emulator_t* emulator);
void emulator_init(emulator_t* emulator);

void emulator_load(emulator_t* emulator, const char* path);

void emulator_run(emulator_t* emulator);

static u8 emulator_read_bus(void* bus, const u16 addr);
static void emulator_write_bus(void* bus, const u16 addr, const u8 val);

#endif //M6502_EMULATOR_H