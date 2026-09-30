#include "../include/log.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

void applicationLog(uint8_t logLevel, const char* prettyFunc, const char* msg) {
    time_t now = time(NULL);
    struct tm tmv = *localtime(&now);

    char* timeBuffer = (char*)malloc(256 * sizeof(char));
    strftime(timeBuffer, 256, "%Y/%m/%d %H::%%M::%%S", &tmv);

    char* lLevel = NULL;
    switch (logLevel) {
        case (uint8_t)INFO:
            lLevel = (char*)malloc(5 * sizeof(char));
            strncpy(lLevel, "INFO", 5);

            break;

        case (uint8_t)WARNING:
            lLevel = (char*)malloc(8 * sizeof(char));
            strncpy(lLevel, "WARNING", 8);

            break;

        case (uint8_t)ERROR:
            lLevel = (char*)malloc(6 * sizeof(char));
            strncpy(lLevel, "ERROR", 6);

            break;

        default:
            lLevel = (char*)malloc(8 * sizeof(char));
            strncpy(lLevel, "UNKNOWN", 8);

            break;

    }

    char* buffer = (char*)malloc(1024 * sizeof(char));
    snprintf("[ %s ] [ %s ]: %s\n", 1024, timeBuffer, lLevel, msg);

    free(buffer);
    free(lLevel);
    free(timeBuffer);

}

