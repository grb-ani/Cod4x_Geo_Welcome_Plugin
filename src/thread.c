#include "thread.h"

#include "structs.h"
#include "curl.h"
#include "cjson.h"
#include "log.h"
#include "geowelcome_queue.h"

#include <malloc.h>
#include <arpa/inet.h>

extern struct applicationConfig* aConfig;

void threadCliEnterWorldFunctionArgsStructDelete(struct threadCliEnterWorldFunctionArgsStruct* cliEnterWorldThreadStruct) {
    DBG(LOG_INFO, "Function called");

    if (cliEnterWorldThreadStruct) {
        if (cliEnterWorldThreadStruct->clientNum) {
            free(cliEnterWorldThreadStruct->clientNum);

        }

        if (cliEnterWorldThreadStruct->networdAddressType) {
            free(cliEnterWorldThreadStruct->networdAddressType);

        }

        if (cliEnterWorldThreadStruct->bootstrapIp) {
            free(cliEnterWorldThreadStruct->bootstrapIp);
        }

        free(cliEnterWorldThreadStruct);

    }

}

void* threadCliEnterWorldFunction(void* arg) {
    DBG(LOG_INFO, "Function called");

    struct threadCliEnterWorldFunctionArgsStruct* cliEnterWorldStruct  = (struct threadCliEnterWorldFunctionArgsStruct*)arg;
    if (!cliEnterWorldStruct) {
        applicationLog(LOG_ERROR, __PRETTY_FUNCTION__, "client is a null pointer, failed to assign value");

        pthread_exit(NULL);

    }

    applicationLog(LOG_INFO, __PRETTY_FUNCTION__,
                "addrType = %d", cliEnterWorldStruct->networdAddressType->type);

    if (!aConfig) {
        applicationLog(LOG_ERROR, __PRETTY_FUNCTION__, "aConfig is a null pointer");

        *cliEnterWorldStruct->thread = 0;
        threadCliEnterWorldFunctionArgsStructDelete(cliEnterWorldStruct);

        pthread_exit(NULL);

    }

    char ip[64];

    if (cliEnterWorldStruct->bootstrapIp) {
        snprintf(ip, sizeof(ip), "%s", cliEnterWorldStruct->bootstrapIp);
    } else {
        netadrtype_t addrType = cliEnterWorldStruct->networdAddressType->type;
        switch (addrType) {
        case NA_IP:
        {
            struct in_addr address;
            memcpy(&address, cliEnterWorldStruct->networdAddressType->ip, 4);
            snprintf(ip, sizeof(ip), "%s", inet_ntoa(address));

            break;

        }
        case NA_IP6:
        {
            struct in6_addr address;
            memcpy(&address, cliEnterWorldStruct->networdAddressType->ip6, 16);
            inet_ntop(AF_INET6, &address, ip, sizeof(ip));

            break;

        }
        default:
            applicationLog(LOG_ERROR, __PRETTY_FUNCTION__, "Couldn't determine user ip address type");

            *cliEnterWorldStruct->thread = 0;
            threadCliEnterWorldFunctionArgsStructDelete(cliEnterWorldStruct);

            pthread_exit(NULL);
        }
    }

    char url[256];
    snprintf(url, sizeof(url), "https://api.ipgeolocation.io/v3/ipgeo?apiKey=%s&ip=%s", aConfig->geoIpApiKey, ip);

    const char* json = curlFetchGeoLocationJson(url);
    if (!json) {
        applicationLog(LOG_ERROR, __PRETTY_FUNCTION__, "json is a null pointer, curl fetch failed");

        *cliEnterWorldStruct->thread = 0;
        threadCliEnterWorldFunctionArgsStructDelete(cliEnterWorldStruct);

        pthread_exit(NULL);

    }

    struct jsonGeoIpStruct* geoIpStruct = jsonGeoIpParse(json);
    if (!geoIpStruct) {
        applicationLog(LOG_ERROR, __PRETTY_FUNCTION__, "geoIpStruct is a null pointer, json parsing failed");

        free((char*)json);

        *cliEnterWorldStruct->thread = 0;
        threadCliEnterWorldFunctionArgsStructDelete(cliEnterWorldStruct);

        pthread_exit(NULL);

    }

    applicationLog(LOG_INFO, __PRETTY_FUNCTION__, "geoIpStruct.countryName: %s, geoIpStruct.cityName: %s, geoIpStruct.asnOrganization: %s, geoIpStruct.timeZone: %s",
                   geoIpStruct->countryName, geoIpStruct->cityName, geoIpStruct->asnOrganization, geoIpStruct->timeZone);
    GeoWelcomeResult* r = malloc(sizeof(GeoWelcomeResult));
    r->clientNum = *(cliEnterWorldStruct->clientNum);
    r->country   = strdup(geoIpStruct->countryName);
    r->city      = strdup(geoIpStruct->cityName);
    r->asn       = strdup(geoIpStruct->asnOrganization);
    r->timezone  = strdup(geoIpStruct->timeZone);

    pthread_mutex_lock(&geoWelcomeMutex);
    pendingResults[pendingCount++] = r;
    pthread_mutex_unlock(&geoWelcomeMutex);

    jsonGeoIpStructDelete(geoIpStruct);

    free((char*)json);

    *cliEnterWorldStruct->thread = 0;
    threadCliEnterWorldFunctionArgsStructDelete(cliEnterWorldStruct);

    pthread_exit(NULL);

}

