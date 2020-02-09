/*

 _____   ____         ______ ____
/__  /  / __ \ _  __ / ____// __ \
  / /  / / / /| |/_//___ \ / / / /
 / /__/ /_/ /_>  < ____/ // /_/ /
/____/\____//_/|_|/_____/ \____/

Zilog 80 Emulator

Basic interface to the Z80 processor and associated modules.
Can be run as a Sinclair ZX Spectrum or used as a basis for a larger project.

Signals.c : Handles all the "signals" that the CPU and other components can accept. 

*/

#include "Signals.h"

/* Signal defs */
// Busses
uint8_t signal_dataBus = 0;
uint16_t signal_addressBus = 0;

// System control
bool signal_M1 = false;
bool signal_MREQ = false;
bool signal_IORQ = false;
bool signal_RD = false;
bool signal_WR = false;
bool signal_RFSH = false;

// CPU control
bool signal_HALT = false;
bool signal_WAIT = false;
bool signal_INT = false;
bool signal_NMI = false;
bool signal_RESET = false;
bool signal_BUSRQ = false;
bool signal_BUSACK = false;