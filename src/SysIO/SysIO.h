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

SysIO.h : System I/O for the filesystem of the host.

*/

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// System file representation. Carries data about the file
typedef struct SysFile {
    FILE* fPtr; // ALLOCATED: MUST FREE
    char path[256];
    long int size;
    unsigned char* data; // ALLOCATED possibly: MUST FREE
    bool cached;
} SysFile_t;

/********************************************************************

    SysIO functions

********************************************************************/

SysFile_t* sysIO_openFile(const char* path);
void sysIO_closeFile(SysFile_t* file);
void sysIO_cacheFile(SysFile_t* file);