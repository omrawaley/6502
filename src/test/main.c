#include <ncurses.h>

#include "emulator.h"

void draw_tui(emulator_t* emulator) {
    
}

int main(int argc, char* argv[]) {

    emulator_t emulator;

    emulator_init(&emulator);

    emulator_load(&emulator, argv[1]);

    initscr();

    while(1) {
        clear();

        printw("A: %u\n", emulator.cpu.a);
        printw("X: %u\n", emulator.cpu.x);
        printw("Y: %u\n", emulator.cpu.y);
        printw("SP: %u\n", emulator.cpu.sp);
        printw("PC: %u\n", emulator.cpu.pc);

        refresh();

        char c = getch();

        if(c == 's') {
            emulator_run(&emulator); 
        }
    }

    endwin();

    return 0;
}