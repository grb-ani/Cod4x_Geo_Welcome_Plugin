#include "../include/curl.h"
#include "../libs/pinc.h"

#include <curl/curl.h>
#include <malloc.h>

struct responseStruct {
    char*                       b;
    size_t                      totalSize;

};

static void responseStructDelete(struct responseStruct* resStruct) {
    if (resStruct) {
        if (resStruct->b) {
            free(resStruct->b);
            
            resStruct->b = NULL;

        }

        free(resStruct);

        resStruct = NULL;

    }

}

static size_t write_callback(void* content, size_t size, size_t nmemb, void* userp) {
    struct responseStruct* resStruct = (struct responseStruct*)userp;
    if (!resStruct) {
        Plugin_PrintError("resStruct is a null pointer\n");
        
        return -1;

    }

    if ((resStruct->totalSize + (size * nmemb)) > BUFFER_SIZE) {
        Plugin_PrintError("Server response exceeds 1024 bytes (1kb)");

        return -1;

    }

    memcpy(resStruct->b + resStruct->totalSize, content, size * nmemb);
    resStruct->totalSize += size * nmemb;

    return size * nmemb;

}

const char* curlGetGeoInfo(const char* url) {
    CURL* curl;

    curl = curl_easy_init();
    if (!curl) {
        Plugin_PrintError("Failed to initialize curl\n");

        return NULL;

    }

    struct responseStruct* resStruct = (struct responseStruct*)malloc(sizeof(struct responseStruct));
    if (!resStruct) {
        Plugin_PrintError("resStruct is a null pointer, failed to allocate memory");

        curl_easy_cleanup(curl);

        return NULL;

    }

    resStruct->b = (char*)malloc(1024 * sizeof(char));
    resStruct->totalSize = 0;

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)resStruct);

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        Plugin_PrintError("Performing curl failed\n");

        responseStructDelete(resStruct);

        curl_easy_cleanup(curl);

        return NULL;

    }

    char* json = resStruct->b;
    resStruct->b = NULL;

    responseStructDelete(resStruct);

    return json;

}

