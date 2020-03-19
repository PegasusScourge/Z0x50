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

Z80.h : Z80 CPU component

*/

#include <stdint.h>
#include <stdbool.h>

/* 16bit register definition */
#define DBLREG_UPPER 0
#define DBLREG_LOWER 1
typedef union Z80_DblRegister {
    uint16_t v;
    uint8_t bytes[2];
} Z80_DblRegister;

/********************************************************************

    Z80 Init Functions

********************************************************************/

void Z80_init();
void Z80_initSignals();

/********************************************************************

    Z80 Signal Functions

********************************************************************/

void Z80_signalCLCKListener(bool rising);
void Z80_signalWAITListener(bool rising);

/********************************************************************

    Z80 Fetch Functions

********************************************************************/

void Z80_M1T1Rise();
void Z80_M1T1Fall();
void Z80_M1T2Fall();
void Z80_M1T3Rise();
void Z80_M1T3Fall();
void Z80_M1T4Fall();

/********************************************************************

    Z80 Memory Read Functions

********************************************************************/

void Z80_memReadT1Rise();
void Z80_memReadT1Fall();
void Z80_memReadT2Rise();
void Z80_memReadT2Fall();

/********************************************************************

    Z80 Memory Write Functions

********************************************************************/

void Z80_memWriteT1Rise();
void Z80_memWriteT1Fall();
void Z80_memWriteT2Fall();
void Z80_memWriteT3Fall();

/********************************************************************

    Z80 Operand Read Functions

********************************************************************/

void Z80_prepReadOperands();

/********************************************************************

    Z80 Execute Functions

********************************************************************/

void Z80_executeInstruction();

/********************************************************************

    Z80 Decode Functions

********************************************************************/

void Z80_decode();