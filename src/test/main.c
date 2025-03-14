#include <ncurses.h>

#include "emulator.h"

void draw_cpu(emulator_t* emulator) {
    printw("-- CPU --\n");
        printw("A: %X\n", emulator->cpu.a);
        printw("X: %X\n", emulator->cpu.x);
        printw("Y: %X\n", emulator->cpu.y);
        printw("SP: %X\n", emulator->cpu.sp);
        printw("PC: %X\n", emulator->cpu.pc);
}

int main(int argc, char* argv[]) {

    emulator_t emulator;

    emulator_init(&emulator);

    if(argc > 1)
        emulator_load(&emulator, argv[1]);

    initscr();
    noecho();

    WINDOW* cpu_win = newwin(10, 10, 0, 0);

    while(1) {
        clear();

        draw_cpu(&emulator);

        refresh();

        // char c = getch();

        // if(c == 's') {
            emulator_run(&emulator); 
        // }
    }

    free(cpu_win);
    endwin();

    return 0;
}