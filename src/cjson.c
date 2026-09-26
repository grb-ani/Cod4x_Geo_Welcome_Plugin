#include "../libs/pinc.h"
#include "../include/cjson.h"
#include "../include/log.h"

#define PCRE2_CODE_UNIT_WIDTH 8

#include <stdio.h>
#include <cjson/cJSON.h>
#include <string.h>
#include <malloc.h>
#include <pcre2.h>

static unsigned char isJsonValid(const char* json) {
    DBG(LOG_INFO, "Function called");

    if (json == NULL) {
        applicationLog(LOG_ERROR, __PRETTY_FUNCTION__, "Provided json is nullptr");

        return 0;

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

        return 0;

    }

    cJSON_Delete(j);

    return 1;

}

void jsonGeoInfoStructDelete(struct jsonGeoInfoStruct* geoInfoStruct) {
    if (geoInfoStruct) {
        if (geoInfoStruct->countryName) {
            free((char*)geoInfoStruct->countryName);

            geoInfoStruct->countryName = NULL;

        }

        if (geoInfoStruct->cityName) {
            free((char*)geoInfoStruct->cityName);

            geoInfoStruct->cityName = NULL;

        }

        if (geoInfoStruct->asnOrganizatoin) {
            free((char*)geoInfoStruct->asnOrganizatoin);

            geoInfoStruct->asnOrganizatoin = NULL;

        }

        if (geoInfoStruct->time) {
            free((char*)geoInfoStruct->time);

            geoInfoStruct->time = NULL;

        }

        geoInfoStruct = NULL;

    }

}

struct jsonGeoInfoStruct* jsonGeoInfoParse(const char* json) {
    if (!isJsonValid(json)) {
        applicationLog(LOG_ERROR, __PRETTY_FUNCTION__, "Provided json is not valid");

        return NULL;

    }

    cJSON* jsonParser = cJSON_Parse(json);
    if (!jsonParser) {
        applicationLog(LOG_ERROR, __PRETTY_FUNCTION__, "jsonParser is a null pointer");

        return NULL;
        
    }

    struct jsonGeoInfoStruct* geoInfoStruct = NULL;

    if (
        cJSON_HasObjectItem(jsonParser, "location") &&
        cJSON_IsObject(cJSON_GetObjectItem(jsonParser, "location"))
       ) {
        cJSON* locationObj = cJSON_GetObjectItem(jsonParser, "location");
        if (!locationObj) {
            applicationLog(LOG_ERROR, __PRETTY_FUNCTION__, "locationObj is a null pointer");

            cJSON_Delete(jsonParser);

            return NULL;

        }

        if (
            cJSON_HasObjectItem(locationObj, "country_name") &&
            cJSON_HasObjectItem(locationObj, "city")
          ) {
            geoInfoStruct = (struct jsonGeoInfoStruct*)malloc(sizeof(struct jsonGeoInfoStruct));
            if (!geoInfoStruct) {
                applicationLog(LOG_ERROR, __PRETTY_FUNCTION__, "geoInfoStruct is a null pointer, failed to allocate space");

                cJSON_Delete(jsonParser);

                return NULL;

            }
            geoInfoStruct->countryName = NULL;
            geoInfoStruct->cityName = NULL;
            geoInfoStruct->asnOrganizatoin = NULL;
            geoInfoStruct->time = NULL;

            char* countryName = cJSON_GetObjectItem(locationObj, "country_name")->valuestring;
            size_t countryNameLen = strlen(countryName);
            char* countryNameCopy = (char*)malloc((countryNameLen + 1) * sizeof(char));
            strncpy(countryNameCopy, countryName, countryNameLen);
            countryNameCopy[countryNameLen] = '\0';
            geoInfoStruct->countryName = countryNameCopy;

            char* cityName = cJSON_GetObjectItem(locationObj, "city")->valuestring;
            size_t cityNameLen = strlen(cityName);
            char* cityNameCopy = (char*)malloc((cityNameLen + 1) * sizeof(char));
            strncpy(cityNameCopy, cityName, cityNameLen);
            cityNameCopy[cityNameLen] = '\0';
            geoInfoStruct->cityName = cityNameCopy;

        } else {
            applicationLog(LOG_ERROR, __PRETTY_FUNCTION__, "Soem element is not present in the provided object");

            cJSON_Delete(jsonParser);

            return NULL;

        }

    } else {
        applicationLog(LOG_ERROR, __PRETTY_FUNCTION__, "location object is not present in the provided json");

        cJSON_Delete(jsonParser);

        return NULL;

    }

    if (
        cJSON_HasObjectItem(jsonParser, "asn") &&
        cJSON_IsObject(cJSON_GetObjectItem(jsonParser, "asn"))
       ) {
        cJSON* asnObj = cJSON_GetObjectItem(jsonParser, "asn");
        if (!asnObj) {
            applicationLog(LOG_ERROR, __PRETTY_FUNCTION__, "asnObj is a null pointer");

            jsonGeoInfoStructDelete(geoInfoStruct);

            cJSON_Delete(jsonParser);

            return NULL;

        }

        if (cJSON_HasObjectItem(asnObj, "organization")) {
            char* asnOrganization = cJSON_GetObjectItem(asnObj, "organization")->valuestring;
            size_t asnOrganizationLen = strlen(asnOrganization);
            char* asnOrganizationCopy = (char*)malloc((asnOrganizationLen + 1) * sizeof(char));
            strncpy(asnOrganizationCopy, asnOrganization, asnOrganizationLen);
            asnOrganizationCopy[asnOrganizationLen] = '\0';
            geoInfoStruct->asnOrganizatoin = asnOrganizationCopy;

        } else {
            applicationLog(LOG_ERROR, __PRETTY_FUNCTION__, "organization object is not present");

            jsonGeoInfoStructDelete(geoInfoStruct);

            cJSON_Delete(jsonParser);

            return NULL;

        }

    } else {
        applicationLog(LOG_ERROR, __PRETTY_FUNCTION__, "asn object is not present in the provided json");

        cJSON_Delete(jsonParser);

        return NULL;
    }

    if (
        cJSON_HasObjectItem(jsonParser, "time_zone") &&
        cJSON_IsObject(cJSON_GetObjectItem(jsonParser, "time_zone"))
       ) {
        cJSON* timeZoneObj = cJSON_GetObjectItem(jsonParser, "time_zone");
        if (!timeZoneObj) {
            applicationLog(LOG_ERROR, __PRETTY_FUNCTION__, "timeZoneObj is a null pointer");

            jsonGeoInfoStructDelete(geoInfoStruct);

            cJSON_Delete(jsonParser);

            return NULL;

        }

        if (
            cJSON_HasObjectItem(timeZoneObj, "current_time")
           ) {
            char* playerCurrentTime = cJSON_GetObjectItem(timeZoneObj, "current_time")->valuestring;
            size_t playerCurrentTimeLen = strlen(playerCurrentTime);

            // 2026-09-26 16:44:16.974+0330
            const char *pattern = "(\\d{2}):(\\d{2}):(\\d{2})";

            int errorNumber;
            PCRE2_SIZE errorOffset;

            pcre2_code* regex = pcre2_compile(
                    (PCRE2_SPTR)pattern,
                    PCRE2_ZERO_TERMINATED,
                    0,
                    &errorNumber,
                    &errorOffset,
                    NULL
            );

            if (!regex) {
                Plugin_PrintError("regex pattern compilation failed\n");

                jsonGeoInfoStructDelete(geoInfoStruct);

                cJSON_Delete(jsonParser);

                return NULL;

            }

            pcre2_match_data* match_data = pcre2_match_data_create_from_pattern(regex, NULL);

            int rc = pcre2_match(
                    regex,
                    (PCRE2_SPTR)playerCurrentTime,
                    playerCurrentTimeLen,
                    0,
                    0,
                    match_data,
                    NULL 
            );

            if (rc) {
                PCRE2_SIZE* ovector = pcre2_get_ovector_pointer(match_data);
                size_t currentTimeLen = (size_t)(ovector[1] - ovector[0]);
                char* currentTime = (char*)malloc((currentTimeLen + 1) * sizeof(char));
                strncpy(currentTime, playerCurrentTime + ovector[0], currentTimeLen);
                currentTime[currentTimeLen] = '\0';
                geoInfoStruct->time = currentTime;

            } else {
                Plugin_PrintError("No matches for the pattern in the playerCurrentTime found\n");

                pcre2_match_data_free(match_data);
                pcre2_code_free(regex);

                jsonGeoInfoStructDelete(geoInfoStruct);

                cJSON_Delete(jsonParser);

                return NULL;

            }

            pcre2_match_data_free(match_data);
            pcre2_code_free(regex);

        } else {
            Plugin_PrintError("current_time object is not present in the provided json\n");

            jsonGeoInfoStructDelete(geoInfoStruct);

            cJSON_Delete(jsonParser);

            return NULL;

        }

    } else {
        Plugin_PrintError("time_zone object is not present in the provided json\n");

        jsonGeoInfoStructDelete(geoInfoStruct);

        cJSON_Delete(jsonParser);

        return NULL;

    }

    cJSON_Delete(jsonParser);

    return geoInfoStruct;

}

const char* jsonApplicationConfigParse(const char* json) {
    if (!isJsonValid(json)) {
        applicationLog(LOG_ERROR, __PRETTY_FUNCTION__, "Provided json is not valid");

        return NULL;

    }

    cJSON* jsonParser = cJSON_Parse(json);
    if (!jsonParser) {
        applicationLog(LOG_ERROR, __PRETTY_FUNCTION__, "jsonParser is a null pointer");

        return NULL;
        
    }

    if (
        cJSON_HasObjectItem(jsonParser, "geoIpApiKey")
       ) {
        char* apiKey = cJSON_GetObjectItem(jsonParser, "geoIpApiKey")->valuestring;
        size_t apiKeyLen = strlen(apiKey);
        char* apiKeyCopy = (char*)malloc((apiKeyLen + 1) * sizeof(char));
        strncpy(apiKeyCopy, apiKey, apiKeyLen);
        apiKeyCopy[apiKeyLen] = '\0';

        cJSON_Delete(jsonParser);

        return apiKeyCopy;

    } else {
        Plugin_PrintError("Couldn't find the api key object inside the provided json, make sure to define the api key name as\"geoIpApiKey\"\n");

        cJSON_Delete(jsonParser);

        return NULL;

    }

}
