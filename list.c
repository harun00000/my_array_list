#include <stdio.h>
#include <stdlib.h>
#include "list.h"

static void listAbort(List *list, list_error err) {
    if (list != NULL) {
        list->error |= err;

        free(list->nodes);
        list->nodes = NULL;

        list->capacity = 0;
        list->size = 0;
        list->head = 0;
        list->freeHead = 0;
    }

    printf("LIST ABORT: error=%d\n", (int)err);
    exit((int)err ? (int)err : 1);
}

#define ASSERT_LIST(cond, err_code, list) \
    do { \
        if (!(cond)) { \
            listAbort((list), (err_code)); \
        } \
    } while (0)

void listCreate(List *list, size_t initCapacity){
    ASSERT_LIST(list != NULL, LIST_NULL_PTR, list);

    list->error = LIST_OK;

    list->nodes = NULL;
    list->capacity = 0;
    list->size = 0;
    list->head = 0;
    list->freeHead = 0;

    if (initCapacity < 1) {
        ASSERT_LIST(0, LIST_BAD_INDEX, list);
    }

    list->nodes = (Node *)calloc(initCapacity + 1, sizeof(Node));
    ASSERT_LIST(list->nodes != NULL, LIST_MEMORY_ERR, list);

    list->capacity = initCapacity;
    list->size = 0;
    list->head = 0;
    list->freeHead = 1;

    int cap = (int)list->capacity;
    for (int idx = 1; idx <= cap; idx++) {
        if (idx == cap) {
            list->nodes[idx].nextIndex = 0;
        } else {
            list->nodes[idx].nextIndex = idx + 1;
        }
        list->nodes[idx].value = 0;
    }
}

int listGrow(List *list) {
    ASSERT_LIST(list != NULL, LIST_NULL_PTR, list);
    ASSERT_LIST(list->nodes != NULL, LIST_MEMORY_ERR, list);

    size_t oldCapacity = list->capacity;
    size_t newCapacity = (oldCapacity == 0) ? 1 : oldCapacity * 2;

    Node *newNodes = (Node *)realloc(list->nodes, (newCapacity + 1) * sizeof(Node));
    ASSERT_LIST(newNodes != NULL, LIST_MEMORY_ERR, list);

    list->nodes = newNodes;

    for (int idx = (int)oldCapacity + 1; idx <= (int)newCapacity; idx++) {
        if ((size_t)idx == newCapacity) {
            list->nodes[idx].nextIndex = list->freeHead;
        } else {
            list->nodes[idx].nextIndex = idx + 1;
        }
        list->nodes[idx].value = 0;
    }

    list->freeHead = (int)oldCapacity + 1;
    list->capacity = newCapacity;

    return 1;
}

void listDestroy(List *list){
    ASSERT_LIST(list != NULL, LIST_NULL_PTR, list);

    free(list->nodes);
    list->nodes = NULL;
    list->capacity = 0;
    list->size = 0;
    list->head = 0;
    list->freeHead = 0;
    list->error = LIST_OK;
}

int allocateNode(List *list){
    ASSERT_LIST(list != NULL, LIST_NULL_PTR, list);
    ASSERT_LIST(list->nodes != NULL, LIST_MEMORY_ERR, list);

    if (list->freeHead == 0){
        int ok = listGrow(list);
        ASSERT_LIST(ok != 0, LIST_MEMORY_ERR, list);
    }

    int idx = list->freeHead;
    ASSERT_LIST(idx > 0 && (size_t)idx <= list->capacity, LIST_CORRUPTED, list);

    list->freeHead = list->nodes[idx].nextIndex;
    list->nodes[idx].nextIndex = 0;

    return idx;
}

void releaseNode(List *list, int index) {
    ASSERT_LIST(list != NULL, LIST_NULL_PTR, list);
    ASSERT_LIST(list->nodes != NULL, LIST_MEMORY_ERR, list);
    ASSERT_LIST(index > 0 && (size_t)index <= list->capacity, LIST_BAD_INDEX, list);

    list->nodes[index].nextIndex = list->freeHead;
    list->freeHead = index;
}

int insert(List *list, int pos, int value){
    ASSERT_LIST(list != NULL, LIST_NULL_PTR, list);
    ASSERT_LIST(list->nodes != NULL, LIST_MEMORY_ERR, list);
    ASSERT_LIST(pos >= 0 && pos <= list->size, LIST_BAD_INDEX, list);

    int newIndex = allocateNode(list);
    ASSERT_LIST(newIndex != 0, LIST_MEMORY_ERR, list);

    list->nodes[newIndex].value = value;

    if (pos == 0){
        list->nodes[newIndex].nextIndex = list->head;
        list->head = newIndex;
    } else{
        int prevIndex = list->head;
        ASSERT_LIST(prevIndex != 0, LIST_CORRUPTED, list);

        for (int idx = 0; idx < pos - 1; idx++){
            prevIndex = list->nodes[prevIndex].nextIndex;
            ASSERT_LIST(prevIndex != 0, LIST_CORRUPTED, list);
        }

        list->nodes[newIndex].nextIndex = list->nodes[prevIndex].nextIndex;
        list->nodes[prevIndex].nextIndex = newIndex;
    }

    list->size++;
    return newIndex;
}

int remove(List *list, int pos){
    ASSERT_LIST(list != NULL, LIST_NULL_PTR, list);
    ASSERT_LIST(list->nodes != NULL, LIST_MEMORY_ERR, list);
    ASSERT_LIST(pos >= 0 && pos < list->size, LIST_BAD_INDEX, list);
    ASSERT_LIST(list->head != 0, LIST_BAD_INDEX, list);

    int deletedIndex = 0;

    if (pos == 0){
        deletedIndex = list->head;
        list->head = list->nodes[deletedIndex].nextIndex;
    } else{
        int prevIndex = list->head;
        ASSERT_LIST(prevIndex != 0, LIST_CORRUPTED, list);

        for (int idx = 0; idx < pos - 1; idx++){
            prevIndex = list->nodes[prevIndex].nextIndex;
            ASSERT_LIST(prevIndex != 0, LIST_CORRUPTED, list);
        }

        deletedIndex = list->nodes[prevIndex].nextIndex;
        ASSERT_LIST(deletedIndex != 0, LIST_CORRUPTED, list);

        list->nodes[prevIndex].nextIndex = list->nodes[deletedIndex].nextIndex;
    }

    list->nodes[deletedIndex].value = 0;
    list->nodes[deletedIndex].nextIndex = 0;

    releaseNode(list, deletedIndex);
    list->size--;

    return 1;
}

int search(List *list, int value){
    ASSERT_LIST(list != NULL, LIST_NULL_PTR, list);
    ASSERT_LIST(list->nodes != NULL, LIST_MEMORY_ERR, list);

    int cur = list->head;

    while (cur != 0){
        if (list->nodes[cur].value == value){
            return cur;
        }
        cur = list->nodes[cur].nextIndex;
    }

    ASSERT_LIST(0, LIST_NOT_FOUND, list);
    return 0;
}

int getLogicalIndex(List *list, int physicalIndex){
    ASSERT_LIST(list != NULL, LIST_NULL_PTR, list);
    ASSERT_LIST(list->nodes != NULL, LIST_MEMORY_ERR, list);

    ASSERT_LIST(physicalIndex > 0 && (size_t)physicalIndex <= list->capacity,
                LIST_BAD_INDEX, list);

    int cur = list->head;
    int logicalIndex = 0;

    while (cur != 0){
        if (cur == physicalIndex){
            return logicalIndex;
        }

        cur = list->nodes[cur].nextIndex;
        logicalIndex++;
    }

    ASSERT_LIST(0, LIST_NOT_FOUND, list);
    return -1;
}
