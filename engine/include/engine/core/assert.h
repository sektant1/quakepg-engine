#pragma once

#include <cstdio>
#include <cstdlib>

#define QP_ASSERT(expr)                                                     \
    do {                                                                    \
        if (!(expr)) {                                                      \
            fprintf(stderr, "[ASSERT] %s:%d: %s\n", __FILE__, __LINE__,     \
                    #expr);                                                 \
            abort();                                                        \
        }                                                                   \
    } while (0)

#define QP_ASSERT_MSG(expr, msg)                                            \
    do {                                                                    \
        if (!(expr)) {                                                      \
            fprintf(stderr, "[ASSERT] %s:%d: %s - %s\n", __FILE__,         \
                    __LINE__, #expr, msg);                                  \
            abort();                                                        \
        }                                                                   \
    } while (0)
