#include <stdint.h>
#include <stdbool.h>

#ifndef DEBUG
#define DEBUG
#endif

#define RAM_SIZE 32000
#define VRAM_SIZE 16000

typedef uint8_t Byte;
typedef uint16_t Mem_ptr;

static Byte RAM[RAM_SIZE];
static Byte VRAM[VRAM_SIZE];

struct CPU{
    unsigned long long cycle;
    unsigned long cycle_queue;
    bool half_clock_speed;
    bool halt;
    Mem_ptr PC;
    Mem_ptr SP;
    Byte A; // Storing data afterarithmetic and logical operations
    Byte F; // Flag register Z N H CY X X X X
    Byte B; Byte C;
    Byte D; Byte E;
    Byte H; Byte L;

};


enum FLAG_MASK{
    FLAG_MASK_Z = 0b1000000,
    FLAG_MASK_N = 0b0100000,
    FLAG_MASK_H = 0b0010000,
    FLAG_MASK_CY = 0b0001000
};


enum ARITHM_ID{
    ARI8_ADD = 0b000,
    ARI8_ADC = 0b001,
    ARI8_SUB = 0b010,
    ARI8_SBC = 0b011,
    ARI8_AND = 0b100,
    ARI8_XOR = 0b101,
    ARI8_OR  = 0b110,
    ARI8_CP  = 0b111,
    ARI_INCR = 0b100,
    ARI_DECR = 0b101,
    ARI16_ADD = 0b001,
};

enum REGISTER_MASK{
    A = 0b111,
    B = 0b000,
    C = 0b001,
    D = 0b010,
    E = 0b011,
    H = 0b100,
    L = 0b101,
    MEM = 0b110,
    IMM = 0b110
};

enum REGISTER_PAIR_MASK{
    BC = 0b00,
    DE = 0b01,
    HL = 0b10,
    SP = 0b11,
};

