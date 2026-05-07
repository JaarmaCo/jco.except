#include "cexcept.h"

#include <stdlib.h>
#include <stdio.h>
#include <time.h>

static void clean(void *arg) {
    puts((char *)arg);
}

int main(void) {

    c_cleanup_pool(2);
    c_except_t except;
    c_cleanup_t cl;

    c_cleanup_start(cl);

    if (c_catch(NULL, except)) {
        goto catch_exception;
    }

    c_defer(cl, &clean, "Running cleanup 1");
    c_defer_except(except, &clean, "Running cleanup 2");

    srand(time(NULL));
    int value = rand();
    if (value % 2 == 1) {
        c_throw(except, value, "Random value was odd.");
    }

    printf("Success! Value was %d\n", value);
    c_cleanup(cl);
    c_cleanup_except(except);
    return 0;

catch_exception:
    if (value > 1000) {
        fprintf(stderr, "Value too large, giving up...\n");
        c_rethrow(except);
    }
    fprintf(stderr, "%.*s\n", (int)except->message_length, except->message);
    return value;
}

