#define JCOEXCEPT_IMPLEMENTATION
#define JCOALLOC_IMPLEMENTATION
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
    int initial_values[] = { 1, 2, 6, 7 };
    array_append(array, initial_values, 
            sizeof initial_values / sizeof initial_values[0]);
    print_array(&array);
    
    array_insert(array, 2, 3);
    print_array(&array);

    int values[] = { 4, 5 };
    array_insert(array, 3, values, sizeof values / sizeof values[0]);
    print_array(&array);

    return 0;
}

