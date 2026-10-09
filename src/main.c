#include "log.h"
#include "structs.h"
#include "thread.h"
#include "cjson.h"

#include <stdio.h>
#include <malloc.h>
#include <pthread.h>
#include <curl/curl.h>
#include <time.h>
#include <stdlib.h>

#include "geowelcome_queue.h"

GeoWelcomeResult* pendingResults[64];
int pendingCount = 0;

static pthread_t cliEnterWorldThreadArr[5] = {0};

extern struct applicationConfig* aConfig;


static int isLocalIp(const netadr_t* addr) {
    unsigned char a = addr->ip[0];
    unsigned char b = addr->ip[1];

    return (
        a == 10 ||                                   // 10.0.0.0/8
        (a == 172 && (b >= 16 && b <= 31)) ||        // 172.16.0.0/12
        (a == 192 && b == 168)                       // 192.168.0.0/16
    );
}


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

PCL void OnPlayerConnect(int clientnum, netadr_t* netaddress, char* pbguid, char* userinfo,
                         int authstatus, char* deniedmsg, int deniedmsgbufmaxlen)
{
    if (!netaddress) {
        return;
    }

    int isLocal = isLocalIp(netaddress);

    for (int i = 0; i < sizeof(cliEnterWorldThreadArr) / sizeof(cliEnterWorldThreadArr[0]); i++) {

        if (!cliEnterWorldThreadArr[i]) {

            struct threadCliEnterWorldFunctionArgsStruct* args =
                malloc(sizeof(struct threadCliEnterWorldFunctionArgsStruct));

            if (!args) {
                applicationLog(LOG_ERROR, __PRETTY_FUNCTION__,
                               "args is a null pointer, failed to allocate memory");
                return;
            }

            args->thread = &cliEnterWorldThreadArr[i];

            // Copy client number
            args->clientNum = malloc(sizeof(int));
            if (!args->clientNum) {
                applicationLog(LOG_ERROR, __PRETTY_FUNCTION__,
                               "clientNum is a null pointer, failed to allocate memory");
                free(args);
                return;
            }
            memcpy(args->clientNum, &clientnum, sizeof(int));

            // Deep-copy netadr_t
            args->networdAddressType = malloc(sizeof(netadr_t));
            if (!args->networdAddressType) {
                applicationLog(LOG_ERROR, __PRETTY_FUNCTION__,
                               "networdAddressType is a null pointer, failed to allocate memory");
                free(args->clientNum);
                free(args);
                return;
            }

            memcpy(args->networdAddressType, netaddress, sizeof(netadr_t));

            // bootstrap IP field
            args->bootstrapIp = NULL;

            // Apply bootstrap IP only for local clients
            if (isLocal) {
                const char* envBootstrap = getenv("GEOWELCOME_BOOTSTRAP_IP");
                if (envBootstrap && strlen(envBootstrap) > 0) {

                    applicationLog(LOG_INFO, __PRETTY_FUNCTION__,
                                   "Using bootstrap IP for local client: %s", envBootstrap);

                    size_t len = strlen(envBootstrap);
                    args->bootstrapIp = malloc(len + 1);
                    if (args->bootstrapIp) {
                        memcpy(args->bootstrapIp, envBootstrap, len);
                        args->bootstrapIp[len] = '\0';
                    }
                }
            }

            // Spawn thread
            int rc = pthread_create(&cliEnterWorldThreadArr[i], NULL,
                                    threadCliEnterWorldFunction, args);

            if (rc != 0) {
                applicationLog(LOG_ERROR, __PRETTY_FUNCTION__,
                               "pthread_create failed with code %d", rc);

                threadCliEnterWorldFunctionArgsStructDelete(args);
                cliEnterWorldThreadArr[i] = 0;
            }

            break;

        } else {
            DBG(LOG_WARNING, "%i thread is not empty", i);
        }
    }
}

PCL void OnClientEnterWorld(client_t* client) {
    int clientnum = Plugin_GetClientNumForClient(client);

    for (int i = 0; i < pendingCount; i++) {
        GeoWelcomeResult* r = pendingResults[i];

        if (r->clientNum == clientnum) {
            Plugin_ChatPrintf(
                -1,
                "^3%s ^7has connected from ^2%s^7, ^2%s ^7[^1%s^7] (^3%s^7)",
                Plugin_GetPlayerName(clientnum),
                r->country,
                r->city,
                r->asn,
                r->timezone
            );

            free(r->country);
            free(r->city);
            free(r->asn);
            free(r->timezone);
            free(r);

            // Shift queue down
            for (int j = i + 1; j < pendingCount; j++) {
                pendingResults[j - 1] = pendingResults[j];
            }
            pendingCount--;

            break;
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
