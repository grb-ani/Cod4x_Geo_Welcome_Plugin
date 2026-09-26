#ifndef LOG_H
#define LOG_H

#ifdef VERBOSE
#define DBG(logLevel, ...) do { \
    char b[1024]; \
    snprintf(b, sizeof(b), __VA_ARGS__); \
    applicationLog(logLevel, __PRETTY_FUNCTION__, b); \
} while (0)

#else
#define DBG(logLevel, ...) do {} while (0)

#endif

#include <stdint.h>

enum logLevel {
    LOG_INFO,
    LOG_WARNING,
    LOG_ERROR

};

void applicationLog(uint8_t logLevel, const char* prettyFunc, const char* msg);

#endif

