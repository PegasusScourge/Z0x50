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

*/

enum Z0StateEnum { Z0State_NONE, Z0State_SPECTRUM, Z0State_DECOMPILE }; // Our possible states we can execute in

/* CONSTS */
extern const char* ASCII_headerArt;
extern const char* ASCII_terminalSlpit;

/* Z0x50 state information */
extern int Z0_state; // The state we are currently executing in. Takes a value of Z0StateEnum

/* Command line paramaters */
extern char** argV;
extern int argC;

/* Z0x50 function predeclarations */
void Z0_parseArguments();