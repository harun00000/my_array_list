#include <stdio.h>
#include <stdlib.h>
#include "list.h"

#define SOFT_ASSERT_LIST(cond, err_code, list, action) \
    do { \
        if (!(cond)) { \
            (list)->error |= (err_code); \
            action; \
        } \
    } while (0)

void listCreate(List *list, size_t initCapacity){
    SOFT_ASSERT_LIST(list != NULL, LIST_NULL_PTR, list, return);

    if (initCapacity < 1){
        SOFT_ASSERT_LIST(0, LIST_BAD_INDEX, list, initCapacity = 1);
    }

    list->nodes = (Node *)calloc(initCapacity + 1, sizeof(Node));
    SOFT_ASSERT_LIST(list->nodes != NULL, LIST_MEMORY_ERR, list, return);

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
    SOFT_ASSERT_LIST(list != NULL, LIST_NULL_PTR, list, return 0);

    size_t oldCapacity = list->capacity;
    size_t newCapacity = (oldCapacity == 0) ? 1 : oldCapacity * 2;

    Node *newNodes = (Node *)realloc(list->nodes, (newCapacity + 1) * sizeof(Node));
    SOFT_ASSERT_LIST(newNodes != NULL, LIST_MEMORY_ERR, list, return 0);

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
    SOFT_ASSERT_LIST(list != NULL, LIST_NULL_PTR, list, return);

    free(list->nodes);
    list->nodes = NULL;
    list->capacity = 0;
    list->size = 0;
    list->head = 0;
    list->freeHead = 0;
}

int allocateNode(List *list){
    SOFT_ASSERT_LIST(list != NULL, LIST_NULL_PTR, list, return 0);

    if (list->freeHead == 0){
        int ok = listGrow(list);
        SOFT_ASSERT_LIST(ok != 0, LIST_MEMORY_ERR, list, return 0);
    }

    int idx = list->freeHead;
    SOFT_ASSERT_LIST(idx > 0 && (size_t)idx <= list->capacity, LIST_CORRUPTED, list, return 0);

    list->freeHead = list->nodes[idx].nextIndex;
    list->nodes[idx].nextIndex = 0;

    return idx;
}

void releaseNode(List *list, int index) {
    SOFT_ASSERT_LIST(list != NULL, LIST_NULL_PTR, list, return);
    SOFT_ASSERT_LIST(index > 0 && (size_t)index <= list->capacity, LIST_BAD_INDEX, list, return);

    list->nodes[index].nextIndex = list->freeHead;
    list->freeHead = index;
}

int insert(List *list, int pos, int value){
    SOFT_ASSERT_LIST(list != NULL, LIST_NULL_PTR, list, return 0);

    SOFT_ASSERT_LIST(pos >= 0 && pos <= list->size, LIST_BAD_INDEX, list, return 0);

    int newIndex = allocateNode(list);
    SOFT_ASSERT_LIST(newIndex != 0, LIST_MEMORY_ERR, list, return 0);

    list->nodes[newIndex].value = value;

    if (pos == 0){
        list->nodes[newIndex].nextIndex = list->head;
        list->head = newIndex;
    } else{
        int prevIndex = list->head;
        SOFT_ASSERT_LIST(prevIndex != 0, LIST_CORRUPTED, list, return 0);

        for (int idx = 0; idx < pos - 1; idx++){
            prevIndex = list->nodes[prevIndex].nextIndex;
            SOFT_ASSERT_LIST(prevIndex != 0, LIST_CORRUPTED, list, return 0);
        }

        list->nodes[newIndex].nextIndex = list->nodes[prevIndex].nextIndex;
        list->nodes[prevIndex].nextIndex = newIndex;
    }

    list->size++;
    return newIndex;
}

int remove(List *list, int pos){
    SOFT_ASSERT_LIST(list != NULL, LIST_NULL_PTR, list, return 0);

    SOFT_ASSERT_LIST(pos >= 0 && pos < list->size, LIST_BAD_INDEX, list, return 0);
    SOFT_ASSERT_LIST(list->head != 0, LIST_BAD_INDEX, list, return 0);

    int deletedIndex;

    if (pos == 0){
        deletedIndex = list->head;
        list->head = list->nodes[deletedIndex].nextIndex;
    } else{
        int prevIndex = list->head;
        SOFT_ASSERT_LIST(prevIndex != 0, LIST_CORRUPTED, list, return 0);

        for (int idx = 0; idx < pos - 1; idx++){
            prevIndex = list->nodes[prevIndex].nextIndex;
            SOFT_ASSERT_LIST(prevIndex != 0, LIST_CORRUPTED, list, return 0);
        }

        deletedIndex = list->nodes[prevIndex].nextIndex;
        SOFT_ASSERT_LIST(deletedIndex != 0, LIST_CORRUPTED, list, return 0);

        list->nodes[prevIndex].nextIndex = list->nodes[deletedIndex].nextIndex;
    }

    list->nodes[deletedIndex].value = 0;
    list->nodes[deletedIndex].nextIndex = 0;

    releaseNode(list, deletedIndex);
    list->size--;

    return 1;
}

int search(List *list, int value){
    SOFT_ASSERT_LIST(list != NULL, LIST_NULL_PTR, list, return 0);

    int cur = list->head;

    while (cur != 0){
        if (list->nodes[cur].value == value){
            return cur;
        }
        cur = list->nodes[cur].nextIndex;
    }

    SOFT_ASSERT_LIST(0, LIST_NOT_FOUND, list, return 0);
    return 0;
}

int getLogicalIndex(List *list, int physicalIndex){
    SOFT_ASSERT_LIST(list != NULL, LIST_NULL_PTR, list, return -1);

    SOFT_ASSERT_LIST(physicalIndex > 0 && (size_t)physicalIndex <= list->capacity,
                     LIST_BAD_INDEX, list, return -1);

    int cur = list->head;
    int logicalIndex = 0;

    while (cur != 0){
        if (cur == physicalIndex){
            return logicalIndex;
        }

        cur = list->nodes[cur].nextIndex;
        logicalIndex++;
    }

    SOFT_ASSERT_LIST(0, LIST_NOT_FOUND, list, return -1);
    return -1;
}
