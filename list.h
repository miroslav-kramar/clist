#ifndef LIST_H
#define LIST_H

#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

typedef struct list_t list_t;

void list_set_global_allocator(void *(*allocator)(size_t), void *(*reallocator)(void *, size_t), void (*deallocator)(void *));

list_t * list_new(size_t item_size, void (*destructor)(void *));
void     list_delete(list_t * l);

size_t list_get_length(const list_t * l);
size_t list_get_capacity(const list_t * l);
size_t list_get_item_size(const list_t * l);
void (*list_get_destructor(const list_t * l))(void *);

bool list_push(list_t * l, const void * data);
bool list_pop(list_t * l, void * out);
bool list_insert(list_t * l, size_t index, const void * data);
bool list_erase(list_t * l, size_t index, void * out);

void list_read(const list_t * l, size_t index, void * out);
bool list_write(list_t * l, size_t index, const void * data);

void list_to_array(const list_t * l, void * out_array);
void list_to_array_range(const list_t * l, void * out_array, size_t idx_start, size_t idx_end);
void list_to_array_count(const list_t * l, void * out_array, size_t idx_start, size_t count);

list_t * list_copy(const list_t * l);
list_t * list_copy_range(const list_t * l, size_t idx_start, size_t idx_end);
list_t * list_copy_count(const list_t * l, size_t idx_start, size_t count);

list_t * list_join(const list_t * a, const list_t * b);
bool list_append(list_t * dst, const list_t * src);
bool list_append_array(list_t * l, size_t count, const void * data);

list_t * list_new_i8();
list_t * list_new_i16();
list_t * list_new_i32();
list_t * list_new_i64();
list_t * list_new_u8();
list_t * list_new_u16();
list_t * list_new_u32();
list_t * list_new_u64();
list_t * list_new_flt();
list_t * list_new_dbl();
list_t * list_new_ldbl();
list_t * list_new_vptr();

bool list_push_i8   (list_t *l, int8_t data);
bool list_push_i16  (list_t *l, int16_t data);
bool list_push_i32  (list_t *l, int32_t data);
bool list_push_i64  (list_t *l, int64_t data);
bool list_push_u8   (list_t *l, uint8_t data);
bool list_push_u16  (list_t *l, uint16_t data);
bool list_push_u32  (list_t *l, uint32_t data);
bool list_push_u64  (list_t *l, uint64_t data);
bool list_push_flt  (list_t *l, float data);
bool list_push_dbl  (list_t *l, double data);
bool list_push_ldbl (list_t *l, long double data);
bool list_push_vptr (list_t *l, const void * data);

int8_t      list_read_i8   (list_t *l, size_t index);
int16_t     list_read_i16  (list_t *l, size_t index);
int32_t     list_read_i32  (list_t *l, size_t index);
int64_t     list_read_i64  (list_t *l, size_t index);
uint8_t     list_read_u8   (list_t *l, size_t index);
uint16_t    list_read_u16  (list_t *l, size_t index);
uint32_t    list_read_u32  (list_t *l, size_t index);
uint64_t    list_read_u64  (list_t *l, size_t index);
float       list_read_flt  (list_t *l, size_t index);
double      list_read_dbl  (list_t *l, size_t index);
long double list_read_ldbl (list_t *l, size_t index);
void *      list_read_vptr (list_t *l, size_t index);

void list_write_i8   (list_t *l, size_t index, int8_t data);
void list_write_i16  (list_t *l, size_t index, int16_t data);
void list_write_i32  (list_t *l, size_t index, int32_t data);
void list_write_i64  (list_t *l, size_t index, int64_t data);
void list_write_u8   (list_t *l, size_t index, uint8_t data);
void list_write_u16  (list_t *l, size_t index, uint16_t data);
void list_write_u32  (list_t *l, size_t index, uint32_t data);
void list_write_u64  (list_t *l, size_t index, uint64_t data);
void list_write_flt  (list_t *l, size_t index, float data);
void list_write_dbl  (list_t *l, size_t index, double data);
void list_write_ldbl (list_t *l, size_t index, long double data);
void list_write_vptr (list_t *l, size_t index, const void * data);

bool list_insert_i8   (list_t *l, size_t index, int8_t data);
bool list_insert_i16  (list_t *l, size_t index, int16_t data);
bool list_insert_i32  (list_t *l, size_t index, int32_t data);
bool list_insert_i64  (list_t *l, size_t index, int64_t data);
bool list_insert_u8   (list_t *l, size_t index, uint8_t data);
bool list_insert_u16  (list_t *l, size_t index, uint16_t data);
bool list_insert_u32  (list_t *l, size_t index, uint32_t data);
bool list_insert_u64  (list_t *l, size_t index, uint64_t data);
bool list_insert_flt  (list_t *l, size_t index, float data);
bool list_insert_dbl  (list_t *l, size_t index, double data);
bool list_insert_ldbl (list_t *l, size_t index, long double data);
bool list_insert_vptr (list_t *l, size_t index, const void * data);

#endif //LIST_H
