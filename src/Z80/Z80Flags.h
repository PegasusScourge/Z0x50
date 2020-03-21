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

Z80FlagAccess.h : Abstraction header to functions that allow good access to the Z80 flags register. For ALU purposes

*/

/*

Flags register layout:

7 6 5 4 3 2   1 0
S Z X H X P/V N C

C: Carry flag
N: Add/subtract -- > ASFlag
P/V: Parity/Overflow flag
H: Half carry flag
Z: Zero flag
S: Sign flag
X: Not used

*/

#include <stdbool.h>

#define Z80FLAGS_CARRY 0
#define Z80FLAGS_AS 1
#define Z80FLAGS_PV 2
#define Z80FLAGS_HCARRY 4
#define Z80FLAGS_ZERO 6
#define Z80FLAGS_SIGN 7

void Z80Flags_setFlag(int f);
void Z80Flags_clearFlag(int f);
bool Z80Flags_readFlag(int f);
