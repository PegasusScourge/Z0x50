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

#include <stdbool.h>
#include <stdint.h>

/* Struct defs */
typedef struct Z80_Instr {
    char* string; // A pointer to the instruction string for human readability
    unsigned int type; // Instruction type reference for the Z80 etc

    /* Breakdown switch values of the opcode */
    uint8_t opcode;
    uint8_t x; // Bits 6 and 7
    uint8_t y; // Bits 3, 4, 5
    uint8_t z; // Bits 0, 1, 2 
    uint8_t p; // Bits 4, 5
    uint8_t q; // Bit 3

    uint8_t operand0; // In a two byte, represents the second byte read. Otherwise is first and only byte read.
    uint8_t operand1; // In a two byte, is first byte read. Otherwise not needed.

    uint8_t numOperands; // Number of operands the instruction has after the opcode
    uint8_t numOperandsToRead; // Number of operands left to be read
} Z80_Instr_t;

extern const Z80_Instr_t instructions_nullInstruction;

/* Human-readable strings for the opcodes table */
extern const char* instructions_mainInstructionText[0x100]; // There are 256 opcodes in the primary table
extern const char* instructions_extendedInstructionText[0xC0];
extern const char* instructions_bitInstructionText[0x100];

/* Instruction length and operand number information */
#define PARAMS_LENGTH 0
#define PARAMS_OPERANDN 1
extern const int instructions_mainInstructionParams[0x100][2];