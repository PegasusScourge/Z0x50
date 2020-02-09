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

Z80InstrTypes.h : Contains and enum list of every Z80 instruction type

*/

/*

b = bit
B = byte

*/

enum Z80InstrTypes {

    /* Load and exchange types */
    Z80Instr_LD_8b,
    Z80Instr_LD_16b,
    Z80Instr_PUSH_16b,
    Z80Instr_POP_16b,
    Z80Instr_EX, // Exchange
    Z80Instr_EXX, // Exchange extra?

    /* Block transfer and search types */
    Z80Instr_BLCK_TRANS,
    Z80Instr_BLCK_SEARCH,

    /* Arithmetic and logic types */
    Z80Instr_ADD_8b,
    Z80Instr_ADC_8b,
    Z80Instr_SUB_8b,
    Z80Instr_SBC_8b,
    Z80Instr_AND_8b,
    Z80Instr_XOR_8b,
    Z80Instr_OR_8b,
    Z80Instr_CP_8b, // Compare
    Z80Instr_INC_8b,
    Z80Instr_DEC_8b,

    Z80Instr_DAA, // Decimal Adjust Accumulator
    Z80Instr_CPL, // Complement Accumulator
    Z80Instr_NEG, // Negate Accumulator (2's complement)
    Z80Instr_CCF, // Complement Carry Flag
    Z80Instr_SCF, // Set Carry Flag

    Z80Instr_ADD_16b,
    Z80Instr_ADC_16b,
    Z80Instr_SUB_16b,
    Z80Instr_SBC_16b,
    Z80Instr_INC_16b,
    Z80Instr_DEC_16b,

    /* Rotate and shift types */
    Z80Instr_RLC, // Rotate Left Circular
    Z80Instr_RRC, // Rotate Right Circular
    Z80Instr_RL,  // Rotate Left
    Z80Instr_RR,  // Rotate Right
    Z80Instr_SLA, // Shift Left Arithmetic
    Z80Instr_SRA, // Shift Right Artihmetic
    Z80Instr_SRL, // Shift Right Logical
    Z80Instr_RDL, // Rotate Digit Left
    Z80Instr_RDR, // Rotate Digit Right

    /* Bit manipulation (set, reset, test) types */
    Z80Instr_TST_BIT, // Test bit
    Z80Instr_RES_BIT, // Rest bit
    Z80Instr_SET_BIT, // Set bit

    /* Jump, call and return types */
    Z80Instr_JP_IMMEXT, // Jump, immediate extended
    Z80Instr_JR,        // Jump, relative
    Z80Instr_JP_REGIND, // Jump, register indirect
    Z80Instr_CALL,      // Call, immediate extended
    Z80Instr_DJNZ,      // Decrement B, jump if not zero
    Z80Instr_RE,        // Return (Register indirect)
    Z80Instr_RETI,      // Return from interrupt (Register indirect)
    Z80Instr_RETN,      // Return from non-maskable interrupt (Register indirect)

    Z80Instr_RST0,  // Restart 0h
    Z80Instr_RST8,  // Restart 8h
    Z80Instr_RST16, // Restart 16h
    Z80Instr_RST24, // Restart 24h
    Z80Instr_RST32, // Restart 32h
    Z80Instr_RST40, // Restart 40h
    Z80Instr_RST48, // Restart 48h
    Z80Instr_RST56, // Restart 56h

    /* Input and output types */
    Z80Instr_IN,        // Input
    Z80Instr_INI,       // Input, inc HL and dec B
    Z80Instr_INIR,      // Input, inc HL, dec B and repeat if B != 0
    Z80Instr_IND,       // Input and inc, dec HL and dec B
    Z80Instr_INDR,      // Input, dec HL, dec B and repeat if B != 0

    // Output types here. They are confusing.

    /* Basic cpu control types */
    Z80Instr_NOP,
    Z80Instr_HALT,
    Z80Instr_DI,  // Disable interrupt
    Z80Instr_EI,  // Enable interrupt
    Z80Instr_IM0, // Set interrupt mode 0
    Z80Instr_IM1, // Set interrupt mode 1
    Z80Instr_Im2, // Set interrupt mode 2
};