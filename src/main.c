#include "../include/log.h"
#include "../include/structs.h"
#include "../include/thread.h"
#include "../include/cjson.h"

#include <stdio.h>
#include <malloc.h>
#include <pthread.h>
#include <curl/curl.h>
#include <time.h>

static pthread_t cliEnterWorldThreadArr[5] = {0};

extern struct applicationConfig* aConfig;

PCL int OnInit() {
    DBG(LOG_INFO, "Function called");

    FILE* fp = fopen("geoWelcomeConfig.json", "r");
    if (!fp) {
        applicationLog(LOG_ERROR, __PRETTY_FUNCTION__, "Failed to read geoWelcomeConfig.json");

        return 1;

    }

    char b[512];
    char json[1024];
    json[0] = '\0';
    size_t spaceLeft = 1024;
    while (fgets(b, sizeof(b), fp)) {
        size_t readSize = strlen(b);
        if (readSize > spaceLeft) {
            applicationLog(LOG_ERROR, __PRETTY_FUNCTION__, "geoWelcomeConfig.json is longer than the buffer size");

            fclose(fp);

            return 1;

        }

        strncat(json, b, readSize);

        spaceLeft -= readSize;

    }
    fclose(fp);

    aConfig = (struct applicationConfig*)malloc(sizeof(struct applicationConfig));
    if (!aConfig) {
        applicationLog(LOG_ERROR, __PRETTY_FUNCTION__, "aConfig is a null pointer, failed to allocate memory");

        return 1;

    }

    aConfig->geoIpApiKey = jsonApplicationApiKeyParse(json);
    if (!aConfig->geoIpApiKey) {
        applicationLog(LOG_ERROR, __PRETTY_FUNCTION__, "geoIpApiKey is a null pointer, cjson parse failed");

        free(aConfig);
        aConfig = NULL;

        return 1;

    }

    return 0;

}

PCL void OnPlayerConnect(int clientnum, netadr_t* netaddress, char* pbguid, char* userinfo, int authstatus, char* deniedmsg,  int deniedmsgbufmaxlen) {
    if (!netaddress) {
        return;

    }

    for (int i = 0; i < sizeof(cliEnterWorldThreadArr) / sizeof(cliEnterWorldThreadArr[0]); i++) {
        if (!cliEnterWorldThreadArr[i]) {
            struct threadCliEnterWorldFunctionArgsStruct* enterWorldFunctionArgsStruct = (struct threadCliEnterWorldFunctionArgsStruct*)malloc(sizeof(struct threadCliEnterWorldFunctionArgsStruct));
            if (!enterWorldFunctionArgsStruct) {
                applicationLog(LOG_ERROR, __PRETTY_FUNCTION__, "enterWorldFunctionArgsStruct is a null pointer, failed to allocate memory");

                return;

            }

            enterWorldFunctionArgsStruct->thread = &cliEnterWorldThreadArr[i];
            enterWorldFunctionArgsStruct->clientNum = (int*)malloc(sizeof(int));
            memcpy(enterWorldFunctionArgsStruct->clientNum, &clientnum, sizeof(int));

            enterWorldFunctionArgsStruct->networdAddressType = (netadr_t*)malloc(sizeof(netadr_t));
            enterWorldFunctionArgsStruct->networdAddressType->type = netaddress->type;
            memcpy(enterWorldFunctionArgsStruct->networdAddressType->ip, netaddress->ip, 4);
            memcpy(enterWorldFunctionArgsStruct->networdAddressType->ip6, netaddress->ip6, 16);

            pthread_create(&cliEnterWorldThreadArr[i], NULL, threadCliEnterWorldFunction, enterWorldFunctionArgsStruct);

            break;

        } else {
            DBG(LOG_WARNING, "%i thread is not empty", i);

        }

    }

}

PCL void OnInfoRequest(pluginInfo_t* info) {
    info->handlerVersion.major = PLUGIN_HANDLER_VERSION_MAJOR;
    info->handlerVersion.minor = PLUGIN_HANDLER_VERSION_MINOR;

    info->pluginVersion.major = 2;
    info->pluginVersion.minor = 0;
    strncpy(info->fullName,"Cod4X Geo Welcome Plugin", 25);
    strncpy(info->shortDescription,"Geo welcome plugin for changing maps.", 38);
    strncpy(info->longDescription,"This plugin is used to print hello messages to the entire chat on a player entering the world. Coded my LM40 ( DevilHunter )", 125);

}

PCL void OnTerminate() {
	struct timespec dur500000000 = {0, 500000000};

    unsigned char run = 0;
    do {
        for (int i = 0; i < sizeof(cliEnterWorldThreadArr) / sizeof(cliEnterWorldThreadArr[0]); i++) {
            if (cliEnterWorldThreadArr[i]) {
                run = 1;

                nanosleep(&dur500000000, NULL);

                break;

            } else if (run) {
                run = 0;

            }

        }

    } while (run);

    if (aConfig) {
        if (aConfig->geoIpApiKey) {
            free((char*)aConfig->geoIpApiKey);

        }

        free(aConfig);

    }

}

