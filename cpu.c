#include <stdio.h>
#include <stdbool.h>
#include "cpu.h"
// TODO: Make instruction fetch according to PM specs
// TOGO: Make load and store work
Byte fetch_instruction(Mem_ptr pc){
    Byte instruction = 0;
    return instruction;
}

Byte fetch_at_addr(Mem_ptr ptr){
    return 0;
}

Byte *get_mem_ptr(Mem_ptr ptr){
    return NULL;
}

Byte *get_register_ptr(struct CPU *cpu, Byte operand){
    Mem_ptr addr;
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
        case MEM:
            addr = (((Mem_ptr) cpu->H) << 8) + cpu->L;
            // Reading from mem increases cycles needed
            // by one
            cpu->cycle_queue += 1;
            return get_mem_ptr(addr);
        default:
#ifdef DEBUG
        fprintf(stderr,"Could decode register pointer!\n");               
#endif      
        return NULL;
    }
}

Mem_ptr get_register_operand_16(struct CPU *cpu, Byte operand){
    Byte h,l;
    switch (operand) {
        case BC:
            h = cpu->B;
            l = cpu->C;
            break;
        case DE:
            h = cpu->D;
            l = cpu->E;
            break;
        case HL:
            h = cpu->H;
            l = cpu->L;
            break;
        case SP:
            return cpu->SP;
        default:
#ifdef DEBUG
            fprintf(stderr, "Error: Could not decode 16-bit register!\n");
#endif
            break;
    }
    return (((Mem_ptr) h) << 8) + l;
}

Byte get_register_operand(struct CPU *cpu, Byte operand){
    Mem_ptr addr;
    switch (operand){
        case A:
            return cpu->A;
        case B:
            return cpu->B;
        case C:
            return cpu->C;
        case D:
            return cpu->D;
        case E:
            return cpu->E;
        case H:
            return cpu->H;
        case L:
            return cpu->L;
        case MEM:
            addr = (((Mem_ptr) cpu->H) << 8) + cpu->L;
            // Reading from mem increases cycles needed
            // by one
            cpu->cycle_queue += 1;
            return fetch_at_addr(addr);
        default:
#ifdef DEBUG
        fprintf(stderr,"Error: Could decode register!\n");               
#endif      
        return 0;
    }
}


Mem_ptr add_instr_16(struct CPU* cpu, Mem_ptr l_operand, Mem_ptr r_operand){
    Byte flag = cpu->F & FLAG_MASK_Z;
    Mem_ptr result = l_operand + r_operand;
    if ((l_operand & 0xFF) + (r_operand & 0xFF) > 0xFF) flag |= FLAG_MASK_H;
    if (result <= l_operand) flag |= FLAG_MASK_CY;
    cpu->F = flag;
    return result;
}

Mem_ptr incr_instr_16(struct CPU* cpu, Mem_ptr operand){
    return ++operand;
}
Mem_ptr decr_instr_16(struct CPU* cpu, Mem_ptr operand){
    return --operand;
}

Byte incr_instr(struct CPU* cpu, Byte operand){
    Byte flag = cpu->F & FLAG_MASK_CY;
    if (operand == 0x0F) flag |= FLAG_MASK_H;
    else if (operand == 0xFF) flag |= FLAG_MASK_Z;
    cpu->F = flag;
    return ++operand;
}
Byte decr_instr(struct CPU* cpu, Byte operand){
    Byte flag = cpu->F & FLAG_MASK_CY;
    if (operand == 0x10) flag |= FLAG_MASK_H;
    else if (operand == 0x01) flag |= FLAG_MASK_Z;
    flag |= FLAG_MASK_N;
    cpu->F = flag;
    return --operand;
}

Byte add_instr(struct CPU* cpu, Byte l_operand, Byte r_operand, Byte carry){
    Byte flag = 0;
    Byte result = l_operand + r_operand + carry;
    if ((l_operand & 0x0F) + (r_operand & 0x0F) + carry > 0x0F) flag |= FLAG_MASK_H;
    if (!result) flag |= FLAG_MASK_Z;
    if (result <= l_operand) flag |= FLAG_MASK_CY;
    cpu->F = flag;
    cpu->cycle_queue += 1;
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
    cpu->cycle_queue += 1;
    return result;
}

Byte and_instr(struct CPU* cpu, Byte l_operand, Byte r_operand){
    Byte flag = 0;
    Byte result = l_operand & r_operand;
    flag |= FLAG_MASK_H;
    if (!result) flag |= FLAG_MASK_Z;
    cpu->F = flag;
    cpu->cycle_queue += 1;
    return result;
}
Byte or_instr(struct CPU* cpu, Byte l_operand, Byte r_operand){
    Byte flag = 0;
    Byte result = l_operand | r_operand;
    if (!result) flag |= FLAG_MASK_Z;
    cpu->F = flag;
    cpu->cycle_queue += 1;
    return result;
}
 
Byte xor_instr(struct CPU* cpu, Byte l_operand, Byte r_operand){
    Byte flag = 0;
    Byte result = l_operand ^ r_operand;
    if (!result) flag |= FLAG_MASK_Z;
    cpu->F = flag;
    cpu->cycle_queue += 1;
    return result;
}

void cp_instr(struct CPU* cpu,Byte l_operand, Byte r_operand){
    sub_instr(cpu, l_operand, r_operand, 0);
}


void instruction_decoder(struct CPU* cpu){
    Byte instruction = fetch_instruction(cpu->PC);
    
    Byte prefix = instruction >> 6;
    Byte suffix = instruction & 0b111;
    Byte middle = instruction & 0b00111000;
    Byte cy;
    Byte result_word;
    Mem_ptr result_dword;
    switch (prefix) {
        case 0b00: // Mem instructions and 16-bit arithemtic;
            switch (suffix) {
                case ARI_INCR:
                    result_word = incr_instr(cpu, get_register_operand(cpu, middle));
                    (*get_register_ptr(cpu, middle)) = result_word;
                    ++cpu->cycle_queue;
                    break;
                case ARI_DECR:
                    result_word = decr_instr(cpu, get_register_operand(cpu, middle));
                    (*get_register_ptr(cpu, middle)) = result_word;
                    ++cpu->cycle_queue;
                    break;
                case ARI16_ADD:
                    result_dword = add_instr_16(cpu,
                                                get_register_operand_16(cpu, HL),
                                                get_register_operand_16(cpu, (middle >> 4))
                                                );
                    cpu->cycle_queue += 2;
                    break;
            }
            break;
        case 0b01: // ld and halt
            break;
        case 0b10: // 8bit arithmetic Registers and mem
            switch (middle) {
                case ARI8_ADD:
                case ARI8_ADC:
                    cy = cpu->F & FLAG_MASK_CY & instruction;
                    cpu->A = add_instr(cpu,
                                       cpu->A,
                                       get_register_operand(cpu,suffix),
                                       cy
                                       );
                    ++cpu->cycle_queue;
                    break;
                case ARI8_SUB:
                case ARI8_SBC:
                    cy = cpu->F & FLAG_MASK_CY & instruction;
                    cpu->A = sub_instr(cpu,
                                       cpu->A,
                                       get_register_operand(cpu, suffix), 
                                       cy
                                       );
                    ++cpu->cycle_queue;
                    break;
                case ARI8_AND:
                    cpu->A = and_instr(cpu,
                                       cpu->A,
                                       get_register_operand(cpu, suffix)
                                       );
                    ++cpu->cycle_queue;
                    break;
                case ARI8_XOR:
                    cpu->A = xor_instr(cpu,
                                       cpu->A,
                                       get_register_operand(cpu, suffix)
                                       );
                    ++cpu->cycle_queue;
                    break;
                case ARI8_OR:
                    cpu->A = or_instr(cpu,
                                      cpu->A,
                                      get_register_operand(cpu,suffix)
                                      );
                    ++cpu->cycle_queue;
                    break;
                case ARI8_CP:
                    cp_instr(cpu,
                            cpu->A,
                            get_register_operand(cpu, suffix)
                            );
                    ++cpu->cycle_queue;
                    break;
                default:
#ifdef DEBUG
                    fprintf(stderr, "Error: Could not decode arithmetics with prefix 0b10!\n");
#endif
                    break;
            }
            ++cpu->PC;
            break;
        case 0b11: // Imm arithmetic and control instr
            if (suffix == IMM){
                switch (middle) {
                    case ARI8_ADD:
                    case ARI8_ADC:
                        cy = cpu->F & FLAG_MASK_CY & instruction;
                        cpu->A = add_instr(cpu,
                                           cpu->A,
                                           fetch_instruction(++cpu->PC),
                                           cy
                                           );
                        ++cpu->cycle_queue;
                        break;
                    case ARI8_SUB:
                    case ARI8_SBC:
                        cy = cpu->F & FLAG_MASK_CY & instruction;
                        cpu->A = sub_instr(cpu,
                                           cpu->A,
                                           fetch_instruction(++cpu->PC), 
                                           cy
                                           );
                        ++cpu->cycle_queue;
                        break;
                    case ARI8_AND:
                        cpu->A = and_instr(cpu,
                                           cpu->A,
                                           fetch_instruction(++cpu->PC) 
                                           );
                        ++cpu->cycle_queue;
                        break;
                    case ARI8_XOR:
                        cpu->A = xor_instr(cpu,
                                           cpu->A,
                                           fetch_instruction(++cpu->PC)
                                           );
                        ++cpu->cycle_queue;
                        break;
                    case ARI8_OR:
                        cpu->A = or_instr(cpu,
                                          cpu->A,
                                          fetch_instruction(++cpu->PC)
                                          );
                        ++cpu->cycle_queue;
                        break;
                    case ARI8_CP:
                        cp_instr(cpu,
                                 cpu->A,
                                 fetch_instruction(++cpu->PC)
                                 );
                        ++cpu->cycle_queue;
                    default:
#ifdef DEBUG
                        fprintf(stderr, "Error: Could not decode arithmetics with prefix 0b11!\n");
#endif
                    break;
                }
            } 
            else if( middle + suffix ==  0b101000){
                cpu->SP = add_instr_16(cpu, cpu->SP, fetch_instruction(++cpu->PC));
                cpu->cycle_queue += 4;
            }
            break;
    }
        

}
