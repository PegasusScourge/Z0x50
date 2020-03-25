#pragma once
/*

StringUtil

String utility files
Created by Matthew Clarke

*/

#include <stdlib.h>
#include <stdint.h>

/********************************************************************

    String Functions

********************************************************************/

char* sutil_trim(char* str, const char* seps);
int sutil_split(char* b, size_t bLen, char** splits, size_t splitsLen, const char* token);
void sutil_byteToBinary(uint8_t byte, char* buffer, int bufferLen);