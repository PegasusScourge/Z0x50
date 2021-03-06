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
#define REG_UPPER(x) (x >> 8)
#define REG_LOWER(x) (x & 0xF)

/* Z80 Internal State Variables */
extern int microcodeState;

/* State */
enum Z80InternalStateEnum { Z80State_Fetch, Z80State_Decode, Z80State_Execute, Z80State_Failure };
int Z80_state();

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

#define Z80_fetchCycleStart Z80_M1T1Rise
void Z80_M1T1Rise();
void Z80_M1T1Fall();
void Z80_M1T2Fall();
void Z80_M1T3Rise();
void Z80_M1T3Fall();
void Z80_M1T4Fall();

/********************************************************************

    Z80 Memory Read Functions
    Reads to memory location identified by 'addressBusLatch' and places value where 'internalDataBus' points

********************************************************************/

#define Z80_memReadCycleStart Z80_memReadT1Rise
void Z80_memReadT1Rise();
void Z80_memReadT1Fall();
void Z80_memReadT2Rise();
void Z80_memReadT2Fall();

/********************************************************************

    Z80 Memory Write Functions
    Writes to memory location identified by 'addressBusLatch' and retrieves value pointed to by 'internalDataBus'

********************************************************************/

#define Z80_memWriteCycleStart Z80_memWriteT1Rise
void Z80_memWriteT1Rise();
void Z80_memWriteT1Fall();
void Z80_memWriteT2Fall();
void Z80_memWriteT3Fall();

/********************************************************************

    Z80 Operand Read Functions

********************************************************************/

void Z80_prepReadOperands();

/********************************************************************

    Z80 Prefix Handling Functions

********************************************************************/

void Z80_prepPrefixedInstructionRead();
void Z80_finalisePrefixedInstructionRead();

/********************************************************************

    Z80 Execute Functions

********************************************************************/

void Z80_executeInstruction();

/********************************************************************

    Z80 Decode Functions

********************************************************************/

void Z80_decode();
void Z80_decodeBranchDecision();