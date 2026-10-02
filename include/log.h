#ifndef COD4X_GEO_WELCOME_PLUGIN_LOG_H
#define COD4X_GEO_WELCOME_PLUGIN_LOG_H

#ifdef VERBOSE
#define DBG(logLevel, ...) do { \
    char b[1024]; \
    snprintf(b, sizeof(b), __VA_ARGS__); \
    applicationLog(logLevel, __PRETTY_FUNCTION__, b); \
} while (0)

#else
#define DBG(logLevel, ...) do {} while (0)

#endif

#include <stddef.h>
#include <stdint.h>

enum logLevel {
    LOG_INFO,
    LOG_WARNING,
    LOG_ERROR

};

struct logCache {
    char* logBuffer;

};

void applicationLog(uint8_t logLevel, const char* prettyFunc, const char* msg);

#endif

