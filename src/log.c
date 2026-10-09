#include <stdarg.h>
#include <stdio.h>
#include <time.h>

#include "pinc.h"
#include "log.h"


void applicationLog(uint8_t logLevel, const char* prettyFunc, const char* fmt, ...)
{
    char timeBuffer[64];
    char messageBuffer[1024];
    char finalBuffer[1400];

    // Timestamp
    time_t now = time(NULL);
    struct tm tmv;
    localtime_r(&now, &tmv);
    strftime(timeBuffer, sizeof(timeBuffer), "%Y/%m/%d %H:%M:%S", &tmv);

    // Log level → string
    const char* lLevel;
    switch (logLevel) {
        case LOG_INFO:    lLevel = "INFO";    break;
        case LOG_WARNING: lLevel = "WARNING"; break;
        case LOG_ERROR:   lLevel = "ERROR";   break;
        default:          lLevel = "UNKNOWN"; break;
    }

    // Format the user message
    va_list args;
    va_start(args, fmt);
    vsnprintf(messageBuffer, sizeof(messageBuffer), fmt, args);
    va_end(args);

    // Build final log line
    snprintf(finalBuffer, sizeof(finalBuffer),
             "[ %s ] [ %s ] [ %s ]: %s\n",
             timeBuffer, lLevel, prettyFunc, messageBuffer);

    Plugin_Printf("%s", finalBuffer);
}
