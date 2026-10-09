#ifndef GEOWELCOME_QUEUE_H
#define GEOWELCOME_QUEUE_H

typedef struct GeoWelcomeResult {
    int   clientNum;
    char* country;
    char* city;
    char* asn;
    char* timezone;
} GeoWelcomeResult;

// Global queue declarations (NOT definitions)
extern GeoWelcomeResult* pendingResults[64];
extern int pendingCount;

#endif