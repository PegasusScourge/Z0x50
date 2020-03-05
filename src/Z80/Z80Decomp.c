/*

 _____   ____         ______ ____
/__  /  / __ \ _  __ / ____// __ \
  / /  / / / /| |/_//___ \ / / / /
 / /__/ /_/ /_>  < ____/ // /_/ /
/____/\____//_/|_|/_____/ \____/

Zilog 80 Emulator

Basic interface to the Z80 processor and associated modules.
Can be run as a Sinclair ZX Spectrum or used as a basis for a larger project.

Z0xDecomp.c : Handles decompilation

*/

#include "Z80Decomp.h"
#include "Z80Instructions.h"
#include "../SysIO/Log.h"

FILE* decompLog = NULL;
SysFile_t* dF = NULL;

#define decompLog(fmt, ...) if(decompLog) { fprintf(decompLog, fmt, __VA_ARGS__); } printf("[DECOMP] "); printf(fmt, __VA_ARGS__);

void decomp_init(SysFile_t* file) {
    if (file == NULL) {
        printf("Failed to get a valid file: NULL ptr\n");
        return;
    }
    if (!file->cached) {
        printf("Failed to get a valid file: no cache\n");
        return;
    }
    dF = file;

    // Initialise the log
    decompLog = fopen("decompilation.log", "w");
    if (decompLog == NULL) {
        // Throw an error
        printf("Failed to create log file '" "decompilation.log" "'\n");
    }

    // Output the file in HEX into terminal
    log_dumpHexToDebug(32, dF);

    decompLog("Init decompilation of file complete. Ready to decompile\n");
}

void decomp_next() {
    // Nothing for now
}