/*

 _____   ____         ______ ____
/__  /  / __ \ _  __ / ____// __ \
  / /  / / / /| |/_//___ \ / / / /
 / /__/ /_/ /_>  < ____/ // /_/ /
/____/\____//_/|_|/_____/ \____/

Zilog 80 Emulator

Basic interface to the Z80 processor and associated modules.
Can be run as a Sinclair ZX Spectrum or used as a basis for a larger project.

LinkedList.c : Provides a linked list

*/

#include "LinkedList.h"
#include "../SysIO/Log.h"

/********************************************************************

    LinkedList functions

********************************************************************/

/*
Initialises a linked list object
*/
LinkedList_t* linkedList_init() {
    LinkedList_t* list = NULL;

    list = malloc(sizeof(LinkedList_t));
    if (list == NULL) {
        // Allocation failed
        formattedLog(stdlog, LOGTYPE_ERROR, "linkedList_init() failed to allocate for a linked list object\n");
        return NULL;
    }

    // Linked list is initialised
    list->numberOfElements = 0;
    list->root = NULL;

    return list;
}

/*
Destroys a linked list object
*/
void linkedList_destroy(LinkedList_t* list) {
    if (list == NULL) {
        formattedLog(stdlog, LOGTYPE_WARN, "Attempted to destroy null linked list\n");
        return;
    }

    // Traverse the tree and get the next element and free it
    ListElement_t* element = list->root;
    ListElement_t* next = NULL;
    formattedLog(debuglog, LOGTYPE_DEBUG, "-- Freeing linked list --\n");
    while (element != NULL) {
        next = element->next;

        // Free element and it's data
        if (element->d)
            free(element->d);
        listElement_destroy(element);

        directLog(debuglog, "Element free\n");
        // Align next element
        element = next;
    }

    // Free the list holder
    free(list);

    directLog(debuglog, "-- Finished free of linked list --\n");
}

/*
Counts the number of elements in the list and updates the internal count
*/
void linkedList_countElements(LinkedList_t* list) {
    // formattedLog(debuglog, LOGTYPE_DEBUG, __FUNCTION__" checking list\n");
    if (list == NULL) {
        formattedLog(stdlog, LOGTYPE_WARN, "Attempted to count null linked list\n");
        return;
    }

    // formattedLog(debuglog, LOGTYPE_DEBUG, __FUNCTION__" clear list count\n");
    list->numberOfElements = 0;

    ListElement_t* next = list->root;
    if (next == NULL) {
        // formattedLog(debuglog, LOGTYPE_DEBUG, __FUNCTION__" next=%p, numElements=%i\n", next, list->numberOfElements);
        return;
    }

    do {
        list->numberOfElements++;
        // formattedLog(debuglog, LOGTYPE_DEBUG, __FUNCTION__" next=%p, numElements=%i\n", next, list->numberOfElements);
    } while ((next = next->next) != NULL);
}

/*
Returns the number of elements in the list after updating the count
*/
int linkedList_numberOfElements(LinkedList_t* list) {
    // formattedLog(debuglog, LOGTYPE_DEBUG, __FUNCTION__" checking list\n");
    if (list == NULL) {
        formattedLog(stdlog, LOGTYPE_WARN, "Attempted to get null linked list element count\n");
        return -1;
    }

    // formattedLog(debuglog, LOGTYPE_DEBUG, __FUNCTION__" call element count\n");
    linkedList_countElements(list);
    // formattedLog(debuglog, LOGTYPE_DEBUG, __FUNCTION__" return elements counted\n");
    return list->numberOfElements;
}

/*
Pushes an element to the beginning of the list
*/
void linkedList_push(LinkedList_t* list, void* data, size_t dataSize) {
    if (list == NULL) {
        formattedLog(stdlog, LOGTYPE_WARN, "Attempted to push to null list\n");
        return;
    }

    ListElement_t* newRoot = listElement_create(data, dataSize);
    if (newRoot == NULL) {
        formattedLog(stdlog, LOGTYPE_ERROR, "Attempted to push null newRoot to list: failed to get valid list element\n");
        return;
    }

    newRoot->next = list->root;
    list->root = newRoot;
}

/*
Pops the top element off the list
*/
void* linkedList_pop(LinkedList_t* list) {
    if (list == NULL) {
        formattedLog(stdlog, LOGTYPE_WARN, "Attempted to pop from null list\n");
        return NULL;
    }

    if (list->root == NULL) {
        formattedLog(stdlog, LOGTYPE_WARN, "Attempted to pop null root\n");
        return NULL;
    }

    // Remove the root
    ListElement_t* oldRoot = list->root;
    void* data = oldRoot->d;
    list->root = oldRoot->next;

    // Destroy the old root
    listElement_destroy(oldRoot);

    // Return the data
    return data;
}

/*
Puts an element in at the specified index, and returns the index it was inserted at
*/
int linkedList_insertAt(LinkedList_t* list, int index, void* data, size_t dataSize) {
    if (list == NULL) {
        formattedLog(stdlog, LOGTYPE_WARN, "Attempted to insertAt null list\n");
        return INSERT_FAILED;
    }

    linkedList_countElements(list);
    if (index == INDEX_END) {
        // Calculate the end of the list and modify the index value
        index = list->numberOfElements - 1;
    }

    // Now perform the addition
    ListElement_t* element = list->root;

    int i;
    for (i = 0; i <= index && i < list->numberOfElements; i++) {
        if (element->next == NULL)
            break;
        element = element->next;
    }

    if (element->next == NULL) {
        // Simply enter the element here
        element->next = listElement_create(data, dataSize);
        if (element->next == NULL) {
            formattedLog(stdlog, LOGTYPE_WARN, "Attempted insertAt index %i of linked list, failed\n", i);
            return INSERT_FAILED;
        }
    }
    else {
        ListElement_t* newElement = listElement_create(data, dataSize);
        if (newElement == NULL) {
            formattedLog(stdlog, LOGTYPE_WARN, "Attempted insertAt index %i of linked list, failed\n", i);
            return INSERT_FAILED;
        }
        newElement->next = element->next;
        element->next = newElement;
    }

    return i;
}

/*
Removes the element at the specified index, if it exists
*/
void* linkedList_removeFrom(LinkedList_t* list, int index) {
    if (list == NULL) {
        formattedLog(stdlog, LOGTYPE_WARN, "Attempted to removeFrom null list\n");
        return NULL;
    }

    linkedList_countElements(list);
    if (index == INDEX_END) {
        // Calculate the end of the list and modify the index value
        index = list->numberOfElements - 1;
    }

    ListElement_t* previousElement = NULL;
    ListElement_t* element = list->root;
    int i;
    for (i = 0; i <= index && i < list->numberOfElements; i++) {
        if (element->next == NULL)
            break;
        previousElement = element;
        element = element->next;
    }

    // Remove the element
    void* data = element->d;
    if (previousElement == NULL) {
        list->root = element->next;
    }
    else {
        previousElement->next = element->next;
    }

    listElement_destroy(element);
    return data;
}

/********************************************************************

    ListElement functions

********************************************************************/

/*
Creates a list element and puts the data into it
*/
ListElement_t* listElement_create(void* data, size_t dataSize) {
    ListElement_t* element = NULL;
    element = malloc(sizeof(ListElement_t));
    if (element == NULL) {
        formattedLog(stdlog, LOGTYPE_WARN, "Could not allocate for ListElement_t\n");
        return NULL;
    }

    // Link the data pointer here
    element->d = data;
    element->dSize = dataSize;
    element->next = NULL;

    return element;
}

/*
Destroys a list element. DOES NOT FREE THE DATA INSIDE IT
*/
void listElement_destroy(ListElement_t* element) {
    if (element == NULL) {
        formattedLog(stdlog, LOGTYPE_WARN, "Attempted to destroy null list element\n");
        return;
    }

    // Free the element
    free(element);
}