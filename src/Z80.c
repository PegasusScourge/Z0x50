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

/* Z80FlagAccess.h functions */
void Z80Flags_setFlag(int f) {
    AF.bytes[Z80DBLREG_LOWER] = AF.bytes[Z80DBLREG_LOWER] | (0b00000001 << f);
}
void Z80Flags_clearFlag(int f) {
    AF.bytes[Z80DBLREG_LOWER] = AF.bytes[Z80DBLREG_LOWER] & ~(0b00000001 << f);
}
bool Z80Flags_readFlag(int f) {
    return (AF.bytes[Z80DBLREG_LOWER] & (0b00000001 << f)) == 1;
}