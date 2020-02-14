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

Signals.h : Handles all the "signals" that the CPU and other components can accept.

*/

#include <stdbool.h>
#include <stdint.h>

/* Struct defs */
typedef struct Signal{
    bool state;

    // Listeners are permenant attachments unfortunately 
    void (*listeners[16])(bool rising); // Function pointers to listening functions. Passes bool signaling a RISE 'true' FALL 'false'
    uint8_t nListeners; // Number of listeners attached, and the next index to place a listener at
} Signal_t;

/* Typedefs */


/* Bus defs */
// Busses
extern uint8_t signal_dataBus; // System IO bus on which data is transfered around
extern uint16_t signal_addressBus; // System address bus. Selects memory or IO device to communicate with using the DataBus

/* Signal defs */
// System control
extern Signal_t signal_M1; // Z80 output, active LOW, Machine Cycle One. Denotes the first cycle of a new instruction.
extern Signal_t signal_MREQ; // Z80 output, active LOW, Memory Request. Indicates the address bus holds a valid address for memory read/write
extern Signal_t signal_IORQ; // Z80 output, active LOW, IO Request. Indicates the lower half of the address bus (bits 0-7) holds a valid address for a IO read/write
extern Signal_t signal_RD; // Z80 output, active LOW, Read. Indicates Z80 wants to read data from the DataBus. IO/Memory should load data on to the bus.
extern Signal_t signal_WR; // Z80 output, active LOW, Write. Indicates Z80 has put data onto the DataBus to be written by IO/Memory as appropriate.
extern Signal_t signal_RFSH; // Z80 output, active LOW, Refresh. RFSH together with MREQ indicates that the lower seven bits of the system's AddressBus can be used as a refresh address for DRAMs

// Clock
extern Signal_t signal_CLCK; // CLOCK

// CPU control
extern Signal_t signal_HALT; // Z80 output, active LOW, Halt State. Indicates the Z80 has exectuted a HALT instruction and is waiting for an interrupt.
extern Signal_t signal_WAIT; // Z80 input, active LOW, Wait. Instructs the Z80 to wait for the IO/Memory addressed to be ready to communicate data. Z80 waits until this signal goes inactive.
extern Signal_t signal_INT; // Z80 input, active LOW, Interrupt Request. If the interrupt enable flag is set, the Z80 executes an interrupt after the current instruction is finished
extern Signal_t signal_NMI; // Z80 input, active LOW, Nonmaskable Interrup. At the end of the curret instruction, Z80 is compelled to restart at location 0066h.
extern Signal_t signal_RESET; // Z80 input, active LOW, Reset. Resets the Z80 to power-up. Must be active for minimum of three clock cyckes before operation is complete.
extern Signal_t signal_BUSRQ; // Z80 input, active LOW, Buss Request. Forces the Z80 to relinquish control of DataBus, AddressBus, MREQ, IORQ, RD, WR to allow other devices to drive them.
extern Signal_t signal_BUSACK; // Z80 output, active LOW. Bus Acknowledge. Z80 puts this active when BUSRQ has been achieved and the external circuitry can now drive these signals.

/* Function defs */
void signals_raiseSignal(Signal_t* signal);
void signals_dropSignal(Signal_t* signal);
bool signals_readSignal(Signal_t* signal);

void signals_addListener(Signal_t* signal, void (*fun)(bool));