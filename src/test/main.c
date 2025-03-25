#include <ncurses.h>

#include "emulator.h"
#include "../../deps/cjson/cJSON.h"

#define MEM_COLS 0x10
#define MEM_ROWS 32

void test_opcode(emulator_t* emulator, u16 test_idx) {
    FILE* file = fopen("SingleStepTests/02.json", "r");

    if (!file) {
        fprintf(stderr, "Error opening file\n");
        return;
    }

    fseek(file, 0, SEEK_END);
    size_t size = ftell(file);
    rewind(file);

    char* buf = malloc(size);
    if (!buf) {
        fprintf(stderr, "Memory allocation failed\n");
        fclose(file);
        return;
    }

    fread(buf, 1, size, file);

    fclose(file);

    cJSON* root = cJSON_Parse(buf);
    if (root == NULL) {
        fprintf(stderr, "Error parsing buffer\n");
        free(buf);
        return;
    }

    free(buf);

    cJSON* test = cJSON_GetArrayItem(root, test_idx);
    if (!test) {
        fprintf(stderr, "Error getting test item\n");
        goto end;
    }

    cJSON* initial = cJSON_GetObjectItem(test, "initial");
    if (!initial) {
        fprintf(stderr, "Error getting initial item\n");
        goto end;
    }

    cJSON* initial_pc = cJSON_GetObjectItem(initial, "pc");
    cJSON* initial_sp = cJSON_GetObjectItem(initial, "s");
    cJSON* initial_sr = cJSON_GetObjectItem(initial, "p");
    cJSON* initial_a = cJSON_GetObjectItem(initial, "a");
    cJSON* initial_x = cJSON_GetObjectItem(initial, "x");
    cJSON* initial_y = cJSON_GetObjectItem(initial, "y");

    cJSON* final = cJSON_GetObjectItem(test, "final");
    if (!final) {
        fprintf(stderr, "Error getting final item\n");
        goto end;
    }

    cJSON* final_pc = cJSON_GetObjectItem(final, "pc");
    cJSON* final_sp = cJSON_GetObjectItem(final, "s");
    cJSON* final_sr = cJSON_GetObjectItem(final, "p");
    cJSON* final_a = cJSON_GetObjectItem(final, "a");
    cJSON* final_x = cJSON_GetObjectItem(final, "x");
    cJSON* final_y = cJSON_GetObjectItem(final, "y");

    do {
        emulator_run(emulator);
    } while(!cpu_is_complete(&emulator->cpu));

    emulator->cpu.pc = initial_pc->valueint;
    emulator->cpu.sp = initial_sp->valueint;
    emulator->cpu.pc = initial_pc->valueint;
    emulator->cpu.a = initial_a->valueint;
    emulator->cpu.x = initial_x->valueint;
    emulator->cpu.y = initial_y->valueint;

    if(emulator->cpu.pc != final_pc->valueint || emulator->cpu.sp != final_sp->valueint || (emulator->cpu.sr.n << 7) | (emulator->cpu.sr.v << 6) | (1 << 5) | (1 << 4) | (emulator->cpu.sr.d << 3) | (emulator->cpu.sr.i << 2) | (emulator->cpu.sr.z << 1) | (emulator->cpu.sr.c) != final_sr->valueint || emulator->cpu.a != initial_a->valueint || emulator->cpu.x != initial_x->valueint || emulator->cpu.y != initial_y->valueint) {
        printf("Test failed :(\n");
    }

    end:

    cJSON_Delete(root);
}

void draw_cpu(emulator_t* emulator) {
    printw("-- CPU --\n");
    printw("A: %X\n", emulator->cpu.a);
    printw("X: %X\n", emulator->cpu.x);
    printw("Y: %X\n", emulator->cpu.y);
    printw("SP: %X\n", emulator->cpu.sp);
    printw("PC: %X\n", emulator->cpu.pc);
}

void draw_mem(emulator_t* emulator) {
    u16 addr = 0x0000;

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

    test_opcode(&emulator, 0);

    // initscr();
    noecho();

    while(1) {
        clear();

        draw_cpu(&emulator);
        draw_mem(&emulator);

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
    }

    endwin();
    
    return 0;
}