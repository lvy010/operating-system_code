#include "cpu.h"
#include "memory.h"

typedef enum {
    carry_flag     = 0x01,
    zero_flag      = 0x02,
    interrupt_flag = 0x04,
    decimal_flag   = 0x08,
    break_flag     = 0x10,
    unused_flag    = 0x20,
    overflow_flag  = 0x40,
    negative_flag  = 0x80
} cpu_p_flag;

typedef enum {
    carry_bp      = 0,
    zero_bp       = 1,
    interrupt_bp  = 2,
    decimal_bp    = 3,
    break_bp      = 4,
    unused_bp     = 5,
    overflow_bp   = 6,
    negative_bp   = 7
} cpu_p_bp;

typedef struct {
    word PC; // Program Counter,
    byte SP; // Stack Pointer,
    byte A, X, Y; // Registers
    byte P; // Flag Register
} CPU_STATE;

extern CPU_STATE cpu;

extern byte CPU_RAM[0x8000];

extern byte op_code;             // Current instruction code
extern int op_value, op_address; // Arguments for current instruction
extern int op_cycles;            // Additional instruction cycles used (e.g. when paging occurs)

extern unsigned long long cpu_cycles;  // Total CPU Cycles Since Power Up (wraps)


// CPU Addressing Modes

void cpu_address_implied()
{
}

void cpu_address_immediate()
{
    op_value = memory_readb(cpu.PC);
    cpu.PC++;

}

void cpu_address_zero_page()
{
    op_address = memory_readb(cpu.PC);
    op_value = CPU_RAM[op_address];
    cpu.PC++;
}

void cpu_address_zero_page_x()
{
    op_address = (memory_readb(cpu.PC) + cpu.X) & 0xFF;
    op_value = CPU_RAM[op_address];
    cpu.PC++;
}

void cpu_address_zero_page_y()
{
    op_address = (memory_readb(cpu.PC) + cpu.Y) & 0xFF;
    op_value = CPU_RAM[op_address];
    cpu.PC++;
}

void cpu_address_absolute()
{
    op_address = memory_readw(cpu.PC);
    op_value = memory_readb(op_address);
    cpu.PC += 2;
}

void cpu_address_absolute_x()
{
    op_address = memory_readw(cpu.PC) + cpu.X;
    op_value = memory_readb(op_address);
    cpu.PC += 2;

    if ((op_address >> 8) != (cpu.PC >> 8)) {
        op_cycles++;
    }
}

void cpu_address_absolute_y()
{
    op_address = (memory_readw(cpu.PC) + cpu.Y) & 0xFFFF;
    op_value = memory_readb(op_address);
    cpu.PC += 2;

    if ((op_address >> 8) != (cpu.PC >> 8)) {
        op_cycles++;
    }
}

void cpu_address_relative()
{
    op_address = memory_readb(cpu.PC);
    cpu.PC++;
    if (op_address & 0x80)
        op_address -= 0x100;
    op_address += cpu.PC;

    if ((op_address >> 8) != (cpu.PC >> 8)) {
        op_cycles++;
    }
}

void cpu_address_indirect()
{
    word arg_addr = memory_readw(cpu.PC);

    // The famous 6502 bug when instead of reading from $C0FF/$C100 it reads from $C0FF/$C000
    if ((arg_addr & 0xFF) == 0xFF) {
        // Buggy code
        op_address = (memory_readb(arg_addr & 0xFF00) << 8) + memory_readb(arg_addr);
    }
    else {
        // Normal code
        op_address = memory_readw(arg_addr);
    }
    cpu.PC += 2;
}

void cpu_address_indirect_x()
{
    byte arg_addr = memory_readb(cpu.PC);
    op_address = (memory_readb((arg_addr + cpu.X + 1) & 0xFF) << 8) | memory_readb((arg_addr + cpu.X) & 0xFF);
    op_value = memory_readb(op_address);
    cpu.PC++;
}

void cpu_address_indirect_y()
{
    byte arg_addr = memory_readb(cpu.PC);
    op_address = (((memory_readb((arg_addr + 1) & 0xFF) << 8) | memory_readb(arg_addr)) + cpu.Y) & 0xFFFF;
    op_value = memory_readb(op_address);
    cpu.PC++;

    if ((op_address >> 8) != (cpu.PC >> 8)) {
        op_cycles++;
    }
}
