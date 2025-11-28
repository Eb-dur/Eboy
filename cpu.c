#include <stdio.h>
#include <stdbool.h>
#include "cpu.h"
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
    cpu->F = flag;
    return result;
}

Byte sub_instr(struct CPU* cpu, Byte l_operand, Byte r_operand, Byte carry){
    Byte flag = 0;
    Byte result = l_operand - r_operand - carry;
    if ((l_operand & 0x0F) < ((r_operand & 0xF0) - carry)) flag |= FLAG_MASK_H;
    if (!result) flag |= FLAG_MASK_Z;
    if (result >= l_operand) flag |= FLAG_MASK_CY;
    flag |= FLAG_MASK_N;
    cpu->F = flag;
    return result;
}

Byte and_instr(struct CPU* cpu, Byte l_operand, Byte r_operand){
    Byte flag = 0;
    Byte result = l_operand & r_operand;
    flag |= FLAG_MASK_H;
    if (!result) flag |= FLAG_MASK_Z;
    cpu->F = flag;
    return result;
}
Byte or_instr(struct CPU* cpu, Byte l_operand, Byte r_operand){
    Byte flag = 0;
    Byte result = l_operand | r_operand;
    if (!result) flag |= FLAG_MASK_Z;
    cpu->F = flag;
    return result;
}

Byte xor_instr(struct CPU* cpu, Byte l_operand, Byte r_operand){
    Byte flag = 0;
    Byte result = l_operand ^ r_operand;
    if (!result) flag |= FLAG_MASK_Z;
    cpu->F = flag;
    return result;
}


void instruction_decoder(struct CPU* cpu){
    Byte instruction = fetch_instruction(cpu->PC);
    //
    // MATHEMATICAL OPERATIONS
    //
    if ((instruction & 0b11111000) == 0b10000000){
        // ACC r into A
        Byte carry = (instruction & 0b00001000) ? 1 : 0;
        Byte operand = *get_register_operand(cpu, instruction & 0b111);
        Byte result = add_instr(cpu, cpu->A, operand, carry);
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

    //
    // BOOLEAN OPERATIONS
    //
    else if ((instruction & 0b11111000) == 0b10100000) {
        // AND R
        Byte operand = *get_register_operand(cpu, instruction & 0b111);
        Byte result = and_instr(cpu, cpu->A, operand);
        cpu->A = result;
        ++cpu->PC;
        ++cpu->cycle_queue;
    }
    else if (instruction == 0b11100110) {
        // AND I
        Byte operand = fetch_instruction(++cpu->PC);
        Byte result = and_instr(cpu, cpu->A, operand);
        cpu->A = result;
        ++cpu->PC;
        cpu->cycle_queue += 2;
    }
    else if (instruction == 0b10100110) {
        // AND mem
        Mem_ptr mem_ptr = (((Mem_ptr) cpu->H) << 8) + cpu->L;
        Byte operand = fetch_addr(mem_ptr);
        Byte result = and_instr(cpu, cpu->A, operand);
        cpu->A = result;
        ++cpu->PC;
        cpu->cycle_queue += 2;
    }
    else if ((instruction & 0b11111000) == 0b10110000) {
        // OR R
        Byte operand = *get_register_operand(cpu, instruction & 0b111);
        Byte result = or_instr(cpu, cpu->A, operand);
        cpu->A = result;
        ++cpu->PC;
        ++cpu->cycle_queue;
    }
    else if (instruction == 0b11110110) {
        // AND I
        Byte operand = fetch_instruction(++cpu->PC);
        Byte result = or_instr(cpu, cpu->A, operand);
        cpu->A = result;
        ++cpu->PC;
        cpu->cycle_queue += 2;
    }
    else if (instruction == 0b10110110) {
        // AND mem
        Mem_ptr mem_ptr = (((Mem_ptr) cpu->H) << 8) + cpu->L;
        Byte operand = fetch_addr(mem_ptr);
        Byte result = or_instr(cpu, cpu->A, operand);
        cpu->A = result;
        ++cpu->PC;
        cpu->cycle_queue += 2;
    }
    // TODO: XOR
    else if ((instruction & 0b11111000) == 0b10101000) {
        // OR R
        Byte operand = *get_register_operand(cpu, instruction & 0b111);
        Byte result = xor_instr(cpu, cpu->A, operand);
        cpu->A = result;
        ++cpu->PC;
        ++cpu->cycle_queue;
    }
    else if (instruction == 0b11101110) {
        // AND I
        Byte operand = fetch_instruction(++cpu->PC);
        Byte result = xor_instr(cpu, cpu->A, operand);
        cpu->A = result;
        ++cpu->PC;
        cpu->cycle_queue += 2;
    }
    else if (instruction == 0b10101110) {
        // AND mem
        Mem_ptr mem_ptr = (((Mem_ptr) cpu->H) << 8) + cpu->L;
        Byte operand = fetch_addr(mem_ptr);
        Byte result = xor_instr(cpu, cpu->A, operand);
        cpu->A = result;
        ++cpu->PC;
        cpu->cycle_queue += 2;
    }
        

}
