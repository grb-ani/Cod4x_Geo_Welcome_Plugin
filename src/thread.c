#include "../include/thread.h"
#include "../include/log.h"
#include "../libs/pinc.h"

void* cliEnterWorldThread(void* arg) {
    DBG(LOG_INFO, "Function called");

    client_t* client = NULL;
    client = (client_t*)arg;
    if (!client) {
        applicationLog(LOG_ERROR, __PRETTY_FUNCTION__, "client is a null pointer, failed to assign value");

        return NULL;

    }

    netadrtype_t addressType = client->netchan.remoteAddress.type;
    switch (addressType) {
        case NA_IP:
        case NA_IP6:
        default:

    }


    return NULL;

}
