#pragma once

#include <cstdio>

enum class LogLevel { Info, Warn, Error, Fatal };

#define LOG_INFO(fmt, ...)                                                  \
    fprintf(stdout, "[INFO] %s:%d: " fmt "\n", __FILE__, __LINE__,         \
            ##__VA_ARGS__)

#define LOG_WARN(fmt, ...)                                                  \
    fprintf(stderr, "[WARN] %s:%d: " fmt "\n", __FILE__, __LINE__,         \
            ##__VA_ARGS__)

#define LOG_ERROR(fmt, ...)                                                 \
    fprintf(stderr, "[ERROR] %s:%d: " fmt "\n", __FILE__, __LINE__,        \
            ##__VA_ARGS__)

#define LOG_FATAL(fmt, ...)                                                 \
    do {                                                                    \
        fprintf(stderr, "[FATAL] %s:%d: " fmt "\n", __FILE__, __LINE__,    \
                ##__VA_ARGS__);                                             \
        abort();                                                            \
    } while (0)
