#include <stdio.h>
#include "list.h"

void list_report(List * l) {
    fprintf(stderr, "List cap is %4zu, len is %4zu\n", list_capacity(l), list_length(l));
}

void print_list_int(List * l) {
    for (size_t i = 0; i < list_capacity(l); i++) {
        fprintf(stderr, "%d, ", list_read_int(l, i));
    }
    fputc('\n', stderr);
}

// void nested_list_destructor(void * ptr) {
//     delete_list(*(List **)ptr);
// }

// int main() {
//     List * l = new_list(sizeof(l), nested_list_destructor);

//     for (int i = 0; i < 10; i++) {
//         List * tmp_push = new_list_int();
//         list_push_ptr(l, tmp_push);
//         for (int j = 0; j < 10; j++) {
//             list_push_int(list_read_ptr(l, i), j);
//         }
//     }

//     for (size_t i = 0; i < list_length(l); i++) {
//         for (size_t j = 0; j < list_length(list_read_ptr(l, i)); j++) {
//             printf("%d ", list_read_int(list_read_ptr(l, i), j));
//         }
//         putchar('\n');
//     }

//     delete_list(l);
// }

// int main() {
//     List * l = new_list_int();

//     for (int i = 0; i < 20; i++) {
//         list_push_int(l, i);
//     }

//     int arr[10];
//     list_range_copy_array(l, arr, 4, 13);

//     print_list_int(l);

//     for (int i = 0; i < 10; i++) {
//         printf("%d ", arr[i]);
//     }
//     putchar('\n');

//     delete_list(l);
//     return 0;
// }

int main() {
    List * l = new_list_int();

    for (int i = 0; i < 20; i++) {
        list_push_int(l, i);
    }

    for (size_t i = 0; i < list_length(l); i++) {
        printf("%d ", list_read_int(l, i));
    }
    putchar('\n');

    delete_list(l);
    return 0;
}