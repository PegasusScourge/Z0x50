/*

 _____   ____         ______ ____
/__  /  / __ \ _  __ / ____// __ \
  / /  / / / /| |/_//___ \ / / / /
 / /__/ /_/ /_>  < ____/ // /_/ /
/____/\____//_/|_|/_____/ \____/

Zilog 80 Emulator

Basic interface to the Z80 processor and associated modules.
Can be run as a Sinclair ZX Spectrum or used as a basis for a larger project.

Z80.c : Z80 CPU component

*/

#include "Z80.h"
#include "Z80FlagAccess.h"
#include "Z80Instructions.h"

#include "Signals.h"

// NOT ACTUALLY NEEDED, JUST FOR TESTING
#include <stdio.h>

/********************************************************************

    VARIABLES / DEFS / STRUCTS

********************************************************************/

/* General Registers */
Z80_DblRegister AF = { 0 };
Z80_DblRegister BC = { 0 };
Z80_DblRegister DE = { 0 };
Z80_DblRegister HL = { 0 };

/* Alternate General Registers */
Z80_DblRegister AFPrime = { 0 };
Z80_DblRegister BCPrime = { 0 };
Z80_DblRegister DEPrime = { 0 };
Z80_DblRegister HLPrime = { 0 };

/* Special Registers */
Z80_DblRegister IVMR = { 0 };
Z80_DblRegister IX = { 0 };
Z80_DblRegister IY = { 0 };
Z80_DblRegister SP = { 0 };
Z80_DblRegister PC = { 0 };

/* Z80 Instruction */
Z80_Instr_t cInstr; // The current instruction we are processing

/* Z80 Internal State */
enum Z80InternalStateEnum { Z80State_Fetch, Z80State_Decode, Z80State_Execute };
uint8_t Z80InternalState = Z80State_Fetch;

/********************************************************************

    Z80FlagAccess.h Functions 

********************************************************************/

void Z80Flags_setFlag(int f) {
    AF.bytes[Z80DBLREG_LOWER] = AF.bytes[Z80DBLREG_LOWER] | (0b00000001 << f);
}
void Z80Flags_clearFlag(int f) {
    AF.bytes[Z80DBLREG_LOWER] = AF.bytes[Z80DBLREG_LOWER] & ~(0b00000001 << f);
}
bool Z80Flags_readFlag(int f) {
    return (AF.bytes[Z80DBLREG_LOWER] & (0b00000001 << f)) == 1;
}

/********************************************************************

    Z80 Init Functions

********************************************************************/

// Init the Z80 chip
void Z80_init() {
    // Firstly connect the signals
    Z80_initSignals();
}

void Z80_initSignals() {
    // Add the clock listener
    signals_addListener(&signal_CLCK, &Z80_signalCLCKListener);
}

/********************************************************************

    Z80 Signal Functions

********************************************************************/

void Z80_signalCLCKListener(bool rising) {
    if (rising) {
        // What we do on a rising edge of the clock
    }
    else {
        // What we do on the falling edge of the clock
    }
}

/********************************************************************

    Z80 Fetch Functions

********************************************************************/

/* 
Activates on the rising edge of T1.
Place PC on the address bus, set MREQ, RD and RFSH high
Set M1 low
*/
void Z80_fetchT1Rise() {
    // Set the necessary signals high
    signals_raiseSignal(&signal_MREQ);
    signals_raiseSignal(&signal_RD);
    signals_raiseSignal(&signal_RFSH);

    // Set the M1 signal low
    signals_dropSignal(&signal_M1);

    // Place PC on the bus
    signal_addressBus = PC.v;
}