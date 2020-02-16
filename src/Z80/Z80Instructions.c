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

const char* instructions_humanOpcodeText[0x100] = {
/*          0           1           2           3           4           5           6           7           8           9           A           B           C           D           E           F    */
/* 0 */     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",
/* 1 */     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",
/* 2 */     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",
/* 3 */     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",
/* 4 */     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",
/* 5 */     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",
/* 6 */     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",
/* 7 */     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",
/* 8 */     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",
/* 9 */     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",
/* A */     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",
/* B */     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",
/* C */     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",
/* D */     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",
/* E */     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",
/* F */     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",     "NOP ",
};

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