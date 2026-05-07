# jco.except

jco.except is my personal attempt at an STB-style error handling system to be used in gcc/clang C.
It contains the following features:

- Hierarchical control flow implemented using setjmp/longjmp
- Error codes and stack allocated exception messages
- Attachable cleanup actions

To access this library, `#include` it and in a source file, define the `JCOEXCEPT_IMPLEMENTATION` macro
before including this file:
```c 
#file "jco-except.c"
#define JCOEXCEPT_IMPLEMENTATION
#include "jco-except.h"
```
The main purpose of this single header library is to simplify programming in C by supplying a 
minimalistic runtime system for exception handling. The system is designed to minimize the amount 
of memory allocated on the heap, and instead opts for stack allocated structures to store state.

This has been achieved using va_list-like schemantics in that context types are declared as single 
element arrays, effectively disabling rvalue schemantics for those types. For instance, to enable 
exception handling for a function `handle_request()`, one would write something like this:
```c
    #include "database.h"
    #include "json.h"

    #define JCOEXCEPT_IMPLEMENTATION
    #include "jco-except.h"

    void handle_request(jco_except_t upstream_except, 
                        http_request_t *request, 
                        http_response_t *response) {
        jco_cleanup_pool(1);
        jco_except_t except;
        if (jco_try(upstream_except, except)) {
            goto internal_server_error;
        }

        json_t json;
        parse_request(request, &json);
        defer_except(except, json_free, &json);

        char *email;
        query_database(&email, "SELECT user_email FROM users WHERE user_name=?", 
            json_obj_string(&json, "name"));

        strcpy(response->body, email);
        response->status_code = 200;
        jco_cleanup_except(except);
        return;
    internal_server_error:
        response->status_code = 500;
        strncpy(response->body, except.message, except.message_length);
    }
```


