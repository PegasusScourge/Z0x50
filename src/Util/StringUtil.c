/*

StringUtil

String utility files
Created by Matthew Clarke

*/

#include "StringUtil.h"
#include <string.h>

/********************************************************************

    String Functions

********************************************************************/

/*
Part of the sutil_trim() function
*/
char* sutil_ltrim(char* str, const char* seps) {
    size_t totrim;
    if (seps == NULL) {
        seps = "\t\n\v\f\r ";
    }
    totrim = strspn(str, seps);
    if (totrim > 0) {
        size_t len = strlen(str);
        if (totrim == len) {
            str[0] = '\0';
        }
        else {
            memmove(str, str + totrim, len + 1 - totrim);
        }
    }
    return str;
}

/*
Part of the sutil_trim() function
*/
char* sutil_rtrim(char* str, const char* seps) {
    int i;
    if (seps == NULL) {
        seps = "\t\n\v\f\r ";
    }
    i = (int)strlen(str) - 1;
    while (i >= 0 && strchr(seps, str[i]) != NULL) {
        str[i] = '\0';
        i--;
    }
    return str;
}

/*
Takes a string char* along with characters to trim off the front and back of the string. If seps is NULL, these are provided automatically
Returns the trimmed string
*/
char* sutil_trim(char* str, const char* seps) {
    return sutil_ltrim(sutil_rtrim(str, seps), seps);
}

int sutil_split(char* b, size_t bLen, char** splits, size_t splitsLen, const char* token) {
    // Prepare the variables to capture the lines
    char* line;
    int linesDetected = 0;

    // Get the first line
    line = strtok(b, token);
    while (line != NULL && linesDetected < splitsLen) {
        // Save the current pointer
        splits[linesDetected] = sutil_trim(line, NULL);

        // Get the next line
        line = strtok(NULL, token);
        linesDetected++;
    }

    return linesDetected;
}

void sutil_byteToBinary(uint8_t byte, char* buffer, int bufferLen) {
    for (int i = 7, j = 0; i >= 0 && j < bufferLen - 1; i--, j++) {
        int value = (byte >> i) & 0x1;
        if (value)
            buffer[j] = '1';
        else
            buffer[j] = '0';
    }
    buffer[bufferLen - 1] = '\0';
}