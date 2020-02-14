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
Signal_t signal_M1 = { false, NULL, 0 };
Signal_t signal_MREQ = { false, NULL, 0 };
Signal_t signal_IORQ = { false, NULL, 0 };
Signal_t signal_RD = { false, NULL, 0 };
Signal_t signal_WR = { false, NULL, 0 };
Signal_t signal_RFSH = { false, NULL, 0 };

// CLOCK
Signal_t signal_CLCK = { false, NULL, 0 };

// CPU control
Signal_t signal_HALT = { false, NULL, 0 };
Signal_t signal_WAIT = { false, NULL, 0 };
Signal_t signal_INT = { false, NULL, 0 };
Signal_t signal_NMI = { false, NULL, 0 };
Signal_t signal_RESET = { false, NULL, 0 };
Signal_t signal_BUSRQ = { false, NULL, 0 };
Signal_t signal_BUSACK = { false, NULL, 0 };

void signals_triggerListeners(Signal_t* signal, bool rising) {
    for (int i = 0; i < signal->nListeners; i++) {
        signal->listeners[i](rising);
    }
}

void signals_raiseSignal(Signal_t* signal) {
    signals_triggerListeners(signal, signal->state = true);
}

void signals_dropSignal(Signal_t* signal) {
    signals_triggerListeners(signal, signal->state = false);
}

bool signals_readSignal(Signal_t* signal) {
    return signal->state;
}

void signals_addListener(Signal_t* signal, void (*fun)(bool)) {
    if (signal->nListeners >= 16) {
        // We are out of listeners to add
        // Do something?
    }
    else {
        // Add the signal listening function to the list of listeners
        signal->listeners[signal->nListeners] = fun;
        // Increment the number of listeners attached to this function
        signal->nListeners++;
    }
}