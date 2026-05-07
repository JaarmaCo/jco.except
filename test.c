///
/// Copyright © 2026 William Jaarma (JaarmaCo@git)
///
/// Permission is hereby granted, free of charge, to any person obtaining a copy of this software 
/// and associated documentation files (the “Software”), to deal in the Software without restriction, 
/// including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, 
/// and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, 
/// subject to the following conditions:
/// 
/// The above copyright notice and this permission notice shall be included in all copies or substantial portions 
/// of the Software.
/// 
/// THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED 
/// TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL 
/// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF 
/// CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS 
/// IN THE SOFTWARE.
///
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

