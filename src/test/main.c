#include <ncurses.h>

#include "emulator.h"

int main(int argc, char* argv[]) {
    // initscr();
    // printw("Hello World!");
    // refresh();
    // getch();
    // endwin();

    emulator_t emulator;

    emulator_init(&emulator);

    // emulator_load(&emulator, argv[1]);

    while(true)
        cpu_clock(&emulator.cpu);

    return 0;
}