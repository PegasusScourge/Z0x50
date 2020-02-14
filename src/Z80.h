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
#define Z80DBLREG_UPPER 0
#define Z80DBLREG_LOWER 1
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

/********************************************************************

    Z80 Fetch Functions

********************************************************************/

void Z80_fetchT1Rise();