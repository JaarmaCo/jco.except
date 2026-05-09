///
/// Copyright © 2026 William Jaarma (JaarmaCo@git)
/// Permission is hereby granted, free of charge, to any person obtaining a copy of this 
/// software and associated documentation files (the “Software”), to deal in the Software 
/// without restriction, including without limitation the rights to use, copy, modify, merge, 
/// publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons 
/// to whom the Software is furnished to do so, subject to the following conditions:
///
/// The above copyright notice and this permission notice shall be included in all copies or 
/// substantial portions of the Software.
/// 
/// THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING 
/// BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
/// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, 
/// DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, 
/// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
///
///      ██╗ ██████╗ ██████╗     █████╗ ██████╗ ██████╗  █████╗ ██╗   ██╗
///      ██║██╔════╝██╔═══██╗   ██╔══██╗██╔══██╗██╔══██╗██╔══██╗╚██╗ ██╔╝
///      ██║██║     ██║   ██║   ███████║██████╔╝██████╔╝███████║ ╚████╔╝ 
/// ██   ██║██║     ██║   ██║   ██╔══██║██╔══██╗██╔══██╗██╔══██║  ╚██╔╝  
/// ╚█████╔╝╚██████╗╚██████╔╝██╗██║  ██║██║  ██║██║  ██║██║  ██║   ██║   
///  ╚════╝  ╚═════╝ ╚═════╝ ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝   
///                                                                      
/// Macro-based generic dynamic array implementation with support for custom 
/// allocators. Uses jco.alloc to for memory allocations and jco.except for 
/// error handling.
///
/// Each macro enterprets their number of arguments, emulating arity-based 
/// function overloading.
/// 
/// Another note about the macros is that they are designed to only evaluate 
/// their arguments once. This does however come with the downside that they 
/// require the newer C23 typeof operator.
///
/// The currently supported operations are:
/// - Preallocation of array capacity 
/// - Dynamic resizing
/// - Appending elements to the end
/// - Removing elements from the end
/// - Stable insertion of elements
/// - Stable erasure of elements
///
/// This file also includes a set of prefined dynamic array structures for 
/// commonly used types. It is also possible to define new array-like structs.
/// To use this library for your own types, define a struct (for an element 
/// type T) containing the following fields:
///
/// - struct jco_allocator *allocator
/// - T *items
/// - size_t count
/// - size_t capacity
///
/// CREDIT: Many of these ideas have been inspired by the nobuild libraries:
///         https://github.com/tsoding/nob.h
///
#ifndef JCOARRAY_H_

#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

#include "jco-alloc.h"

typedef struct jco_cstr_array {
    struct jco_allocator *allocator;
    char **items;
    size_t count;
    size_t capacity;
} jco_cstr_array_t;

typedef struct jco_ptr_array {
    struct jco_allocator *allocator;
    void **items;
    size_t count;
    size_t capacity;
} jco_ptr_array_t;

typedef struct jco_char_array {
    struct jco_allocator *allocator;
    char *items;
    size_t count;
    size_t capacity;
} jco_char_array_t;

typedef struct jco_int8_array {
    struct jco_allocator *allocator;
    int8_t *items;
    size_t count;
    size_t capacity;
} jco_int8_array_t;

typedef struct jco_int16_array {
    struct jco_allocator *allocator;
    int16_t *items;
    size_t count;
    size_t capacity;
} jco_int16_array_t;

typedef struct jco_int32_array {
    struct jco_allocator *allocator;
    int32_t *items;
    size_t count;
    size_t capacity;
} jco_int32_array_t;

typedef struct jco_int64_array {
    struct jco_allocator *allocator;
    int64_t *items;
    size_t count;
    size_t capacity;
} jco_int64_array_t;

typedef struct jco_uint8_array {
    struct jco_allocator *allocator;
    uint8_t *items;
    size_t count;
    size_t capacity;
} jco_uint8_array_t;

typedef struct jco_uint16_array {
    struct jco_allocator *allocator;
    uint16_t *items;
    size_t count;
    size_t capacity;
} jco_uint16_array_t;

typedef struct jco_uint32_array {
    struct jco_allocator *allocator;
    uint32_t *items;
    size_t count;
    size_t capacity;
} jco_uint32_array_t;

typedef struct jco_uint64_array {
    struct jco_allocator *allocator;
    uint64_t *items;
    size_t count;
    size_t capacity;
} jco_uint64_array_t;

typedef struct jco_ptrdiff_array {
    struct jco_allocator *allocator;
    ptrdiff_t *items;
    size_t count;
    size_t capacity;
} jco_ptrdiff_array_t;

typedef struct jco_size_array {
    struct jco_allocator *allocator;
    size_t *items;
    size_t count;
    size_t capacity;
} jco_size_array_t;

typedef struct jco_intptr_array {
    struct jco_allocator *allocator;
    intptr_t *items;
    size_t count;
    size_t capacity;
} jco_intptr_array_t;

typedef struct jco_uintptr_array {
    struct jco_allocator *allocator;
    uintptr_t *items;
    size_t count;
    size_t capacity;
} jco_uintptr_array_t;

typedef struct jco_schar_array {
    struct jco_allocator *allocator;
    signed char *items;
    size_t count;
    size_t capacity;
} jco_schar_array_t;

typedef struct jco_short_array {
    struct jco_allocator *allocator;
    short *items;
    size_t count;
    size_t capacity;
} jco_short_array_t;

typedef struct jco_int_array {
    struct jco_allocator *allocator;
    int *items;
    size_t count;
    size_t capacity;
} jco_int_array_t;

typedef struct jco_long_array {
    struct jco_allocator *allocator;
    long *items;
    size_t count;
    size_t capacity;
} jco_long_array_t;

typedef struct jco_uchar_array {
    struct jco_allocator *allocator;
    unsigned char *items;
    size_t count;
    size_t capacity;
} jco_uchar_array_t;

typedef struct jco_ushort_array {
    struct jco_allocator *allocator;
    unsigned short *items;
    size_t count;
    size_t capacity;
} jco_ushort_array_t;

typedef struct jco_uint_array {
    struct jco_allocator *allocator;
    unsigned int *items;
    size_t count;
    size_t capacity;
} jco_uint_array_t;

typedef struct jco_ulong_array {
    struct jco_allocator *allocator;
    unsigned long *items;
    size_t count;
    size_t capacity;
} jco_ulong_array_t;

typedef struct jco_float_array {
    struct jco_allocator *allocator;
    float *items;
    size_t count;
    size_t capacity;
} jco_float_array_t;

typedef struct jco_double_array {
    struct jco_allocator *allocator;
    double *items;
    size_t count;
    size_t capacity;
} jco_double_array_t;

typedef struct jco_ldouble_array {
    struct jco_allocator *allocator;
    long double *items;
    size_t count;
    size_t capacity;
} jco_ldouble_array_t;

/// Differs depending on the number of arguments passed.
/// See:
///     - jco_array_append_1
///     - jco_array_append_2
///     - jco_array_append_3
///
#define jco_array_append(...)\
    JCODEFER_CAT(jco_array_append_, JCODEFER_GET_ARGN(__VA_ARGS__))(__VA_ARGS__)

/// Append a single uninitialized value at the end of the array.
///
/// array: array-like lvalue (evaluated once)
///
#define jco_array_append_1(array)\
    do {\
        typeof(&(array)) array_ptr_A1_ = &(array);\
        jco_array_reserve(*array_ptr_A1_, array_ptr_A1_->count + 1);\
        ++array_ptr_A1_->count;\
    } while (0)

/// Append a value at the end of the array.
///
/// array: array-like lvalue (evaluated once)
/// value: value to append (by value, evaluated once)
///
#define jco_array_append_2(array, value)\
    do {\
        typeof(&(array)) array_ptr_A2_ = &(array);\
        jco_array_reserve(*array_ptr_A2_, array_ptr_A2_->count + 1);\
        array_ptr_A2_->items[array_ptr_A2_->count++] = (value);\
    } while (0)

/// Append a number of values at the end of the array.
///
/// array:  array-like lvalue (evaluated once)
/// values: pointer to an array of values to append (or NULL, evaluated once)
/// nvalue: number of values to append (evaluated once)
///
#define jco_array_append_3(array, values, nvalues)\
    do {\
        typeof(&(array)) array_ptr_A3_ = &(array);\
        size_t count_A3_ = (nvalues);\
        typeof(array_ptr_A3_->items) values_A3_ = (values);\
        jco_array_reserve(*array_ptr_A3_, array_ptr_A3_->count + count_A3_);\
        if (NULL != values_A3_) {\
            memcpy(\
                array_ptr_A3_->items + array_ptr_A3_->count, \
                values_A3_, \
                count_A3_ * sizeof *values_A3_);\
        }\
        array_ptr_A3_->count += count_A3_;\
    } while (0)

/// Extend or shrink the size of an array.
///
/// array: array-like lvalue (evaluated once)
/// n:     new number of elements (evaluated once)
///
#define jco_array_resize(array, n)\
    do {\
        typeof(&(array)) array_ptr_R_ = &(array);\
        size_t count_R_ = (n);\
        jco_array_reserve(*array_ptr_R_, count_R_);\
        array_ptr_R_->count = count_R_;\
    } while (0)

/// Preallocate space in an array.
///
/// array: array-like lvalue (evaluated once)
/// n:     number of elements to reserve (evaluated once)
///
#define jco_array_reserve(array, n)\
    do {\
        size_t required_RR_ = (n);\
        typeof(&(array)) array_ptr_RR_ = &(array);\
        if (required_RR_ <= array_ptr_RR_->capacity) {\
            break;\
        }\
        size_t new_capacity_RR_ = required_RR_ + required_RR_ / 2;\
        array_ptr_RR_->items = jco_alloc(\
                array_ptr_RR_->allocator, \
                typeof(*array_ptr_RR_->items),\
                array_ptr_RR_->items, \
                array_ptr_RR_->capacity, \
                new_capacity_RR_);\
        array_ptr_RR_->capacity = new_capacity_RR_;\
    } while(0)

/// Behavior depends on the number of arguments called.
///
/// See:
/// - jco_array_insert_3
/// - jco_array_insert_4
///
#define jco_array_insert(...) \
    JCODEFER_CAT(jco_array_insert_, JCODEFER_GET_ARGN(__VA_ARGS__))(__VA_ARGS__)

/// Stable insert a value into the array.
///
/// array: array-like lvalue (evaluated once)
/// index: point of insertion (evaluated once)
/// value: value to insert (by value, evaluated once)
///
#define jco_array_insert_3(array, index, value)\
    do {\
        typeof(&(array)) array_ptr_I3_ = &(array);\
        size_t start_I3_ = (index);\
        assert(start_I3_ <= array_ptr_I3_->count);\
        jco_array_append(*array_ptr_I3_);\
        memmove(array_ptr_I3_->items + start_I3_ + 1,\
                array_ptr_I3_->items + start_I3_,\
                (array_ptr_I3_->count - start_I3_) * sizeof *array_ptr_I3_->items);\
        array_ptr_I3_->items[start_I3_] = (value);\
    } while (0)

/// Stable insert a number of elements into the array.
///
/// array:   array-like lvalue (evaluated once)
/// index:   point of insertion (evaluated once)
/// values:  pointer to an array of elements to insert (or NULL, evaluated once)
/// nvalues: number of values to insert (evaluated once)
///
#define jco_array_insert_4(array, index, values, nvalues)\
    do {\
        typeof(&(array)) array_ptr_I4_ = &(array);\
        size_t start_I4_ = (index);\
        size_t count_I4_ = (nvalues);\
        typeof(array_ptr_I4_->items) values_I4_ = (values);\
        assert(start_I4_ <= array_ptr_I4_->count);\
        jco_array_append(*array_ptr_I4_, NULL, count_I4_);\
        memmove(array_ptr_I4_->items + start_I4_ + count_I4_,\
                array_ptr_I4_->items + start_I4_,\
                (array_ptr_I4_->count - start_I4_) * sizeof *array_ptr_I4_->items);\
        if (NULL != values_I4_) {\
            memcpy(array_ptr_I4_->items, values_I4_, count_I4_ * sizeof(*values_I4_));\
        }\
    } while (0)

/// Behavior depends on the number of arguments used.
///
/// See:
/// - jco_array_erase_2
/// - jco_array_erase_3
///
#define jco_array_erase(...) \
    JCODEFER_CAT(jco_array_erase_, JCODEFER_GET_ARGN(__VA_ARGS__))(__VA_ARGS__)

/// Stable erase a single element from the array.
///
/// array: array-like lvalue (evaluated once)
/// index: index of the element to erase (evaluated once)
///
#define jco_array_erase_2(array, index)\
    do {\
        size_t start = (index);\
        typeof(&(array)) array_ptr = &(array);\
        assert(start < array_ptr->count);\
        memmove(array_ptr->items + start, \
                array_ptr->items + start + 1, \
                (array_ptr->count - start) * sizeof *array_ptr->items);\
        --array_ptr->count;\
    } while (0)

/// Stable erase a range of elements from the array.
///
/// array: array-like lvalue (evaluated once)
/// index: index of the first element to erase (evaluated once)
/// n:     number of elements to erase (evaluated once)
///
#define jco_array_erase_3(array, index, n)\
    do {\
        size_t start = (index);\
        size_t count_ = (n);\
        typeof(&(array)) array_ptr = &(array);\
        assert(start < array_ptr->count);\
        assert(start + count_ <= array_ptr->count);\
        memmove(array_ptr->items + start,\
                array_ptr->items + start + count_,\
                (array_ptr->count - start) * sizeof *array_ptr->items);\
        array_ptr->count -= count_;\
    } while (0)

/// Behavior depends on the number of arguments used.
///
/// See:
/// - jco_array_pop_1
/// - jco_array_pop_2
///
#define jco_array_pop(...)\
    JCODEFER_CAT(jco_array_pop_, JCODEFER_GET_ARGN(__VA_ARGS__))(__VA_ARGS__)

/// Remove the last element from the array.
///
/// array: array-like lvalue (evaluated once)
///
#define jco_array_pop_1(array)\
    do {\
        typeof(&(array)) array_ptr = &(array);\
        assert(array_ptr->count > 0);\
        --array_ptr->count;\
    } while (0)

/// Remove the last n elements from the array.
///
/// array: array-like lvalue (evaluated once)
/// n:     number of elements to erase (evaluated once)
///
#define jco_array_pop_2(array, n)\
    do {\
        typeof(&(array)) array_ptr = &(array);\
        size_t count_ = (n);\
        assert(count_ <= array_ptr->count);\
        array_ptr->count -= count_;\
    } while (0)

#ifdef JCOARRAY_DROP_PREFIX
#define array_append(...)  jco_array_append(__VA_ARGS__)
#define array_resize(...)  jco_array_resize(__VA_ARGS__)
#define array_reserve(...) jco_array_reserve(__VA_ARGS__)
#define array_insert(...)  jco_array_insert(__VA_ARGS__)
#define array_erase(...)   jco_array_erase(__VA_ARGS__)
#define array_pop(...)     jco_array_pop(__VA_ARGS__)

typedef struct jco_cstr_array    cstr_array_t;
typedef struct jco_ptr_array     ptr_array_t;
typedef struct jco_char_array    char_array_t;
typedef struct jco_int8_array    int8_array_t;
typedef struct jco_int16_array   int16_array_t;
typedef struct jco_int32_array   int32_array_t;
typedef struct jco_int64_array   int64_array_t;
typedef struct jco_uint8_array   uint8_array_t;
typedef struct jco_uint16_array  uint16_array_t;
typedef struct jco_uint32_array  uint32_array_t;
typedef struct jco_uint64_array  uint64_array_t;
typedef struct jco_ptrdiff_array ptrdiff_array_t;
typedef struct jco_size_array    size_array_t;
typedef struct jco_intptr_array  intptr_array_t;
typedef struct jco_uintptr_array uintptr_array_t;
typedef struct jco_schar_array   schar_array_t;
typedef struct jco_short_array   short_array_t;
typedef struct jco_int_array     int_array_t;
typedef struct jco_long_array    long_array_t;
typedef struct jco_uchar_array   uchar_array_t;
typedef struct jco_ushort_array  ushort_array_t;
typedef struct jco_uint_array    uint_array_t;
typedef struct jco_ulong_array   ulong_array_t;
typedef struct jco_double_array  double_array_t;
typedef struct jco_ldouble_array ldouble_array_t;

#endif

#define JCOARRAY_H_
#endif

