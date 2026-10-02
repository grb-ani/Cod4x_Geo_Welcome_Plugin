#include "../include/curl.h"
#include "../include/log.h"

#include <stdio.h>
#include <string.h>
#include <curl/curl.h>
#include <malloc.h>

#define BUFFER_SIZE 2048

struct curlCacheStruct* cCacheStruct = NULL;

struct writeCallBackStruct {
    char* contents;
    size_t size;

};

void curlCacheStructDelete(struct curlCacheStruct* cCacheStruct) {
    if (cCacheStruct) {
        if (cCacheStruct->curl) {
            curl_easy_cleanup(cCacheStruct->curl);

        }

        if (cCacheStruct->curlCallbackStruct) {
            free(cCacheStruct->curlCallbackStruct);

        }

        free(cCacheStruct);

        cCacheStruct->curlCallbackStruct = NULL;

    }

}

static size_t write_callback(void* contents, size_t size, size_t nmemb, void* arg) {
    DBG(LOG_INFO, "Function called");

    struct writeCallBackStruct* wCallbackStruct = (struct writeCallBackStruct*)arg;
    if (!wCallbackStruct) {
        applicationLog(LOG_ERROR, __PRETTY_FUNCTION__, "Failed to assing pointer");

        return -1;

    }

    size_t readSize = size * nmemb;
    if (readSize + wCallbackStruct->size >= BUFFER_SIZE) {
        applicationLog(LOG_ERROR, __PRETTY_FUNCTION__, "Callback data is longer than the allocated buffer size");

        return -1;

    }

    memcpy(wCallbackStruct->contents + wCallbackStruct->size, contents, readSize);

    wCallbackStruct->size += readSize;

    return readSize;

}

const char* curlFetchGeoLocationJson(const char* url) {
    DBG(LOG_INFO, "Function called");

    if (!url) {
        applicationLog(LOG_ERROR, __PRETTY_FUNCTION__, "url is not present");

        return NULL;

    }

    CURL* curl = NULL;
    struct writeCallBackStruct* wCallbackStruct = NULL;
    if (!cCacheStruct) {
        cCacheStruct = (struct curlCacheStruct*)malloc(sizeof(struct curlCacheStruct));
        if (!cCacheStruct) {
            applicationLog(LOG_ERROR, __PRETTY_FUNCTION__, "cCacheStruct is a null pointer, failed to allocte space");

            return NULL;

        }

        goto initiate_cache_curl_instance;

    } else if (!cCacheStruct->curl || !cCacheStruct->curlCallbackStruct) {
initiate_cache_curl_instance:
        if (!cCacheStruct->curl) {
            cCacheStruct->curl = curl_easy_init();
            if (!cCacheStruct->curl) {
                applicationLog(LOG_ERROR, __PRETTY_FUNCTION__, "Failed to initialize curl");

                free(cCacheStruct);

                cCacheStruct = NULL;

                return NULL;

            }

            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, wCallbackStruct);

        }

        if (!cCacheStruct->curlCallbackStruct) {
            cCacheStruct->curlCallbackStruct = malloc(sizeof(struct writeCallBackStruct));
            if (!cCacheStruct->curlCallbackStruct) {
                applicationLog(LOG_ERROR, __PRETTY_FUNCTION__, "cCacheStruct->curlCallbackStruct is a null pointer, failed to allocate memory");

                curl_easy_cleanup(curl);

                free(cCacheStruct);

                cCacheStruct = NULL;

                return NULL;

            }

        }

    }
    curl = cCacheStruct->curl;
    wCallbackStruct = cCacheStruct->curlCallbackStruct;
    wCallbackStruct->size = 0;

    curl_easy_setopt(curl, CURLOPT_URL, url);

    int rc = curl_easy_perform(curl);
    if (rc != CURLE_OK) {
        char b[1024];
        snprintf(b, sizeof(b), "Performing of curl failed: %s", curl_easy_strerror(rc));
        applicationLog(LOG_ERROR, __PRETTY_FUNCTION__, b);

        return NULL;

    }

    wCallbackStruct->contents[wCallbackStruct->size] = '\0';
    const char* json = wCallbackStruct->contents;

    return json;

}

