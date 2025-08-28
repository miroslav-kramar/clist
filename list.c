#include "list.h"

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>

// -------
// DEFINES
// -------

#define DEFAULT_CAPACITY 8
#define INCREASE_FACTOR 2

// ----------
// STRUCT DEF
// ----------

typedef struct Allocator {
    void * (*alloc)(size_t);
    void * (*realloc)(void *, size_t);
    void   (*free)(void *);
} Allocator;

typedef struct list_t {
    void * data;
    size_t capacity;
    size_t length;
    size_t item_size;
    void (*destructor)(void *);
} list_t;

// ----------------
// GLOBAL VARIABLES
// ----------------

Allocator _allocator = {.alloc = malloc, .realloc = realloc, .free = free};

// -------------
// HELPER MACROS
// -------------

#define FAIL(msg) \
    do { \
        fprintf(stderr, "ERROR: %s:" msg "\n", __func__); \
        exit(EXIT_FAILURE); \
    } while (0)

#define FAIL_FMT(fmt, ...) \
    do { \
        fprintf(stderr, "ERROR: %s: " fmt "\n", __func__, ##__VA_ARGS__); \
        exit(EXIT_FAILURE); \
    } while (0)

#define CHCK_BOUNDS(index, cap) \
    do { \
        if ((index) >= (cap)) {FAIL_FMT("Index %zu out of bounds! Max is %zu.", (index), (cap)-1);} \
    } while (0)

#define CHCK_BOUNDS_START(idx_start, cap) \
    do { \
        if ((idx_start) >= (cap)) {FAIL_FMT("Start index %zu out of bounds! Max is %zu.", (idx_start), (cap)-1);} \
    } while (0)

#define CHCK_BOUNDS_END(idx_end, cap) \
    do { \
        if ((idx_end) >= (cap)) {FAIL_FMT("End index %zu out of bounds! Max is %zu.", (idx_end), (cap)-1);} \
    } while (0)

#define CHCK_IDX_RANGE(idx_start, idx_end) \
    do { \
        if ((idx_start) > (idx_end)) {FAIL_FMT("Start index (%zu) can't be larger than end index (%zu)!", (idx_start), (idx_end));} \
    } while (0)

#define CHCK_MATCHING_ITEM_SIZE(size_a, size_b) \
    do { \
        if ((size_a) != (size_b)) {FAIL("Lists can't differ in 'item_size!'");}\
    } while (0)

#define CHCK_MATCHING_DESTRUCTOR(destructor_a, destructor_b) \
    do { \
        if ((destructor_a) != (destructor_b)) {FAIL("Different destructors are disallowed!");}\
    } while (0)

#define CHCK_COPY_COUNT(count, capacity, idx_start, idx_end) \
    do { \
        if ((idx_end) >= (capacity)) {FAIL_FMT("Copying %zu elements from index %zu exceedes the bounds of the list!", (count), (idx_start));} \
    } while (0)

// ----------------
// HELPER FUNCTIONS
// ----------------

static list_t * _list_new(const size_t item_size, const size_t capacity, void (*destructor)(void *)) {
    list_t * l = _allocator.alloc(sizeof(*l));
    if (l == NULL) {goto ERR_LIST_ALLOC;}
    l->data = _allocator.alloc(capacity * item_size);
    if (l->data == NULL) {goto ERR_DATA_ALLOC;}
    l->capacity = capacity;
    l->length = 0;
    l->item_size = item_size;
    l->destructor = destructor;
    return l;

    ERR_DATA_ALLOC:
    _allocator.free(l);
    ERR_LIST_ALLOC:
    return NULL;
}

static bool _list_realloc(list_t * l, const size_t new_capacity) {
    if (l->capacity == new_capacity) {return true;}
    void * new_data = _allocator.realloc(l->data, new_capacity * l->item_size);
    if (new_data == NULL) {return false;}
    l->data = new_data;
    l->capacity = new_capacity;
    return true;
}

static bool _list_grow(list_t * l) {
    if (l->length == l->capacity) {
        if (!_list_realloc(l, l->capacity * INCREASE_FACTOR)) {return false;}
    }
    return true;
}

static bool _list_shrink(list_t * l) {
    const size_t quarter_capacity = l->capacity / 4;
    if (l->capacity > DEFAULT_CAPACITY && l->length < quarter_capacity) {
        const size_t new_capacity = (quarter_capacity > DEFAULT_CAPACITY) ? quarter_capacity : DEFAULT_CAPACITY;
        if (!_list_realloc(l, new_capacity)) {return false;}
    }
    return true;
}

static inline unsigned char * _get_idx_ptr(const list_t *l, const size_t index) {
    return (unsigned char *)l->data + index * l->item_size;
}

static inline unsigned char * _get_last_val_ptr(const list_t *l) {
    return (unsigned char *)l->data + (l->length - 1) * l->item_size;
}

static inline unsigned char * _get_last_idx_ptr(const list_t *l) {
    return (unsigned char *)l->data + (l->capacity - 1) * l->item_size;
}

static inline unsigned char * _get_len_ptr(const list_t *l) {
    return (unsigned char *)l->data + l->length * l->item_size;
}

static inline unsigned char * _get_cap_ptr(const list_t *l) {
    return (unsigned char *)l->data + l->capacity * l->item_size;
}

static inline size_t _n_in_range(const size_t start, const size_t end) {
    return end - start + 1;
}

static list_t * _copy_range_list(const list_t * l, const size_t idx_start, const size_t idx_end) {
    const size_t n = _n_in_range(idx_start, idx_end);
    list_t * copy = _list_new(l->item_size, n, l->destructor);
    if (copy == NULL) {return NULL;}
    memcpy(copy->data, _get_idx_ptr(l, idx_start), n * l->item_size);
    copy->length = n;
    return copy;
}

static void _copy_range_array(const list_t * l, void * out_array, const size_t idx_start, const size_t idx_end) {
    memcpy(out_array, _get_idx_ptr(l, idx_start), _n_in_range(idx_start, idx_end) * l->item_size);
}

// --------------
// IMPLEMENTATION
// --------------

void list_set_global_allocator(void *(*allocator)(size_t), void *(*reallocator)(void *, size_t), void (*deallocator)(void *)) {
    _allocator.alloc   = allocator;
    _allocator.realloc = reallocator;
    _allocator.free    = deallocator;
}

list_t * list_new(const size_t item_size, void (*destructor)(void *)) {
    if (item_size == 0) {FAIL("item_size can't be 0!");}

    return _list_new(item_size, DEFAULT_CAPACITY, destructor);
}

void list_delete(list_t * l) {
    if (l->destructor) {
        for (size_t i = 0; i < l->length; i++) {
            l->destructor(_get_idx_ptr(l, i));
        }
    }
    _allocator.free(l->data);
    _allocator.free(l);
}

size_t list_get_length(const list_t * l) {
    return l->length;
}

size_t list_get_capacity(const list_t * l) {
    return l->capacity;
}

size_t list_get_item_size(const list_t * l) {
    return l->item_size;
}

void (*list_get_destructor(const list_t * l))(void *) {
    return l->destructor;
}

bool list_push(list_t * l, const void * data) {
    if (!_list_grow(l)) {return false;}
    void * ptr = _get_len_ptr(l);
    memcpy(ptr, data, l->item_size);
    l->length += 1;
    return true;
}

bool list_pop(list_t * l, void * out) {
    if (l->length == 0) {return true;}

    void * idx_ptr = _get_last_val_ptr(l);
    if (out != NULL) {
        memcpy(out, idx_ptr, l->item_size);
    }
    memset(idx_ptr, 0, l->item_size);
    l->length -= 1;

    return _list_shrink(l);
}

void list_read(const list_t * l, const size_t index, void * out) {
    CHCK_BOUNDS(index, l->capacity);
    void * ptr = _get_idx_ptr(l, index);
    memcpy(out, ptr, l->item_size);
}

bool list_write(list_t * l, const size_t index, const void * data) {
    if (index >= l->capacity) {
        if (!_list_realloc(l, index + 1)) {return false;}
    }
    if (index >= l->length) {
        memset(_get_len_ptr(l), 0, (l->capacity - l->length) * l->item_size);
        l->length = index + 1;
    }
    memcpy(_get_idx_ptr(l, index), data, l->item_size);
    return true;
}

void list_to_array(const list_t * l, void * out_array) {
    _copy_range_array(l, out_array, 0, l->length - 1);
}

void list_to_array_range(const list_t * l, void * out_array, const size_t idx_start, const size_t idx_end) {
    CHCK_BOUNDS_START(idx_start, l->capacity);
    CHCK_BOUNDS_END(idx_end, l->capacity);
    CHCK_IDX_RANGE(idx_start, idx_end);

    _copy_range_array(l, out_array, idx_start, idx_end);
}

void list_to_array_count(const list_t * l, void * out_array, const size_t idx_start, const size_t count) {
    CHCK_BOUNDS(idx_start, l->capacity);
    const size_t idx_end = idx_start + count - 1;
    CHCK_COPY_COUNT(count, l->capacity, idx_start, idx_end);
    _copy_range_array(l, out_array, idx_start, idx_end);
}

list_t * list_copy(const list_t * l) {
    return _copy_range_list(l, 0, l->length - 1);
}

list_t * list_copy_range(const list_t * l, const size_t idx_start, const size_t idx_end) {
    if (idx_start > idx_end) {FAIL("Start index can't be larger than end index!");}
    return _copy_range_list(l, idx_start, idx_end);
}

list_t * list_copy_count(const list_t * l, const size_t idx_start, const size_t count) {
    CHCK_BOUNDS(idx_start, l->capacity);
    const size_t idx_end = idx_start + count - 1;
    CHCK_COPY_COUNT(count, l->capacity, idx_start, idx_end);
    return _copy_range_list(l, idx_start, idx_end);
}

list_t * list_join(const list_t * a, const list_t * b) {
    CHCK_MATCHING_ITEM_SIZE(a->item_size, b->item_size);
    CHCK_MATCHING_DESTRUCTOR(a->destructor, b->destructor);

    list_t * out = _list_new(a->item_size, a->length + b->length, a->destructor);
    if (out == NULL) {return NULL;}
    out->length = out->capacity;
    memcpy(out->data, a->data, a->length * out->item_size);
    memcpy((unsigned char *)out->data + a->length * out->item_size, b->data, b->length * out->item_size);
    return out;
}

bool list_append(list_t * dst, const list_t * src) {
    CHCK_MATCHING_ITEM_SIZE(dst->item_size, src->item_size);
    CHCK_MATCHING_DESTRUCTOR(dst->destructor, src->destructor);

    const size_t new_capacity = dst->length + src->length;
    if (new_capacity > dst->capacity) {
        if (!_list_realloc(dst, new_capacity)) {return false;}
    }
    memcpy(_get_len_ptr(dst), src->data, src->length * src->item_size);
    dst->capacity = new_capacity;
    dst->length = new_capacity;
    return true;
}

bool list_append_array(list_t * l, const size_t count, const void * data) {
    if (count == 0) {return true;}

    if (l->capacity - l->length < count) {
        if (!_list_realloc(l, l->length + count)) {return false;}
    }
    memcpy(_get_len_ptr(l), data, count * l->item_size);
    l->length += count;
    return true;
}

bool list_insert(list_t * l, const size_t index, const void * data) {
    if (index >= l->length) {
        if (index >= l->capacity) {
            if (!_list_realloc(l, index + 1)) {return false;}
        }
        memset(_get_len_ptr(l), 0, (index - l->length) * l->item_size);
    }
    else {
        if (!_list_grow(l)) {return false;}
        unsigned char * idx_ptr = _get_idx_ptr(l, index);
        memmove(idx_ptr + 1 * l->item_size, idx_ptr, (l->length - index) * l->item_size);
    }
    memcpy(_get_idx_ptr(l, index), data, l->item_size);
    l->length += 1;
    return true;
}

bool list_erase(list_t * l, const size_t index, void * out) {
    CHCK_BOUNDS(index, l->capacity);
    unsigned char * idx_ptr = _get_idx_ptr(l, index);
    if (out != NULL) {
        memcpy(out, idx_ptr, l->item_size);
    }
    memmove(idx_ptr, idx_ptr + 1 * l->item_size, (l->length - index) * l->item_size);
    memset(_get_last_val_ptr(l), 0, l->item_size);
    l->length -= 1;
    
    return _list_shrink(l);
}

// --------------------
// TYPED API GENERATION
// --------------------

// type, name

#define LIST_OF_TYPES \
    X(bool,        bool) \
    X(int8_t,      i8) \
    X(int16_t,     i16) \
    X(int32_t,     i32) \
    X(int64_t,     i64) \
    X(uint8_t,     u8) \
    X(uint16_t,    u16) \
    X(uint32_t,    u32) \
    X(uint64_t,    u64) \
    X(float,       flt) \
    X(double,      dbl) \
    X(long double, ldbl) \
    X(void *,      vptr)

#define X(type, name) \
    list_t * list_new_##name () { \
        return list_new(sizeof(type), NULL); \
    } \
    \
    bool list_push_##name (list_t * l, const type data) { \
        return list_push(l, (void *)&data); \
    } \
    \
    type list_read_##name (list_t * l, const size_t index) { \
        type out; \
        list_read(l, index, (void *)&out); \
        return out; \
    } \
    \
    void list_write_##name (list_t * l, const size_t index, const type data) { \
        list_write(l, index, (void *)&data); \
    } \
    \
    bool list_insert_##name (list_t * l, const size_t index, const type data) { \
        return list_insert(l, index, (void *)&data); \
    }
LIST_OF_TYPES
#undef X
