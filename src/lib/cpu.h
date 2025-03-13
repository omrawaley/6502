#ifndef M6502_CPU_H
#define M6502_CPU_H

#include <stdlib.h>
#include "types.h"

typedef struct cpu {
    u8 a, x, y;
    u8 sp;
    u16 pc;
    struct {
        u1 c : 1;
        u1 z : 1;
        u1 i : 1;
        u1 d : 1;
        u1 b : 1;
        u1 v : 1;
        u1 n : 1;
    } sr;

    void* bus;
    u8 (*read_bus)(void* ctx, u16 addr);
    void (*write_bus)(void* ctx, u16 addr, u8 val);
} cpu_t;

void cpu_clock(cpu_t* cpu);

static inline _Bool cpu_is_complete(cpu_t* cpu);

void cpu_reset(cpu_t* cpu);
void cpu_irq(cpu_t* cpu);
void cpu_nmi(cpu_t* cpu);

#endif //M6502_CPU_H
