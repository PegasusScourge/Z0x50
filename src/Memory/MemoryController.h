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

MemoryController.h : Memory controller system

*/

#include "MemoryDevice.h"

#define MAX_NUMBER_OF_MEMORIES 32

/********************************************************************

    MemoryController init functions

********************************************************************/

void memoryController_init();

/********************************************************************

    MemoryController create/destroy functions

********************************************************************/

void memoryController_createDevice(uint16_t startAdd, uint16_t size, bool writeable, bool readable);
// void memoryController_destroyDevice();

/********************************************************************

    MemoryController clock response functions

********************************************************************/

void memoryController_onCLCK(bool rising);
void memoryController_processOnCLCK(MemoryDevice_t* device, bool rising);

/********************************************************************

    MemoryController read functions

********************************************************************/

void memoryController_attemptRead(MemoryDevice_t* device);

/********************************************************************

    MemoryController write functions

********************************************************************/

void memoryController_attemptWrite(MemoryDevice_t* device);