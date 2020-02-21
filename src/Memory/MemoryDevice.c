/*

 _____   ____         ______ ____
/__  /  / __ \ _  __ / ____// __ \
  / /  / / / /| |/_//___ \ / / / /
 / /__/ /_/ /_>  < ____/ // /_/ /
/____/\____//_/|_|/_____/ \____/

Zilog 80 Emulator

Basic interface to the Z80 processor and associated modules.
Can be run as a Sinclair ZX Spectrum or used as a basis for a larger project.

MemoryDevice.c : Memory device definitions

*/

#include "MemoryDevice.h"
#include "../Debug.h"

#include <stdlib.h>

/*
Creates a memory device
*/
MemoryDevice_t* memoryDevice_create(uint16_t sOffset, uint16_t len, bool wEn, bool rEn) {
    // Create the device
    MemoryDevice_t* device = malloc(sizeof(MemoryDevice_t));
    if (device == NULL) {
        debug_printf("[ERROR] Failed to create memory device!\n");
        return NULL;
    }

    // Store the data in the device
    device->len = len;
    device->startOffset = sOffset;
    device->readEnable = rEn;
    device->writeEnable = wEn;

    // Create the data buffer
    device->data = calloc(len, sizeof(uint8_t));
    if (device->data == NULL) {
        debug_printf("[ERROR] Failed to allocate memory for memory device (len=%i bytes)\n", len);
        // Deallocate the memory device and return NULL
        memoryDevice_deconstruct(device);
        return NULL;
    }

    // We are ready to return the device
    return device;
}

/*
Deconstructs a device and frees the memory associated with it
*/
void memoryDevice_deconstruct(MemoryDevice_t* device) {
    // Check the device is valid
    if (device == NULL) {
        debug_printf("[ERROR] Cannot deallocate a NULL device!\n");
        return;
    }

    // Deallocate the data inside the memory device first
    // Check the data is valid
    if (device->data == NULL) {
        debug_printf("[ERROR] Cannot deallocate a NULL data!\n");
    }
    else {
        // Free the data
        free(device->data);
        device->data = NULL;
    }

    // Free the device memory
    free(device);
    device = NULL;

    // All freed
}