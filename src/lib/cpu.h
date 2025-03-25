#ifndef M6502_CPU_H
#define M6502_CPU_H

#include <stdlib.h>
#include "types.h"

#define NMI_START 0xFFFA
// #define NMI_END 0xFFFB
#define RST_START 0xFFFC
// #define RST_END 0xFFFD
#define IRQ_START 0xFFFE
// #define IRQ_END 0xFFFF

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

_Bool cpu_is_complete(cpu_t* cpu);

void cpu_reset(cpu_t* cpu);
void cpu_irq(cpu_t* cpu);
void cpu_nmi(cpu_t* cpu);

void cpu_set_status(cpu_t* cpu, u8 byte);
u8 cpu_get_status(cpu_t* cpu);

#endif //M6502_CPU_H
