#include "list.h"

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>

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

typedef struct List {
    void * data;
    size_t capacity;
    size_t length;
    size_t item_size;
    void (*destructor)(void *);
} List;

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

static List * _list_new(const size_t item_size, const size_t capacity, void (*destructor)(void *)) {
    List * l = _allocator.alloc(sizeof(*l));
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

static bool _list_realloc(List * l, const size_t new_capacity) {
    if (l->capacity == new_capacity) {return true;}
    void * new_data = _allocator.realloc(l->data, new_capacity * l->item_size);
    if (new_data == NULL) {return false;}
    l->data = new_data;
    l->capacity = new_capacity;
    return true;
}

static bool _list_grow(List * l) {
    if (l->length == l->capacity) {
        if (!_list_realloc(l, l->capacity * INCREASE_FACTOR)) {return false;}
    }
    return true;
}

static bool _list_shrink(List * l) {
    const size_t quarter_capacity = l->capacity / 4;
    if (l->capacity > DEFAULT_CAPACITY && l->length < quarter_capacity) {
        const size_t new_capacity = (quarter_capacity > DEFAULT_CAPACITY) ? quarter_capacity : DEFAULT_CAPACITY;
        if (!_list_realloc(l, new_capacity)) {return false;}
    }
    return true;
}

static inline unsigned char * _get_idx_ptr(const List *l, const size_t index) {
    return (unsigned char *)l->data + index * l->item_size;
}

static inline unsigned char * _get_last_val_ptr(const List *l) {
    return (unsigned char *)l->data + (l->length - 1) * l->item_size;
}

static inline unsigned char * _get_last_idx_ptr(const List *l) {
    return (unsigned char *)l->data + (l->capacity - 1) * l->item_size;
}

static inline unsigned char * _get_len_ptr(const List *l) {
    return (unsigned char *)l->data + l->length * l->item_size;
}

static inline unsigned char * _get_cap_ptr(const List *l) {
    return (unsigned char *)l->data + l->capacity * l->item_size;
}

static inline size_t _n_in_range(const size_t start, const size_t end) {
    return end - start + 1;
}

static List * _copy_range_list(const List * l, const size_t idx_start, const size_t idx_end) {
    const size_t n = _n_in_range(idx_start, idx_end);
    List * copy = _list_new(l->item_size, n, l->destructor);
    if (copy == NULL) {return NULL;}
    memcpy(copy->data, _get_idx_ptr(l, idx_start), n * l->item_size);
    copy->length = n;
    return copy;
}

static void _copy_range_array(const List * l, void * out_array, const size_t idx_start, const size_t idx_end) {
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

List * new_list(const size_t item_size, void (*destructor)(void *)) {
    if (item_size == 0) {FAIL("item_size can't be 0!");}

    return _list_new(item_size, DEFAULT_CAPACITY, destructor);
}

void delete_list(List * l) {
    if (l->destructor) {
        for (size_t i = 0; i < l->length; i++) {
            l->destructor(_get_idx_ptr(l, i));
        }
    }
    _allocator.free(l->data);
    _allocator.free(l);
}

size_t list_length(const List * l) {
    return l->length;
}

size_t list_capacity(const List * l) {
    return l->capacity;
}

size_t list_item_size(const List * l) {
    return l->item_size;
}

void (*list_destructor(const List * l))(void *) {
    return l->destructor;
}

bool list_push(List * l, const void * data) {
    if (!_list_grow(l)) {return false;}
    void * ptr = _get_len_ptr(l);
    memcpy(ptr, data, l->item_size);
    l->length += 1;
    return true;
}

bool list_pop(List * l, void * out) {
    if (l->length == 0) {return true;}

    void * idx_ptr = _get_last_val_ptr(l);
    if (out != NULL) {
        memcpy(out, idx_ptr, l->item_size);
    }
    memset(idx_ptr, 0, l->item_size);
    l->length -= 1;

    return _list_shrink(l);
}

void list_read(const List * l, const size_t index, void * out) {
    CHCK_BOUNDS(index, l->capacity);
    void * ptr = _get_idx_ptr(l, index);
    memcpy(out, ptr, l->item_size);
}

bool list_write(List * l, const size_t index, const void * data) {
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

void list_capacity_copy_array(const List * l, void * out_array) {
    _copy_range_array(l, out_array, 0, l->capacity - 1);
}

void list_length_copy_array(const List * l, void * out_array) {
    _copy_range_array(l, out_array, 0, l->length - 1);
}

void list_range_copy_array(const List * l, void * out_array, const size_t idx_start, const size_t idx_end) {
    CHCK_BOUNDS_START(idx_start, l->capacity);
    CHCK_BOUNDS_END(idx_end, l->capacity);
    CHCK_IDX_RANGE(idx_start, idx_end);

    _copy_range_array(l, out_array, idx_start, idx_end);
}

void list_copy_count_array(const List * l, void * out_array, const size_t idx_start, const size_t count) {
    CHCK_BOUNDS(idx_start, l->capacity);
    const size_t idx_end = idx_start + count - 1;
    CHCK_COPY_COUNT(count, l->capacity, idx_start, idx_end);
    _copy_range_array(l, out_array, idx_start, idx_end);
}

List * list_copy(const List * l) {
    return _copy_range_list(l, 0, l->length - 1);
}

List * list_range_copy(const List * l, const size_t idx_start, const size_t idx_end) {
    if (idx_start > idx_end) {FAIL("Start index can't be larger than end index!");}
    return _copy_range_list(l, idx_start, idx_end);
}

List * list_copy_count(const List * l, const size_t idx_start, const size_t count) {
    CHCK_BOUNDS(idx_start, l->capacity);
    const size_t idx_end = idx_start + count - 1;
    CHCK_COPY_COUNT(count, l->capacity, idx_start, idx_end);
    return _copy_range_list(l, idx_start, idx_end);
}

List * list_join(const List * a, const List * b) {
    CHCK_MATCHING_ITEM_SIZE(a->item_size, b->item_size);
    CHCK_MATCHING_DESTRUCTOR(a->destructor, b->destructor);

    List * out = _list_new(a->item_size, a->length + b->length, a->destructor);
    if (out == NULL) {return NULL;}
    out->length = out->capacity;
    memcpy(out->data, a->data, a->length * out->item_size);
    memcpy((unsigned char *)out->data + a->length * out->item_size, b->data, b->length * out->item_size);
    return out;
}

bool list_append(List * dst, const List * src) {
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

bool list_append_array(List * l, void * data, size_t count) {
    if (count == 0) {return true;}

    if (l->capacity - l->length < count) {
        if (!_list_realloc(l, l->length + count)) {return false;}
    }
    memcpy(_get_len_ptr(l), data, count * l->item_size);
    return true;
}

bool list_insert(List * l, const size_t index, const void * data) {
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

bool list_erase(List * l, const size_t index, void * out) {
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
