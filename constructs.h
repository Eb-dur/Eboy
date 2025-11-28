#include <stdio.h>
#include <stdbool.h>
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
    // Z: Set to 1 when the result of an operation is 0; otherwise reset.
    // N: Set to 1 following execution of the substruction instruction, regardless of the result.
    // H: Set to 1 when an operation results in carrying from or borrowing to bit 3.
    // CY: Set to 1 when an operation results in carrying from or borrowing to bit 7.
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

// TODO: Make instruction fetch according to PM specs
Byte fetch_instruction(Mem_ptr pc){
    Byte instruction = 0;
    return instruction;
}

Mem_ptr fetch_addr(Mem_ptr ptr){
    return 0;
}


Byte* get_register_operand(struct CPU *cpu, Byte operand){
    switch (operand){
        case A:
            return &(cpu->A);
        case B:
            return &(cpu->B);
        case C:
            return &(cpu->C);
        case D:
            return &(cpu->D);
        case E:
            return &(cpu->E);
        case H:
            return &(cpu->H);
        case L:
            return &(cpu->L);
        default:
#ifdef DEBUG
        fprintf(stderr,"Could decode register!\n");               
#endif      
        return NULL;
    }
}


Byte add_instr(struct CPU* cpu, Byte l_operand, Byte r_operand, Byte carry){
    Byte flag = 0;
    Byte result = l_operand + r_operand + carry;
    if ((l_operand & 0x0F) + (r_operand & 0x0F) + carry > 0x0F) flag |= FLAG_MASK_H;
    if (!result) flag |= FLAG_MASK_Z;
    if (result <= l_operand) flag |= FLAG_MASK_CY;
    return result;
}

Byte sub_instr(struct CPU* cpu, Byte l_operand, Byte r_operand, Byte carry){
    Byte flag = 0;
    Byte result = l_operand - r_operand - carry;
    if ((l_operand & 0x0F) < ((r_operand & 0xF0) - carry)) flag |= FLAG_MASK_H;
    if (!result) flag |= FLAG_MASK_Z;
    if (result >= l_operand) flag |= FLAG_MASK_CY;
    flag |= FLAG_MASK_Z;
    return result;
}


void instruction_decoder(struct CPU* cpu){
    Byte instruction = fetch_instruction(cpu->PC);
    
    // ADD 
    if ((instruction & 0b11111000) == 0b10000000){
        // ACC r into A
        Byte carry = (instruction & 0b00001000) ? 1 : 0;
        Byte operand = *get_register_operand(cpu, instruction & 0b111);
        Byte result = add_instr(cpu, cpu->A, operand, carry);
        if (instruction & 0b00001000) result++;
        cpu->A = result;
        ++cpu->PC;
        ++cpu->cycle_queue;
    }
    // ADD immediate
    else if (instruction == 0b11000110){
        Byte carry = (instruction & 0b00001000) ? 1 : 0;
        Byte operand = fetch_instruction(++cpu->PC);
        Byte result = add_instr(cpu, cpu->A, operand, carry);
        cpu->A = result;
        ++cpu->PC;
        cpu->cycle_queue += 2;
    }
    else if (instruction == 0b10000110){
    // ADD contents of memory specified by pair HL into register A
        // and stores it in A.
        // A <- A + [HL]
        Byte carry = (instruction & 0b00001000) ? 1 : 0;
        Mem_ptr mem_ptr = (((Mem_ptr) cpu->H) << 8) + cpu->L;
        Byte operand = fetch_addr(mem_ptr);
        Byte result = add_instr(cpu, cpu->A, operand, carry);
        cpu->A = result;
        ++cpu->PC;
        cpu->cycle_queue += 2;
    }
    else if ((instruction & 0b11111000) == 0b10010000) {
        // SUBR
        Byte carry = (instruction & 0b00001000) ? 1 : 0;
        Byte operand = *get_register_operand(cpu, instruction & 0b111);
        Byte result = sub_instr(cpu, cpu->A, operand, carry);
        if (instruction & 0b00001000) result++;
        cpu->A = result;
        ++cpu->PC;
        ++cpu->cycle_queue;
    }
    else if (instruction == 0b11010110) {
        // SUBI
        Byte carry = (instruction & 0b00001000) ? 1 : 0;
        Byte operand = fetch_instruction(++cpu->PC);
        Byte result = sub_instr(cpu, cpu->A, operand, carry);
        cpu->A = result;
        ++cpu->PC;
        cpu->cycle_queue += 2;
    }
    else if (instruction == 0b10010110) {
        // SUBMEM
        Byte carry = (instruction & 0b00001000) ? 1 : 0;
        Mem_ptr mem_ptr = (((Mem_ptr) cpu->H) << 8) + cpu->L;
        Byte operand = fetch_addr(mem_ptr);
        Byte result = add_instr(cpu, cpu->A, operand, carry);
        cpu->A = result;
        ++cpu->PC;
        cpu->cycle_queue += 2;
    }
        

}
