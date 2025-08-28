#include <stdio.h>
#include "list.h"

void list_report(list_t * l) {
    fprintf(stderr, "list_t cap is %4zu, len is %4zu\n", list_get_capacity(l), list_get_length(l));
}

void print_list_int(list_t * l) {
    for (size_t i = 0; i < list_get_length(l); i++) {
        fprintf(stderr, "%d, ", list_read_i32(l, i));
    }
    fputc('\n', stderr);
}

// void nested_list_destructor(void * ptr) {
//     list_delete(*(list_t **)ptr);
// }

// int main() {
//     list_t * l = list_new(sizeof(l), nested_list_destructor);

//     for (int i = 0; i < 10; i++) {
//         list_t * tmp_push = new_list_int();
//         list_push_ptr(l, tmp_push);
//         for (int j = 0; j < 10; j++) {
//             list_push_int(list_read_ptr(l, i), j);
//         }
//     }

//     for (size_t i = 0; i < list_get_length(l); i++) {
//         for (size_t j = 0; j < list_get_length(list_read_ptr(l, i)); j++) {
//             printf("%d ", list_read_int(list_read_ptr(l, i), j));
//         }
//         putchar('\n');
//     }

//     list_delete(l);
// }

// int main() {
//     list_t * l = new_list_int();

//     for (int i = 0; i < 20; i++) {
//         list_push_int(l, i);
//     }

//     int arr[10];
//     list_to_array_range(l, arr, 4, 13);

//     print_list_int(l);

//     for (int i = 0; i < 10; i++) {
//         printf("%d ", arr[i]);
//     }
//     putchar('\n');

//     list_delete(l);
//     return 0;
// }

// int main() {
//     list_t * l = new_list_int();

//     for (int i = 0; i < 20; i++) {
//         list_push_int(l, i);
//     }

//     for (size_t i = 0; i < list_get_length(l); i++) {
//         printf("%d ", list_read_int(l, i));
//     }
//     putchar('\n');

//     list_delete(l);
//     return 0;
// }

int main() {
    list_t * l = list_new_i32();
    list_append_array(l, 4, (int[4]){1,2,3,4});
    print_list_int(l);
    list_delete(l);
}