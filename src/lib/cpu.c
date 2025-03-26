#include "cpu.h"

#define STACK_START 0x0100
#define STACK_END 0x01FF

#define SP_START 0xFD

#define NUM_MAX_OPCODES 256

#define MSB 0x80
#define LSB 0x1

static u8 cycles;

static u16 addr;
static u8 val;

typedef enum addr_mode {
    IMPLICIT,
    ACCUMULATOR,
    IMMEDIATE,
    ZERO_PAGE,
    ZERO_PAGE_X,
    ZERO_PAGE_Y,
    RELATIVE,
    ABSOLUTE,
    ABSOLUTE_X,
    ABSOLUTE_Y,
    INDIRECT,
    INDEXED_INDIRECT,
    INDIRECT_INDEXED,
} addr_mode_t;

typedef struct instr {
    u8 cycles;
    addr_mode_t addr_mode;
    void (*exec_instruction)(cpu_t* cpu);
} instr_t;

// =================================================================================
// UTILITY FUNCTIONS
// =================================================================================

_Bool cpu_is_complete(cpu_t* cpu) {
    return cycles == 0;
}

static inline u16 cpu_read_word_from_bus(cpu_t* cpu, u16 addr) {
    return cpu->read_bus(cpu->bus, addr) | (cpu->read_bus(cpu->bus, addr + 1) << 8);
}

static inline u8 cpu_fetch_byte(cpu_t* cpu) {
    return cpu->read_bus(cpu->bus, cpu->pc++);
}

static inline u16 cpu_fetch_word(cpu_t* cpu) {
    const u16 val =cpu_read_word_from_bus(cpu, cpu->pc);
    cpu->pc += 2;
    return val;
}

static inline void cpu_push(cpu_t* cpu, u8 byte) {
    cpu->write_bus(cpu->bus, STACK_START + cpu->sp, byte);
    --cpu->sp;
}

static inline u8 cpu_pop(cpu_t* cpu) {
    ++cpu->sp;
    return cpu->read_bus(cpu->bus, STACK_START + cpu->sp);
}

void cpu_set_status(cpu_t* cpu, u8 byte) {
    cpu->sr.c = byte & 0x1;
    cpu->sr.z = (byte & 0x2) >> 1;
    cpu->sr.i = (byte & 0x4) >> 2;
    cpu->sr.d = (byte & 0x8) >> 3;
    cpu->sr.v = (byte & 0x40) >> 6;
    cpu->sr.n = (byte & MSB) >> 7;
}

u8 cpu_get_status(cpu_t* cpu) {
    return (cpu->sr.n << 7) | (cpu->sr.v << 6) | (1 << 5) | (cpu->sr.d << 3) | (cpu->sr.i << 2) | (cpu->sr.z << 1) | (cpu->sr.c);
}

static inline void cpu_push_status(cpu_t* cpu) {
    cpu_push(cpu, cpu_get_status(cpu) | 0x10);
}

static inline void cpu_pop_status(cpu_t* cpu) {
    cpu_set_status(cpu, cpu_pop(cpu));
}

// =================================================================================
// INTERRUPTS
// =================================================================================

void cpu_reset(cpu_t* cpu) {
    cpu->pc = cpu_read_word_from_bus(cpu->bus, RST_START);

    cpu->a = cpu->x = cpu->y = 0;
    cpu->sp = SP_START;
    cpu->sr.c = 0;
    cpu->sr.z = 0;
    cpu->sr.i = 0;
    cpu->sr.d = 0;
    cpu->sr.v = 0;
    cpu->sr.n = 0;

    cycles = 7;
}

void cpu_irq(cpu_t* cpu) {
    if(cpu->sr.i == 1)
        return;

    cpu_push(cpu, cpu->pc & 0xFF);
    cpu_push(cpu, (cpu->pc >> 8) & 0xFF);

    cpu_push_status(cpu);

    cpu->pc = cpu_read_word_from_bus(cpu->bus, IRQ_START);

    cycles = 7;
}

void cpu_nmi(cpu_t* cpu) {
    cpu_push(cpu, cpu->pc & 0xFF);
    cpu_push(cpu, (cpu->pc >> 8) & 0xFF);

    cpu_push_status(cpu);

    cpu->pc = cpu_read_word_from_bus(cpu->bus, NMI_START);

    cycles = 7;
}

// =================================================================================
// ADDRESSING MODES
// =================================================================================

static void process_addr_mode(cpu_t* cpu, addr_mode_t addr_mode) {
    switch(addr_mode) {
        case IMPLICIT:
            break;
        case ACCUMULATOR:
            val = cpu->a;
            break;
        case IMMEDIATE:
            val = cpu_fetch_byte(cpu);
            break;
        case ZERO_PAGE:
            addr = cpu_fetch_byte(cpu);
            val = cpu->read_bus(cpu->bus, addr);
            break;
        case ZERO_PAGE_X:
            addr = (cpu_fetch_byte(cpu) + cpu->x) & 0xFF;
            val = cpu->read_bus(cpu->bus, addr);
            break;
        case ZERO_PAGE_Y:
            addr = (cpu_fetch_byte(cpu) + cpu->y) & 0xFF;
            val = cpu->read_bus(cpu->bus, addr);
            break;
        case RELATIVE:
            addr = cpu->pc - 1 + (int8_t)cpu_fetch_byte(cpu);
            break;
        case ABSOLUTE:
            addr = cpu_fetch_word(cpu) & 0xFFFF;
            val = cpu->read_bus(cpu->bus, addr);
            break;
        case ABSOLUTE_X:
            addr = (cpu_fetch_word(cpu) + cpu->x) & 0xFFFF;
            val = cpu->read_bus(cpu->bus, addr);
            break; 
        case ABSOLUTE_Y:
            addr = (cpu_fetch_word(cpu) + cpu->y) & 0xFFFF;
            val = cpu->read_bus(cpu->bus, addr);
            break;
        case INDIRECT:
            addr = cpu_read_word_from_bus(cpu, cpu_fetch_word(cpu));
            val = cpu->read_bus(cpu->bus, addr);
            break;
        case INDEXED_INDIRECT:
            addr = cpu_read_word_from_bus(cpu, (cpu_fetch_byte(cpu) + cpu->x) & 0xFF);
            val = cpu->read_bus(cpu->bus, addr);
            break;
        case INDIRECT_INDEXED:
            addr = cpu_read_word_from_bus(cpu, cpu_fetch_byte(cpu)) + cpu->y;
            val = cpu->read_bus(cpu->bus, addr);
            break; 
    }
}

// =================================================================================
// INSTRUCTIONS
// =================================================================================

//To-Do: Implement decimal mode
static void adc(cpu_t* cpu) {
    const u16 sum = (u16)cpu->a + (u16)val + (u16)cpu->sr.c;

    cpu->sr.c = sum > 255;
    cpu->sr.v = ~(cpu->a ^ val) & (cpu->a ^ sum) & MSB;

    cpu->a = (u8)sum;

    cpu->sr.z = cpu->a == 0;
    cpu->sr.n = cpu->a & MSB;
}

static void and(cpu_t* cpu) {
    cpu->a &= val;

    cpu->sr.z = cpu->a == 0;
    cpu->sr.n = cpu->a & MSB;
}

static void asl_acc(cpu_t* cpu) {
    cpu->sr.c = cpu->a & MSB;

    cpu->a <<= 1;

    cpu->sr.z = cpu->a == 0;
    cpu->sr.n = cpu->a & MSB;
}

static void asl(cpu_t* cpu) {
    cpu->sr.c = val & MSB;

    cpu->write_bus(cpu->bus, addr, val << 1);

    cpu->sr.z = (val << 1) == 0;
    cpu->sr.n = (val << 1) & MSB;
}

static void bcc(cpu_t* cpu) {
    if(cpu->sr.c == 0)
        cpu->pc = addr;
}

static void bcs(cpu_t* cpu) {
    if(cpu->sr.c == 1)
        cpu->pc = addr;
}

static void beq(cpu_t* cpu) {
    if(cpu->sr.z == 1)
        cpu->pc = addr;
}

static void bit(cpu_t* cpu) {
    u8 res = cpu->a & val;

    cpu->sr.z = res == 0;
    cpu->sr.v = val & 0x40;
    cpu->sr.n = val & MSB;
}

static void bmi(cpu_t* cpu) {
    if(cpu->sr.n == 1)
        cpu->pc = addr;
}

static void bne(cpu_t* cpu) {
    if(cpu->sr.z == 0)
        cpu->pc = addr;
}

static void bpl(cpu_t* cpu) {
    if(cpu->sr.n == 0)
        cpu->pc = addr;
}

static void brk(cpu_t* cpu) {
    ++cpu->pc;
    cpu_push(cpu, cpu->pc >> 8);
    cpu_push(cpu, cpu->pc & 0xFF);

    cpu_push_status(cpu);

    cpu->sr.i = 1;

    cpu->pc = cpu_read_word_from_bus(cpu, IRQ_START);
}

static void bvc(cpu_t* cpu) {
    if(cpu->sr.v == 0)
        cpu->pc = addr;
}

static void bvs(cpu_t* cpu) {
    if(cpu->sr.v == 1)
        cpu->pc = addr;
}

static void clc(cpu_t* cpu) {
    cpu->sr.c = 0;
}

static void cld(cpu_t* cpu) {
    cpu->sr.d = 0;
}

static void cli(cpu_t* cpu) {
    cpu->sr.i = 0;
}

static void clv(cpu_t* cpu) {
    cpu->sr.v = 0;
}

static void cmp(cpu_t* cpu) {
    cpu->sr.c = cpu->a >= val;
    cpu->sr.z = cpu->a == val;
    cpu->sr.n = ((u8)(cpu->a - val)) & MSB;
}

static void cpx(cpu_t* cpu) {
    cpu->sr.c = cpu->x >= val;
    cpu->sr.z = cpu->x == val;
    cpu->sr.n = ((u8)(cpu->x - val)) & MSB;
}

static void cpy(cpu_t* cpu) {
    cpu->sr.c = cpu->y >= val;
    cpu->sr.z = cpu->y == val;
    cpu->sr.n = ((u8)(cpu->y - val)) & MSB;
}

static void dec(cpu_t* cpu) {
    cpu->write_bus(cpu->bus, addr, cpu->read_bus(cpu->bus, addr) - 1);

    cpu->sr.z = cpu->read_bus(cpu->bus, addr) == 0;
    cpu->sr.n = cpu->read_bus(cpu->bus, addr) & MSB;
}

static void dex(cpu_t* cpu) {
    --cpu->x;

    cpu->sr.z = cpu->x == 0;
    cpu->sr.n = cpu->x & MSB;
}

static void dey(cpu_t* cpu) {
    --cpu->y;

    cpu->sr.z = cpu->y == 0;
    cpu->sr.n = cpu->y & MSB;
}

static void eor(cpu_t* cpu) {
    cpu->a ^= val;

    cpu->sr.z = cpu->a == 0;
    cpu->sr.n = cpu->a & MSB;
}

static void inc(cpu_t* cpu) {
    cpu->write_bus(cpu->bus, addr, cpu->read_bus(cpu->bus, addr) + 1);

    cpu->sr.z = cpu->read_bus(cpu->bus, addr) == 0;
    cpu->sr.n = cpu->read_bus(cpu->bus, addr) & MSB;
}

static void inx(cpu_t* cpu) {
    ++cpu->x;

    cpu->sr.z = cpu->x == 0;
    cpu->sr.n = cpu->x & MSB;
}

static void iny(cpu_t* cpu) {
    ++cpu->y;

    cpu->sr.z = cpu->y == 0;
    cpu->sr.n = cpu->y & MSB;
}

static void jmp(cpu_t* cpu) {
    cpu->pc = addr;
}

static void jsr(cpu_t* cpu) {
    cpu_push(cpu, (cpu->pc - 1) >> 8);
    cpu_push(cpu, (cpu->pc - 1) & 0xFF);
    cpu->pc = addr;
}

static void lda(cpu_t* cpu) {
    cpu->a = val;

    cpu->sr.z = cpu->a == 0;
    cpu->sr.n = cpu->a & MSB;
}

static void ldx(cpu_t* cpu) {
    cpu->x = val;

    cpu->sr.z = cpu->x == 0;
    cpu->sr.n = cpu->x & MSB;
}

static void ldy(cpu_t* cpu) {
    cpu->y = val;

    cpu->sr.z = cpu->y == 0;
    cpu->sr.n = cpu->y & MSB;
}

static void lsr_acc(cpu_t* cpu) {
    cpu->sr.c = cpu->a & LSB;

    cpu->a >>= 1;

    cpu->sr.z = cpu->a == 0;
    cpu->sr.n = cpu->a & MSB;
}

static void lsr(cpu_t* cpu) {
    cpu->sr.c = val & LSB;

    cpu->write_bus(cpu->bus, addr, val >> 1);

    cpu->sr.z = (val >> 1) == 0;
    cpu->sr.n = (val >> 1) & MSB;
}

static void nop(cpu_t* cpu) {
    ++cpu->pc;
};

static void ora(cpu_t* cpu) {
    cpu->a |= val;

    cpu->sr.z = cpu->a == 0;
    cpu->sr.n = cpu->a & MSB;
}

static void pha(cpu_t* cpu) {
    cpu_push(cpu, cpu->a);
}

static void php(cpu_t* cpu) {
    cpu_push_status(cpu);
}

static void pla(cpu_t* cpu) {
    cpu->a = cpu_pop(cpu);

    cpu->sr.z = cpu->a == 0;
    cpu->sr.n = cpu->a & MSB;
}

static void plp(cpu_t* cpu) {
    cpu_pop_status(cpu);
}

static void rol_acc(cpu_t* cpu) {
    const u1 old_carry = cpu->sr.c;

    cpu->sr.c = cpu->a & MSB;

    cpu->a = (cpu->a << 1) | old_carry;

    cpu->sr.z = cpu->a == 0;
    cpu->sr.n = cpu->a & MSB;
}

static void rol(cpu_t* cpu) {
    const u1 old_carry = cpu->sr.c;

    cpu->sr.c = val & MSB;

    cpu->write_bus(cpu->bus, addr, (val << 1) | old_carry);

    cpu->sr.z = ((val << 1) | old_carry) == 0;
    cpu->sr.n = ((val << 1) | old_carry) & MSB;
}

static void ror_acc(cpu_t* cpu) {
    const u1 old_carry = cpu->sr.c;

    cpu->sr.c = cpu->a & LSB;
    
    cpu->a = (cpu->a >> 1) | (old_carry << 7);

    cpu->sr.z = cpu->a == 0;
    cpu->sr.n = cpu->a & MSB;
}

static void ror(cpu_t* cpu) {
    const u1 old_carry = cpu->sr.c;

    cpu->sr.c = val & LSB;
    
    cpu->write_bus(cpu->bus, addr, (val >> 1) | (old_carry << 7));

    cpu->sr.z = ((val >> 1) | (old_carry << 7)) == 0;
    cpu->sr.n = ((val >> 1) | (old_carry << 7)) & MSB;
}

static void rti(cpu_t* cpu) {
    cpu_pop_status(cpu);

    const u8 hi = cpu_pop(cpu);
    const u8 lo = cpu_pop(cpu);

    cpu->pc = (hi << 8) | lo;
}

static void rts(cpu_t* cpu) {
    const u8 lo = cpu_pop(cpu);
    const u8 hi = cpu_pop(cpu);

    cpu->pc = (hi << 8) | lo;
    cpu->pc++;
}

static void sbc(cpu_t* cpu) {
    val = ~val;
    adc(cpu);
}

static void sec(cpu_t* cpu) {
    cpu->sr.c = 1;
}

static void sed(cpu_t* cpu) {
    cpu->sr.d = 1;
}

static void sei(cpu_t* cpu) {
    cpu->sr.i = 1;
}

static void sta(cpu_t* cpu) {
    cpu->write_bus(cpu->bus, addr, cpu->a);
}

static void stx(cpu_t* cpu) {
    cpu->write_bus(cpu->bus, addr, cpu->x);
}

static void sty(cpu_t* cpu) {
    cpu->write_bus(cpu->bus, addr, cpu->y);
}

static void tax(cpu_t* cpu) {
    cpu->x = cpu->a;

    cpu->sr.z = cpu->x == 0;
    cpu->sr.n = cpu->x & MSB;
}

static void tay(cpu_t* cpu) {
    cpu->y = cpu->a;

    cpu->sr.z = cpu->y == 0;
    cpu->sr.n = cpu->y & MSB;
}

static void tsx(cpu_t* cpu) {
    cpu->x = cpu->sp;

    cpu->sr.z = cpu->x == 0;
    cpu->sr.n = cpu->x & MSB;
}

static void txa(cpu_t* cpu) {
    cpu->a = cpu->x;

    cpu->sr.z = cpu->a == 0;
    cpu->sr.n = cpu->a & MSB;
}

static void txs(cpu_t* cpu) {
    cpu->sp = cpu->x;
}

static void tya(cpu_t* cpu) {
    cpu->a = cpu->y;

    cpu->sr.z = cpu->a == 0;
    cpu->sr.n = cpu->a & MSB;
}

// =================================================================================
// OPCODES
// =================================================================================

static instr_t opcode_table[NUM_MAX_OPCODES] = {
    {.exec_instruction = brk, .addr_mode = IMPLICIT, .cycles = 7},          //0x00 
    {.exec_instruction = ora, .addr_mode = INDEXED_INDIRECT, .cycles = 6},  //0x01
    {.exec_instruction = nop, .addr_mode = IMPLICIT, .cycles = 2},          //0x02
    {.exec_instruction = nop, .addr_mode = IMPLICIT, .cycles = 2},          //0x03
    {.exec_instruction = nop, .addr_mode = IMPLICIT, .cycles = 2},          //0x04
    {.exec_instruction = ora, .addr_mode = ZERO_PAGE, .cycles = 3},         //0x05
    {.exec_instruction = asl, .addr_mode = ZERO_PAGE, .cycles = 5},         //0x06
    {.exec_instruction = nop, .addr_mode = IMPLICIT, .cycles = 2},          //0x07
    {.exec_instruction = php, .addr_mode = IMPLICIT, .cycles = 3},          //0x08
    {.exec_instruction = ora, .addr_mode = IMMEDIATE, .cycles = 2},         //0x09
    {.exec_instruction = asl_acc, .addr_mode = ACCUMULATOR, .cycles = 2},   //0x0A
    {.exec_instruction = nop, .addr_mode = IMPLICIT, .cycles = 2},          //0x0B
    {.exec_instruction = nop, .addr_mode = IMPLICIT, .cycles = 2},          //0x0C
    {.exec_instruction = ora, .addr_mode = ABSOLUTE, .cycles = 4},          //0x0D
    {.exec_instruction = asl, .addr_mode = ABSOLUTE, .cycles = 6},          //0x0E
    {.exec_instruction = nop, .addr_mode = IMPLICIT, .cycles = 2},          //0x0F
    {.exec_instruction = bpl, .addr_mode = RELATIVE, .cycles = 2},          //0x10
    {.exec_instruction = ora, .addr_mode = INDIRECT_INDEXED, .cycles = 5},  //0x11
    {.exec_instruction = nop, .addr_mode = IMPLICIT, .cycles = 2},          //0x12
    {.exec_instruction = nop, .addr_mode = IMPLICIT, .cycles = 2},          //0x13
    {.exec_instruction = nop, .addr_mode = IMPLICIT, .cycles = 2},          //0x14
    {.exec_instruction = ora, .addr_mode = ZERO_PAGE_X, .cycles = 4},       //0x15
    {.exec_instruction = asl, .addr_mode = ZERO_PAGE_X, .cycles = 6},       //0x16
    {.exec_instruction = nop, .addr_mode = IMPLICIT, .cycles = 2},          //0x17
    {.exec_instruction = clc, .addr_mode = IMPLICIT, .cycles = 2},          //0x18
    {.exec_instruction = ora, .addr_mode = ABSOLUTE_Y, .cycles = 4},        //0x19
    {.exec_instruction = nop, .addr_mode = IMPLICIT, .cycles = 2},          //0x1A
    {.exec_instruction = nop, .addr_mode = IMPLICIT, .cycles = 2},          //0x1B
    {.exec_instruction = nop, .addr_mode = IMPLICIT, .cycles = 2},          //0x1C
    {.exec_instruction = ora, .addr_mode = ABSOLUTE_X, .cycles = 4},        //0x1D
    {.exec_instruction = asl, .addr_mode = ABSOLUTE_X, .cycles = 7},        //0x1E
    {.exec_instruction = nop, .addr_mode = IMPLICIT, .cycles = 2},          //0x1F
    {.exec_instruction = jsr, .addr_mode = ABSOLUTE, .cycles = 6},          //0x20
    {.exec_instruction = and, .addr_mode = INDEXED_INDIRECT, .cycles = 6},  //0x21
    {.exec_instruction = nop, .addr_mode = IMPLICIT, .cycles = 2},          //0x22   
    {.exec_instruction = nop, .addr_mode = IMPLICIT, .cycles = 2},          //0x23     
    {.exec_instruction = bit, .addr_mode = ZERO_PAGE, .cycles = 3},         //0x24
    {.exec_instruction = and, .addr_mode = ZERO_PAGE, .cycles = 3},         //0x25
    {.exec_instruction = rol, .addr_mode = ZERO_PAGE, .cycles = 5},         //0x26
    {.exec_instruction = nop, .addr_mode = IMPLICIT, .cycles = 2},          //0x27
    {.exec_instruction = plp, .addr_mode = IMPLICIT, .cycles = 4},          //0x28
    {.exec_instruction = and, .addr_mode = IMMEDIATE, .cycles = 2},         //0x29
    {.exec_instruction = rol_acc, .addr_mode = ACCUMULATOR, .cycles = 2},   //0x2A
    {.exec_instruction = nop, .addr_mode = IMPLICIT, .cycles = 2},          //0x2B
    {.exec_instruction = bit, .addr_mode = ABSOLUTE, .cycles = 4},          //0x2C
    {.exec_instruction = and, .addr_mode = ABSOLUTE, .cycles = 4},          //0x2D
    {.exec_instruction = rol, .addr_mode = ABSOLUTE, .cycles = 6},          //0x2E
    {.exec_instruction = nop, .addr_mode = IMPLICIT, .cycles = 2},          //0x2F
    {.exec_instruction = bmi, .addr_mode = RELATIVE, .cycles = 2},          //0x30
    {.exec_instruction = and, .addr_mode = INDIRECT_INDEXED, .cycles = 5},  //0x31
    {.exec_instruction = nop, .addr_mode = IMPLICIT, .cycles = 2},          //0x32
    {.exec_instruction = nop, .addr_mode = IMPLICIT, .cycles = 2},          //0x33
    {.exec_instruction = nop, .addr_mode = IMPLICIT, .cycles = 2},          //0x34
    {.exec_instruction = and, .addr_mode = ZERO_PAGE, .cycles = 3},         //0x35
    {.exec_instruction = rol, .addr_mode = ZERO_PAGE_X, .cycles = 6},       //0x36
    {.exec_instruction = nop, .addr_mode = IMPLICIT, .cycles = 2},          //0x37
    {.exec_instruction = sec, .addr_mode = IMPLICIT, .cycles = 2},          //0x38
    {.exec_instruction = and, .addr_mode = ABSOLUTE_Y, .cycles = 4},        //0x39
    {.exec_instruction = nop, .addr_mode = IMPLICIT, .cycles = 2},          //0x3A
    {.exec_instruction = nop, .addr_mode = IMPLICIT, .cycles = 2},          //0x3B
    {.exec_instruction = nop, .addr_mode = IMPLICIT, .cycles = 2},          //0x3C
    {.exec_instruction = and, .addr_mode = ABSOLUTE_X, .cycles = 4},        //0x3D
    {.exec_instruction = rol, .addr_mode = ABSOLUTE_X, .cycles = 7},        //0x3E
    {.exec_instruction = nop, .addr_mode = IMPLICIT, .cycles = 2},          //0x3F
    {.exec_instruction = rti, .addr_mode = IMPLICIT, .cycles = 6},          //0x40
    {.exec_instruction = eor, .addr_mode = INDEXED_INDIRECT, .cycles = 6},  //0x41
    {.exec_instruction = nop, .addr_mode = IMPLICIT, .cycles = 2},          //0x42
    {.exec_instruction = nop, .addr_mode = IMPLICIT, .cycles = 2},          //0x43
    {.exec_instruction = nop, .addr_mode = IMPLICIT, .cycles = 2},          //0x44
    {.exec_instruction = eor, .addr_mode = ZERO_PAGE, .cycles = 3},         //0x45
    {.exec_instruction = lsr, .addr_mode = ZERO_PAGE, .cycles = 5},         //0x46
    {.exec_instruction = nop, .addr_mode = IMPLICIT, .cycles = 2},          //0x47
    {.exec_instruction = pha, .addr_mode = IMPLICIT, .cycles = 3},          //0x48
    {.exec_instruction = eor, .addr_mode = IMMEDIATE, .cycles = 2},         //0x49
    {.exec_instruction = lsr_acc, .addr_mode = ACCUMULATOR, .cycles = 2},   //0x4A
    {.exec_instruction = nop, .addr_mode = IMPLICIT, .cycles = 2},          //0x4B
    {.exec_instruction = jmp, .addr_mode = ABSOLUTE, .cycles = 3},          //0x4C
    {.exec_instruction = eor, .addr_mode = ABSOLUTE, .cycles = 4},          //0x4D
    {.exec_instruction = lsr, .addr_mode = ABSOLUTE, .cycles = 6},          //0x4E
    {.exec_instruction = nop, .addr_mode = IMPLICIT, .cycles = 2},          //0x4F
    {.exec_instruction = bvc, .addr_mode = RELATIVE, .cycles = 2},          //0x50
    {.exec_instruction = eor, .addr_mode = INDIRECT_INDEXED, .cycles = 5},  //0x51
    {.exec_instruction = nop, .addr_mode = IMPLICIT, .cycles = 2},          //0x52
    {.exec_instruction = nop, .addr_mode = IMPLICIT, .cycles = 2},          //0x53
    {.exec_instruction = nop, .addr_mode = IMPLICIT, .cycles = 2},          //0x54
    {.exec_instruction = eor, .addr_mode = ZERO_PAGE_X, .cycles = 4},       //0x55
    {.exec_instruction = lsr, .addr_mode = ZERO_PAGE_X, .cycles = 6},       //0x56
    {.exec_instruction = nop, .addr_mode = IMPLICIT, .cycles = 2},          //0x57
    {.exec_instruction = cli, .addr_mode = IMPLICIT, .cycles = 2},          //0x58
    {.exec_instruction = eor, .addr_mode = ABSOLUTE_Y, .cycles = 4},        //0x59
    {.exec_instruction = nop, .addr_mode = IMPLICIT, .cycles = 2},          //0x5A
    {.exec_instruction = nop, .addr_mode = IMPLICIT, .cycles = 2},          //0x5B
    {.exec_instruction = nop, .addr_mode = IMPLICIT, .cycles = 2},          //0x5C
    {.exec_instruction = eor, .addr_mode = ABSOLUTE_X, .cycles = 4},        //0x5D
    {.exec_instruction = lsr, .addr_mode = ABSOLUTE_X, .cycles = 7},        //0x5E
    {.exec_instruction = nop, .addr_mode = IMPLICIT, .cycles = 2},          //0x5F
    {.exec_instruction = rts, .addr_mode = IMPLICIT, .cycles = 6},          //0x60
    {.exec_instruction = adc, .addr_mode = INDEXED_INDIRECT, .cycles = 6},  //0x61
    {.exec_instruction = nop, .addr_mode = IMPLICIT, .cycles = 2},          //0x62
    {.exec_instruction = nop, .addr_mode = IMPLICIT, .cycles = 2},          //0x63
    {.exec_instruction = nop, .addr_mode = IMPLICIT, .cycles = 2},          //0x64
    {.exec_instruction = adc, .addr_mode = ZERO_PAGE, .cycles = 3},         //0x65
    {.exec_instruction = ror, .addr_mode = ZERO_PAGE, .cycles = 5},         //0x66
    {.exec_instruction = nop, .addr_mode = IMPLICIT, .cycles = 2},          //0x67
    {.exec_instruction = pla, .addr_mode = IMPLICIT, .cycles = 4},          //0x68
    {.exec_instruction = adc, .addr_mode = IMMEDIATE, .cycles = 2},         //0x69
    {.exec_instruction = ror_acc, .addr_mode = ACCUMULATOR, .cycles = 2},   //0x6A
    {.exec_instruction = nop, .addr_mode = IMPLICIT, .cycles = 2},          //0x6B
    {.exec_instruction = jmp, .addr_mode = INDIRECT, .cycles = 5},          //0x6C
    {.exec_instruction = adc, .addr_mode = ABSOLUTE, .cycles = 4},          //0x6D
    {.exec_instruction = ror, .addr_mode = ABSOLUTE, .cycles = 6},          //0x6E
    {.exec_instruction = nop, .addr_mode = IMPLICIT, .cycles = 2},          //0x6F
    {.exec_instruction = bvs, .addr_mode = RELATIVE, .cycles = 2},          //0x70
    {.exec_instruction = adc, .addr_mode = INDIRECT_INDEXED, .cycles = 5},  //0x71
    {.exec_instruction = nop, .addr_mode = IMPLICIT, .cycles = 2},          //0x72
    {.exec_instruction = nop, .addr_mode = IMPLICIT, .cycles = 2},          //0x73
    {.exec_instruction = nop, .addr_mode = IMPLICIT, .cycles = 2},          //0x74
    {.exec_instruction = adc, .addr_mode = ZERO_PAGE_X, .cycles = 4},       //0x75
    {.exec_instruction = ror, .addr_mode = ZERO_PAGE_X, .cycles = 6},       //0x76
    {.exec_instruction = nop, .addr_mode = IMPLICIT, .cycles = 2},          //0x77
    {.exec_instruction = sei, .addr_mode = IMPLICIT, .cycles = 2},          //0x78
    {.exec_instruction = adc, .addr_mode = ABSOLUTE_Y, .cycles = 4},        //0x79
    {.exec_instruction = nop, .addr_mode = IMPLICIT, .cycles = 2},          //0x7A
    {.exec_instruction = nop, .addr_mode = IMPLICIT, .cycles = 2},          //0x7B
    {.exec_instruction = nop, .addr_mode = IMPLICIT, .cycles = 2},          //0x7C
    {.exec_instruction = adc, .addr_mode = ABSOLUTE_X, .cycles = 4},        //0x7D
    {.exec_instruction = ror, .addr_mode = ABSOLUTE_X, .cycles = 7},        //0x7E
    {.exec_instruction = nop, .addr_mode = IMPLICIT, .cycles = 2},          //0x7F
    {.exec_instruction = nop, .addr_mode = IMPLICIT, .cycles = 2},          //0x80
    {.exec_instruction = sta, .addr_mode = INDEXED_INDIRECT, .cycles = 6},  //0x81
    {.exec_instruction = nop, .addr_mode = IMPLICIT, .cycles = 2},          //0x82
    {.exec_instruction = nop, .addr_mode = IMPLICIT, .cycles = 2},          //0x83
    {.exec_instruction = sty, .addr_mode = ZERO_PAGE, .cycles = 3},         //0x84
    {.exec_instruction = sta, .addr_mode = ZERO_PAGE, .cycles = 3},         //0x85
    {.exec_instruction = stx, .addr_mode = ZERO_PAGE, .cycles = 3},         //0x86
    {.exec_instruction = nop, .addr_mode = IMPLICIT, .cycles = 2},          //0x87
    {.exec_instruction = dey, .addr_mode = IMPLICIT, .cycles = 2},          //0x88
    {.exec_instruction = nop, .addr_mode = IMPLICIT, .cycles = 2},          //0x89
    {.exec_instruction = txa, .addr_mode = IMPLICIT, .cycles = 2},          //0x8A
    {.exec_instruction = nop, .addr_mode = IMPLICIT, .cycles = 2},          //0x8B
    {.exec_instruction = sty, .addr_mode = ABSOLUTE, .cycles = 4},          //0x8C
    {.exec_instruction = sta, .addr_mode = ABSOLUTE, .cycles = 4},          //0x8D
    {.exec_instruction = stx, .addr_mode = ABSOLUTE, .cycles = 4},          //0x8E
    {.exec_instruction = nop, .addr_mode = IMPLICIT, .cycles = 2},          //0x8F
    {.exec_instruction = bcc, .addr_mode = RELATIVE, .cycles = 2},          //0x90
    {.exec_instruction = sta, .addr_mode = INDIRECT_INDEXED, .cycles = 6},  //0x91
    {.exec_instruction = nop, .addr_mode = IMPLICIT, .cycles = 2},          //0x92
    {.exec_instruction = nop, .addr_mode = IMPLICIT, .cycles = 2},          //0x93
    {.exec_instruction = sty, .addr_mode = ZERO_PAGE_X, .cycles = 4},       //0x94
    {.exec_instruction = sta, .addr_mode = ZERO_PAGE_X, .cycles = 4},       //0x95
    {.exec_instruction = stx, .addr_mode = ZERO_PAGE_Y, .cycles = 4},       //0x96
    {.exec_instruction = nop, .addr_mode = IMPLICIT, .cycles = 2},          //0x97
    {.exec_instruction = tya, .addr_mode = IMPLICIT, .cycles = 2},          //0x98
    {.exec_instruction = sta, .addr_mode = ABSOLUTE_Y, .cycles = 5},        //0x99
    {.exec_instruction = txs, .addr_mode = IMPLICIT, .cycles = 2},          //0x9A
    {.exec_instruction = nop, .addr_mode = IMPLICIT, .cycles = 2},          //0x9B
    {.exec_instruction = nop, .addr_mode = IMPLICIT, .cycles = 2},          //0x9C
    {.exec_instruction = sta, .addr_mode = ABSOLUTE_X, .cycles = 5},        //0x9D
    {.exec_instruction = nop, .addr_mode = IMPLICIT, .cycles = 2},          //0x9E
    {.exec_instruction = nop, .addr_mode = IMPLICIT, .cycles = 2},          //0x9F
    {.exec_instruction = ldy, .addr_mode = IMMEDIATE, .cycles = 2},         //0xA0
    {.exec_instruction = lda, .addr_mode = INDEXED_INDIRECT, .cycles = 6},  //0xA1
    {.exec_instruction = ldx, .addr_mode = IMMEDIATE, .cycles = 2},         //0xA2
    {.exec_instruction = nop, .addr_mode = IMPLICIT, .cycles = 2},          //0xA3
    {.exec_instruction = ldy, .addr_mode = ZERO_PAGE, .cycles = 3},         //0xA4
    {.exec_instruction = lda, .addr_mode = ZERO_PAGE, .cycles = 3},         //0xA5
    {.exec_instruction = ldx, .addr_mode = ZERO_PAGE, .cycles = 3},         //0xA6
    {.exec_instruction = nop, .addr_mode = IMPLICIT, .cycles = 2},          //0xA7
    {.exec_instruction = tay, .addr_mode = IMPLICIT, .cycles = 2},          //0xA8
    {.exec_instruction = lda, .addr_mode = IMMEDIATE, .cycles = 2},         //0xA9
    {.exec_instruction = tax, .addr_mode = IMPLICIT, .cycles = 2},          //0xAA
    {.exec_instruction = nop, .addr_mode = IMPLICIT, .cycles = 2},          //0xAB
    {.exec_instruction = ldy, .addr_mode = ABSOLUTE, .cycles = 4},          //0xAC
    {.exec_instruction = lda, .addr_mode = ABSOLUTE, .cycles = 4},          //0xAD
    {.exec_instruction = ldx, .addr_mode = ABSOLUTE, .cycles = 4},          //0xAE
    {.exec_instruction = nop, .addr_mode = IMPLICIT, .cycles = 2},          //0xAF
    {.exec_instruction = bcs, .addr_mode = RELATIVE, .cycles = 2},          //0xB0
    {.exec_instruction = lda, .addr_mode = INDIRECT_INDEXED, .cycles = 5},  //0xB1
    {.exec_instruction = nop, .addr_mode = IMPLICIT, .cycles = 2},          //0xB2
    {.exec_instruction = nop, .addr_mode = IMPLICIT, .cycles = 2},          //0xB3
    {.exec_instruction = ldy, .addr_mode = ZERO_PAGE_X, .cycles = 4},       //0xB4
    {.exec_instruction = lda, .addr_mode = ZERO_PAGE_X, .cycles = 4},       //0xB5
    {.exec_instruction = ldx, .addr_mode = ZERO_PAGE_Y, .cycles = 4},       //0xB6
    {.exec_instruction = nop, .addr_mode = IMPLICIT, .cycles = 2},          //0xB7
    {.exec_instruction = clv, .addr_mode = IMPLICIT, .cycles = 2},          //0xB8
    {.exec_instruction = lda, .addr_mode = ABSOLUTE_Y, .cycles = 4},        //0xB9
    {.exec_instruction = tsx, .addr_mode = IMPLICIT, .cycles = 2},          //0xBA
    {.exec_instruction = nop, .addr_mode = IMPLICIT, .cycles = 2},          //0xBB
    {.exec_instruction = ldy, .addr_mode = ABSOLUTE_X, .cycles = 4},        //0xBC
    {.exec_instruction = lda, .addr_mode = ABSOLUTE_X, .cycles = 4},        //0xBD
    {.exec_instruction = ldx, .addr_mode = ABSOLUTE_Y, .cycles = 4},        //0xBE
    {.exec_instruction = nop, .addr_mode = IMPLICIT, .cycles = 2},          //0xBF
    {.exec_instruction = cpy, .addr_mode = IMMEDIATE, .cycles = 2},         //0xC0
    {.exec_instruction = cmp, .addr_mode = INDEXED_INDIRECT, .cycles = 6},  //0xC1
    {.exec_instruction = nop, .addr_mode = IMPLICIT, .cycles = 2},          //0xC2
    {.exec_instruction = nop, .addr_mode = IMPLICIT, .cycles = 2},          //0xC3
    {.exec_instruction = cpy, .addr_mode = ZERO_PAGE, .cycles = 3},         //0xC4
    {.exec_instruction = cmp, .addr_mode = ZERO_PAGE, .cycles = 3},         //0xC5
    {.exec_instruction = dec, .addr_mode = ZERO_PAGE, .cycles = 5},         //0xC6
    {.exec_instruction = nop, .addr_mode = IMPLICIT, .cycles = 2},          //0xC7
    {.exec_instruction = iny, .addr_mode = IMPLICIT, .cycles = 2},          //0xC8
    {.exec_instruction = cmp, .addr_mode = IMMEDIATE, .cycles = 2},         //0xC9
    {.exec_instruction = dex, .addr_mode = IMPLICIT, .cycles = 2},          //0xCA
    {.exec_instruction = nop, .addr_mode = IMPLICIT, .cycles = 2},          //0xCB
    {.exec_instruction = cpy, .addr_mode = ABSOLUTE, .cycles = 4},          //0xCC
    {.exec_instruction = cmp, .addr_mode = ABSOLUTE, .cycles = 4},          //0xCD
    {.exec_instruction = dec, .addr_mode = ABSOLUTE, .cycles = 6},          //0xCE
    {.exec_instruction = nop, .addr_mode = IMPLICIT, .cycles = 2},          //0xCF
    {.exec_instruction = bne, .addr_mode = RELATIVE, .cycles = 2},          //0xD0
    {.exec_instruction = cmp, .addr_mode = INDIRECT_INDEXED, .cycles = 5},  //0xD1
    {.exec_instruction = nop, .addr_mode = IMPLICIT, .cycles = 2},          //0xD2
    {.exec_instruction = nop, .addr_mode = IMPLICIT, .cycles = 2},          //0xD3
    {.exec_instruction = nop, .addr_mode = IMPLICIT, .cycles = 2},          //0xD4
    {.exec_instruction = cmp, .addr_mode = ZERO_PAGE_X, .cycles = 4},       //0xD5
    {.exec_instruction = dec, .addr_mode = ZERO_PAGE_X, .cycles = 6},       //0xD6
    {.exec_instruction = nop, .addr_mode = IMPLICIT, .cycles = 2},          //0xD7
    {.exec_instruction = cld, .addr_mode = IMPLICIT, .cycles = 2},          //0xD8
    {.exec_instruction = cmp, .addr_mode = ABSOLUTE_Y, .cycles = 4},        //0xD9
    {.exec_instruction = nop, .addr_mode = IMPLICIT, .cycles = 2},          //0xDA
    {.exec_instruction = nop, .addr_mode = IMPLICIT, .cycles = 2},          //0xDB
    {.exec_instruction = nop, .addr_mode = IMPLICIT, .cycles = 2},          //0xDC
    {.exec_instruction = cmp, .addr_mode = ABSOLUTE_X, .cycles = 4},        //0xDD
    {.exec_instruction = dec, .addr_mode = ABSOLUTE_X, .cycles = 7},        //0xDE
    {.exec_instruction = nop, .addr_mode = IMPLICIT, .cycles = 2},          //0xDF
    {.exec_instruction = cpx, .addr_mode = IMMEDIATE, .cycles = 2},         //0xE0
    {.exec_instruction = sbc, .addr_mode = INDEXED_INDIRECT, .cycles = 6},  //0xE1
    {.exec_instruction = nop, .addr_mode = IMPLICIT, .cycles = 2},          //0xE2
    {.exec_instruction = nop, .addr_mode = IMPLICIT, .cycles = 2},          //0xE3
    {.exec_instruction = cpx, .addr_mode = ZERO_PAGE, .cycles = 3},         //0xE4
    {.exec_instruction = sbc, .addr_mode = ZERO_PAGE, .cycles = 3},         //0xE5
    {.exec_instruction = inc, .addr_mode = ZERO_PAGE, .cycles = 5},         //0xE6
    {.exec_instruction = nop, .addr_mode = IMPLICIT, .cycles = 2},          //0xE7
    {.exec_instruction = inx, .addr_mode = IMPLICIT, .cycles = 2},          //0xE8
    {.exec_instruction = sbc, .addr_mode = IMMEDIATE, .cycles = 2},         //0xE9
    {.exec_instruction = nop, .addr_mode = IMPLICIT, .cycles = 2},          //0xEA
    {.exec_instruction = nop, .addr_mode = IMPLICIT, .cycles = 2},          //0xEB
    {.exec_instruction = cpx, .addr_mode = ABSOLUTE, .cycles = 4},          //0xEC
    {.exec_instruction = sbc, .addr_mode = ABSOLUTE, .cycles = 4},          //0xED
    {.exec_instruction = inc, .addr_mode = ABSOLUTE, .cycles = 6},          //0xEE
    {.exec_instruction = nop, .addr_mode = IMPLICIT, .cycles = 2},          //0xEF
    {.exec_instruction = beq, .addr_mode = RELATIVE, .cycles = 2},          //0xF0
    {.exec_instruction = sbc, .addr_mode = INDIRECT_INDEXED, .cycles = 5},  //0xF1
    {.exec_instruction = nop, .addr_mode = IMPLICIT, .cycles = 2},          //0xF2
    {.exec_instruction = nop, .addr_mode = IMPLICIT, .cycles = 2},          //0xF3
    {.exec_instruction = nop, .addr_mode = IMPLICIT, .cycles = 2},          //0xF4
    {.exec_instruction = sbc, .addr_mode = ZERO_PAGE_X, .cycles = 4},       //0xF5
    {.exec_instruction = inc, .addr_mode = ZERO_PAGE_X, .cycles = 6},       //0xF6
    {.exec_instruction = nop, .addr_mode = IMPLICIT, .cycles = 2},          //0xF7
    {.exec_instruction = sed, .addr_mode = IMPLICIT, .cycles = 2},          //0xF8
    {.exec_instruction = sbc, .addr_mode = ABSOLUTE_Y, .cycles = 4},        //0xF9
    {.exec_instruction = nop, .addr_mode = IMPLICIT, .cycles = 2},          //0xFA
    {.exec_instruction = nop, .addr_mode = IMPLICIT, .cycles = 2},          //0xFB
    {.exec_instruction = nop, .addr_mode = IMPLICIT, .cycles = 2},          //0xFC
    {.exec_instruction = sbc, .addr_mode = ABSOLUTE_X, .cycles = 4},        //0xFD
    {.exec_instruction = inc, .addr_mode = ABSOLUTE_X, .cycles = 7},        //0xFE
    {.exec_instruction = nop, .addr_mode = IMPLICIT, .cycles = 2},          //0xFF
};

_Bool cpu_is_illegal(cpu_t* cpu) {
    if(cycles == 0)
        return opcode_table[cpu->read_bus(cpu->bus, cpu->pc)].exec_instruction == nop;
    return 0;
}

void cpu_clock(cpu_t* cpu) {
    if(cycles == 0) {
        const u8 opcode = cpu_fetch_byte(cpu);

        cycles = opcode_table[opcode].cycles;

        process_addr_mode(cpu, opcode_table[opcode].addr_mode);

        opcode_table[opcode].exec_instruction(cpu);
    }

    cycles--;
}