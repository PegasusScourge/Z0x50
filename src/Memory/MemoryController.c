/*

 _____   ____         ______ ____
/__  /  / __ \ _  __ / ____// __ \
  / /  / / / /| |/_//___ \ / / / /
 / /__/ /_/ /_>  < ____/ // /_/ /
/____/\____//_/|_|/_____/ \____/

Zilog 80 Emulator

Basic interface to the Z80 processor and associated modules.
Can be run as a Sinclair ZX Spectrum or used as a basis for a larger project.

MemoryController.c : Memory controller system

*/

#include "../Signals.h"
#include "../Debug.h"

#include "MemoryController.h"

/* Memories */
#define MAX_NUMBER_OF_MEMORIES 32
MemoryDevice_t* memories[MAX_NUMBER_OF_MEMORIES];

/********************************************************************

    MemoryController init functions

********************************************************************/

/*
Initialise the memory controller and the memories
*/
void memoryController_init() {
    // Connect the clock signal
    signals_addListener(&signal_CLCK, &memoryController_onCLCK);
}

/********************************************************************

    MemoryController create/destroy functions

********************************************************************/

/*
Create a device
*/
void memoryController_createDevice(uint16_t startAdd, uint16_t size, bool writeable, bool readable) {
    // Check we don't want to create something useless
    if (size == 0 || (!readable && !writeable)) {
        debug_printf("Attempted to create new device with useless qualities: size=%i, readable=%i, writeable=%i\n", size, readable, writeable);
        return;
    }
    
    int i;
    bool hasFreeSpace = false;
    // Identify where the first space is in the devices
    for (i = 0; i < MAX_NUMBER_OF_MEMORIES; i++) {
        if (memories[i] == NULL) {
            hasFreeSpace = true;
            break;
        }
    }

    if (!hasFreeSpace) {
        // ERROR
        debug_printf("Unable to create new device: no free space\n");
        return;
    }

    // Create the device
    memories[i] = memoryDevice_create(startAdd, size, writeable, readable);
}

/********************************************************************

    MemoryController clock response functions

********************************************************************/

/*
On CLCK
*/
void memoryController_onCLCK(bool rising) {
    // We need to process this for each memory, so we shall pass it off to a handler for each
    for (int i = 0; i < MAX_NUMBER_OF_MEMORIES; i++) {
        if (memories[i] != NULL)
            memoryController_processOnCLCK(memories[i], rising);
    }
}

/*
On CLCK
*/
void memoryController_processOnCLCK(MemoryDevice_t* device, bool rising) {
    // Check for the MREQ signal, as this will enable read/write decisions
    if (signals_readSignal(&signal_MREQ)) {
        // We will check for reads first as priority
        if (signals_readSignal(&signal_RD)) {
            // Read from the memory if it is in range
            memoryController_attemptRead(device);

        }
        else if (signals_readSignal(&signal_WR)) {
            // Write to the memory if it is in range
            memoryController_attemptWrite(device);
        }
    }
}

/********************************************************************

    MemoryController read functions

********************************************************************/

/*
If the address bus is set to a value in the range of the provided device, place the data at that address on the data bus
*/
void memoryController_attemptRead(MemoryDevice_t* device) {
    // Check if read is enabled
    if (!device->readEnable)
        return;

    // Get the effectiveAddress, and then test if it is in range
    int effectiveAddress = signal_addressBus - device->startOffset;
    if (effectiveAddress >= 0 && effectiveAddress < device->len) {
        // We are in range, put the value on the bus
        debug_printf("[MEMORY] Device read @ %04X(dev_add=%04X) of value %04X\n", signal_addressBus, effectiveAddress, device->data[effectiveAddress]);
        signal_dataBus = device->data[effectiveAddress];
    }
}

/********************************************************************

    MemoryController write functions

********************************************************************/

/*
If the address bus is set to a value in the range of the provided device, read the value off the data bus and save in memory
*/
void memoryController_attemptWrite(MemoryDevice_t* device) {
    // Check if write is enabled
    if (!device->writeEnable)
        return;

    // Get the effectiveAddress, and then test if it is in range
    int effectiveAddress = signal_addressBus - device->startOffset;
    if (effectiveAddress >= 0 && effectiveAddress < device->len) {
        // We are in range, store the value from the bus
        debug_printf("[MEMORY] Device write @ %04X(dev_add=%04X) of value %04X\n", signal_addressBus, effectiveAddress, device->data[effectiveAddress]);
        device->data[effectiveAddress] = signal_dataBus;
    }
}