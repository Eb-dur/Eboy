#!/usr/bin/env python3
from sys import argv

class OperationType():
    LD = "ld"
    PUSH = "push"
    LDHL = "ldhl"
    ADD = "add"
    ADC = "adc"
    SUB = "sub"
    SBC = "sbc"
    AND = "and"
    OR = "or"
    XOR = "xor"
    CP = "cp"
    INC = "inc"
    DEC = "dec"

LINE_NUMBER = 0

def get_LD_register_number(register: str) -> int:
    match register:
        case "a":
            return 0b111
        case "b":
            return 0b000
        case "c":
            return 0b001
        case "d":
            return 0b101
        case "e":
            return 0b011
        case "h":
            return 0b100
        case "l":
            return 0b101
        case _:
            return -1


def get_ld_bytes(command: list[str]) -> list[int]:
    binary = []
    if len(command) != 3:
        print_error("Too few operands for LD operation")
    target = command[1]
    sender = command[2]
    
    if target in "abcdehl":
        if sender in "abcdehl":
            binary += [0b01000000 + (get_LD_register_number(target) << 3) + get_LD_register_number(sender)]
        if sender.startswith(("0x", "0b")) or sender.isdigit():
            base = 10
            if sender.startswith("0x"): base = 16
            if sender.startswith("0b"): base = 2
            binary += [0b00000110 + (get_LD_register_number(target) << 3)]
            binary += [int(sender, base)]
    return binary

def print_error(error_desc: str):
    print("Error:", error_desc, "at line:", LINE_NUMBER)
    return

def split_command(line : str) -> list[str]:
    first_split = line.lower().split()
    return [x.strip(" ,\n") for x in first_split]


def build_binary(asm_file : str, bin_file : str):
    binary = bytes()
    with open(asm_file, "r") as asm:
        while line := asm.readline():
            global LINE_NUMBER
            LINE_NUMBER += 1
            command = split_command(line)
            binary = []
            print(command)

            if command[0] == OperationType.LD:
                binary = get_ld_bytes(command)

            print(binary)

def main():
    if len(argv) < 2:
        print("Error: too few arguments")
        print("  Example: assembler.py -o a.out program.asm")
    

    parsed = 1
    output_filename = "a.out"
    input_filename = None
    while parsed < len(argv):
        curr = argv[parsed]
        if curr == "-o":
            parsed += 1
            output_filename = argv[parsed]
        else:
            input_filename = argv[parsed]
        parsed += 1
    
    if input_filename:
        build_binary(input_filename, output_filename)
    else:
        print("Error: No input file")


if __name__ == "__main__":
    main()
    


