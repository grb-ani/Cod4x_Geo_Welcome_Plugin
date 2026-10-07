#include "../include/curl.h"
#include "../include/log.h"

#include <stdio.h>
#include <string.h>
#include <curl/curl.h>
#include <malloc.h>

#define BUFFER_SIZE 2048

struct writeCallBackStruct {
    char* contents;
    size_t size;

};

static size_t write_callback(void* contents, size_t size, size_t nmemb, void* arg) {
    DBG(LOG_INFO, "Function called");

    struct writeCallBackStruct* wCallbackStruct = (struct writeCallBackStruct*)arg;
    if (!wCallbackStruct) {
        applicationLog(LOG_ERROR, __PRETTY_FUNCTION__, "Failed to assing pointer");

        return 0;

    }

    size_t readSize = size * nmemb;
    if (readSize + wCallbackStruct->size >= BUFFER_SIZE) {
        applicationLog(LOG_ERROR, __PRETTY_FUNCTION__, "Callback data is longer than the allocated buffer size");

        return 0;

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

    CURL* curl = curl_easy_init();
    if (!curl) {
        applicationLog(LOG_ERROR, __PRETTY_FUNCTION__, "curl is a null pointer, failed to initialize");

        return NULL;

    }

    struct writeCallBackStruct* wCallbackStruct = (struct writeCallBackStruct*)malloc(sizeof(struct writeCallBackStruct));
    if (!wCallbackStruct) {
        applicationLog(LOG_ERROR, __PRETTY_FUNCTION__, "wCallbackStruct is a null pointer, failed to allocate space");

        curl_easy_cleanup(curl);

        return NULL;

    }

    wCallbackStruct->contents = (char*)malloc(BUFFER_SIZE * sizeof(char));
    if (!wCallbackStruct->contents) {
        applicationLog(LOG_ERROR, __PRETTY_FUNCTION__, "contents is a null pointer, failed to allocate memory");

        free(wCallbackStruct);

        curl_easy_cleanup(curl);

        return NULL;

    }

    wCallbackStruct->size = 0;

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, wCallbackStruct);

    int rc = curl_easy_perform(curl);
    if (rc != CURLE_OK) {
        char b[1024];
        snprintf(b, sizeof(b), "Performing of curl failed: %s", curl_easy_strerror(rc));
        applicationLog(LOG_ERROR, __PRETTY_FUNCTION__, b);

        free(wCallbackStruct->contents);
        free(wCallbackStruct);

        curl_easy_cleanup(curl);

        return NULL;

    }

    wCallbackStruct->contents[wCallbackStruct->size] = '\0';
    const char* json = wCallbackStruct->contents;

    free(wCallbackStruct);

    curl_easy_cleanup(curl);

    return json;

}

