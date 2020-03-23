#pragma once
/*

StringUtil

String utility files
Created by Matthew Clarke

*/

#include <stdlib.h>

/********************************************************************

    String Functions

********************************************************************/

char* sutil_trim(char* str, const char* seps);
int sutil_split(char* b, size_t bLen, char** splits, size_t splitsLen, const char* token);