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

#include <stdbool.h>

#define NO_SETTING -32000

typedef struct Setting {
    char* name;
    struct SettingValue {
        int _int;
        char* _str;
    } value;
} Setting_t;

/********************************************************************

    Setting_t functions

********************************************************************/

Setting_t* cfgReader_createSetting(const char* name);
void cfgReader_destroySetting(Setting_t* s);
void cfgReader_cleanSettings();

/********************************************************************

    Cfg Functions

********************************************************************/

void cfgReader_readConfiguration(const char* path);
void cfgReader_processConfiguration(const char* data, const long int size);
void cfgReader_processLine(const char* ln);

/********************************************************************

    Cfg Query Functions

********************************************************************/

int cfgReader_querySettingValue(const char* n);
bool cfgReader_querySettingExist(const char* n);