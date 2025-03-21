#include <ncurses.h>

#include "emulator.h"

#define MEM_COLS 0x10
#define MEM_ROWS 32

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
        emulator_load(&emulator, argv[1]);

    initscr();
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