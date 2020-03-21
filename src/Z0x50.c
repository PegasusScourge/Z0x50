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
#include "Signals.h"
#include "Z80/Z80.h"
#include "Memory/MemoryController.h"
#include "CfgReader.h"
#include "SysIO/Log.h"
#include "SysIO/SysIO.h"
#include "Z80/Z80Decomp.h"

#define MATCHARG(a, b) strcmp(argV[a], b) == 0

/* CONSTS */
const char* ASCII_headerArt = " _____   ____         ______ ____\n/__  /  / __ \\ _  __ / ____// __ \\\n  / /  / / / /| |/_//___ \\ / / / /\n / /__/ /_/ /_>  < ____/ // /_/ /\n/____/\\____//_/|_|/_____/ \\____/\n";
const char* ASCII_terminalSlpit = "---------------------------------------\n";

/* Z0x50 state information */
int state = Z0State_NONE; // The state we are currently executing in. Takes a value of Z0StateEnum
char* defaultCfg = "configuration.cfg"; // Default configuration file
char* overrideCfg = NULL; // If this is non-NULL, we override the default CFG file

/* Z0x50 decompilation */
char* decompFile = NULL;
SysFile_t* decompilationFp = NULL;

/* Command line paramaters */
char** argV;
int argC;

/* RAND */
bool clkV = false;
int i = 0;

/* MAIN FUNCTION */

void Z0_main() {
    switch (state) {
    case Z0State_DECOMPILE: // Process each instruction and report it to the decompilation log
        if (!decomp_next()) {
            decomp_finalise();
            state = Z0State_NONE;
        }
        break;

    case Z0State_TEST:
        // Rotate the clock signal 200 times
        if(i < 200) {
            clkV = !clkV;
            // printf("Clock now %i, iteration %i\n", clkV, i);
            if (clkV)
                signals_raiseSignal(&signal_CLCK);
            else
                signals_dropSignal(&signal_CLCK);
            i++;
        }
        else {
            formattedLog(stdlog, LOGTYPE_MSG, "Done!\n");
            state = Z0State_NONE;
        }
        break;

    default: // If we enter an unknown state, just terminate the program
        printf("ERROR: unknown Z0x50 state, exiting...\n");
        state = Z0State_NONE;
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

    // Do a switch for init here
    switch (state) {
    case Z0State_DECOMPILE: // Load the specified file into memory for decompilation
        if (decompFile == NULL) {
            // We don't have a decomp file, fail
            state = Z0State_NONE;
            break;
        }

        // Continue by loading the decompilation file
        decompilationFp = sysIO_openFile(decompFile);
        if (decompilationFp == NULL) {
            // We don't have a file, failed to open, fail
            state = Z0State_NONE;
            break;
        }
        // Attempt to cache the file
        sysIO_cacheFile(decompilationFp);
        if (!decompilationFp->cached) {
            // We don't have a file, failed to open, fail
            state = Z0State_NONE;
            break;
        }

        // We have the file loaded and can proceed
        decomp_init(decompilationFp);
        break;

    default:
        // Do nothing
        break;
    }
}

// Argument parsing function. Modifies var Z0_State
void Z0_parseArguments() {
    // If argc == 1, we don't have any arguments. Return
    if (argC == 1) return;

    // Iterate through the arguments, starting at index 1
    for (int i = 1; i < argC; i++) {
        if (MATCHARG(i, "-d") && i < (argC - 1)) { // Decompile mode switch
            formattedLog(stdlog, LOGTYPE_MSG, "Set state: DECOMPILE\n");
            // Set the state
            state = Z0State_DECOMPILE;
            // Select the file to load from the next arg
            decompFile = argV[++i];
        }
        if (MATCHARG(i, "-T")) { // Test mode switch
            formattedLog(stdlog, LOGTYPE_MSG, "Set state: TEST\n");
            // Set the state
            state = Z0State_TEST;
        }
        if (MATCHARG(i, "-c") && i < (argC - 1)){ // CFG select switch, only triggers if there is at least one more argument
            // Set the CFG
            overrideCfg = argV[++i];
            formattedLog(stdlog, LOGTYPE_MSG, "Set CFG: %s\n", overrideCfg);
        }
    }

}

/* Entry Point */

int main(int argc, char* argv[]) {
    // Transfer ownership
    argV = argv;
    argC = argc;

    // Open the log files
    log_initLogFiles();

    // Print the header art from the ASCII file.
    directLog(stdlog, ASCII_terminalSlpit);
    directLog(stdlog, ASCII_headerArt);
    directLog(stdlog, "\n\nZ0x50 | Zilog 80 Emulator | Created by Matthew Clarke\n");
#ifdef _DEBUG
    directLog(stdlog, "[COMPILED IN DEBUG MODE]\n");
#endif
    directLog(stdlog, ASCII_terminalSlpit);

    formattedLog(stdlog, LOGTYPE_MSG, "Parsing command line arguments\n");
    // Call the parsing function, passing the arguments we recieved
    Z0_parseArguments();

    // Parse cfg
    formattedLog(stdlog, LOGTYPE_MSG, "Parsing CFG\n");
    if (overrideCfg == NULL)
        overrideCfg = defaultCfg;
    cfgReader_readConfiguration(overrideCfg);

    formattedLog(stdlog, LOGTYPE_MSG, "Initialising system\n");
    Z0_initSystem();

    formattedLog(stdlog, LOGTYPE_MSG, "Launching\n");
    // Call the main function
    while (state != Z0State_NONE) {
        Z0_main();
    }

    formattedLog(stdlog, LOGTYPE_MSG, "Exiting\n");
    // Check for a decomp file
    if (decompilationFp) {
        sysIO_closeFile(decompilationFp);
        decompilationFp = NULL;
    }

    // Clean up the settings
    cfgReader_cleanSettings();

    // Close
    log_closeLogFiles();
}
