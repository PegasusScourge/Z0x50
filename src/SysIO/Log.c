/*

 _____   ____         ______ ____
/__  /  / __ \ _  __ / ____// __ \
  / /  / / / /| |/_//___ \ / / / /
 / /__/ /_/ /_>  < ____/ // /_/ /
/____/\____//_/|_|/_____/ \____/

Zilog 80 Emulator

Basic interface to the Z80 processor and associated modules.
Can be run as a Sinclair ZX Spectrum or used as a basis for a larger project.

Log.c : Handles logging

*/

#include "Log.h"

FILE* stdlog;
FILE* debuglog;

void log_initLogFiles() {
    // Attempt to open the file pointers
    stdlog = fopen(STDLOG_FILE, "w");
    if (stdlog == NULL) {
        // Throw an error
        printf("Failed to create log file '" STDLOG_FILE "'\n");
    }

#ifdef _DEBUG
    debuglog = fopen(DBGLOG_FILE, "w");
#else
    debuglog = NULL;
#endif
    if (debuglog == NULL) {
        // Throw an error
        printf("Failed to create log file '" DBGLOG_FILE "'\n");
    }
}

void log_closeLogFiles() {
    if (stdlog)
        fclose(stdlog);
    if (debuglog)
        fclose(debuglog);
}