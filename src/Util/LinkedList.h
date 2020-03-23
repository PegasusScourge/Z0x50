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

LinkedList.h : Provides a linked list

*/

#include <stdlib.h>

#define INDEX_END -1
#define INDEX_BEGINNING 0

#define INSERT_FAILED -2

typedef struct ListElement {
    void* d;
    size_t dSize;
    struct ListElement* next;
} ListElement_t;

typedef struct LinkedList {
    ListElement_t* root;
    int numberOfElements;
} LinkedList_t;

/********************************************************************

    LinkedList macros

********************************************************************/

/*

Define as below:

LinkedList_t* listOfErrors = NULL;
#define listOfErrors_DTYPE DecompError_t
#define listOfErrors_DSIZE sizeof(DecompError_t)

Use in 'type' of macros

*/

// Casts the value we have to the type
#define linkedList_asType(type, b) (type)b

/********************************************************************

    LinkedList functions

********************************************************************/

LinkedList_t* linkedList_init();
void linkedList_destroy(LinkedList_t* list);

void linkedList_countElements(LinkedList_t* list);
int linkedList_numberOfElements(LinkedList_t* list);

void linkedList_push(LinkedList_t* list, void* data, size_t dataSize);
void* linkedList_pop(LinkedList_t* list);

int linkedList_insertAt(LinkedList_t* list, int index, void* data, size_t dataSize);
void* linkedList_removeFrom(LinkedList_t* list, int index);

/********************************************************************

    ListElement functions

********************************************************************/

ListElement_t* listElement_create(void* data, size_t dataSize);
void listElement_destroy(ListElement_t* element);