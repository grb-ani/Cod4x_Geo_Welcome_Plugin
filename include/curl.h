#ifndef COD4X_GEO_WELCOME_PLUGIN_CURL_H
#define COD4X_GEO_WELCOME_PLUGIN_CURL_H

#include <curl/curl.h>

struct curlCacheStruct {
    CURL* curl;
    void* curlCallbackStruct;

};

void curlCacheStructDelete(struct curlCacheStruct* cCacheStruct);
const char* curlFetchGeoLocationJson(const char* url);

#endif

