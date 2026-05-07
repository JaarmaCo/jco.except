#define JCOEXCEPT_IMPLEMENTATION
#include "jco-except.h"

#include <stdlib.h>
#include <stdio.h>
#include <time.h>

static void clean(void *arg) {
    puts((char *)arg);
}

int main(void) {

    jco_cleanup_pool(2);
    jco_except_t except;
    jco_cleanup_t cl;

    jco_cleanup_start(cl);

    if (jco_catch(NULL, except)) {
        goto catch_exception;
    }

    jco_defer(cl, &clean, "Running cleanup 1");
    jco_defer_except(except, &clean, "Running cleanup 2");

    srand(time(NULL));
    int value = rand();
    if (value % 2 == 1) {
        jco_throw(except, value, "Random value was odd.");
    }

    printf("Success! Value was %d\n", value);
    jco_cleanup(cl);
    jco_cleanup_except(except);
    return 0;

catch_exception:
    if (value > 1000) {
        fprintf(stderr, "Value too large, giving up...\n");
        jco_rethrow(except);
    }
    fprintf(stderr, "%.*s\n", (int)except->message_length, except->message);
    return value;
}

