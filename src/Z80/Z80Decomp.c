/*

 _____   ____         ______ ____
/__  /  / __ \ _  __ / ____// __ \
  / /  / / / /| |/_//___ \ / / / /
 / /__/ /_/ /_>  < ____/ // /_/ /
/____/\____//_/|_|/_____/ \____/

Zilog 80 Emulator

Basic interface to the Z80 processor and associated modules.
Can be run as a Sinclair ZX Spectrum or used as a basis for a larger project.

Z0xDecomp.c : Handles decompilation

*/

#include "Z80Decomp.h"
#include "Z80Instructions.h"
#include "../SysIO/Log.h"
#include "../Util/LinkedList.h"

FILE* decompLog = NULL;
SysFile_t* dF = NULL;

#define decompLog(fmt, ...) if(decompLog) { fprintf(decompLog, fmt, __VA_ARGS__); } printf("[DECOMP] "); printf(fmt, __VA_ARGS__);
#define decompL(fmt, ...) if(decompLog) { fprintf(decompLog, fmt, __VA_ARGS__); } printf(fmt, __VA_ARGS__);

void decomp_addError(int index, const char* errorComment);
void decomp_printErrors();

uint16_t prefix;
uint16_t operand;
uint8_t instruction;
int currentIndex = 0;
int instrByteLen;
int instrNumOperands;
const char* instrHumanString;
const int (*funcPointer)();

typedef struct DecompError {
    uint16_t prefix;
    uint8_t instruction;
    uint16_t operand;
    int index;
    const char* humanString;
    const char* errorComment;
} DecompError_t;

LinkedList_t* listOfErrors = NULL;
#define listOfErrors_DTYPE DecompError_t*
#define listOfErrors_DSIZE sizeof(DecompError_t)

void decomp_init(SysFile_t* file) {
    if (file == NULL) {
        formattedLog(stdlog, LOGTYPE_ERROR, "Failed to get a valid file: NULL ptr\n");
        return;
    }
    if (!file->cached) {
        formattedLog(stdlog, LOGTYPE_ERROR, "Failed to get a valid file: no cache\n");
        return;
    }
    dF = file;

    // Initialise the log
    decompLog = fopen("decompilation.log", "w");
    if (decompLog == NULL) {
        // Throw an error
        formattedLog(stdlog, LOGTYPE_ERROR, "Failed to create log file '" "decompilation.log" "'\n");
    }

    // Output the file in HEX into terminal
    log_dumpHexToDebug(32, dF);

    decompLog("Init decompilation of file complete\n");
    currentIndex = 0;

    // Init the linked list
    listOfErrors = linkedList_init();
    if (listOfErrors == NULL) {
        formattedLog(stdlog, LOGTYPE_ERROR, "Init Error Tracking: [FAIL]\n");
    }
    else {
        decompLog("Init Error Tracking: [OK]\n");
    }
}

void decomp_finalise() {
    decomp_printErrors();

    if (listOfErrors) {
        linkedList_destroy(listOfErrors);
        decompLog("Finalised Error Tracking: [OK]\n");
    }
    else {
        decompLog("Finalised Error Tracking: [FAIL]\n");
    }
}

void decomp_addError(int index, const char* errorComment) {
    if (listOfErrors == NULL) {
        formattedLog(stdlog, LOGTYPE_ERROR, "Attempted to add error during decompilation at index %04X, but failed\n", index);
        return;
    }

    DecompError_t* err = malloc(sizeof(DecompError_t));
    if (err == NULL) {
        // Throw error
        formattedLog(stdlog, LOGTYPE_ERROR, "Attempted to add error during decompilation at index %04X, but failed\n", index);
        return;
    }

    // Configure and add to list
    err->humanString = instrHumanString;
    err->instruction = instruction;
    err->operand = operand;
    err->prefix = prefix;
    err->index = index;
    if (errorComment)
        err->errorComment = errorComment;
    else
        err->errorComment = NULL;

    linkedList_push(listOfErrors, err, listOfErrors_DSIZE);
}

void decomp_printErrors() {
    if (listOfErrors == NULL) {
        formattedLog(stdlog, LOGTYPE_ERROR, "Attempted to print content of LinkedList listOfErrors, but failed\n");
        return;
    }

    decompLog("\n--- Decompilation Errors: %i ---\n", linkedList_numberOfElements(listOfErrors));

    while (linkedList_numberOfElements(listOfErrors) > 0) {
        void* v = linkedList_pop(listOfErrors);
        if (v == NULL) {
            decompLog("NULL ptr\n");
        }

        DecompError_t* err = linkedList_asType(listOfErrors_DTYPE, v);
        if (err == NULL) {
            decompLog("NULL error\n");
        }
        else {
            decompLog("[%04X] |%04X|%02X| $%04X, %s: %s\n", err->index, err->prefix, err->instruction, err->operand, err->humanString, err->errorComment);

            // Now free the error, as it wasn't freed when it was popped off the list
            free(err);
        }
    }

    decompLog("--- End Errors --\n");
}

void decomp_deriveInstructionInformation() {
    if (prefix == 0) {
        instrByteLen = instructions_mainInstructionParams[instruction];
        instrNumOperands = instrByteLen > 1 ? instrByteLen - 1 : 0;
        instrHumanString = instructions_mainInstructionText[instruction];
        funcPointer = instructions_mainInstructionFuncs[instruction];

        return;
    }

    // Check to see if we have a prefix
    if (prefix > 0) {
        switch (prefix) {
        case PREFIX_BITS:
            instrByteLen = instructions_bitInstructionParams[instruction];
            instrNumOperands = instrByteLen > 2 ? instrByteLen - 2 : 0;
            instrHumanString = instructions_bitInstructionText[instruction];
            funcPointer = instructions_bitInstructionFuncs[instruction];
            break;
        case PREFIX_IY:
            instrByteLen = instructions_IYInstructionParams[instruction];
            instrNumOperands = instrByteLen > 2 ? instrByteLen - 2 : 0;
            instrHumanString = instructions_IYInstructionText[instruction];
            funcPointer = instructions_IYInstructionFuncs[instruction];
            break;
        case PREFIX_EXX:
            // decompLog("Unhandled prefix code: %04X, %s\n", prefix, instructions_mainInstructionText[prefix]);
            // instrByteLen = -2;
            instrByteLen = instructions_extendedInstructionParams[instruction];
            instrNumOperands = instrByteLen > 2 ? instrByteLen - 2 : 0;
            instrHumanString = instructions_extendedInstructionText[instruction];
            funcPointer = instructions_extendedInstructionFuncs[instruction];
            break;
        case PREFIX_IX:
            instrByteLen = instructions_IXInstructionParams[instruction];
            instrNumOperands = instrByteLen > 2 ? instrByteLen - 2 : 0;
            instrHumanString = instructions_IXInstructionText[instruction];
            funcPointer = instructions_IXInstructionFuncs[instruction];
            break;
        case PREFIX_IX_BITS:
            decompLog("Unhandled prefix code: %04X\n", prefix);
            instrByteLen = -2;
            break;
        case PREFIX_IY_BITS:
            instrByteLen = instructions_IYBitInstructionParams[instruction];
            instrNumOperands = instrByteLen > 3 ? instrByteLen - 3 : 0;
            instrHumanString = instructions_IYBitInstructionText[instruction];
            funcPointer = instructions_IYBitInstructionFuncs[instruction];
            break;
        default:
            decompLog("Unknown prefix code: %04X, %s\n", prefix, instructions_mainInstructionText[prefix]);
            instrByteLen = -2;
            break;
        }
    }
}

bool decomp_next() {
    // Check for currentIndex out of bounds
    if (currentIndex >= dF->size && decompLog != NULL) {
        decompLog("End decompilation\n");
        return false;
    }

    // Clear the prefix cache
    prefix = 0;

    instruction = dF->data[currentIndex];

    // Check for the function information
    decomp_deriveInstructionInformation();

    int startIndex = currentIndex;

    while (true) {
        // Handle the extension opcodes
        if (instrByteLen == -1) {
            // Move forward one index
            currentIndex++;
            // Recalculate the information, we need to derive from the correct instruction set so we update the instruction first
            if (prefix == 0)
                prefix = instruction;
            else
                prefix = (prefix << 8) | instruction;

            instruction = dF->data[currentIndex];
            decomp_deriveInstructionInformation();
            // break;
        }
        else {
            // Break out here
            break;
        }
    }

    if (prefix > 0) {
        decompLog("[%04X] |%04X|%02X|    %s ", startIndex, prefix, instruction, instrHumanString);
    }
    else {
        decompLog("[%04X] |%02X|         %s ", startIndex, instruction, instrHumanString);
    }

    if (instrNumOperands > 0) {
        // Display the operands for the instruction
        if (instrNumOperands == 1) {
            // Display the number in correct notation
            decompL("$%02X", operand = dF->data[currentIndex + 1]);
        }
        else if (instrNumOperands == 2) {
            operand = (dF->data[currentIndex + 2] << 8) | dF->data[currentIndex + 1];
            // Display the number in correct notation
            decompL("$%04X", operand);
        }
        else {
            for (int i = 0; i < instrNumOperands; i++) {
                decompL("[%02X]", dF->data[currentIndex + i + 1]);
            }
        }
    }

    if (instrByteLen == 0) {
        decompL("   [WARN: instrByteLen == 0] ");
        // Add this as an error
        decomp_addError(startIndex, "instrByteLen == 0");
    }
    if (instrHumanString == NULL) {
        decompL("   [WARN: instrHumanString == NULL] ");
        decomp_addError(startIndex, "instrHumanString == NULL");
    }

    decompL("\n");
    
    // Check for failure conditions
    if (funcPointer == NULL || instrByteLen < 0) {
        decompLog("Decompilation failed! (func pointer == NULL: %i, instrByteLen = %i)\n", funcPointer == NULL, instrByteLen);
        decomp_addError(startIndex, "instrByteLen < 0 || funcPointer == NULL");
        return false;
    }
    currentIndex += instrByteLen;
    return true;
}