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

uint16_t Z80Alu_add16(uint16_t a, uint16_t b, bool affectFlags) {
    resultStore._32 = a + b;

    // Manipulate flags if allowed
    if (affectFlags) {
        // Set carry flag
        if (resultStore._32 > 0xFFFF)
            Z80Flags_setFlag(Z80FLAGS_CARRY);
        else
            Z80Flags_clearFlag(Z80FLAGS_CARRY);

        // Set other flags?
    }

    // Return the 16 bit result
    return resultStore._16;
}