/*

 _____   ____         ______ ____
/__  /  / __ \ _  __ / ____// __ \
  / /  / / / /| |/_//___ \ / / / /
 / /__/ /_/ /_>  < ____/ // /_/ /
/____/\____//_/|_|/_____/ \____/

Zilog 80 Emulator

Basic interface to the Z80 processor and associated modules.
Can be run as a Sinclair ZX Spectrum or used as a basis for a larger project.

Z80Alu.c : Z80 CPU ALU component

*/

#include "Z80Alu.h"
#include "Z80FlagAccess.h"

/* Internal ALU values to simplify C variable handling */
union resultStore {
    uint32_t _32;
    uint16_t _16;
    uint8_t _8;
} resultStore; // Stores the result of a calculation. Uses a 32 bit "input" so we can do tests if necessary
