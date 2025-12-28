#include <stdio.h>
#include "list.h"

int main(void) {
    List list = {0};

    listCreate(&list, 2);

    insert(&list, 0, 10);
    insert(&list, 1, 20);
    insert(&list, 2, 30);

    printf("size=%d capacity=%zu head=%d freeHead=%d\n",
           list.size, list.capacity, list.head, list.freeHead);

    int phys = search(&list, 20);
    printf("search 20 -> phys=%d\n", phys);

    int logical = getLogicalIndex(&list, phys);
    printf("logical index of phys=%d -> %d\n", phys, logical);

    remove(&list, 1);

    printf("after remove pos=1: size=%d head=%d\n", list.size, list.head);

    listDestroy(&list);
    return 0;
}
