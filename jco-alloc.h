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
///      ██╗ ██████╗ ██████╗     █████╗ ██╗     ██╗      ██████╗  ██████╗
///      ██║██╔════╝██╔═══██╗   ██╔══██╗██║     ██║     ██╔═══██╗██╔════╝
///      ██║██║     ██║   ██║   ███████║██║     ██║     ██║   ██║██║     
/// ██   ██║██║     ██║   ██║   ██╔══██║██║     ██║     ██║   ██║██║     
/// ╚█████╔╝╚██████╗╚██████╔╝██╗██║  ██║███████╗███████╗╚██████╔╝╚██████╗
///  ╚════╝  ╚═════╝ ╚═════╝ ╚═╝╚═╝  ╚═╝╚══════╝╚══════╝ ╚═════╝  ╚═════╝
///
/// jco.alloc is a work-in-progress single-header allocation library in C.
/// The idea is to use jco.except to simplify handling of allocation failures,
/// and to use a similar interface of stack-allocated state.
///
/// For now, this library supports:
/// - Statically allocated arenas of sizes 1KiB, 4KiB, 8KiB and 10KiB
/// - Dynamically resizable arenas 
/// - Exception enabled malloc/realloc/free wrapper
///
/// Example usage:
///
///     jco_except_t except;
///     jco_4k_arena_t arena;
///     jco_4k_arena(arena, except);
///
///     if (jco_catch(NULL, except)) {
///         puts(except.message);
///         exit(1);
///     }
///
///     int *array = jco_alloc(arena, int, 256);
///     for (size_t i = 0; i < 256; ++i) {
///         array[i] = i;
///     }
///     
///     use_array(array);
///
///     // Arena was stack-allocated, no need to 
///     // free it.
///     //
///     return;
///
#ifndef JCOALLOC_H_

#include <string.h>

#include "jco-except.h"

#define JCO_ERROR_BAD_ALLOC -1001

#ifdef __cplusplus 
extern "C" {
#endif

/// Groups allocation information into a single struct.
///
typedef struct jco_alloc_info {
    void *old;        // Pointer to an old allocation
    size_t size;      // Size of the array element type
    size_t alignment; // Alignment of the array element type
    size_t old_count; // Number of elements in the old allocation
    size_t new_count; // Number of elements to allocate (0 to free)
} jco_alloc_info_t;

/// Structure holding allocator callbacks and context.
///
typedef struct jco_allocator {
    
    // Exception context to be invoked on allocation failure.
    struct jco_exception_context *except;

    // Readable name of the allocator kind.
    const char *kind_name;
    // Context needed by the allocator.
    void *context;
    
    // Callback function that handles the allocation action.
    //
    // It will:
    //
    // 1. Allocate new memory if new_count > 0
    // 2. Copy old memory into the new allocation of old_count > 0 
    //    (or reuse the allocation)
    // 3. Free memory if new_count == 0
    //
    void *(*allocate_callback)(
            void *context,
            const jco_alloc_info_t *info);
    
    // Frees the allocator context and performs necessary cleanup 
    // actions that should be performed when the allocator is no 
    // longer used.
    //
    void(*free_context)(void *);

} jco_allocator_t[1];

// Header containing the state of a static arena.
//
typedef struct jco_static_arena {
    size_t occupied;
    size_t capacity;
    void *pointer;
} jco_static_arena_t[1];

// A 1KiB static arena allocator.
//
typedef struct jco_1k_arena {
    jco_static_arena_t arena_header;
    jco_allocator_t allocator;
    char buffer[1 << 10];
} jco_1k_arena_t[1];

// A 4KiB static arena allocator.
//
typedef struct jco_4k_arena {
    jco_static_arena_t arena_header;
    jco_allocator_t allocator;
    char buffer[1 << 11];
} jco_4k_arena_t[1];

// A 8KiB static arena allocator.
//
typedef struct jco_8k_arena {
    jco_static_arena_t arena_header;
    jco_allocator_t allocator;
    char buffer[1 << 12];
} jco_8k_arena_t[1];

// A 10KiB static arena allocator.
//
typedef struct jco_10k_arena {
    jco_static_arena_t arena_header;
    jco_allocator_t allocator;
    char buffer[1 << 13];
} jco_10k_arena_t[1];

// Context for the malloc/realloc/free allocator.
//
typedef struct jco_malloc {
    jco_allocator_t allocator;
} jco_malloc_t[1];

// Arena block allocated by a dynamic arena.
//
struct jco_dynamic_arena_block {
    size_t size;
    struct jco_dynamic_arena_block *next;
};

// Holds the state of a dynamic arena.
//
struct jco_dynamic_arena_state {
    struct jco_allocator *upstream_allocator;    // Allocator used when allocating new arenas.
    jco_static_arena_t current_arena;            // State of the current arena.
    struct jco_dynamic_arena_block *first_block; // Linked list of arena blocks.
};

// Dynamically resizable arena.
//
typedef struct jco_dynamic_arena {
    struct jco_dynamic_arena_state state;
    jco_allocator_t allocator;
} jco_dynamic_arena_t[1];

/// Initializes a malloc/realloc/free allocator, using the 
/// given exception context to handle allocation failures.
///
void jco_malloc(jco_malloc_t malloc, jco_except_t except);

/// Initializes the different sized static arenas using a 
/// given exception context to handle allocation failures.
///
void jco_1k_arena(jco_1k_arena_t arena, jco_except_t except);
void jco_4k_arena(jco_4k_arena_t arena, jco_except_t except);
void jco_8k_arena(jco_8k_arena_t arena, jco_except_t except);
void jco_10k_arena(jco_10k_arena_t arena, jco_except_t except);

/// Initializes a dynamically resizable arena that uses the 
/// given allocator to allocate new blocks, where the first 
/// block has size initial_size.
///
void jco_dynamic_arena(
        jco_dynamic_arena_t arena, 
        jco_allocator_t upstream, 
        size_t initial_size);

// Implementation detail, use jco_alloc
//
void *jco_alloc_invoke(
        jco_allocator_t alloc, 
        const jco_alloc_info_t *info);

// Implementation detail, use jco_alloc
//
static inline void *jco_alloc_invoke_(
        jco_allocator_t alloc, 
        void *old,
        size_t size,
        size_t alignment,
        size_t old_count,
        size_t new_count) {
    jco_alloc_info_t info = {
        .old = old,
        .size = size,
        .alignment = alignment,
        .old_count = old_count,
        .new_count = new_count,
    };
    return jco_alloc_invoke(alloc, &info);
}

/// Destroy a resource-owning allocator.
///
void jco_alloc_destroy(jco_allocator_t alloc);

/// Allocate, or extend a contiguous array of memory.
///
/// - When used with two arguments (alloc, T):
///
///     Allocate a single element of type T using alloc.
///
/// - When used with three arguments (alloc, T, count):
///
///     Allocates an array of type T containing count 
///     elements using alloc.
///
/// - When used with five arguments 
///   (alloc, T, pointer, old_count, new_count):
///
///     Extends an array (pointer[old_count] of type T
///     to hold new_count elements.
///
#define jco_alloc(...) JCODEFER_CAT(jco_alloc_, JCODEFER_GET_ARGN(__VA_ARGS__))(__VA_ARGS__)

/// Deallocates an array (pointer[count]) of type T.
///
#define jco_free(alloc, T, pointer, count) \
    (void)jco_alloc_invoke_((alloc)->allocator, (pointer), sizeof(T),\
            _Alignof(T), (count), 0)

#define jco_alloc_2(alloc, T) \
    (T *)jco_alloc_invoke_((alloc)->allocator, NULL, \
            sizeof(T), _Alignof(T), 0, 1)

#define jco_alloc_3(alloc, T, count) \
    (T *)jco_alloc_invoke_((alloc)->allocator, NULL, sizeof(T), \
            _Alignof(T), 0, (count))

#define jco_alloc_5(alloc, T, pointer, old_count, new_count)\
    (T *)jco_alloc_invoke_((alloc)->allocator, (pointer), sizeof(T), \
            _Alignof(T), (old_count), (new_count))

#ifdef __cplusplus
}
#endif

#define JCOALLOC_H_
#endif

#ifdef JCOALLOC_IMPLEMENTATION

#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

void jco_dynamic_arena_free(void *);

void *jco_static_arena_allocate_callback(
        void *context,
        const jco_alloc_info_t *info);

void *jco_dynamic_arena_allocate_callback(
        void *context,
        const jco_alloc_info_t *info);

void *jco_malloc_allocate_callback(
        void *context,
        const jco_alloc_info_t *info);

void jco_1k_arena(jco_1k_arena_t arena, jco_except_t except) {
    arena->arena_header[0] = (struct jco_static_arena){
        .occupied = 0,
        .capacity = sizeof arena->buffer,
        .pointer = arena->buffer,
    };
    arena->allocator[0] = (struct jco_allocator){
        .except    = except,
        .kind_name = "1 KiB arena allocator",
        .context   = arena->arena_header,
        .allocate_callback = jco_static_arena_allocate_callback,
        .free_context = NULL,
    };
}

void jco_4k_arena(jco_4k_arena_t arena, jco_except_t except) {
    arena->arena_header[0] = (struct jco_static_arena){
        .occupied = 0,
        .capacity = sizeof arena->buffer,
        .pointer = arena->buffer,
    };
    arena->allocator[0] = (struct jco_allocator){
        .except    = except,
        .kind_name = "1 KiB arena allocator",
        .context   = arena->arena_header,
        .allocate_callback = jco_static_arena_allocate_callback,
        .free_context = NULL,
    };
}

void jco_8k_arena(jco_8k_arena_t arena, jco_except_t except) {
    arena->arena_header[0] = (struct jco_static_arena){
        .occupied = 0,
        .capacity = sizeof arena->buffer,
        .pointer = arena->buffer,
    };
    arena->allocator[0] = (struct jco_allocator){
        .except    = except,
        .kind_name = "1 KiB arena allocator",
        .context   = arena->arena_header,
        .allocate_callback = jco_static_arena_allocate_callback,
        .free_context = NULL,
    };
}

void jco_10k_arena(jco_10k_arena_t arena, jco_except_t except) {
    arena->arena_header[0] = (struct jco_static_arena){
        .occupied = 0,
        .capacity = sizeof arena->buffer,
        .pointer = arena->buffer,
    };
    arena->allocator[0] = (struct jco_allocator){
        .except    = except,
        .kind_name = "1 KiB arena allocator",
        .context   = arena->arena_header,
        .allocate_callback = jco_static_arena_allocate_callback,
        .free_context = NULL,
    };
}

void jco_malloc(jco_malloc_t malloc, jco_except_t except) {
    malloc->allocator[0] = (struct jco_allocator){
        .except    = except,
        .kind_name = "malloc/realloc/free",
        .context   = NULL,
        .allocate_callback = jco_malloc_allocate_callback,
        .free_context = NULL,
    };
}

void jco_dynamic_arena(
        jco_dynamic_arena_t arena, 
        jco_allocator_t upstream, 
        size_t initial_size) {
    arena->state = (struct jco_dynamic_arena_state){
        .upstream_allocator = upstream,
        .current_arena = {
            (struct jco_static_arena){0}
        },
        .first_block = NULL,
    };
    arena->allocator[0] = (struct jco_allocator){
        .except    = upstream->except,
        .kind_name = "dynamic arena allocator",
        .context   = &arena->state,
        .allocate_callback = jco_dynamic_arena_allocate_callback,
        .free_context      = jco_dynamic_arena_free,
    };
    struct jco_dynamic_arena_block *first_block 
        = (struct jco_dynamic_arena_block *)jco_alloc_invoke_(
                arena->state.upstream_allocator,
                NULL/*old*/, 
                1/*size*/, 
                1/*alignment*/, 
                0/*old_count*/, 
                initial_size + sizeof (struct jco_dynamic_arena_block));
    first_block->size = initial_size;
    first_block->next = NULL;
    arena->state.first_block = first_block;
    arena->state.current_arena[0] = (struct jco_static_arena) {
        .occupied = 0,
        .capacity = initial_size,
        .pointer  = first_block + 1,
    };
}

void *jco_dynamic_arena_allocate_callback(
        void *context, 
        const jco_alloc_info_t *info) {
    struct jco_dynamic_arena_state *state 
        = (struct jco_dynamic_arena_state *)context;
    size_t position = state->current_arena->occupied;
    if (position % info->alignment != 0) {
        position += info->alignment - position % info->alignment;
    }
    size_t required_size = info->size * info->new_count;
    if (position >= state->current_arena->capacity 
            || position + required_size > state->current_arena->capacity) {
        size_t next_block_size = state->first_block->size;
        size_t required_size = info->size * info->new_count;
        if (required_size > next_block_size) {
            next_block_size = required_size;
        }
        struct jco_dynamic_arena_block *next_block 
            = (struct jco_dynamic_arena_block *)jco_alloc_invoke_(
                    state->upstream_allocator, 
                    NULL, 1, 1, 0,
                    next_block_size + sizeof(struct jco_dynamic_arena_block));
        next_block->size = next_block_size;
        next_block->next = state->first_block;
        state->first_block = next_block;
        state->current_arena[0] = (struct jco_static_arena){
            .occupied = 0,
            .capacity = next_block_size,
            .pointer = next_block + 1,
        };
    }
    return jco_static_arena_allocate_callback(&state->current_arena, info);
}

void jco_dynamic_arena_free(void *context) {
    struct jco_dynamic_arena_state *state 
        = (struct jco_dynamic_arena_state *)context;
    struct jco_dynamic_arena_block *block = state->first_block;
    while (block) {
        struct jco_dynamic_arena_block *next = block->next;
        (void)jco_alloc_invoke_(
                state->upstream_allocator, 
                block, 1, 1, 
                block->size + sizeof *block, 0);
        block = next;
    }
    memset(state, 0, sizeof *state);
}

void *jco_static_arena_allocate_callback(
        void *context,
        const jco_alloc_info_t *info) {
    if (info->new_count == 0) {
        return NULL;
    }

    if (info->old_count > 0) {
        jco_alloc_info_t new_info = {
            .old       = NULL,
            .size      = info->size,
            .alignment = info->alignment,
            .old_count = 0,
            .new_count = info->new_count,
        };
        void *mem = jco_static_arena_allocate_callback(context, &new_info);
        if (mem) {
            memcpy(mem, info->old, info->old_count * info->size);
        }
        return mem;
    }

    struct jco_static_arena *arena = (struct jco_static_arena *)context;
    size_t required_size = info->size * info->new_count;
    size_t position = arena->occupied;
    if (position % info->alignment == 0) {
        position += info->alignment - position % info->alignment;
    }
    if (position >= arena->capacity 
            || position + required_size > arena->capacity) {
        return NULL;
    }
    arena->occupied = position + required_size;
    return (char *)arena->pointer + position;
}

void *jco_malloc_allocate_callback(
        void *context,
        const jco_alloc_info_t *info) {
    (void)context;
    if (info->new_count == 0) {
        free(info->old);
        return NULL;
    }
    if (info->old_count != 0) {
        return realloc(info->old, 
                info->new_count * info->size);
    }
    return malloc(info->new_count * info->size);
}

void *jco_alloc_invoke(
        jco_allocator_t alloc, 
        const jco_alloc_info_t *info) {
    void *mem = alloc->allocate_callback(alloc->context, info);
    if (NULL == mem && info->new_count > 0) { 
        jco_throw(alloc->except, JCO_ERROR_BAD_ALLOC, 
                "Memory allocation failed. Parameters where:\n"
                "  allocator kind: %s\n"
                "  old:       %p\n"
                "  size:      %zu\n"
                "  alignment: %zu\n"
                "  old_count: %zu\n"
                "  new_count: %zu\n",
                alloc->kind_name ? alloc->kind_name : "<unknown>",
                info->old, info->size, info->alignment, 
                info->old_count, info->new_count);
    }
    return mem;
}

void jco_alloc_destroy(jco_allocator_t alloc) {
    jco_cleanup_except(alloc->except);
    if (alloc->free_context) {
        alloc->free_context(alloc->context);
    }
    memset(alloc, 0, sizeof *alloc);
}

#ifdef __cplusplus 
}
#endif
#endif

