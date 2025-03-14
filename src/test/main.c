#include "emulator.h"

int main(int argc, char* argv[]) {

    emulator_t emulator;

    emulator_init(&emulator);

    emulator_load(&emulator, argv[1]);

    while(1) {
        emulator_run(&emulator);
    }

    return 0;
}