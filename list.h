#ifndef LIST_H
#define LIST_H

#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

typedef struct List List;

void list_set_global_allocator(void *(*allocator)(size_t), void *(*reallocator)(void *, size_t), void (*deallocator)(void *));

List * new_list(size_t item_size, void (*destructor)(void *));
void delete_list(List * l);

size_t list_length(const List * l);
size_t list_capacity(const List * l);
size_t list_item_size(const List * l);
void (*list_destructor(const List * l))(void *);

bool list_push(List * l, const void * data);
bool list_pop(List * l, void * out);

void list_read(const List * l, size_t index, void * out);
bool list_write(List * l, size_t index, const void * data);

void list_capacity_copy_array(const List * l, void * out_array);
void list_length_copy_array(const List * l, void * out_array);
void list_range_copy_array(const List * l, void * out_array, size_t idx_start, size_t idx_end);
void list_copy_count_array(const List * l, void * out_array, size_t idx_start, size_t count);

List * list_copy(const List * l);
List * list_range_copy(const List * l, size_t idx_start, size_t idx_end);
List * list_copy_count(const List * l, size_t idx_start, size_t count);

List * list_join(const List * a, const List * b);
bool list_append(List * dst, const List * src);
bool list_append_array(List * l, void * data, size_t count);

bool list_insert(List * l, size_t index, const void * data);
bool list_erase(List * l, size_t index, void * out);

#define LIST_OF_TYPES \
    X(bool, bool) \
    X(char, char) \
    X(unsigned char, uchar) \
    X(short, short) \
    X(unsigned short, ushort) \
    X(int, int) \
    X(unsigned int, uint) \
    X(long, long) \
    X(unsigned long, ulong) \
    X(long long, ll) \
    X(unsigned long long, ull) \
    X(float, float) \
    X(double, double) \
    X(long double, ldouble) \
    X(void *, ptr)

#define X(type, name) \
    static inline List * new_list_##name () { \
        return new_list(sizeof(type), NULL); \
    } \
    \
    static inline bool list_push_##name (List * l, const type data) { \
        return list_push(l, (void *)&data); \
    } \
    \
    static inline type list_read_##name (List * l, const size_t index) { \
        type out; \
        list_read(l, index, (void *)&out); \
        return out; \
    } \
    \
    static inline void list_write_##name (List * l, const size_t index, const type data) { \
        list_write(l, index, (void *)&data); \
    } \
    \
    static inline bool list_insert_##name (List * l, const size_t index, const type data) { \
        return list_insert(l, index, (void *)&data); \
    }
LIST_OF_TYPES
#undef X
#undef LIST_OF_TYPES

#endif //LIST_H
