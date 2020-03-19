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

#include "../Signals.h"
#include "../SysIO/Log.h"

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

/* Z80 Internal State Variables */
enum Z80InternalStateEnum { Z80State_Fetch, Z80State_Decode, Z80State_Execute };
uint8_t internalState = Z80State_Fetch; // The broad state we are in

// uint8_t TNumber = 0; // Each T represents a clock cycle within a machine cycle
// uint8_t MNumber = 0; // Each M represents a machine cycle in an instruction cycle.

uint16_t addressBusLatch = 0; // This value is pushed to the address bus during the start of memory read and write cycles. Needs to be preloaded
uint8_t* internalDataBus = NULL; // This is a pointer to where the data we read/write goes to/comes from when doing memory read and write cycles

bool wait = false; // If true, the CPU stops at its current step and doesn't advance

/* Clock timing pointers */
void (*onNextRisingCLCK)() = NULL; // If non-NULL, called on next rising clock edge. Set to NULL before call
void (*onNextFallingCLCK)() = NULL; // If non-NULL, called on next falling clock edge. Set to NULL before call

/* Next M cycle return pointer */
void (*onFinishMCycle)() = NULL;

/********************************************************************

    Z80FlagAccess.h Functions 

********************************************************************/

void Z80Flags_setFlag(int f) {
    AF.bytes[DBLREG_LOWER] = AF.bytes[DBLREG_LOWER] | (0b00000001 << f);
}
void Z80Flags_clearFlag(int f) {
    AF.bytes[DBLREG_LOWER] = AF.bytes[DBLREG_LOWER] & ~(0b00000001 << f);
}
bool Z80Flags_readFlag(int f) {
    return (AF.bytes[DBLREG_LOWER] & (0b00000001 << f)) == 1;
}

/********************************************************************

    Z80 Init Functions

********************************************************************/

// Init the Z80 chip
void Z80_init() {
    // Zeroth order: Set to a fetch state and reset T and M numbers
    // TNumber = 0;
    // MNumber = 0;
    internalState = Z80State_Fetch;

    // Firstly connect the signals
    Z80_initSignals();

    // Lastly, connect the first function on the rising clock edge (the fetch T1Rising function)
    onNextRisingCLCK = &Z80_M1T1Rise;
}

void Z80_initSignals() {
    // Add the clock listener
    signals_addListener(&signal_CLCK, &Z80_signalCLCKListener);
    // Add the wait listener
    signals_addListener(&signal_WAIT, &Z80_signalWAITListener);
}

/********************************************************************

    Z80 Signal Functions

********************************************************************/

void Z80_signalCLCKListener(bool rising) {
    // If waiting, just ignore the CLCK for now
    if (wait)
        return;

    // Complete a clock tick
    void (*func)() = NULL;
    if (rising) {
        // Check if we have a rising clck function to do
        if (onNextRisingCLCK != NULL) {
            func = onNextRisingCLCK; // Transfer the pointer
            onNextRisingCLCK = NULL; // NULL out the pointer
        }
    }
    else {
        if (onNextFallingCLCK != NULL) {
            func = onNextFallingCLCK; // Transfer the pointer
            onNextFallingCLCK = NULL; // NULL out the pointer
        }
    }

    if (func != NULL) {
        // We have a non-NULL function, call it
        func();
    }
    else if (rising && onFinishMCycle != NULL) {
        // We have no function to complete, do we have a return function?
        func = onFinishMCycle; // Transfer the pointer
        onFinishMCycle = NULL; // NULL out the pointer
        func();
    }
}

void Z80_signalWAITListener(bool rising) {
    wait = rising; // Just directly set the wait variable
}


/********************************************************************

    Z80 Fetch Functions

********************************************************************/

/* 
Activates on the rising edge of M1T1.
Place PC on the address bus, set MREQ, RD and RFSH high, set M1 low
This allows the external circuitry to prepare to recieve the memory location we want to read from
*/
#define Z80_fetchCycleStart Z80_M1T1Rise
void Z80_M1T1Rise() {
    if (internalState != Z80State_Fetch) {
        // We require to match the fetch state!
        // Generate an error here?
        formattedLog(stdlog, LOGTYPE_ERROR, "Fetch start failed: not in fetch state\n");
        return; // For now just return. This will halt the processor
    }
    printf("[FETCH]\n");

    // Set the necessary signals high
    signals_raiseSignal(&signal_MREQ);
    signals_raiseSignal(&signal_RD);
    signals_raiseSignal(&signal_RFSH);

    // Set the M1 signal low
    signals_dropSignal(&signal_M1);

    // Place PC on the bus
    signal_addressBus = PC.v;

    // Setup the next function to be called, which is a falling edge. RD and MREQ disable
    onNextFallingCLCK = &Z80_M1T1Fall;
}

/*
Activates on the falling edge of M1T1.
Set MREQ and RD low
This informs the external circuitry it is time to put the data on the bus
*/
void Z80_M1T1Fall() {
    if (internalState != Z80State_Fetch) {
        // We require to match the fetch state!
        // Generate an error here?
        return; // For now just return. This will halt the processor
    }

    // Set the necessary signals low
    signals_dropSignal(&signal_MREQ);
    signals_dropSignal(&signal_RD);

    // Setup the next function to be called, which is on the falling edge of the next clock cycle. Data IN
    onNextFallingCLCK = &Z80_M1T2Fall;
}

/*
Activates on the falling edge of M1T2.
Takes in the data on the bus and parses it to generate an opcode
*/
void Z80_M1T2Fall() {
    if (internalState != Z80State_Fetch) {
        // We require to match the fetch state!
        // Generate an error here?
        return; // For now just return. This will halt the processor
    }

    // No signals to manipulate
    // Take in a value from the data bus and store in cInstr
    cInstr = instructions_NULLInstr; // Clear the instruction to null
    cInstr.opcode = signal_dataBus; // HERE we take in the value

    // The next thing to do is perform a rudimentary decode on the next rising edge (M1T3). At the same time various signals toggle
    onNextRisingCLCK = &Z80_M1T3Rise;
}

/*
Activates on the rising edge of M1T3.
Address bus to refresh addr, MREQ + RD + M1 rise, RFSH fall
Does a rudimentary decode of the opcode to see if we need to take in successive bytes.
*/
void Z80_M1T3Rise() {
    if (internalState != Z80State_Fetch) {
        // We require to match the fetch state!
        // Generate an error here?
        return; // For now just return. This will halt the processor
    }

    // Raise appropriate signals
    signals_raiseSignal(&signal_MREQ);
    signals_raiseSignal(&signal_RD);
    signals_raiseSignal(&signal_M1);

    // Drop apppropriate signals
    signals_dropSignal(&signal_RFSH); // The refresh signal does technically need a refresh address, but we will just use the PC for now as it's still on the bus

    // Do a decode
    internalState = Z80State_Decode;
    Z80_decode();

    // Setup the next function. The falling edge of M1T3 requires a MREQ fall and is where we decide the course of action: execution or memory read/write
    onNextFallingCLCK = &Z80_M1T3Fall;
}

/*
Activates on the falling edge of M1T3.
Drops MREQ.
Decides if we can execute on the next rising clock, or if we need to get a memory read
*/
void Z80_M1T3Fall() {
    if (internalState != Z80State_Decode) {
        // We require to match the decode state!
        // Generate an error here?
        return; // For now just return. This will halt the processor
    }

    // If we have no operands, we can just do an execution
    if (cInstr.numOperands == 0) {
        onNextRisingCLCK = &Z80_executeInstruction;
    }
    else {
        // We need to read operands from memory to continue the execution, so we shall set up the pathway
        onNextRisingCLCK = &Z80_prepReadOperands; // this is just a dummy function that leads into the real pathway after M1 has elapsed
    }

    // On the next falling edge we need to raise MREQ regardless of what we do here
    onNextFallingCLCK = &Z80_M1T4Fall;
}

/*
Activates on the falling edge of M1T4.
Raises MREQ.
*/
void Z80_M1T4Fall() {
    signals_raiseSignal(&signal_MREQ);

    // This is the end of the M1 Mcycle path, we decided our direction in Z80_M1T3Fall() so no function pointers set here
}

/********************************************************************

    Z80 Memory Read Functions

********************************************************************/

/*
Activates on the rising edge of MREADT1
Puts addressBusLatch on the address bus
*/
#define Z80_memReadCycleStart Z80_memReadT1Rise
void Z80_memReadT1Rise() {
    printf("[MEM READ]\n");
    // We are doing a memory read cycle, so irrespective of state we continue

    // Put the address on the bus
    signal_addressBus = addressBusLatch;

    // Set up the next step
    onNextFallingCLCK = &Z80_memReadT1Fall;
}

/*
Activates on the falling edge of MREADT1
Puts MREQ and RD low
*/
void Z80_memReadT1Fall() {
    // Do signal dropping
    signals_dropSignal(&signal_MREQ);
    signals_dropSignal(&signal_RD);

    // On the next rising edge try and sample the data in (likely to be WAIT)
    onNextRisingCLCK = &Z80_memReadT2Rise;
}

/*
Activates on the falling edge of MREADT2
Reads the data from the data bus to the pointed location
*/
void Z80_memReadT2Rise() {
    // Sample the data from the data bus and write it to the internalDataBus pointer
    if (internalDataBus) {
        // We have a valid place to write to perhaps?
        *internalDataBus = signal_dataBus;
    }

    // Set the final part of the memRead cycle
    onNextFallingCLCK = &Z80_memReadT2Fall;
}

/*
Activates on the falling edge of MREADT2
Puts MREQ and RD high
*/
void Z80_memReadT2Fall() {
    // Do signal raising
    signals_raiseSignal(&signal_MREQ);
    signals_raiseSignal(&signal_RD);

    // This is the end of the memRead cycle, let the return address take hold now
}

/********************************************************************

    Z80 Memory Write Functions

********************************************************************/

/*
Activates on the rising edge of MWRITET1
Puts addressBusLatch on the address bus
*/
#define Z80_memWriteCycleStart Z80_memWriteT1Rise
void Z80_memWriteT1Rise() {
    printf("[MEM WRITE]\n");
    // We are doing a memory write cycle, so irrespective of state we continue

    // Put the address on the bus
    signal_addressBus = addressBusLatch;

    // Set up the next step
    onNextFallingCLCK = &Z80_memWriteT1Fall;
}

/*
Activates on the falling edge of MWRITET1
Puts MREQ low and the data on the bus
*/
void Z80_memWriteT1Fall() {
    // Do signal dropping
    signals_dropSignal(&signal_MREQ);

    // Put data on bus
    signal_dataBus = *internalDataBus;

    // On the next falling edge set WR low
    onNextFallingCLCK = &Z80_memWriteT2Fall;
}

/*
Activates on the falling edge of MWRITET2
Puts WR low
*/
void Z80_memWriteT2Fall() {
    // Put WR low
    signals_dropSignal(&signal_WR);

    // On the next falling edge we put everything back
    onNextFallingCLCK = &Z80_memWriteT3Fall;
}

/*
Activates on the falling edge of MWRITET3
Puts WR, MREQ high
*/
void Z80_memWriteT3Fall() {
    // Put signals high
    signals_raiseSignal(&signal_MREQ);
    signals_raiseSignal(&signal_WR);

    // This is the end of the memory write so we let the return ability work
}

/********************************************************************

    Z80 Operand Read Functions

********************************************************************/

/*
Activates on the rising edge of M1T4
Sets up the operand read
*/
void Z80_prepReadOperands() {
    printf("[OPERANDS: %i]\n", cInstr.numOperandsToRead);

    // The next rising edge needs to be a memory read cycle
    onNextRisingCLCK = &Z80_memReadCycleStart;

    // The return location is either the execute instruction function or a second operand read
    if (cInstr.numOperandsToRead == 1) {
        onFinishMCycle = &Z80_executeInstruction;
        // Point to operand0
        internalDataBus = &cInstr.operand0;
    }
    else if (cInstr.numOperandsToRead == 2) {
        onFinishMCycle = &Z80_prepReadOperands;
        // Point to operand1
        internalDataBus = &cInstr.operand1;
    }
    else {
        // We have no more operands to read, move to execution. Somehow we ended up here? Maybe detected 3 operands?????
        onFinishMCycle = &Z80_executeInstruction;
        onNextRisingCLCK = NULL; // we don't need to read anything
    }

    // Decrement operands to read
    cInstr.numOperandsToRead--;
}

/********************************************************************

    Z80 Execute Functions

********************************************************************/

/*
Activates on the rising edge of M1T4 or when an execution can be completed in a memory cycle after fetching operands
Executes the opcode and decides if we need a memory write cycle after
*/
void Z80_executeInstruction() {
    // Set to execute state
    internalState = Z80State_Execute;

    printf("[EXECUTE]\n");
    // Check the processor state: if we are in a decode mode, defer execution to the decode module


    // Excution finished for now, set to the next M1 cycle
    onFinishMCycle = &Z80_fetchCycleStart;
    // Set back to fetch state
    internalState = Z80State_Fetch;
}

/********************************************************************

    Z80 Decode Functions

********************************************************************/

/*
Activates on the rising edge of M1T3 as part of the M1T3Rise fetch function
Decodes the opcode
*/
void Z80_decode() {
    // We have a cInstr struct with a .opcode value
    // We take this and split the bit fields for easier decoding
    cInstr.z = (cInstr.opcode & 0b11000000) >> 6;
    cInstr.y = (cInstr.opcode & 0b00111000) >> 3;
    cInstr.z = (cInstr.opcode & 0b00000111);
    cInstr.p = cInstr.y >> 1;
    cInstr.q = cInstr.y % 2;

    // Now we retrieve the human-readable pointer
    cInstr.string = instructions_mainInstructionText[cInstr.opcode];
    formattedLog(debuglog, LOGTYPE_DEBUG, "[DECODE]\n");
    formattedLog(debuglog, LOGTYPE_DEBUG, "opcode %s (pc: %04X, %02X)\n", cInstr.string, PC.v, cInstr.opcode);
}