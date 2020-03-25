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
#include "Z80Flags.h"
#include "Z80Instructions.h"

#include "../Signals.h"
#include "../SysIO/Log.h"
#include "../Video/VideoAdaptor.h"

#define debugFuncTell formattedLog(debuglog, LOGTYPE_DEBUG, "Microstate Exec %s\n", __FUNCTION__)

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
int internalState = Z80State_Fetch; // The broad state we are in
bool wait = false; // If true, the CPU stops at its current step and doesn't advance
int microcodeState = 0; // Used by instruction functions to control their internal affairs
int Z80_state() { return internalState; }

/* Data Movement Variables */
uint16_t addressBusLatch = 0; // This value is pushed to the address bus during the start of memory read and write cycles. Needs to be preloaded
uint8_t* internalDataBus = NULL; // This is a pointer to where the data we read/write goes to/comes from when doing memory read and write cycles

/* Clock timing pointers */
void (*onNextRisingCLCK)() = NULL; // If non-NULL, called on next rising clock edge. Set to NULL before call
void (*onNextFallingCLCK)() = NULL; // If non-NULL, called on next falling clock edge. Set to NULL before call

/* Next M cycle return pointer */
void (*onFinishMCycle)() = NULL;

/********************************************************************

    Z80FlagAccess.h Functions 

********************************************************************/

void Z80Flags_setFlag(int f) {
    AF.bytes[LOWER] = AF.bytes[LOWER] | (0b00000001 << f);
}
void Z80Flags_clearFlag(int f) {
    AF.bytes[LOWER] = AF.bytes[LOWER] & ~(0b00000001 << f);
}
bool Z80Flags_readFlag(int f) {
    return (AF.bytes[LOWER] & (0b00000001 << f)) == 1;
}

/********************************************************************

    Z80 Init Functions

********************************************************************/

// Init the Z80 chip
void Z80_init() {
    internalState = Z80State_Fetch;

    // Firstly connect the signals
    Z80_initSignals();

    // Lastly, connect the first function on the rising clock edge (the fetch T1Rising function)
    onNextRisingCLCK = &Z80_fetchCycleStart;
}

void Z80_initSignals() {
    // Add the clock listener
    signals_addListener(&signal_CLCK, &Z80_signalCLCKListener);
    // Add the wait listener
    signals_addListener(&signal_WAIT, &Z80_signalWAITListener);

    // Connect the video adaptor hooks
    displayInfo.AReg = &AF.bytes[UPPER];
    displayInfo.FReg = &AF.bytes[LOWER];
    displayInfo.BCReg = &BC.v;
    displayInfo.DEReg = &DE.v;
    displayInfo.HLReg = &HL.v;
    displayInfo.IX = &IX.v;
    displayInfo.IY = &IY.v;
    displayInfo.SP = &SP.v;
    displayInfo.PC = &PC.v;
    displayInfo.cInstr = &cInstr;
}

/********************************************************************

    Z80 Signal Functions

********************************************************************/

void Z80_signalCLCKListener(bool rising) {
    // If waiting, just ignore the CLCK for now
    if (wait || internalState == Z80State_Failure)
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
    else if (rising && onFinishMCycle == NULL && onNextFallingCLCK == NULL) {
        // We have no function to complete but we need to! Fail the processor here
        formattedLog(stdlog, LOGTYPE_ERROR, "Processor microstate execution has failed: no function to execute!\n");
        wait = true; // Force a wait condition
        internalState = Z80State_Failure;
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
void Z80_M1T1Rise() {
    if (internalState != Z80State_Fetch) {
        // We require to match the fetch state!
        // Generate an error here?
        formattedLog(stdlog, LOGTYPE_ERROR, "Fetch start failed: not in fetch state\n");
        internalState = Z80State_Failure;
        return; // This will halt the processor
    }

    // Set the necessary signals high
    signals_raiseSignal(&signal_MREQ);
    signals_raiseSignal(&signal_RD);
    signals_raiseSignal(&signal_RFSH);

    // Set the M1 signal low
    signals_dropSignal(&signal_M1);

    // Place PC on the bus
    signal_addressBus = PC.v;

    // Update our internal addressLatch
    addressBusLatch = PC.v;

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

    // Decide
    Z80_decodeBranchDecision();

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
    Reads to memory location identified by 'addressBusLatch' and places value where 'internalDataBus' points

********************************************************************/

/*
Activates on the rising edge of MREADT1
Puts addressBusLatch on the address bus
*/
void Z80_memReadT1Rise() {
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
    Writes to memory location identified by 'addressBusLatch' and retrieves value pointed to by 'internalDataBus'

********************************************************************/

/*
Activates on the rising edge of MWRITET1
Puts addressBusLatch on the address bus
*/
void Z80_memWriteT1Rise() {

    // printf("[MEM WRITE]\n");
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
    // printf("[OPERANDS: %i]\n", cInstr.numOperandsToRead);

    // The next rising edge needs to be a memory read cycle
    onNextRisingCLCK = &Z80_memReadCycleStart;

    // The return location is either the execute instruction function or a second operand read
    if (cInstr.numOperandsToRead == 1) {
        onFinishMCycle = &Z80_executeInstruction;
        // Point to operand0
        internalDataBus = &cInstr.operand0;
        addressBusLatch++;
    }
    else if (cInstr.numOperandsToRead == 2) {
        onFinishMCycle = &Z80_prepReadOperands;
        // Point to operand1
        internalDataBus = &cInstr.operand1;
        addressBusLatch++;
    }
    else {
        // We have no more operands to read, move to execution. Somehow we ended up here? Maybe detected 3 operands?????
        formattedLog(stdlog, LOGTYPE_WARN, __FUNCTION__" attempted operand read, INVALID: cInstr.numOperandsToRead = %i. Performing execution of instruction anyway\n", cInstr.numOperandsToRead);
        onFinishMCycle = &Z80_executeInstruction;
        onNextRisingCLCK = NULL; // we don't need to read anything, so cancel the proceedure here
    }

    // Decrement operands to read
    cInstr.numOperandsToRead--;
}

/********************************************************************

    Z80 Prefix Handling Functions

********************************************************************/

/*
Similar to if we had encountered an operand to read, we have encountered a prefix and thus need to get the instruction in the next position in memory
*/
void Z80_prepPrefixedInstructionRead() {
    // We must transfer the "opcode" into our prefix buffer
    cInstr.prefix = (cInstr.prefix << 8) | cInstr.opcode;

    // Now we must set up the memory read cycle to read into the opcode location by pointing the internalDataBus at it
    addressBusLatch++;
    internalDataBus = &cInstr.opcode;
    // Now set up the return function
    onFinishMCycle = &Z80_finalisePrefixedInstructionRead;
    
    // The next rising edge needs to be a memory read cycle
    onNextRisingCLCK = &Z80_memReadCycleStart;
}

/*
This function is analagous to Z80_M1T3Rise and Z80_M1T3Fall, where we decide what to do. 
This takes place on a rising edge though, so we must in actuality EXECUTE the next function, not point to it like in others. <----- we don't do this for now, do we need to?
*/
void Z80_finalisePrefixedInstructionRead() {
    internalState = Z80State_Decode;
    // formattedLog(debuglog, LOGTYPE_DEBUG, "[PREFIXED DECODE]\n");
    // Perform the decode
    Z80_decode();

    // Perform the branch decision
    Z80_decodeBranchDecision();
}

/********************************************************************

    Z80 Execute Functions

********************************************************************/

/*
Activates on the rising edge of M1T4 or when an execution can be completed in a memory cycle after fetching operands
Executes the opcode and decides if we need a memory write cycle after
*/
void Z80_executeInstruction() {
    // Increment PC
    PC.v += cInstr.instrByteLen;
    
    // Set to execute state
    internalState = Z80State_Execute;

    // printf("[EXECUTE]\n");
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
    switch (cInstr.prefix) {
    case PREFIX_BITS:
        cInstr.string = instructions_bitInstructionText[cInstr.opcode];
        cInstr.instrByteLen = instructions_bitInstructionParams[cInstr.opcode];
        cInstr.numOperands = cInstr.instrByteLen > 2 ? cInstr.instrByteLen - 2 : 0;
        break;
    case PREFIX_EXX:
        cInstr.string = instructions_extendedInstructionText[cInstr.opcode];
        cInstr.instrByteLen = instructions_extendedInstructionParams[cInstr.opcode];
        cInstr.numOperands = cInstr.instrByteLen > 2 ? cInstr.instrByteLen - 2 : 0;
        break;
    case PREFIX_IX:
        cInstr.string = instructions_IXInstructionText[cInstr.opcode];
        cInstr.instrByteLen = instructions_IXInstructionParams[cInstr.opcode];
        cInstr.numOperands = cInstr.instrByteLen > 2 ? cInstr.instrByteLen - 2 : 0;
        break;
    case PREFIX_IY:
        cInstr.string = instructions_IYInstructionText[cInstr.opcode];
        cInstr.instrByteLen = instructions_IYInstructionParams[cInstr.opcode];
        cInstr.numOperands = cInstr.instrByteLen > 2 ? cInstr.instrByteLen - 2 : 0;
        break;
    case PREFIX_IX_BITS:
        cInstr.string = instructions_IXBitInstructionText[cInstr.opcode];
        cInstr.instrByteLen = instructions_IXBitInstructionParams[cInstr.opcode];
        cInstr.numOperands = cInstr.instrByteLen > 3 ? cInstr.instrByteLen - 3 : 0;
        break;
    case PREFIX_IY_BITS:
        cInstr.string = instructions_IYBitInstructionText[cInstr.opcode];
        cInstr.instrByteLen = instructions_IYBitInstructionParams[cInstr.opcode];
        cInstr.numOperands = cInstr.instrByteLen > 3 ? cInstr.instrByteLen - 3 : 0;
        break;
    default:
        cInstr.string = instructions_mainInstructionText[cInstr.opcode];
        cInstr.instrByteLen = instructions_mainInstructionParams[cInstr.opcode];
        cInstr.numOperands = cInstr.instrByteLen > 1 ? cInstr.instrByteLen - 1 : 0;
        break;
    }

    // Determine if the opcode is a prefix
    switch (cInstr.opcode) {
    case PREFIX_BITS:
    case PREFIX_EXX:
    case PREFIX_IX:
    case PREFIX_IY:
        cInstr.detectedPrefix = true;
        // formattedLog(debuglog, LOGTYPE_DEBUG, "prefix %s (pc: %04X, %04X %02X)\n", cInstr.string, PC.v, cInstr.prefix, cInstr.opcode);
        break;

    default:
        cInstr.detectedPrefix = false;
        // formattedLog(debuglog, LOGTYPE_DEBUG, "opcode %s (pc: %04X, %04X %02X)\n", cInstr.string, PC.v, cInstr.prefix, cInstr.opcode);
        break;
    }

    // Catch massive operand counts
    if (cInstr.numOperands >= 3) {
        cInstr.numOperands = 0;
    }
    cInstr.numOperandsToRead = cInstr.numOperands;
}

/*
We decide here if we need to execute the instruction, handle a prefix or read memory
*/
void Z80_decodeBranchDecision() {
    // We have a prefix code, so we need to handle it
    if (cInstr.detectedPrefix) {
        // We need to move to a prefix read branch
        onNextRisingCLCK = &Z80_prepPrefixedInstructionRead;
    }
    // If we have no operands, we can just do an execution
    else if (cInstr.numOperands == 0) {
        onNextRisingCLCK = &Z80_executeInstruction;
    }
    else {
        // We need to read operands from memory to continue the execution, so we shall set up the pathway
        onNextRisingCLCK = &Z80_prepReadOperands; // this is just a dummy function that leads into the real pathway after M1 has elapsed
    }
}