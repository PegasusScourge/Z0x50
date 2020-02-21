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

MemoryDevice.h : Memory device definitions

*/

#include <stdint.h>
#include <stdbool.h>

typedef struct MemoryDevice {
    bool chipEnable; // When true, the memory device can be processed
    bool writeEnable; // When true, the memory device can be writen to
    bool readEnable; // When true, the memory device can be read from

    uint16_t startOffset;
    uint16_t len;

    uint8_t* data;
} MemoryDevice_t;

/* Constructor and destructor functions */
MemoryDevice_t* memoryDevice_create(uint16_t sOffset, uint16_t len, bool wEn, bool rEn);
void memoryDevice_deconstruct(MemoryDevice_t* device);