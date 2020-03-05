/*

 _____   ____         ______ ____
/__  /  / __ \ _  __ / ____// __ \
  / /  / / / /| |/_//___ \ / / / /
 / /__/ /_/ /_>  < ____/ // /_/ /
/____/\____//_/|_|/_____/ \____/

Zilog 80 Emulator

Basic interface to the Z80 processor and associated modules.
Can be run as a Sinclair ZX Spectrum or used as a basis for a larger project.

SysIO.c : System I/O for the filesystem of the host.

*/

#include "Log.h"
#include "SysIO.h"

/********************************************************************

    SysIO functions

********************************************************************/

/*
Opens a file and places it in a struct
*/
SysFile_t* sysIO_openFile(const char* path) {
    const char* mode = "r+";

    SysFile_t* file = malloc(sizeof(SysFile_t));
    // Check we got the struct made
    if (file == NULL) {
        formattedLog(stdlog, LOGTYPE_ERROR, "Cannot open file '%s' with mode '%s': unable to allocate struct\n", path, mode);
        return NULL;
    }

    strcpy(file->path, path);

    // Time to open the file handle
    file->fPtr = fopen(path, mode);
    if (file->fPtr == NULL) {
        formattedLog(stdlog, LOGTYPE_ERROR, "Cannot open file '%s' with mode '%s'\n", path, mode);
        free(file);
        return NULL;
    }

    // We have opened the file, get the size of the file
    if (fseek(file->fPtr, 0, SEEK_END) == 0)
        file->size = ftell(file->fPtr) + 1; // Make room for a '\0' character in caching
    else
        file->size = -1;

    // Return the struct
    file->cached = false;
    formattedLog(debuglog, LOGTYPE_MSG, "File '%s' opened successfully\n", file->path);
    return file;
}

/*
Close a file represented by a struct
*/
void sysIO_closeFile(SysFile_t* file) {
    // Check the file struct is value
    if (file == NULL) {
        return;
    }
    
    formattedLog(debuglog, LOGTYPE_DEBUG, "[SYS IO] Closing file '%s':", file->path);
    
    // Start deallocating the parts

    directLog(debuglog, " [DATA:");
    if (file->data != NULL) {
        free(file->data);
        directLog(debuglog, "OK]");
    }
    else {
        directLog(debuglog, "NULL]");
    }

    // Close the file stream
    directLog(debuglog, " [FPTR:");
    if (file->fPtr != NULL) {
        fclose(file->fPtr);
        directLog(debuglog, "OK]");
    }
    else {
        directLog(debuglog, "NULL]");
    }

    // Deallocate the struct
    free(file);
    directLog(debuglog, " [CLOSED]\n");
}

/*
Cache the file data into file->data
*/
void sysIO_cacheFile(SysFile_t* file) {
    // Check for file validity
    if (file == NULL) {
        formattedLog(stdlog, LOGTYPE_ERROR, "Unable to cache file content: file struct ptr is null!\n");
        return;
    }

    // Check that we have a file handle
    if (file->fPtr == NULL) {
        formattedLog(stdlog, LOGTYPE_ERROR, "Unable to cache file content: file handle ptr is null!\n");
        return;
    }

    // Check we have a size quantity
    if (file->size <= 0) {
        formattedLog(stdlog, LOGTYPE_ERROR, "Unable to cache file content of file '%s': could not find a length of file\n", file->path);
        return;
    }

    // Allocate space for the file to be read into
    file->data = calloc(file->size, sizeof(char));
    if (file->data == NULL) {
        formattedLog(stdlog, LOGTYPE_ERROR, "Unable to cache file content of file '%s': could not allocate memory\n", file->path);
        return;
    }

    // Time to read the file data in
    // Seek to the beginning of the file
    rewind(file->fPtr);
    // Begin reading the information

    /*
    int numX = 32;
    int cX = 0;
    int i = 0;
    int c = 0;
    directLog(debuglog, "\n[Cache | Begin File : Size %04X | Values in Hex]\n[%04X]", file->size, i);
    while (!feof(file->fPtr) && i < file->size) {
        c = fgetc(file->fPtr);
    // while (i < file->size) { c = fgetc(file->fPtr);
        file->data[i] = (unsigned char)c;
        directLog(debuglog, " %02X", file->data[i]);
        cX++;
        i++;
        if (cX >= numX) {
            cX = 0;
            directLog(debuglog, "\n[%04X]", i);
        }
    }
    directLog(debuglog, "\n[End File]\n\n");
    */
    fread(file->data, file->size, 1, file->fPtr);

    // Place the null terminator
    file->data[file->size - 1] = '\0';

    // Check for an EOF failure
    //if (c == EOF) {
    //    formattedLog(stdlog, LOGTYPE_ERROR, "File cache of file '%s' failed with EOF at i=0x%02X\n", file->path, i - 1);
    //}

    // File read complete!
    formattedLog(debuglog, LOGTYPE_DEBUG, "Cached file '%s' of size %i bytes (finished at byte %i)\n", file->path, file->size, ftell(file->fPtr));
    file->cached = true;
}