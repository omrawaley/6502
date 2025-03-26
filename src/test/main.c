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

#include <ncurses.h>

#include "emulator.h"
#include "../../deps/cjson/cJSON.h"

#define MEM_COLS 0x10
#define MEM_ROWS 32

#define MEM_SCROLL_SPEED 8

// Test an opcode with only one test, specified by index
u8 test_opcode_single(emulator_t* emulator, const char* path, u16 test_idx) {
    FILE* file = fopen(path, "r");

    if (!file) {
        fprintf(stderr, "Error opening file\n");
        return 1;
    }

    fseek(file, 0, SEEK_END);
    size_t size = ftell(file);
    rewind(file);

    char* buf = malloc(size);
    if (!buf) {
        fprintf(stderr, "Memory allocation failed\n");
        fclose(file);
        return 1;
    }

    fread(buf, 1, size, file);

    fclose(file);

    cJSON* root = cJSON_Parse(buf);
    if (root == NULL) {
        fprintf(stderr, "Error parsing buffer\n");
        free(buf);
        return 1;
    }

    free(buf);

    cJSON* test = cJSON_GetArrayItem(root, test_idx);
    if (!test) {
        fprintf(stderr, "Error getting test item\n");
        cJSON_Delete(root);
        return 1;
    }

    cJSON* initial = cJSON_GetObjectItem(test, "initial");
    if (!initial) {
        fprintf(stderr, "Error getting initial item\n");
        cJSON_Delete(root);
        return 1;
    }

    cJSON* initial_pc = cJSON_GetObjectItem(initial, "pc");
    cJSON* initial_sp = cJSON_GetObjectItem(initial, "s");
    cJSON* initial_sr = cJSON_GetObjectItem(initial, "p");
    cJSON* initial_a = cJSON_GetObjectItem(initial, "a");
    cJSON* initial_x = cJSON_GetObjectItem(initial, "x");
    cJSON* initial_y = cJSON_GetObjectItem(initial, "y");

    cJSON* initial_ram_array = cJSON_GetObjectItem(initial, "ram");
    cJSON* initial_ram_element = NULL;

    cJSON* final = cJSON_GetObjectItem(test, "final");
    if (!final) {
        fprintf(stderr, "Error getting final item\n");
        cJSON_Delete(root);
        return 1;
    }

    cJSON* final_pc = cJSON_GetObjectItem(final, "pc");
    cJSON* final_sp = cJSON_GetObjectItem(final, "s");
    cJSON* final_sr = cJSON_GetObjectItem(final, "p");
    cJSON* final_a = cJSON_GetObjectItem(final, "a");
    cJSON* final_x = cJSON_GetObjectItem(final, "x");
    cJSON* final_y = cJSON_GetObjectItem(final, "y");

    cJSON* final_ram_array = cJSON_GetObjectItem(final, "ram");
    cJSON* final_ram_element = NULL;

    emulator_reset(emulator);

    emulator->cpu.pc = initial_pc->valueint;
    emulator->cpu.sp = initial_sp->valueint;
    cpu_set_status(&emulator->cpu, initial_sr->valueint);
    emulator->cpu.a = initial_a->valueint;
    emulator->cpu.x = initial_x->valueint;
    emulator->cpu.y = initial_y->valueint;

    cJSON_ArrayForEach(initial_ram_element, initial_ram_array) {
        cJSON* addr = cJSON_GetArrayItem(initial_ram_element, 0);
        cJSON* val = cJSON_GetArrayItem(initial_ram_element, 1);

        emulator->cpu.write_bus(emulator->cpu.bus, addr->valueint, val->valueint);
    }

    // Finish the reset routine
    do {
        emulator_run(emulator);
    } while(!cpu_is_complete(&emulator->cpu));

    if(cpu_is_illegal(&emulator->cpu))
        return 2;

    // Execute one instruction
    do {
        emulator_run(emulator);
    } while(!cpu_is_complete(&emulator->cpu));

    if((emulator->cpu.sp != final_sp->valueint) || (cpu_get_status(&emulator->cpu) != final_sr->valueint) || (emulator->cpu.a != final_a->valueint) || (emulator->cpu.x != final_x->valueint) || (emulator->cpu.y != final_y->valueint))
        return 1;

    cJSON_ArrayForEach(final_ram_element, final_ram_array) {
        cJSON* addr = cJSON_GetArrayItem(final_ram_element, 0);
        cJSON* val = cJSON_GetArrayItem(final_ram_element, 1);

        if(emulator->cpu.read_bus(emulator->cpu.bus, addr->valueint) != val->valueint)
            return 1;
    }

    return 0;
}

// // Test a single opcode with all 10,000 tests
// void test_opcode_all(emulator_t* emulator, const char* path) {
//     for(u16 i; i < 10000; ++i) {
//         u8 res = test_opcode_single(emulator, path, i);
//         res == true ? printf("Test PASSED!\n") : printf("Test FAILED!\n");
//     }
// }

// Test all opcodes with a single test each
void test_all_opcodes(emulator_t* emulator) {
    FILE* file = fopen("logs/log.txt", "w+");
    
    char path[32];
    for(u16 i = 0; i <= 0xFF; i++)
    {
        snprintf(path, 32, "SingleStepTests/%02x.json", i);

        u8 res = test_opcode_single(emulator, path, (u16)i);

        fprintf(file, "%X: ", i);

        switch(res) {
            case 0:
                fprintf(file, "PASS\n");
                break;
            case 1:
                fprintf(file, "FAIL\n");
                break;
            case 2:
                fprintf(file, "ILLEGAL\n");
                break;
        }
    }

    fclose(file);
}

void draw_cpu(emulator_t* emulator) {
    printw("-- CPU --\n");
    printw("F: %X\n", cpu_get_status(&emulator->cpu));
    printw("A: %X\n", emulator->cpu.a);
    printw("X: %X\n", emulator->cpu.x);
    printw("Y: %X\n", emulator->cpu.y);
    printw("SP: %X\n", emulator->cpu.sp);
    printw("PC: %X\n", emulator->cpu.pc);
}

void draw_mem(emulator_t* emulator, u16 base_addr) {
    u16 addr = base_addr;

    printw("-- MEM --\n");
    for(size_t y = 0; y < MEM_ROWS; ++y) {
        printw("%04X: ", addr);
        for(size_t x = 0; x < MEM_COLS; ++x) {
            printw("%02X ", emulator->mem[addr]);
            ++addr;
        }
        printw("\n");
    }
}

int main(int argc, char* argv[]) {

    emulator_t emulator;
    emulator_init(&emulator);

    if(argc > 1)
        emulator_load(&emulator, argv[0]);

    test_all_opcodes(&emulator);

    initscr();
    noecho();

    u16 addr = 0x0000;

    while(1) {
        clear();

        draw_cpu(&emulator);
        draw_mem(&emulator, addr);

        refresh();

        char c = getch();

        if(c == 's') {
            emulator_run(&emulator); 
        }

        if(c == 'd') {
            do {
                emulator_run(&emulator);
            } while(!cpu_is_complete(&emulator.cpu));
        }

        if(c == 'q') {
            if(addr > 0) {
                addr -= MEM_COLS * MEM_SCROLL_SPEED;
            }
        }

        else if(c == 'e') {
            if(addr < MEM_SIZE - (MEM_COLS * MEM_ROWS)) {
                addr += MEM_COLS * MEM_SCROLL_SPEED;
            }
        }
    }

    endwin();
    
    return 0;
}