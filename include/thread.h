#ifndef COD4X_GEO_WELCOME_PLUGIN_THREAD_H
#define COD4X_GEO_WELCOME_PLUGIN_THREAD_H

#include "pinc.h"

#include <pthread.h>


struct threadCliEnterWorldFunctionArgsStruct {
    pthread_t*      thread; // This is passed to set it to zero on exit, so we can check if its finished or not
    int*            clientNum;
    netadr_t*       networdAddressType;
    char*      bootstrapIp;
};

void threadCliEnterWorldFunctionArgsStructDelete(struct threadCliEnterWorldFunctionArgsStruct* cliEnterWorldThreadStruct);
void* threadCliEnterWorldFunction(void* arg);

#endif

