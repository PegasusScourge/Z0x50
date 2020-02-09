#pragma once

/*

 _____   ____         ______ ____
/__  /  / __ \ _  __ / ____// __ \
  / /  / / / /| |/_//___ \ / / / /
 / /__/ /_/ /_>  < ____/ // /_/ /
/____/\____//_/|_|/_____/ \____/

Zilog 80 Emulator

Basic interface to the Z80 processor and associated modules.
Can be run as a Sinclair ZX Spectrum or used as a basis for a larger project.

Z80Instructions.h : Instructions that the Z80 CPU can perform

*/

/* Struct defs */
typedef struct Z80_Instr {
    char* string; // A pointer to the instruction string for human readability
    unsigned int type; // Instruction type reference for the Z80 etc

    /* Breakdown switch values of the opcode */
    unsigned int x; // Bits 6 and 7
    unsigned int y; // Bits 3, 4, 5
    unsigned int z; // Bits 0, 1, 2 
    unsigned int p; // Bits 4, 5
    unsigned int q; // Bit 3

    unsigned int numOperands; // Number of operands the instruction has after the opcode
} Z80_Instr_t;