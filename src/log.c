#include "../include/log.h"
#include "../libs/pinc.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#define BUFFER_SIZE 2048

struct logCache* lCache = NULL;

void applicationLog(uint8_t logLevel, const char* prettyFunc, const char* msg) {
    time_t now = time(NULL);
    struct tm tmv = *localtime(&now);

    char* timeBuffer = (char*)malloc(256 * sizeof(char));
    strftime(timeBuffer, 256, "%Y/%m/%d %H::%M::%S", &tmv);

    const char* lLevel = NULL;
    switch (logLevel) {
        case (uint8_t)LOG_INFO:
            lLevel = "INFO";

            break;

        case (uint8_t)LOG_WARNING:
            lLevel = "WARNING";

            break;

        case (uint8_t)LOG_ERROR:
            lLevel = "ERROR";

            break;

        default:
            lLevel = "UNKNOWN";

            break;

    }

    char* buffer = NULL;
    if (!lCache) {
        lCache = (struct logCache*)malloc(sizeof(struct logCache));
        if (!lCache) {
            applicationLog(LOG_ERROR, __PRETTY_FUNCTION__, "lCache is a null pointer, failed to allocate memory");

            return;

        }

        goto log_buffer_allocate;

    } else if (!lCache->logBuffer){
log_buffer_allocate:
        lCache->logBuffer = (char*)malloc(BUFFER_SIZE * sizeof(char));
        if (!lCache->logBuffer) {
            applicationLog(LOG_ERROR, __PRETTY_FUNCTION__, "lCache->logBuffer is a null pointer, failed to allocate memory");

            free(lCache);

            return;

        }

    }
    buffer = lCache->logBuffer;

    snprintf(buffer, BUFFER_SIZE, "[ %s ] [ %s ]: %s\n", timeBuffer, lLevel, msg);
    Plugin_Printf(buffer);

    free(timeBuffer);

}

