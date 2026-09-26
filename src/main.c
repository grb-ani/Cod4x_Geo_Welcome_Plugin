#include "../libs/pinc.h"
#include "../include/structs.h"
#include "../include/cjson.h"
#include "../include/curl.h"

#include <stdio.h>
#include <malloc.h>

extern struct applicationConfig* aConfig;

PCL int OnInit() {
    aConfig = (struct applicationConfig*)malloc(sizeof(struct applicationConfig));
    if (!aConfig) {
        Plugin_PrintError("aConfig is a null pointer, failed to allocate memory\n");

        return 1;

    }

    FILE* fp = fopen("geoip.cfg", "r");
    if (!fp) {
        Plugin_PrintError("Couldn't find geoip.cfg for fetching the api key");

        free(aConfig);

        return 1;

    }

    char b[128];
    while (fgets(b, sizeof(b), fp)) {

    }

    aConfig->geoLocationApiKey = jsonApplicationConfigParse(b);
    if (!aConfig->geoLocationApiKey) {
        Plugin_PrintError("geoLocationApiKey is a null pointer, json parsing failed");

        free(aConfig);

        return 1;

    }

    return 0;

}

PCL void OnClientEnterWorld(client_t* client) {
    char url[256];
    snprintf(
        url,
        sizeof(url),
        "https://api.ipgeolocation.io/v3/ipgeo?apiKey=%s&ip=%u.%u.%u.%u",
        aConfig->geoLocationApiKey,
        client->netchan.remoteAddress.ip[0],
        client->netchan.remoteAddress.ip[1],
        client->netchan.remoteAddress.ip[2],
        client->netchan.remoteAddress.ip[3]
    );

    const char* json = curlGetGeoInfo(url);
    if (!json) {
        Plugin_PrintError("Error occurred on getting response data from server\n");

        return;

    }

    struct jsonGeoInfoStruct* geoInfoStruct = jsonGeoInfoParse(json);
    if (!geoInfoStruct) {
        Plugin_PrintError("Parsing fetched data failed\n");

        free((char*)json);

        return;

    }

    Plugin_ChatPrintf(
        -1,
        "%s from %s/%s connected to the server via network: %s, local time: %s", 
        Plugin_GetPlayerName(NUMFORCLIENT(client)),
        geoInfoStruct->countryName,
        geoInfoStruct->cityName,
        geoInfoStruct->asnOrganizatoin,
        geoInfoStruct->time
    );

    jsonGeoInfoStructDelete(geoInfoStruct);

    free((char*)json);

}

PCL void OnInfoRequest(pluginInfo_t *info) {
    info->handlerVersion.major = PLUGIN_HANDLER_VERSION_MAJOR;
    info->handlerVersion.minor = PLUGIN_HANDLER_VERSION_MINOR;

    info->pluginVersion.major = 2;
    info->pluginVersion.minor = 0;
    strncpy(info->fullName,"Cod4X GeoIp Welcome Plugin", 27);
    strncpy(info->shortDescription,"Geolocation based welcome plugin", 33);
    strncpy(info->longDescription,"This plugin is used send welcome messages to every player entering the game. Coded my LM40 ( DevilHunter )", 107);
}

PCL void OnTerminate() {

}

