#!/usr/bin/env python3
from sys import argv


def build_binary(asm_file : str, bin_file : str):
    binary = bytes()
    with open(asm_file, "r") as asm:
        while line := asm.readline():
            pass

def main():
    if len(argv) < 2:
        print("Error, too few arguments")
        print("  Example: assembler.py -o a.out program.asm")
    

    parsed = 1
    output_filename = "a.out"
    while parsed < len(argv):
        curr = argv[parsed]
        if curr == "-o":
            parsed += 1
            output_filename = argv[parsed]
        else:
            inut_filename = argv[parsed]
        parsed += 1
    


