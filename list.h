#ifndef LIST_WORK_H
#define LIST_WORK_H

#include <stddef.h>

typedef enum {
    LIST_OK         = 0,
    LIST_NULL_PTR   = 1 << 0,
    LIST_BAD_INDEX  = 1 << 1,
    LIST_MEMORY_ERR = 1 << 2,
    LIST_CORRUPTED  = 1 << 3,
    LIST_NOT_FOUND  = 1 << 4
} list_error;

typedef struct Node{
    int value;
    int nextIndex;
} Node;

typedef struct List{
    Node *nodes;
    size_t capacity;
    int head;
    int freeHead;
    int size;
    list_error error;
} List;

void listCreate(List *list, size_t initCapacity);
int  listGrow(List *list);
void listDestroy(List *list);

int  allocateNode(List *list);
void releaseNode(List *list, int index);

int  insert(List *list, int pos, int value);
int  remove(List *list, int pos);
int  search(List *list, int value);
int  getLogicalIndex(List *list, int physicalIndex);

#endif
