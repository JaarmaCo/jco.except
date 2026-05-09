#define JCOALLOC_IMPLEMENTATION
#define JCOEXCEPT_IMPLEMENTATION
#define JCOARRAY_DROP_PREFIX
#include "../jco-array.h"

#include <stdio.h>
#include <stdlib.h>

void print_array(int_array_t *array) {

    if (array->count > array->capacity) {
        printf("array.count > array.capacity\n");
        exit(1);
    }
    printf("count = %zu\n", 
           array->count);
    for (size_t i = 0; i < array->count; ++i) {
        printf("items[%zu] = %d\n", i, array->items[i]);
    }
}

int main(void) {
    jco_except_t except;
    jco_1k_arena_t arena;
    jco_1k_arena(arena, except);

    if (jco_catch(NULL, except)) {
        printf("ERROR: %.*s\n", 
                (int)except->message_length, 
                except->message);
        return 1;
    }
    int_array_t array = {
        .allocator = arena->allocator,
    };
    for (int i = 0; i < 10; ++i) {
        array_append(array, i);
    }
    print_array(&array);

    int next[] = { 10, 11, 12, 13 };
    array_append(array, next, sizeof next / sizeof next[0]);
    print_array(&array);

    array_append(array);
    array.items[array.count - 1] = 100;
    print_array(&array);

    return 0;
}

