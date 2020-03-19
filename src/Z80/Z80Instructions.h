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

#define PREFIX_BITS 0xCB
#define PREFIX_EXX 0xED
#define PREFIX_IX 0xDD
#define PREFIX_IY 0xFD

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

extern const Z80_Instr_t instructions_NULLInstr;

/* Instruction execution return values. Less than 0 is a failure */
#define INSTR_EXEC_FAILED -1
#define INSTR_EXEC_NOTIMPL -2
#define INSTR_EXEC_SUCCESS 0

/* Instruction pointer tables */
int instructions_NInstr();
extern const int (*instructions_mainInstructionFuncs[0x100])(); // There are 256 opcodes in the primary table
extern const int (*instructions_extendedInstructionFuncs[0x100])(); // PREFIX 0xED
extern const int (*instructions_bitInstructionFuncs[0x100])(); // PREFIX 0xCB
extern const int (*instructions_IXInstructionFuncs[0x100])(); // PREFIX 0xDD
extern const int (*instructions_IXBitInstructionFuncs[0x100])(); // PREFIX 0xDDCB
extern const int (*instructions_IYInstructionFuncs[0x100])(); // PREFIX 0xFD
extern const int (*instructions_IYBitInstructionFuncs[0x100])(); // PREFIX 0xFDCB

/* Human-readable strings for the opcodes table */
extern const char* instructions_mainInstructionText[0x100]; // There are 256 opcodes in the primary table
extern const char* instructions_extendedInstructionText[0x100]; // PREFIX 0xED
extern const char* instructions_bitInstructionText[0x100]; // PREFIX 0xCB
extern const char* instructions_IXInstructionText[0x100]; // PREFIX 0xDD
extern const char* instructions_IXBitInstructionText[0x100]; // PREFIX 0xDDCB
extern const char* instructions_IYInstructionText[0x100]; // PREFIX 0xFD
extern const char* instructions_IYBitInstructionText[0x100]; // PREFIX 0xFDCB

/* Instruction length and operand number information. The table contains the length of the instruction in bytes. If > 1, num operands = (length in bytes - 1). If == -1, marks a prefix */
extern const char instructions_mainInstructionParams[0x100]; // There are 256 opcodes in the primary table
extern const char instructions_extendedInstructionParams[0x100]; // PREFIX 0xED
extern const char instructions_bitInstructionParams[0x100]; // PREFIX 0xCB
extern const char instructions_IXInstructionParams[0x100]; // PREFIX 0xDD
extern const char instructions_IXBitInstructionParams[0x100]; // PREFIX 0xDDCB
extern const char instructions_IYInstructionParams[0x100]; // PREFIX 0xFD
extern const char instructions_IYBitInstructionParams[0x100]; // PREFIX 0xFDCB