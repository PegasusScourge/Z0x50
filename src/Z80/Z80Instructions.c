/*

 _____   ____         ______ ____
/__  /  / __ \ _  __ / ____// __ \
  / /  / / / /| |/_//___ \ / / / /
 / /__/ /_/ /_>  < ____/ // /_/ /
/____/\____//_/|_|/_____/ \____/

Zilog 80 Emulator

Basic interface to the Z80 processor and associated modules.
Can be run as a Sinclair ZX Spectrum or used as a basis for a larger project.

Z80Instructions.c : Instructions that the Z80 CPU can perform

*/

#include "Z80Instructions.h"

const Z80_Instr_t instructions_nullInstruction;

const bool instructions_opcodeEnable[0x100] = {
    /*          0           1           2           3           4           5           6           7           8           9           A           B           C           D           E           F    */
    /* 0 */      false,      false,      false,      false,      false,      false,      false,      false,      false,      false,      false,      false,      false,      false,      false,      false,
    /* 1 */      false,      false,      false,      false,      false,      false,      false,      false,      false,      false,      false,      false,      false,      false,      false,      false,
    /* 2 */      false,      false,      false,      false,      false,      false,      false,      false,      false,      false,      false,      false,      false,      false,      false,      false,
    /* 3 */      false,      false,      false,      false,      false,      false,      false,      false,      false,      false,      false,      false,      false,      false,      false,      false,
    /* 4 */      false,      false,      false,      false,      false,      false,      false,      false,      false,      false,      false,      false,      false,      false,      false,      false,
    /* 5 */      false,      false,      false,      false,      false,      false,      false,      false,      false,      false,      false,      false,      false,      false,      false,      false,
    /* 6 */      false,      false,      false,      false,      false,      false,      false,      false,      false,      false,      false,      false,      false,      false,      false,      false,
    /* 7 */      false,      false,      false,      false,      false,      false,      false,      false,      false,      false,      false,      false,      false,      false,      false,      false,
    /* 8 */      false,      false,      false,      false,      false,      false,      false,      false,      false,      false,      false,      false,      false,      false,      false,      false,
    /* 9 */      false,      false,      false,      false,      false,      false,      false,      false,      false,      false,      false,      false,      false,      false,      false,      false,
    /* A */      false,      false,      false,      false,      false,      false,      false,      false,      false,      false,      false,      false,      false,      false,      false,      false,
    /* B */      false,      false,      false,      false,      false,      false,      false,      false,      false,      false,      false,      false,      false,      false,      false,      false,
    /* C */      false,      false,      false,      false,      false,      false,      false,      false,      false,      false,      false,      false,      false,      false,      false,      false,
    /* D */      false,      false,      false,      false,      false,      false,      false,      false,      false,      false,      false,      false,      false,      false,      false,      false,
    /* E */      false,      false,      false,      false,      false,      false,      false,      false,      false,      false,      false,      false,      false,      false,      false,      false,
    /* F */      false,      false,      false,      false,      false,      false,      false,      false,      false,      false,      false,      false,      false,      false,      false,      false,
};

const char* instructions_humanOpcodeText[0x100] = {
    "NOP",
    "LD BC,NN",
    "LD (BC),A",
    "INC BC",
    "INC B",
    "DEC B",
    "LD B,N",
    "RCLA",
    "EX AF,AF'",
    "ADD HL,BC",
    "LD A,(BC)",
    "DEC BC",
    "INC C",
    "DEC C",
    "LD C,N",
    "RRCA",
    "DJNZ N",
    "LD DE,NN",
    "LD (DE),A",
    "INC DE",
    "INC D",
    "DEC D",
    "LD D,N",
    "RLA",
    "JR N",
    "ADD HL,DE",
    "LD A,(DE)",
    "DEC DE",
    "INC E",
    "DEC E",
    "LD E,N",
    "RRA",
    "JR NZ,N",
    "LD HL,NN",
    "LD (NN),HL",
    "INC HL",
    "INC H",
    "DEC H",
    "LD H,N",
    "DAA",
    "JR Z,N",
    "ADD HL,HL",
    "LD HL,NN",
    "DEC HL",
    "INC L",
    "DEC L",
    "LD L,N",
    "CPL",
};