/*

StringUtil

String utility files
Created by Matthew Clarke

*/

#include "StringUtil.h"
#include <stdlib.h>
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