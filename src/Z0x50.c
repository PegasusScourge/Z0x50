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

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include "Z0x50.h"
#include "Debug.h"
#include "Signals.h"
#include "Z80/Z80.h"
#include "Memory/MemoryController.h"
#include "CfgReader.h"

#define MATCHARG(a, b) strcmp(argV[a], b) == 0

/* CONSTS */
const char* ASCII_headerArt = " _____   ____         ______ ____\n/__  /  / __ \\ _  __ / ____// __ \\\n  / /  / / / /| |/_//___ \\ / / / /\n / /__/ /_/ /_>  < ____/ // /_/ /\n/____/\\____//_/|_|/_____/ \\____/\n";
const char* ASCII_terminalSlpit = "---------------------------------------\n";

/* Z0x50 state information */
int Z0_state = Z0State_NONE; // The state we are currently executing in. Takes a value of Z0StateEnum
char* Z0_sefaultCfg = "Z0x50.cfg"; // Default configuration file
char* Z0_overrideCfg = NULL; // If this is non-NULL, we override the default CFG file

/* Command line paramaters */
char** argV;
int argC;

/* MAIN FUNCTION */

void Z0_main() {
    bool clkV = false;

    switch (Z0_state) {
    case Z0State_DECOMPILE: // Our task is to initialise the Z80 and the memory, then load the program we are to decompile. Then raise DECOMPILE_SIGNAL

        break;

    case Z0State_TEST:
        // Rotate the clock signal 200 times
        for (int i = 0; i < 1; i++) {
            clkV = !clkV;
            // printf("Clock now %i, iteration %i\n", clkV, i);
            if (clkV)
                signals_raiseSignal(&signal_CLCK);
            else
                signals_dropSignal(&signal_CLCK);
        }
        break;

    default: // If we enter an unknown state, just terminate the program
        printf("ERROR: unknown Z0x50 state, exiting...\n");
        exit(-1);
        break;
    }
}

/* Functions */

// Initialisation of the system from the arguments and CFG
void Z0_initSystem() {
    // Init the memory
    memoryController_init();
    // Init the Z80
    Z80_init();
}

// Argument parsing function. Modifies var Z0_State
void Z0_parseArguments() {
    // If argc == 1, we don't have any arguments. Return
    if (argC == 1) return;

    // Iterate through the arguments, starting at index 1
    for (int i = 1; i < argC; i++) {
        if (MATCHARG(i, "-D")) { // Decompile mode switch
            debug_printf("Set state: DECOMPILE\n");
            // Set the state
            Z0_state = Z0State_DECOMPILE;
        }
        if (MATCHARG(i, "-T")) { // Test mode switch
            debug_printf("Set state: TEST\n");
            // Set the state
            Z0_state = Z0State_TEST;
        }
        if (MATCHARG(i, "-c") && i < (argC - 1)){ // CFG select switch, only triggers if there is at least one more argument
            // Set the CFG
            Z0_overrideCfg = argV[++i];
            debug_printf("Set CFG: %s\n", Z0_overrideCfg);
        }
    }

}

/* Entry Point */

int main(int argc, char* argv[]) {
    // Transfer ownership
    argV = argv;
    argC = argc;

    // Print the header art from the ASCII file.
    printf(ASCII_terminalSlpit);
    printf(ASCII_headerArt);
    printf("\n\nZ0x50 | Zilog 80 Emulator | Created by Matthew Clarke\n");
#ifdef _DEBUG
    printf("[COMPILED IN DEBUG MODE]\n");
#endif
    printf(ASCII_terminalSlpit);

    printf("Parsing command line arguments\n");
    // Call the parsing function, passing the arguments we recieved
    Z0_parseArguments();

    // TODO - parse the CFG file
    printf("Parsing CFG:\n");
    cfgReader_readConfiguration("configuration.cfg");

    printf("Initialising system\n");
    Z0_initSystem();

    printf("Launching\n");
    // Call the main function
    Z0_main();
}
