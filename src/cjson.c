#include "cjson.h"
#include "log.h"

#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <cjson/cJSON.h>

static cJSON* isJsonValid(const char* json) {
    DBG(LOG_INFO, "Function called");

    if (json == NULL) {
        applicationLog(LOG_ERROR, __PRETTY_FUNCTION__, "Provided json is nullptr");

        return NULL;

    }

    const char* ptrErr = NULL;
    cJSON* j = cJSON_ParseWithOpts(json, &ptrErr, 1);
    if (j == NULL) {
        applicationLog(LOG_ERROR, __PRETTY_FUNCTION__, "Parsing failed, printing error location");

        size_t errOffset = ptrErr - json;

        size_t startOffset = (errOffset > 10) ? errOffset - 10 : 0;
        size_t offsetLen = (errOffset + 10ULL < strlen(json)) ? 10 : strlen(json) - startOffset;
        char errMsg[1024];
        sprintf(
            errMsg,
            "Error occurred on parsing the json configuration file, the error occurred on:\n%.*s\nexiting...\n",
            (int)offsetLen,
            json + startOffset
        );

        applicationLog(LOG_ERROR, __PRETTY_FUNCTION__, errMsg);

        return NULL;

    }

    return j;

}

const char* jsonApplicationApiKeyParse(const char* json) {
    cJSON* jsonParser = isJsonValid(json);
    if (!jsonParser) {
        applicationLog(LOG_ERROR, __PRETTY_FUNCTION__, "jsonParser is a null pointer, json is not valid");

        return NULL;

    }

    if (!cJSON_HasObjectItem(jsonParser, "Api key")) {
        applicationLog(LOG_ERROR, __PRETTY_FUNCTION__, "Api key object is not found");

        cJSON_Delete(jsonParser);

        return NULL;

    }

    if (!cJSON_IsString(cJSON_GetObjectItem(jsonParser, "Api key"))) {
        applicationLog(LOG_ERROR, __PRETTY_FUNCTION__, "Api key object value is not found");

        cJSON_Delete(jsonParser);

        return NULL;

    }

    char* apiKey = cJSON_GetObjectItem(jsonParser, "Api key")->valuestring;
    size_t apiKeyLen = strlen(apiKey);
    char* apiKeyCopy = (char*)malloc((apiKeyLen + 1) * sizeof(char));
    if (!apiKeyCopy) {
        applicationLog(LOG_ERROR, __PRETTY_FUNCTION__, "apiKeyCopy is a null pointer, failed to allocate memory");

        cJSON_Delete(jsonParser);

        return NULL;

    }

    strncpy(apiKeyCopy, apiKey, apiKeyLen);
    apiKeyCopy[apiKeyLen] = '\0';

    cJSON_Delete(jsonParser);

    return apiKeyCopy;

}

void jsonGeoIpStructDelete(struct jsonGeoIpStruct* geoIpStruct) {
    if (geoIpStruct) {
        if (geoIpStruct->countryName) {
            free((char*)geoIpStruct->countryName);

        }

        if (geoIpStruct->cityName) {
            free((char*)geoIpStruct->cityName);

        }

        if (geoIpStruct->asnOrganization) {
            free((char*)geoIpStruct->asnOrganization);

        }

        if (geoIpStruct->timeZone) {
            free((char*)geoIpStruct->timeZone);

        }

        free(geoIpStruct);

    }

}

struct jsonGeoIpStruct* jsonGeoIpParse(const char* json) {
    cJSON* jsonParser = isJsonValid(json);
    if (!jsonParser) {
        applicationLog(LOG_ERROR, __PRETTY_FUNCTION__, "jsonParser is a null pointer, json is not valid");
        return NULL;
    }

    // Only require "location" and "asn"
    if (
        !cJSON_HasObjectItem(jsonParser, "location") ||
        !cJSON_HasObjectItem(jsonParser, "asn")
    ) {
        applicationLog(LOG_ERROR, __PRETTY_FUNCTION__, "Missing required top-level elements (location/asn)");
        cJSON_Delete(jsonParser);
        return NULL;
    }

    cJSON* locationObj = cJSON_GetObjectItem(jsonParser, "location");
    if (
        !cJSON_HasObjectItem(locationObj, "country_name") ||
        !cJSON_HasObjectItem(locationObj, "city")
    ) {
        applicationLog(LOG_ERROR, __PRETTY_FUNCTION__, "Missing required location fields");
        cJSON_Delete(jsonParser);
        return NULL;
    }

    if (
        !cJSON_IsString(cJSON_GetObjectItem(locationObj, "country_name")) ||
        !cJSON_IsString(cJSON_GetObjectItem(locationObj, "city"))
    ) {
        applicationLog(LOG_ERROR, __PRETTY_FUNCTION__, "Invalid location field types");
        cJSON_Delete(jsonParser);
        return NULL;
    }

    cJSON* asnObj = cJSON_GetObjectItem(jsonParser, "asn");
    if (!cJSON_HasObjectItem(asnObj, "organization") ||
        !cJSON_IsString(cJSON_GetObjectItem(asnObj, "organization")))
    {
        applicationLog(LOG_ERROR, __PRETTY_FUNCTION__, "Missing or invalid ASN organization");
        cJSON_Delete(jsonParser);
        return NULL;
    }

    // Time zone is OPTIONAL — fallback to "Unknown"
    const char* timeZoneStr = "Unknown";

    cJSON* timeZoneObj = cJSON_GetObjectItem(jsonParser, "time_zone");
    if (timeZoneObj) {
        cJSON* tzCurrent = cJSON_GetObjectItem(timeZoneObj, "current_time");
        cJSON* tzName    = cJSON_GetObjectItem(timeZoneObj, "name");

        if (tzCurrent && cJSON_IsString(tzCurrent)) {
            timeZoneStr = tzCurrent->valuestring;
        } else if (tzName && cJSON_IsString(tzName)) {
            timeZoneStr = tzName->valuestring;
        }
    }

    // Allocate struct
    struct jsonGeoIpStruct* geoIpStruct = malloc(sizeof(struct jsonGeoIpStruct));
    if (!geoIpStruct) {
        applicationLog(LOG_ERROR, __PRETTY_FUNCTION__, "geoIpStruct allocation failed");
        cJSON_Delete(jsonParser);
        return NULL;
    }

    geoIpStruct->countryName     = NULL;
    geoIpStruct->cityName        = NULL;
    geoIpStruct->asnOrganization = NULL;
    geoIpStruct->timeZone        = NULL;

    // Copy country_name
    const char* countryName = cJSON_GetObjectItem(locationObj, "country_name")->valuestring;
    geoIpStruct->countryName = strdup(countryName);

    // Copy city
    const char* cityName = cJSON_GetObjectItem(locationObj, "city")->valuestring;
    geoIpStruct->cityName = strdup(cityName);

    // Copy ASN organization
    const char* asnOrganization = cJSON_GetObjectItem(asnObj, "organization")->valuestring;
    geoIpStruct->asnOrganization = strdup(asnOrganization);

    // Copy time zone (fallback-safe)
    geoIpStruct->timeZone = strdup(timeZoneStr);

    cJSON_Delete(jsonParser);
    return geoIpStruct;
}

