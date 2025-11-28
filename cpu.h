#include <stdint.h>

#ifndef DEBUG
#define DEBUG
#endif

#define RAM_SIZE 32000
#define VRAM_SIZE 16000

typedef unsigned char Byte;
typedef uint16_t Mem_ptr;
static Byte RAM[RAM_SIZE];
static Byte VRAM[VRAM_SIZE];

struct CPU{
    unsigned long long cycle;
    unsigned long cycle_queue;
    Mem_ptr PC;
    Mem_ptr SP;
    Byte A; // Storing data afterarithmetic and logical operations
    Byte F; // Flag register Z N H CY X X X X
    Byte B; Byte C;
    Byte D; Byte E;
    Byte H; Byte L;

};


enum FLAG_MASM{
    FLAG_MASK_Z = 0b1000000,
    FLAG_MASK_N = 0b0100000,
    FLAG_MASK_H = 0b0010000,
    FLAG_MASK_CY = 0b0001000
};

enum REGISTER_MASK{
    A = 0b111,
    B = 0b000,
    C = 0b001,
    D = 0b010,
    E = 0b011,
    H = 0b100,
    L = 0b101
};

