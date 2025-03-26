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