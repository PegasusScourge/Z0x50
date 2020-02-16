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

Z80Decompile.h : Decompilation code to take an input instruction and provide a breakdown of the instruction.

*/

/*

Instruction format:

[prefix byte] opcode [displacement byte] [immediate data]

displacement byte: signed 8-bit integer
immediate data: zero, one or two bytes. In case of two bytes, Least Significant Byte is first, so byte two is upper 8 bits.

*/

#include <stdint.h>
#include "Z80Instructions.h"

/* Enums */

/* Functions */
// Mallocs memory for an instruction and fills it with the appropriate data
Z80_Instr_t* Z80Decomp_decodeInstruction(uint8_t opcode);

// Destroys an instruction malloced in the decodeInstruction function
void Z80Decomp_freeInstruction(Z80_Instr_t* instr);