#include <stdio.h>
#include <stdlib.h>

typedef struct Node {
    int value;
    int nextIndex;   
} Node;

typedef struct List {
    Node *nodes;         
    size_t capacity;     
    int head;            
    int freeHead;        
    int size;            
} List;

int listCreate(List *list, size_t initCapacity) {
    if (list == NULL) return 0;
    if (initCapacity < 1) initCapacity = 1;

    list->nodes = (Node*)calloc(initCapacity + 1, sizeof(Node));
    if (list->nodes == NULL) {
        list->capacity = 0;
        list->head = 0;
        list->freeHead = 0;
        list->size = 0;
        return 0;
    }

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
    }
    return 1;
}

int listGrow(List *list) {
    if (list == NULL) return 0;

    size_t oldCapacity = list->capacity;
    size_t newCapacity = (oldCapacity == 0) ? 1 : oldCapacity * 2;

    Node *newNodes = (Node*)realloc(list->nodes, (newCapacity + 1) * sizeof(Node));
    if (newNodes == NULL) {
        return 0;
    }

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

void listDestroy(List *list) {
    if (list == NULL) return;

    free(list->nodes);
    list->nodes = NULL;
    list->capacity = 0;
    list->size = 0;
    list->head = 0;
    list->freeHead = 0;
}

int allocateNode(List *list) {
    if (list == NULL) return 0;

    if (list->freeHead == 0) {
        if (!listGrow(list)) {
            return 0;
        }
    }

    int idx = list->freeHead;
    list->freeHead = list->nodes[idx].nextIndex;
    list->nodes[idx].nextIndex = 0;
    return idx;
}

void releaseNode(List *list, int index) {
    if (list == NULL) return;
    if (index <= 0) return;
    if ((size_t)index > list->capacity) return;

    list->nodes[index].nextIndex = list->freeHead;
    list->freeHead = index;
}

int insert(List *list, int pos, int value) {
    if (list == NULL) return 0;
    if (pos < 0 || pos > list->size) return 0;

    int newIndex = allocateNode(list);
    if (newIndex == 0) return 0;

    list->nodes[newIndex].value = value;

    if (pos == 0) {
        list->nodes[newIndex].nextIndex = list->head;
        list->head = newIndex;
    } else {
        int prevIndex = list->head;
        for (int idx = 0; idx < pos - 1; idx++) {
            prevIndex = list->nodes[prevIndex].nextIndex;
        }

        list->nodes[newIndex].nextIndex = list->nodes[prevIndex].nextIndex;
        list->nodes[prevIndex].nextIndex = newIndex;
    }

    list->size++;
    return newIndex;
}

int removeAt(List *list, int pos) {
    if (list == NULL) return 0;
    if (pos < 0 || pos >= list->size) return 0;
    if (list->head == 0) return 0;

    int deletedIndex;

    if (pos == 0) {
        deletedIndex = list->head;
        list->head = list->nodes[deletedIndex].nextIndex;
    } else {
        int prevIndex = list->head;
        for (int idx = 0; idx < pos - 1; idx++) {
            prevIndex = list->nodes[prevIndex].nextIndex;
        }

        deletedIndex = list->nodes[prevIndex].nextIndex;
        list->nodes[prevIndex].nextIndex = list->nodes[deletedIndex].nextIndex;
    }

    list->nodes[deletedIndex].value = 0;
    list->nodes[deletedIndex].nextIndex = 0;

    releaseNode(list, deletedIndex);
    list->size--;
    return 1;
}

int search(const List *list, int value) {
    if (list == NULL || list->head == 0) return 0;

    int cur = list->head;
    while (cur != 0) {
        if (list->nodes[cur].value == value) {
            return cur;
        }
        cur = list->nodes[cur].nextIndex;
    }
    return 0;
}

int getLogicalIndex(const List *list, int physicalIndex) {
    if (list == NULL) return -1;
    if (list->head == 0) return -1;
    if (physicalIndex <= 0) return -1;
    if ((size_t)physicalIndex > list->capacity) return -1;

    int cur = list->head;
    int logicalIndex = 0;

    while (cur != 0) {
        if (cur == physicalIndex) {
            return logicalIndex;
        }
        cur = list->nodes[cur].nextIndex;
        logicalIndex++;
    }

    return -1;
}

void printList(const List *list) {
    if (list == NULL) {
        printf("List is NULL\n");
        return;
    }

    printf("LIST: size=%d head=%d\n", list->size, list->head);

    int cur = list->head;
    int logical = 0;
    while (cur != 0) {
        printf("  logical=%d phys=%d value=%d next=%d\n",
               logical, cur, list->nodes[cur].value, list->nodes[cur].nextIndex);
        cur = list->nodes[cur].nextIndex;
        logical++;
    }
}

void printFreeList(const List *list) {
    if (list == NULL) {
        printf("List is NULL\n");
        return;
    }

    printf("FREE: freeHead=%d capacity=%zu\n", list->freeHead, list->capacity);

    int cur = list->freeHead;
    while (cur != 0) {
        printf("  free phys=%d next=%d\n", cur, list->nodes[cur].nextIndex);
        cur = list->nodes[cur].nextIndex;
    }
}

/* ================== SIMPLE TEST MAIN ================== */

int main(void) {
    List list;

    if (!listCreate(&list, 2)) {
        printf("create failed\n");
        return 1;
    }

    // вставили 3 элемента (3-й проверит рост через realloc)
    insert(&list, 0, 10);
    insert(&list, 1, 20);
    insert(&list, 2, 30);

    // удалили середину (20)
    removeAt(&list, 1);

    // поиск: 20 уже не должно быть
    printf("%d\n", search(&list, 20)); // ожидаем 0

    listDestroy(&list);
    return 0;
}
