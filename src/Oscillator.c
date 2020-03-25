/*

 _____   ____         ______ ____
/__  /  / __ \ _  __ / ____// __ \
  / /  / / / /| |/_//___ \ / / / /
 / /__/ /_/ /_>  < ____/ // /_/ /
/____/\____//_/|_|/_____/ \____/

Zilog 80 Emulator

Basic interface to the Z80 processor and associated modules.
Can be run as a Sinclair ZX Spectrum or used as a basis for a larger project.

Oscillator.c : Provides the clock signal

*/

#include "Oscillator.h"
#include "Signals.h"
#include "SysIO/Log.h"
#include <stdlib.h>
#include <stdint.h>

#include "SFML/System.h"
#include "CfgReader.h"

double freqMHz = 0.1;
double microsPerClock = 0.0;
double overflow = 0;

bool clockState = false;

sfClock* clock;

void oscillator_init() {
    double fMHz = 0.00001;

    if (cfgReader_querySettingExist("oscillator_freq")) {
        double value = cfgReader_querySettingValueDouble("oscillator_freq");
        if (value > fMHz)
            fMHz = value;
    }

    freqMHz = fMHz;
    microsPerClock = 1.0 / freqMHz;

    clock = sfClock_create();
    if (clock == NULL) {
        // Failed to initialise oscillator!
        formattedLog(stdlog, LOGTYPE_ERROR, "FAILED TO INITIALISE OSCIALLATOR!\n");
        exit(EXIT_FAILURE);
    }

    sfClock_restart(clock);

    formattedLog(stdlog, LOGTYPE_MSG, "Osciallator settings: freqMHz = %f (%f Hz), microsPerClock = %f\n", freqMHz, freqMHz * 1000000, microsPerClock);
}

bool oscillator_tick() {
    sfInt64 elapsedMicros = sfTime_asMicroseconds(sfClock_getElapsedTime(clock));
    sfClock_restart(clock);
    overflow += elapsedMicros;

    if (overflow < microsPerClock) {
        return false;
    }

    int ticksDone = 0;
    // formattedLog(debuglog, LOGTYPE_DEBUG, "Overflow of %f.", overflow);
    while (overflow > microsPerClock) {
        overflow -= microsPerClock;
        ticksDone++;
        clockState = !clockState;
        if (clockState) {
            signals_raiseSignal(&signal_CLCK);
        }   
        else {
            signals_dropSignal(&signal_CLCK);
        }
    }
    // directLog(debuglog, "Completed %i ticks\n", ticksDone);

    return true;
}