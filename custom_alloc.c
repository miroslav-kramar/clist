#include <stdio.h>
#include "list.h"
#include "../salloc/salloc.h"

int main() {
    list_set_global_allocator(salloc, srealloc, sfree);

    List * l = new_list_int();

    for (int i = 0; i < 20; i++) {
        list_push_int(l, i);
    }

    for (size_t i = 0; i < list_length(l); i++) {
        printf("%d ", list_read_int(l, i));
    }
    putchar('\n');

    heap_dump();
    bitmap_dump();
    putchar('\n');

    delete_list(l);

    heap_dump();
    bitmap_dump();
    putchar('\n');
    return 0;
}