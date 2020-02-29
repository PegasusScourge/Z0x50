/*

 _____   ____         ______ ____
/__  /  / __ \ _  __ / ____// __ \
  / /  / / / /| |/_//___ \ / / / /
 / /__/ /_/ /_>  < ____/ // /_/ /
/____/\____//_/|_|/_____/ \____/

Zilog 80 Emulator

Basic interface to the Z80 processor and associated modules.
Can be run as a Sinclair ZX Spectrum or used as a basis for a larger project.

CfgReader.c : Reads the CFG file and exposes the settings

*/

#include "Debug.h"
#include "CfgReader.h"
#include "SysIO/SysIO.h"
#include "StringUtil.h"

#include <stdlib.h>
#include <stdio.h>

SysFile_t* cfgFile;

#define MAX_NUM_SETTINGS 256
Setting_t* settings[MAX_NUM_SETTINGS];
int currentSetting = 0;

/********************************************************************

    Setting_t functions

********************************************************************/

/*
Creates a Setting_t to hold information about settings
*/
Setting_t* cfgReader_createSetting(const char* name) {
    Setting_t* s = malloc(sizeof(Setting_t));
    if (s == NULL) {
        debug_printf("Unable to create Setting_t: can't allocate memory for struct\n");
        return NULL;
    }

    s->name = calloc(strlen(name), sizeof(char));
    if (s->name == NULL) {
        debug_printf("Unable to create Setting_t: can't allocate memory for name str\n");
        return NULL;
    }

    // Move the name across
    strcpy(s->name, name);

    return s;
}

/*
Destroys a Setting_t and cleans up memory after it
*/
void cfgReader_destroySetting(Setting_t* s) {
    if (s == NULL) {
        debug_printf("Attempted to destroy NULL Setting_t\n");
        return;
    }

    // Attempt to free the name string
    if (s->name != NULL) {
        free(s->name);
    }

    // Free the setting
    free(s);
}

/********************************************************************

    Cfg Functions

********************************************************************/

void cfgReader_readConfiguration(const char* path) {
    // Open the CFG file
    cfgFile = sysIO_openFile(path);

    // Cache the content
    sysIO_cacheFile(cfgFile);

    // Check it is cached
    if (!cfgFile->cached) {
        fprintf(stderr, "Unable to read configuration file! Terminating\n");
        exit(-1);
    }

    // Process the CFG file
    cfgReader_processConfiguration(cfgFile->data, cfgFile->size);

    // Close the CFG file
    sysIO_closeFile(cfgFile);
    cfgFile = NULL;
}

void cfgReader_processConfiguration(const char* data, const long int size) {
    // Copy the data into another buffer
    char* buffer = calloc(size, sizeof(char));
    if (buffer == NULL) {
        debug_printf("[CFG PROCESS] Failed to process CFG: could not allocate %i bytes for buffer. Terminating\n", size);
        exit(-1);
    }
    debug_printf("[CFG PROCESS] Allocated buffer of %i bytes\n", size);
    strcpy(buffer, data);
    
    // Check that the last character is a '\0'
    if (buffer[size - 1] != '\0') {
        debug_printf("[CFG PROCESS] Added null char, overwrote char '%c'\n", buffer[size - 1]);
        buffer[size - 1] = '\0';
    }

    // Prepare the variables to capture the lines
    char* line;
#define MAX_NUM_LINES 50
    char* lines[MAX_NUM_LINES];
    int linesDetected = 0;

    // Prepare the token
    char token[2] = "\n";

    // Prepare the buffer freeing
    char* baseOfBuffer = buffer; // Save the pointer so we can free it later

    // FROM THIS POINT buffer WILL CHANGE VALUE, FREE MEMORY WITH baseOfBuffer

    // NOW break the file down into lines
    // Get the first line
    line = strtok(buffer, token);

    while (line != NULL && linesDetected < MAX_NUM_LINES) {
        // Save the current pointer
        lines[linesDetected] = sutil_trim(line, NULL);

        // Get the next line
        line = strtok(NULL, token);
        linesDetected++;
    }

    // Debug printing
    debug_printf("[CFG PROCESS] Split file into %i lines:\n", linesDetected);
    for (int i = 0; i < linesDetected; i++) {
        fprintf(DEBUG_OUT, "'");
        fprintf(DEBUG_OUT, lines[i]);
        fprintf(DEBUG_OUT, "'\n");
    }

    // Now iterate through the lines and process them
    for (int i = 0; i < linesDetected; i++) {
        cfgReader_processLine(lines[i]);
    }

    // Free the buffer
    debug_printf("[CFG PROCESS] Freeing buffer\n");
    free(baseOfBuffer);
}

#define strequal(a, b) strcmp(a, b) == 0
#define checkSplit(a, b) strequal(splits[0], a) && splitsDetected >= b
void cfgReader_processLine(const char* ln) {
    // We now need to split the line by '=' chars and trim
    // Create a buffer to play with
    int buffLen = (int)strlen(ln) + 1; // Make space for a padding null terminator
    char* buff = calloc(buffLen, sizeof(char)); // NEED TO FREE
    if (buff == NULL) {
        debug_printf("[CFG PROCESS] Could not process line: allocation of line buffer failed\n");
        return; // Just continue
    }
    char* buffBase = buff;

#define NUM_SPLITS 16
    char* splits[NUM_SPLITS]; // Allow there to be NUM_SPLITS splits
    int splitsDetected = 0;
    char token[2] = "=";

    // Fill the buff
    strcpy(buff, ln);
    buff[buffLen - 1] = '\0'; // Make sure we have a null terminator

    // FROM THIS POINT buff WILL CHANGE VALUE, FREE MEMORY WITH buffBase

    // Get the first split
    char* split;
    split = strtok(buff, token);

    while (split != NULL && splitsDetected < NUM_SPLITS) {
        splits[splitsDetected] = sutil_trim(split, NULL);

        // Get the next line
        split = strtok(NULL, token);
        splitsDetected++;
    }

    /*
    debug_printf("[CFG PROCESS] Split line '%s' into %i splits:\n", l, splitsDetected);
    for (int i = 0; i < splitsDetected; i++) {
        fprintf(DEBUG_OUT, splits[i]);
        fprintf(DEBUG_OUT, "\n");
    }
    */

    // Check that we have splits
    if (splitsDetected > 0) {
        Setting_t* s = NULL;

        // Process the line
        if (checkSplit("testLine", 2)) {
            debug_printf("testLine says: '%s'\n", splits[1]);
        }
        // --- CLOCK SETTINGS
        else if (checkSplit("clock_speed", 2)) {
            s = cfgReader_createSetting("clock_speed");
            s->value = atoi(splits[1]);
        }
        // --- ANYTHING ELSE?
        else if(splitsDetected == 2) {
            // If we have 2 splits, just add the setting anyway at this point
            s = cfgReader_createSetting(splits[0]);
            s->value = atoi(splits[1]);
            debug_printf("Auto-setting generation:\n");
        }

        // If the setting isn't NULL, add it to the list
        if (s != NULL) {
            settings[currentSetting] = s;
            debug_printf("Added setting '%s' at index %i\n", s->name, currentSetting);
            currentSetting++;
        }
    }

    // Free the buffer
    free(buffBase);
}