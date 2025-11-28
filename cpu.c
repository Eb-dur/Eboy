#include <stdio.h>
#include <stdbool.h>
#include "cpu.h"
// TODO: Make instruction fetch according to PM specs
Byte fetch_instruction(Mem_ptr pc){
    Byte instruction = 0;
    return instruction;
}

Byte fetch_at_addr(Mem_ptr ptr){
    return 0;
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
    
    // Nytt steg: 
    // Checka 2 första
    // Om tvetydighet checka 3 sista
    //
    // Problem jag vet: add imm och ctrl-instr

    Byte prefix = instruction >> 6;
    Byte suffix = instruction & 0b111;
    Byte arithm_operand = instruction & 0b00111000;
    Byte cy;

    switch (prefix) {
        case 0b00: // Mem instructions and 16-bit arithemtic;
            break;
        case 0b01: // ld and halt
            break;
        case 0b10: // 8bit arithmetic Registers and mem
            switch (arithm_operand) {
                case ARI8_ADD:
                case ARI8_ADC:
                    cy = cpu->F & FLAG_MASK_CY & instruction;
                    cpu->A = add_instr(cpu,
                                       cpu->A,
                                       get_register_operand(cpu,suffix),
                                       cy
                                       );
                    break;
                case ARI8_SUB:
                case ARI8_SBC:
                    cy = cpu->F & FLAG_MASK_CY & instruction;
                    cpu->A = sub_instr(cpu,
                                       cpu->A,
                                       get_register_operand(cpu, suffix), 
                                       cy
                                       );
                    break;
                case ARI8_AND:
                    cpu->A = and_instr(cpu,
                                       cpu->A,
                                       get_register_operand(cpu, suffix)
                                       );
                    break;
                case ARI8_XOR:
                    cpu->A = xor_instr(cpu,
                                       cpu->A,
                                       get_register_operand(cpu, suffix)
                                       );
                    break;
                case ARI8_OR:
                    cpu->A = or_instr(cpu,
                                      cpu->A,
                                      get_register_operand(cpu,suffix)
                                      );
                    break;
                case ARI8_CP:
                    cp_instr(cpu,
                            cpu->A,
                            get_register_operand(cpu, suffix)
                            );
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
                switch (arithm_operand) {
                    case ARI8_ADD:
                    case ARI8_ADC:
                        cy = cpu->F & FLAG_MASK_CY & instruction;
                        cpu->A = add_instr(cpu,
                                           cpu->A,
                                           fetch_instruction(++cpu->PC),
                                           cy
                                           );
                        break;
                    case ARI8_SUB:
                    case ARI8_SBC:
                        cy = cpu->F & FLAG_MASK_CY & instruction;
                        cpu->A = sub_instr(cpu,
                                           cpu->A,
                                           fetch_instruction(++cpu->PC), 
                                           cy
                                           );
                    case ARI8_AND:
                        cpu->A = and_instr(cpu,
                                           cpu->A,
                                           fetch_instruction(++cpu->PC) 
                                           );
                    case ARI8_XOR:
                        cpu->A = xor_instr(cpu,
                                           cpu->A,
                                           fetch_instruction(++cpu->PC)
                                           );
                    case ARI8_OR:
                        cpu->A = or_instr(cpu,
                                          cpu->A,
                                          fetch_instruction(++cpu->PC)
                                          );
                    case ARI8_CP:
                        cp_instr(cpu,
                                 cpu->A,
                                 fetch_instruction(++cpu->PC)
                                 );
                    default:
#ifdef DEBUG
                        fprintf(stderr, "Error: Could not decode arithmetics with prefix 0b11!\n");
#endif
                    break;
                }
            } 
            break;
    }
        

}
