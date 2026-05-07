#ifndef CEXCEPT_H_
#define CEXCEPT_H_

#include <stddef.h>
#include <setjmp.h>

/// Declares a buffer with the capacity of size cleanup actions that may 
/// be used by c_defer().
///
/// If the size parameter is not large enough to fit all cleanup actions, 
/// the behaviour is undefined.
///
#define c_cleanup_pool(size) \
    struct {\
        size_t count;\
        struct c_cleanup store[size];\
    } c_cleanup_pool_ = {0}

typedef void(*c_clean_callback_0_t)(void);
typedef void(*c_clean_callback_1_t)(void *);
typedef void(*c_clean_callback_2_t)(void *, void *);
typedef void(*c_clean_callback_3_t)(void *, void *, void *);
typedef void(*c_clean_callback_4_t)(void *, void *, void *, void *);
typedef void(*c_clean_callback_5_t)(void *, void *, void *, void *, void *);


/// Stores a chain of cleanup actions that can be 
/// triggered using a call to c_cleanup().
///
/// Note that instances of this type is expected to 
/// be zero-initialized. Calling c_cleanup() with 
/// a structure that was not initialized is undefined 
/// behaviour.
///
/// This structure has va_list-like schemantics and 
/// should therefore not be copied or moved.
///
typedef struct c_cleanup {
    
    size_t argument_count;
    void *arguments[5];
    c_clean_callback_0_t callback;

    struct c_cleanup *next;

} *c_cleanup_t[1];

/// Context for handling exceptions.
///
/// This structure has va_list-like schemantics and 
/// should therefore not be copied or moved.
///
typedef struct c_exception_context {

    jmp_buf jmp_location;
    struct c_exception_context *upstream;
    
    c_cleanup_t cleanup;

    int exception_code;

    size_t message_length;
    char message[1024];

} c_exception_context_t[1];

/// Optional shorter alias for c_exception_context_t. 
///
typedef c_exception_context_t c_except_t;

/// Throw an exception inside the given context.
///
/// If this function call was preceeded with a call to c_catch,
/// then execution will resume to the call-site of c_catch. If 
/// not, exit() is called, and the exception is printed.
///
/// The fmt and variadic arguments are used to specify an exception 
/// message. If fmt is NULL, a default exception message is used.
///
/// If the code parameter was 0, then this function will invoke
/// exit(0) with the provided error message.
///
/// Exception context also have an associated chain of cleanup actions 
/// that is invoked at the invocation of this function, or a corresponding 
/// call to c_rethrow().
///
_Noreturn 
void c_throw(c_exception_context_t ctx, int code, const char *fmt, ...);

/// Yield handling of the in-flight exception to the upstream context.
///
/// If the upstream context had registered cleanup actions, those actions 
/// are triggered at a call to this function.
///
_Noreturn
void c_rethrow(c_exception_context_t ctx);

/// va_start-esc function used to make a cleanup chain ready to be used.
/// This has the same effect as just zero-initializing the variable.
///
static inline 
void c_cleanup_start(c_cleanup_t chain) {
    chain[0] = NULL;
}

/// Run all cleanup actions and reset the variable containing the cleanup chain.
///
void c_cleanup(c_cleanup_t cleanup);

/// Calls c_cleanup() on the cleanup chain associated with the given exception 
/// context.
///
static inline 
void c_cleanup_except(c_exception_context_t ctx) {
    c_cleanup(ctx->cleanup);
}

/// Adds a cleanup action to the given chain.
///
static inline 
void c_chain_cleanup(c_cleanup_t root, struct c_cleanup *next) {
    next->next = root[0];
    root[0] = next;
}

/// Is to be used through the c_catch() macro. Manages assigning an upstream context 
/// and any other setup required for correct `catch` behaviour (such as invoking c_cleanup()).
///
int c_do_catch(struct c_exception_context *up, c_exception_context_t ctx, int status);

/// Sets the return location to resume to if an exception was thrown. The up parameter 
/// is an optional parameter (can be set to NULL) that specifies an upstream context 
/// to defer to on c_rethrow(). The second parameter should be an identifier of the 
/// exception context to register a catch in.
///
/// Example:
///
///     int foo(c_exception_context_t upstream) {
///         c_exception_context_t except;
///         if (c_catch(upstream, except)) {
///             goto handle_exception;
///         }
///
///         function_that_may_throw();
///         return 0;
///     handle_exception:
///         return 1;
///     }
///
/// On the first call, this function returns 0. If an exception was thrown, execution 
/// resumes to the call-site of this macro and the exception code is returned.
///
/// An important thing of note is that any resources assigned a cleanup action in 
/// the exception context used by this catch call may be freed by the second time 
/// c_catch() returns.
///
/// If a resource needs to live past c_catch(), it will have to be managed outside 
/// of c_cleanup()
///
#define c_catch(up, ctx) c_do_catch((up), (ctx), setjmp((ctx)->jmp_location))

/// If the c_cleanup_pool() was used in an accessible scope, then this macro 
/// may be used to add a callback that will be invoked on c_do_cleanup() in the 
/// given cleanup chain (passed as struct c_cleanup **).
///
/// Cleanup actions can be any function that accepts between 0 and 5 arguments 
/// that can be packed into void *. 
///
/// The variadic part of this macro must start with the callback to register 
/// (a pointer to a function), and should be followed by all pointer arguments 
/// to pass to the function at cleanup.
///
#define c_defer(chain, ...) \
    (c_cleanup_pool_.store[c_cleanup_pool_.count].callback = CDEFER_GET_CB(__VA_ARGS__), \
     c_cleanup_pool_.store[c_cleanup_pool_.count].argument_count = CDEFER_GET_ARGN(__VA_ARGS__) - 1,\
     CDEFER_GET_ARGS(__VA_ARGS__) \
     c_chain_cleanup((chain), &c_cleanup_pool_.store[c_cleanup_pool_.count++]))

/// Calls c_defer() with cleanup chain that is automatically triggered when 
/// an exception is thrown inside the given context, or in any downstream 
/// contexts.
///
#define c_defer_except(ctx, ...) c_defer((ctx)->cleanup, __VA_ARGS__)

#define CDEFER_GET_CB(cb, ...) (c_clean_callback_0_t)(cb)
#define CDEFER_GET_ARGN_(_0, _1, _2, _3, _4, _5, _6, ...) _6
#define CDEFER_GET_ARGN(...) CDEFER_GET_ARGN_(__VA_ARGS__, 6, 5, 4, 3, 2, 1)
#define CDEFER_EXPAND(...) CDEFER_EXPAND1(__VA_ARGS__)
#define CDEFER_EXPAND1(...) CDEFER_EXPAND2(__VA_ARGS__)
#define CDEFER_EXPAND2(...) __VA_ARGS__
#define CDEFER_CAT(x, y) CDEFER_CAT1(x, y)
#define CDEFER_CAT1(x, y) CDEFER_CAT2(x, y)
#define CDEFER_CAT2(x, y) x##y
#define CDEFER_GET_ARGS_1(cb) 
#define CDEFER_GET_ARGS_2(cb, arg0) \
    c_cleanup_pool_.store[c_cleanup_pool_.count].arguments[0] = (void *)(arg0),
#define CDEFER_GET_ARGS_3(cb, arg0, arg1) \
    c_cleanup_pool_.store[c_cleanup_pool_.count].arguments[0] = (void *)(arg0), \
    c_cleanup_pool_.store[c_cleanup_pool_.count].arguments[1] = (void *)(arg1),
#define CDEFER_GET_ARGS_4(cb, arg0, arg1, arg2) \
    c_cleanup_pool_.store[c_cleanup_pool_.count].arguments[0] = (void *)(arg0), \
    c_cleanup_pool_.store[c_cleanup_pool_.count].arguments[1] = (void *)(arg1), \
    c_cleanup_pool_.store[c_cleanup_pool_.count].arguments[2] = (void *)(arg2),
#define CDEFER_GET_ARGS_5(cb, arg0, arg1, arg2, arg3) \
    c_cleanup_pool_.store[c_cleanup_pool_.count].arguments[0] = (void *)(arg0), \
    c_cleanup_pool_.store[c_cleanup_pool_.count].arguments[1] = (void *)(arg1), \
    c_cleanup_pool_.store[c_cleanup_pool_.count].arguments[2] = (void *)(arg2), \
    c_cleanup_pool_.store[c_cleanup_pool_.count].arguments[3] = (void *)(arg3),
#define CDEFER_GET_ARGS_6(cb, arg0, arg1, arg2, arg3, arg4) \
    c_cleanup_pool_.store[c_cleanup_pool_.count].arguments[0] = (void *)(arg0), \
    c_cleanup_pool_.store[c_cleanup_pool_.count].arguments[1] = (void *)(arg1), \
    c_cleanup_pool_.store[c_cleanup_pool_.count].arguments[2] = (void *)(arg2), \
    c_cleanup_pool_.store[c_cleanup_pool_.count].arguments[3] = (void *)(arg3), \
    c_cleanup_pool_.store[c_cleanup_pool_.count].arguments[4] = (void *)(arg4),

#define CDEFER_GET_ARGS(...) \
    CDEFER_CAT(CDEFER_GET_ARGS_, CDEFER_GET_ARGN(__VA_ARGS__))(__VA_ARGS__)

#endif

#ifdef CEXCEPT_IMPLEMENTATION

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

_Noreturn
void c_throw(c_exception_context_t ctx, int code, const char *fmt, ...) {

    c_cleanup(ctx->cleanup);

    va_list va;
    if (code == 0) {
        va_start(va);
        vfprintf(stderr, fmt, va);
        va_end(va);
        exit(0);
    }
    
    if (NULL != fmt) {
        va_start(va, fmt);
        int n = vsnprintf(ctx->message, 
                sizeof ctx->message, fmt, va);
        va_end(va);
        ctx->message_length = n < 0 ? 0 : (size_t)n;
    } else {
        strcpy(ctx->message, "An unexpected exception occured.");
        ctx->message_length = sizeof "An unexpected exception occured." - 1;
    }
    ctx->exception_code = code;
    longjmp(ctx->jmp_location, code);
}

_Noreturn 
void c_rethrow(c_exception_context_t ctx) {
    if (ctx->upstream && ctx->exception_code) {
        c_cleanup(ctx->upstream->cleanup);
        longjmp(ctx->upstream->jmp_location, ctx->exception_code);
    }
    fprintf(stderr, "%.*s", (int)ctx->message_length, ctx->message);
    exit(ctx->exception_code);
}

void c_cleanup(c_cleanup_t root) {
    for (struct c_cleanup *cleanup = root[0]; cleanup; cleanup = cleanup->next) {
        switch (cleanup->argument_count)
        {
        case 0:
            cleanup->callback();
            break;
        case 1:
            ((c_clean_callback_1_t)cleanup->callback)(cleanup->arguments[0]);
            break;
        case 2:
            ((c_clean_callback_2_t)cleanup->callback)(cleanup->arguments[0], 
                cleanup->arguments[1]);
            break;
        case 3:
            ((c_clean_callback_3_t)cleanup->callback)(cleanup->arguments[0], 
                cleanup->arguments[1], cleanup->arguments[2]);
            break;
        case 4:
            ((c_clean_callback_4_t)cleanup->callback)(cleanup->arguments[0], 
                cleanup->arguments[1], cleanup->arguments[2], cleanup->arguments[3]);
            break;
        case 5:
             ((c_clean_callback_5_t)cleanup->callback)(cleanup->arguments[0], 
                cleanup->arguments[1], cleanup->arguments[2], cleanup->arguments[3], 
                cleanup->arguments[4]);
            break;
        default:
            break;
        }
    }
    root[0] = NULL;
}

int c_do_catch(struct c_exception_context *up, c_exception_context_t ctx, int status) {
    if (status) {
        return status;
    }
    ctx->message_length = 0;
    ctx->exception_code = 0;
    ctx->cleanup[0] = NULL;
    ctx->upstream = up;
    return 0;
}

#endif

