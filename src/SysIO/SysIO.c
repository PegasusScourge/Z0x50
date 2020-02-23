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

#include "../Debug.h"
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
        debug_printf("Cannot open file '%s' with mode '%s': unable to allocate struct\n", path, mode);
        return NULL;
    }

    strcpy(file->path, path);

    // Time to open the file handle
    file->fPtr = fopen(path, mode);
    if (file->fPtr == NULL) {
        debug_printf("Cannot open file '%s' with mode '%s'\n", path, mode);
        return NULL;
    }

    // We have opened the file, get the size of the file
    if (fseek(file->fPtr, 0, SEEK_END) == 0)
        file->size = ftell(file->fPtr) + 1; // Make room for a '\0' character in caching
    else
        file->size = -1;

    // Return the struct
    file->cached = false;
    debug_printf("File '%s' opened successfully\n", file->path);
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
    
    debug_printf("[SYS IO] Closing file '%s':", file->path);
    
    // Start deallocating the parts

    fprintf(DEBUG_OUT, " [DATA:");
    if (file->data != NULL) {
        free(file->data);
        fprintf(DEBUG_OUT, "OK]");
    }
    else {
        fprintf(DEBUG_OUT, "NULL]");
    }

    // Close the file stream
    fprintf(DEBUG_OUT, " [FPTR:");
    if (file->fPtr != NULL) {
        fclose(file->fPtr);
        fprintf(DEBUG_OUT, "OK]");
    }
    else {
        fprintf(DEBUG_OUT, "NULL]");
    }

    // Deallocate the struct
    free(file);
    fprintf(DEBUG_OUT, " [CLOSED]\n");
}

/*
Cache the file data into file->data
*/
void sysIO_cacheFile(SysFile_t* file) {
    // Check for file validity
    if (file == NULL) {
        debug_printf("Unable to cache file content: file struct ptr is null!\n");
        return;
    }

    // Check that we have a file handle
    if (file->fPtr == NULL) {
        debug_printf("Unable to cache file content: file handle ptr is null!\n");
        return;
    }

    // Check we have a size quantity
    if (file->size <= 0) {
        debug_printf("Unable to cache file content of file '%s': could not find a length of file\n", file->path);
        return;
    }

    // Allocate space for the file to be read into
    file->data = calloc(file->size, sizeof(char));
    if (file->data == NULL) {
        debug_printf("Unable to cache file content of file '%s': could not allocate memory\n", file->path);
        return;
    }

    // Time to read the file data in
    // Seek to the beginning of the file
    fseek(file->fPtr, 0, SEEK_SET);
    // Begin reading the information
    int i = 0;
    char c = 0;
    while ((c = fgetc(file->fPtr)) != EOF && i < file->size) {
        file->data[i] = c;
        i++;
    }

    // Place the null terminator
    file->data[file->size - 1] = '\0';

    // File read complete!
    debug_printf("Cached file '%s' of size %i bytes (finished at byte %i)\n", file->path, file->size, ftell(file->fPtr));
    file->cached = true;
}