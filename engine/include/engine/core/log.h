#pragma once

#include <cstdio>
#include <cstring>

#define COLOR_RESET "\033[0m"
#define COLOR_INFO  "\033[32m"    // Verde
#define COLOR_WARN  "\033[33m"    // Amarelo
#define COLOR_ERROR "\033[31m"    // Vermelho
#define COLOR_FATAL "\033[1;31m"  // Vermelho brilhante + negrito

inline const char *short_file(const char *file)
{
    static char buffer[256];

    // Pula até depois de "quakepg/"
    const char *p = std::strstr(file, "quakepg/");
    if (p) {
        p += 8;
    } else {
        p = file;
    }

    // Detecta se é engine ou game
    const char *module = nullptr;
    if (std::strncmp(p, "engine/", 7) == 0) {
        module = "engine/";
    } else if (std::strncmp(p, "game/", 5) == 0) {
        module = "game/";
    } else {
        return p;  // fallback (caso esteja fora do projeto)
    }

    // Pega só o nome do arquivo (tudo depois do último '/')
    const char *last_slash = std::strrchr(p, '/');
    const char *base_name  = last_slash ? last_slash + 1 : p;

    // Monta "engine/nome.cpp" ou "game/nome.cpp"
    std::snprintf(buffer, sizeof(buffer), "%s%s", module, base_name);
    return buffer;
}
enum class LogLevel
{
    Info,
    Warn,
    Error,
    Fatal
};

// Forward declaration for console integration (avoids circular include)
namespace qp {
enum class ConsoleMsgLevel : uint8_t;
void console_push(ConsoleMsgLevel level, const char *fmt, ...);
}

#define LOG_INFO(fmt, ...)                                                                                      \
    do {                                                                                                        \
        fprintf(stdout, COLOR_INFO "[INFO] %s:%d: " fmt COLOR_RESET "\n", short_file(__FILE__), __LINE__, ##__VA_ARGS__); \
        qp::console_push((qp::ConsoleMsgLevel)0, "[INFO] " fmt, ##__VA_ARGS__);                                \
    } while (0)

#define LOG_WARN(fmt, ...)                                                                                      \
    do {                                                                                                        \
        fprintf(stderr, COLOR_WARN "[WARN] %s:%d: " fmt COLOR_RESET "\n", short_file(__FILE__), __LINE__, ##__VA_ARGS__); \
        qp::console_push((qp::ConsoleMsgLevel)1, "[WARN] " fmt, ##__VA_ARGS__);                                \
    } while (0)

#define LOG_ERROR(fmt, ...)                                                                                     \
    do {                                                                                                        \
        fprintf(stderr, COLOR_ERROR "[ERROR] %s:%d: " fmt COLOR_RESET "\n", short_file(__FILE__), __LINE__, ##__VA_ARGS__); \
        qp::console_push((qp::ConsoleMsgLevel)2, "[ERROR] " fmt, ##__VA_ARGS__);                               \
    } while (0)

#define LOG_FATAL(fmt, ...) \
    do { \
        fprintf(stderr, \
                COLOR_FATAL "[FATAL] %s:%d: " fmt COLOR_RESET "\n", \
                short_file(__FILE__), \
                __LINE__, \
                ##__VA_ARGS__); \
        abort(); \
    } while (0)
