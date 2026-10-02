#include "../include/cjson.h"
#include "../include/log.h"

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

    if (
        !cJSON_HasObjectItem(jsonParser, "location") ||
        !cJSON_HasObjectItem(jsonParser, "asn") ||
        !cJSON_HasObjectItem(jsonParser, "time_zone")
       ) {
        applicationLog(LOG_ERROR, __PRETTY_FUNCTION__, "Some element is not present in the provided jsonParser");

        cJSON_Delete(jsonParser);

        return NULL;

    }

    cJSON* locationObj = cJSON_GetObjectItem(jsonParser, "location");
    if (
        !cJSON_HasObjectItem(locationObj, "country_name") ||
        !cJSON_HasObjectItem(locationObj, "city")
       ) {
        applicationLog(LOG_ERROR, __PRETTY_FUNCTION__, "Some element is not present in the provided locationObj");

        goto child_element_not_present_exit;

    }

    cJSON* asnObj = cJSON_GetObjectItem(jsonParser, "asn");
    if (!cJSON_HasObjectItem(asnObj, "organization")) {
        applicationLog(LOG_ERROR, __PRETTY_FUNCTION__, "Some element is not present in the provided asnObj");

        goto child_element_not_present_exit;

    }

    cJSON* timeZoneObj = cJSON_GetObjectItem(jsonParser, "time_zone");
    if (!cJSON_HasObjectItem(timeZoneObj, "current_time")) {
        applicationLog(LOG_ERROR, __PRETTY_FUNCTION__, "Some element is not present in the provided timeZoneObj");

child_element_not_present_exit:
        cJSON_Delete(jsonParser);

        return NULL;

    }

    struct jsonGeoIpStruct* geoIpStruct = (struct jsonGeoIpStruct*)malloc(sizeof(struct jsonGeoIpStruct));
    if (!geoIpStruct) {
        applicationLog(LOG_ERROR, __PRETTY_FUNCTION__, "geoIpStruct is a null pointer, failed to allocate memory");

        cJSON_Delete(jsonParser);

        return NULL;

    }

    char* countryName = cJSON_GetObjectItem(locationObj, "country_name")->valuestring;
    size_t countryNameLen = strlen(countryName);
    char* countryNameCopy = (char*)malloc((countryNameLen + 1) * sizeof(char));
    strncpy(countryNameCopy, countryName, countryNameLen);
    countryNameCopy[countryNameLen] = '\0';
    geoIpStruct->countryName = countryNameCopy;

    char* cityName = cJSON_GetObjectItem(locationObj, "city")->valuestring;
    size_t cityNameLen = strlen(cityName);
    char* cityNameCopy = (char*)malloc((cityNameLen + 1) * sizeof(char));
    strncpy(cityNameCopy, cityName, cityNameLen);
    cityNameCopy[cityNameLen] = '\0';
    geoIpStruct->cityName = cityNameCopy;

    char* asnOrganization = cJSON_GetObjectItem(asnObj, "organization")->valuestring;
    size_t asnOrganizationLen = strlen(asnOrganization);
    char* asnOrganizationCopy = (char*)malloc((asnOrganizationLen + 1) * sizeof(char));
    strncpy(asnOrganizationCopy, asnOrganization, asnOrganizationLen);
    asnOrganizationCopy[asnOrganizationLen] = '\0';
    geoIpStruct->asnOrganization = asnOrganizationCopy;

    char* timeZone = cJSON_GetObjectItem(asnObj, "organization")->valuestring;
    size_t timeZoneLen = strlen(timeZone);
    char* timeZoneCopy = (char*)malloc((timeZoneLen + 1) * sizeof(char));
    strncpy(timeZoneCopy, timeZone, timeZoneLen);
    timeZoneCopy[timeZoneLen] = '\0';
    geoIpStruct->timeZone = timeZoneCopy;

    cJSON_Delete(jsonParser);

    return geoIpStruct;

}

