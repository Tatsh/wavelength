#include <string.h>

#include <eekernel.h>
#include <libeznetcnf.h>
#include <sifdev.h>
#include <sifrpc.h>

enum {
    kEzNetCnfServerId = 0x75499128,
    // The server functions.
    kEzNetCnfFunctionGetList = 1,
    kEzNetCnfFunctionLoadEntry = 2,
    // The iterations of the wait between two attempts to bind.
    kEzNetCnfBindDelay = 0x10000,
    kEzNetCnfFileNameSize = 256,
    kEzNetCnfUserNameSize = 48,
    kEzNetCnfNotBound = -1,
};

// The request the server reads and answers in place.
typedef struct {
    union {
        void *destination; // The address in main memory that receives the reply.
        int result;        // The result the server reports.
    };
    unsigned int flags;
    unsigned int reserved[2];
    char fileName[kEzNetCnfFileNameSize];
    char userName[kEzNetCnfUserNameSize];
} EzNetCnfRequest;

static const char kEzNetCnfModuleName[] = "eznetcnf";

// NTSC-U/C: 0x003b14c0
static int g_nEznetcnfBound = 0;
// NTSC-U/C: 0x00440780
static sceSifClientData g_eznetcnfClient __attribute__((aligned(64)));
// NTSC-U/C: 0x004407c0
static EzNetCnfRequest g_eznetcnfRequest __attribute__((aligned(64)));

int ezNetCnfInit(void) {
    const int result = sceSifSearchModuleByName(kEzNetCnfModuleName);
    if (result < 0) {
        scePrintf("\"%s\" module not found (%d)\n", kEzNetCnfModuleName, result);
    }
    for (;;) {
        if (sceSifBindRpc(&g_eznetcnfClient, kEzNetCnfServerId, 0) < 0) {
            scePrintf("could not bind to eznetcnf service\n");
        }
        if (g_eznetcnfClient.serve != 0) {
            break;
        }
        for (int i = kEzNetCnfBindDelay; i != -1; --i) {
            __asm__ volatile("nop\n\tnop\n\tnop\n\tnop\n\tnop");
        }
    }
    g_nEznetcnfBound = 1;
    return 1;
}

int ezNetCnfGetList(const char *fname, int flags, ezNetCnfCombinationList_t *list) {
    if (g_nEznetcnfBound == 0) {
        scePrintf("**** eznetcnf RPC server not currently running\n");
        return kEzNetCnfNotBound;
    }
    strcpy(g_eznetcnfRequest.fileName, fname);
    g_eznetcnfRequest.destination = list;
    g_eznetcnfRequest.flags = flags;
    FlushCache(WRITEBACK_DCACHE);
    sceSifCallRpc(&g_eznetcnfClient,
                  kEzNetCnfFunctionGetList,
                  0,
                  &g_eznetcnfRequest,
                  sizeof(g_eznetcnfRequest),
                  &g_eznetcnfRequest,
                  sizeof(g_eznetcnfRequest),
                  0,
                  0);
    return g_eznetcnfRequest.result;
}

ezNetCnfCombination_t *ezNetCnfGetCombination(ezNetCnfCombinationList_t *list, int number) {
    if (list == 0 || list->listLength == 0) {
        scePrintf("ezNetCnfGetCombination: pList is empty\n");
        return 0;
    }
    if (number <= 0 || list->listLength < number) {
        scePrintf("ezNetCnfGetCombination: invalid Combination number (%d)\n", number);
        return 0;
    }
    return &list->list[number - 1];
}

int ezNetCnfLoadEntry(const char *fname, const char *usrname, ezNetCnfData_t *dest) {
    if (g_nEznetcnfBound == 0) {
        scePrintf("**** eznetcnf RPC server not currently running\n");
        return kEzNetCnfNotBound;
    }
    strcpy(g_eznetcnfRequest.fileName, fname);
    strcpy(g_eznetcnfRequest.userName, usrname);
    // Unlike ezNetCnfGetList(), the call does not write the data cache back first.
    g_eznetcnfRequest.destination = dest;
    sceSifCallRpc(&g_eznetcnfClient,
                  kEzNetCnfFunctionLoadEntry,
                  0,
                  &g_eznetcnfRequest,
                  sizeof(g_eznetcnfRequest),
                  &g_eznetcnfRequest,
                  sizeof(g_eznetcnfRequest),
                  0,
                  0);
    return g_eznetcnfRequest.result;
}
