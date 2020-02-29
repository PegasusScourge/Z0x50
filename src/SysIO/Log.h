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

Log.h : Handles logging

*/

#include <stdio.h>

#define STDLOG_FILE "Z0x50.log"
#define DBGLOG_FILE "Debug.log"

extern FILE* stdlog;
extern FILE* debuglog;

#define LOGTYPE_ERROR "[ERR] "
#define LOGTYPE_MSG "[MSG] "
#define LOGTYPE_WARN "[WARN] "
#define LOGTYPE_DEBUG "[DEBUG] "

void log_initLogFiles();
void log_closeLogFiles();

#define directLog(fp, fmt, ...) if(fp) { fprintf(fp, fmt, __VA_ARGS__); } printf(fmt, __VA_ARGS__);
#define formattedLog(fp, type, fmt, ...) if(fp) { fprintf(fp, type); fprintf(fp, fmt, __VA_ARGS__); } printf(type); printf(fmt, __VA_ARGS__);