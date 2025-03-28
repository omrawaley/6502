// Copyright (C) 2025 Om Rawaley (@omrawaley)

// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at

//     http://www.apache.org/licenses/LICENSE-2.0

// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "emulator.h"

#include <unistd.h>

#define CYCLES_PER_SECOND 1790000 // 1.79 MHz

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

void emulator_reset(emulator_t* emulator) {
    memset(emulator->mem, 0, MEM_SIZE);
    cpu_reset(&emulator->cpu);
}

void emulator_init(emulator_t* emulator) {
    emulator->cpu.bus = emulator;
    emulator->cpu.read_bus = &emulator_read_bus;
    emulator->cpu.write_bus = &emulator_write_bus;

    // emulator->cpu.write_bus(emulator->cpu.bus, RST_START, 0x00);
    // emulator->cpu.write_bus(emulator->cpu.bus, RST_START + 1, 0xC0);

    emulator_reset(emulator);
}

void emulator_load(emulator_t* emulator, const char* path) {
    FILE* file = fopen(path, "rb");

    if(!file) {
        fprintf(stderr, "Error opening file");
    }

    fseek(file, 0, SEEK_END);
    size_t size = ftell(file);
    rewind(file);

    if(fread(emulator->mem, sizeof(u8), MEM_SIZE, file) != size) {
        fprintf(stderr, "Error reading program");
        fclose(file);
        return;
    }

    // == NESTEST ==

    // fseek(file, 0x10, SEEK_SET);

    // size_t size = 0x4000;
    // u8 buf[0x4000];
    // if (fread(buf, sizeof(u8), size, file) != size) {
    //     fprintf(stderr, "Error reading program");
    //     fclose(file);
    //     return;
    // }

    // memcpy(emulator->mem + 0x8000, buf, size);
    // memcpy(emulator->mem + 0xC000, buf, size);

    fclose(file);
}

void emulator_run(emulator_t* emulator) {
    // for(size_t i = 0; i < CYCLES_PER_SECOND; ++i) {
        cpu_clock(&emulator->cpu);
    // }
    
    // sleep(1);
}