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

CfgReader.h : Reads the CFG file and exposes the settings

*/

#include <stdint.h>

typedef struct Setting {
    char* name;
    int value;
} Setting_t;

/********************************************************************

    Setting_t functions

********************************************************************/

Setting_t* cfgReader_createSetting(const char* name);
void cfgReader_destroySetting(Setting_t* s);

/********************************************************************

    Cfg Functions

********************************************************************/

void cfgReader_readConfiguration(const char* path);
void cfgReader_processConfiguration(const uint8_t* data, const long int size);
void cfgReader_processLine(const char* line);