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
#include "Oscillator.h"
#include "StringUtil.h"

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
unsigned long long numOscillations = 0;

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
        // Rotate the clock signal 50 times
        if(i < 50) {
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

    case Z0State_NORMAL:
        if (numOscillations > 1000) {
            formattedLog(stdlog, LOGTYPE_MSG, "Z80 has reached termination\n");
            state = Z0State_NONE;
            break;
        }

        if (Z80_state() != Z80State_Failure) {
            // Ask the oscillator to function
            if(oscillator_tick())
                numOscillations++;
        }
        else {
            formattedLog(stdlog, LOGTYPE_MSG, "Z80 has issued a termination request\n");
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

    // Init the oscillator
    oscillator_init();

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

    case Z0State_NORMAL:
        // Load memory devices here
        Z0_loadMemoryDevices();

        // Load the bios ROM
        if (!Z0_loadBiosROM())
            break;
        // state = Z0State_NONE;
        break;

    default:
        // Do nothing
        break;
    }
}

bool Z0_loadBiosROM() {
    // Load the ROM file specified by the settings
        // This is a required step: we fail if it can't be done
    if (!cfgReader_querySettingExist("bios_rom")) {
        formattedLog(stdlog, LOGTYPE_ERROR, "Cfg file missing 'bios_rom' setting, unable to load\n");
        state = Z0State_NONE;
        return false;
    }
    char* biosRomFilePath = cfgReader_querySettingValueStr("bios_rom");
    SysFile_t* biosRomFile = sysIO_openFile(biosRomFilePath);
    if (biosRomFile == NULL) {
        formattedLog(stdlog, LOGTYPE_ERROR, "BIOS ROM file error: unable to find file '%s'\n", biosRomFilePath);
        state = Z0State_NONE;
        return false;
    }

    // Cache the file
    sysIO_cacheFile(biosRomFile);
    if (!biosRomFile->cached) {
        formattedLog(stdlog, LOGTYPE_ERROR, "BIOS ROM file '%s' could not be cached\n", biosRomFilePath);
        state = Z0State_NONE;
        return false;
    }

    // Load the file into memory now, at position 0 by default
    uint16_t romAddress = 0;
    formattedLog(stdlog, LOGTYPE_MSG, "Loading BIOS ROM file '%s' into address ", biosRomFilePath);
    if (cfgReader_querySettingExist("bios_address")) {
        romAddress = cfgReader_querySettingValueInt("bios_address");
    }
    directLog(stdlog, "%04X\n", romAddress);

    // We need to drive the signal_addressBus and signal_dataBus with the appropriate values as we scan through the data
    // To allow writing, we need to put signal_MREQ and signal_WR high to make the controller write on the memoryController_onCLCK(true) trigger
    signals_raiseSignal(&signal_MREQ);
    signals_raiseSignal(&signal_WR);

    // Do the writing
    formattedLog(debuglog, LOGTYPE_DEBUG, "Writing BIOS ROM file to memory. Bytes to write: %04X\n", biosRomFile->size);
    formattedLog(stdlog, LOGTYPE_MSG, "Writing BIOS ROM file to memory. Bytes to write: %04X\n", biosRomFile->size);

    signal_addressBus = romAddress;
    for (uint16_t i = 0; i < biosRomFile->size; i++) {
        // Put the data on the bus
        signal_addressBus = i + romAddress;
        signal_dataBus = biosRomFile->data[i];

        // Trigger the write
        memoryController_onCLCK(true);
    }
    directLog(debuglog, "\n");
    formattedLog(debuglog, LOGTYPE_DEBUG, "BIOS ROM file write complete\n");
    formattedLog(stdlog, LOGTYPE_MSG, "BIOS ROM file write complete\n");

    // Clean up the signals
    signals_dropSignal(&signal_MREQ);
    signals_dropSignal(&signal_WR);

    sysIO_closeFile(biosRomFile);
    return true;
}

void Z0_loadMemoryDevices() {
    formattedLog(debuglog, LOGTYPE_DEBUG, "Memory device configuation\n");

    for (int i = 0; i < MAX_NUMBER_OF_MEMORIES; i++) {
        // Generate the matcher string we look for as a setting
        char prefix[50] = "memdev";
        char* matcher = prefix;
        char number[3];
        _itoa(i, number, 10);
        strcat(matcher, number);
        directLog(debuglog, "Looking for memory device '%s' definition...\n", matcher);

        if (cfgReader_querySettingExist(matcher)) {
            // We have the definition, but we must check it to be sure
            char* rawVal = cfgReader_querySettingValueStr(matcher);
            int buffLen = (int)strlen(rawVal) + 1;
            char* buff = calloc(buffLen, sizeof(char));
            if (buff == NULL) {
                // We fail!
                formattedLog(stdlog, LOGTYPE_WARN, "Detected setting for '%s' but failed to allocate memory for buffer\n", matcher);
            }

            memcpy(buff, rawVal, buffLen); // Copy the null char too
            char* splits[8];
            int splitsMade = sutil_split(buff, buffLen, splits, 8, ",");
            if (splitsMade == 4) {
                // We are valid!
                formattedLog(stdlog, LOGTYPE_MSG, "Detected setting for '%s'\n", matcher);
                // Time to configure the memory device
                memoryController_createDevice(atoi(splits[0]), atoi(splits[1]), atoi(splits[2]), atoi(splits[3]));
            }
            else {
                formattedLog(stdlog, LOGTYPE_WARN, "Detected setting for '%s', but it was of the incorrect format. Had %i elements.\n", matcher, splitsMade);
            }

            free(buff);
        }
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
    state = Z0State_NORMAL;
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
