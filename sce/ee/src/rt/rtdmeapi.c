#include <stddef.h>
#include <string.h>

#include <librtdme.h>

#include "rt/rtdme.h"
#include "rt/rtmutex.h"
#include "rt/rttitle.h"

enum {
    // The result of an entry point whose lock failed.
    kResultLockFailed = 43,
    // The characters of the version text before the version itself.
    kVersionPrefixLength = 13,
};

// NTSC-U/C: 0x003b6170
static const char g_szDmeVersion[] = "dme version: 1.25.0003";
// NTSC-U/C: 0x003b6187
static unsigned char g_bNetStarted;
// NTSC-U/C: 0x003b6188
static int g_nNetStartCount;
// NTSC-U/C: 0x003b618c
static unsigned char g_bNetLockCreated;
// NTSC-U/C: 0x00519bc8
static RtMutex g_netLock;

// Take the lock of the library for an entry point.
static inline int RtNetEnter(void) {
    if (g_bNetStarted != 1) {
        return RT_DME_RESULT_NOT_STARTED;
    }
    if (g_bNetLockCreated != g_bNetStarted) {
        return kResultLockFailed;
    }
    if (RtLockMutex(&g_netLock, NULL) != 0) {
        return kResultLockFailed;
    }
    return 0;
}

// Release the lock of the library and report the result of an entry point.
static inline int RtNetLeave(int nResult) {
    if (RtUnlockMutex(&g_netLock, NULL) != 0) {
        return kResultLockFailed;
    }
    return nResult;
}

int RtDmeGetVersion(char *pszVersion) {
    if (pszVersion == NULL) {
        return RT_DME_RESULT_INVALID;
    }
    strcpy(pszVersion, &g_szDmeVersion[kVersionPrefixLength]);
    return 0;
}

int rtNetDefaultSettings(rtNetSettings *params) {
    if (params == NULL) {
        return RT_DME_RESULT_INVALID;
    }
    params->version = RT_NET_SETTINGS_VERSION;
    params->port = 0;
    params->keyPair = NULL;
    params->applicationId = RtTitleGetApplicationId();
    return 0;
}

int rtNetInitialize(const rtNetSettings *params, rtNetInitStatus *status) {
    int nVersion;
    int nResult;

    if (status == NULL) {
        return RT_DME_RESULT_INVALID;
    }
    status->result = RT_DME_RESULT_STARTUP_FAILED;
    status->version = RT_NET_SETTINGS_VERSION;
    if (params == NULL) {
        status->result = RT_DME_RESULT_INVALID;
        return RT_DME_RESULT_INVALID;
    }
    nVersion = params->version;
    if (nVersion != RT_NET_SETTINGS_VERSION) {
        // Yes, the binary returns INVALID while the status reports BAD_VERSION.
        status->result = RT_DME_RESULT_BAD_VERSION;
        return RT_DME_RESULT_INVALID;
    }
    if (g_nNetStartCount != 0) {
        ++g_nNetStartCount;
        status->result = 0;
        return 0;
    }
    if (g_bNetLockCreated == nVersion || RtCreateMutex(&g_netLock) != 0) {
        status->result = kResultLockFailed;
        return kResultLockFailed;
    }
    g_bNetLockCreated = nVersion;
    if (RtLockMutex(&g_netLock, NULL) != 0) {
        return kResultLockFailed;
    }
    nResult = RtDmeStartup(params, status);
    if (RtUnlockMutex(&g_netLock, NULL) != 0) {
        return kResultLockFailed;
    }
    if (nResult != 0) {
        RtDeleteMutex(&g_netLock);
        g_bNetLockCreated = 0;
        g_bNetStarted = 0;
        return nResult;
    }
    g_nNetStartCount = nVersion;
    g_bNetStarted = nVersion;
    return 0;
}

int rtNetClose(void) {
    int nResult;

    if (g_bNetStarted != 1 || g_nNetStartCount == 0) {
        return RT_DME_RESULT_NOT_STARTED;
    }
    if (g_bNetLockCreated != g_bNetStarted) {
        return kResultLockFailed;
    }
    if (g_nNetStartCount >= 2) {
        --g_nNetStartCount;
        return 0;
    }
    if (RtLockMutex(&g_netLock, NULL) != 0) {
        return kResultLockFailed;
    }
    nResult = RtDmeShutdown();
    if (RtUnlockMutex(&g_netLock, NULL) != 0) {
        return kResultLockFailed;
    }
    if (nResult != 0) {
        return nResult;
    }
    if (RtDeleteMutex(&g_netLock) != 0) {
        return kResultLockFailed;
    }
    g_bNetLockCreated = 0;
    g_bNetStarted = 0;
    g_nNetStartCount = 0;
    return nResult;
}

int rtDmeBeginFields(void) {
    int nResult;

    nResult = RtNetEnter();
    if (nResult != 0) {
        return nResult;
    }
    return RtNetLeave(RtDmeBeginFields());
}

int rtDmeAddField(const void *record, const void *field, int size, int count) {
    int nResult;

    nResult = RtNetEnter();
    if (nResult != 0) {
        return nResult;
    }
    return RtNetLeave(RtDmeAddField(record, field, size, count));
}

int rtDmeRegisterClassMessage(int *type, int messageClass, rtDmeMessageHandler handler) {
    int nResult;

    nResult = RtNetEnter();
    if (nResult != 0) {
        return nResult;
    }
    return RtNetLeave(RtDmeRegisterMessageType(type, messageClass, handler));
}

int rtDmeRegisterMessage(int *type, rtDmeMessageHandler handler) {
    int nResult;

    nResult = RtNetEnter();
    if (nResult != 0) {
        return nResult;
    }
    return RtNetLeave(RtDmeRegisterApplicationMessage(type, handler));
}

int rtDmeSend(signed char flags,
              int connection,
              int receiver,
              int type,
              unsigned short size,
              const rtDmeMessage *message) {
    int nResult;

    nResult = RtNetEnter();
    if (nResult != 0) {
        return nResult;
    }
    return RtNetLeave(
        RtDmeSendApplicationMessage(flags, connection, receiver, type, size, message));
}

int rtNetSetMemoryFunctions(rtAllocFunc alloc, rtFreeFunc release) {
    return RtDmeSetMemoryFunctions(alloc, release);
}

int rtDmeInitConnectParams(rtDmeConnectParams *params) {
    int nResult;

    nResult = RtNetEnter();
    if (nResult != 0) {
        return nResult;
    }
    return RtNetLeave(RtDmeInitConnectParams(params));
}

int rtDmeConnect(const rtDmeConnectParams *params, rtDmeConnectStatus *status) {
    int nResult;

    nResult = RtNetEnter();
    if (nResult != 0) {
        return nResult;
    }
    return RtNetLeave(RtDmeConnect(params, status));
}

int rtDmeInitHostParams(rtDmeHostParams *params) {
    int nResult;

    nResult = RtNetEnter();
    if (nResult != 0) {
        return nResult;
    }
    return RtNetLeave(RtDmeInitHostParams(params));
}

int rtDmeHost(const rtDmeHostParams *params, rtDmeConnectStatus *status) {
    int nResult;

    nResult = RtNetEnter();
    if (nResult != 0) {
        return nResult;
    }
    return RtNetLeave(RtDmeHost(params, status));
}

int rtDmeInitAddressQuery(rtDmeAddressQuery *query) {
    int nResult;

    nResult = RtNetEnter();
    if (nResult != 0) {
        return nResult;
    }
    return RtNetLeave(RtDmeInitAddressQuery(query));
}

int rtDmeQueryAddresses(const rtDmeAddressQuery *query, rtDmeConnectStatus *status) {
    int nResult;

    nResult = RtNetEnter();
    if (nResult != 0) {
        return nResult;
    }
    return RtNetLeave(RtDmeQueryAddresses(query, status));
}

int rtDmeInitAddressReport(rtDmeAddressReport *report) {
    int nResult;

    nResult = RtNetEnter();
    if (nResult != 0) {
        return nResult;
    }
    return RtNetLeave(RtDmeInitAddressReport(report));
}

int rtDmeReportAddresses(const rtDmeAddressReport *report, rtDmeConnectStatus *status) {
    int nResult;

    nResult = RtNetEnter();
    if (nResult != 0) {
        return nResult;
    }
    return RtNetLeave(RtDmeReportAddresses(report, status));
}

int rtDmeEnableConnection(int connection, int enable) {
    int nResult;

    nResult = RtNetEnter();
    if (nResult != 0) {
        return nResult;
    }
    return RtNetLeave(RtDmeEnableConnection(connection, enable));
}

int rtNetSetBufferSizes(int sendSize, int receiveSize) {
    int nResult;

    nResult = RtNetEnter();
    if (nResult != 0) {
        return nResult;
    }
    return RtNetLeave(RtDmeStartDatagramService(sendSize, receiveSize));
}

int rtDmeGetLocalAddress(char address[RT_NET_ADDRESS_SIZE]) {
    int nResult;

    nResult = RtNetEnter();
    if (nResult != 0) {
        return nResult;
    }
    return RtNetLeave(RtDmeGetLocalAddress(address));
}

int rtDmeGetSenderAddress(unsigned char address[4]) {
    int nResult;

    nResult = RtNetEnter();
    if (nResult != 0) {
        return nResult;
    }
    return RtNetLeave(RtDmeGetSenderAddress(address));
}

int rtDmeGetLocalClient(int *client, int connection) {
    int nResult;

    nResult = RtNetEnter();
    if (nResult != 0) {
        return nResult;
    }
    return RtNetLeave(RtDmeGetLocalClient(client, connection));
}

int rtDmeGetLocalTime(unsigned int *time) {
    int nResult;

    nResult = RtNetEnter();
    if (nResult != 0) {
        return nResult;
    }
    return RtNetLeave(RtDmeGetLocalTime(time));
}

int rtDmeUpdate(void) {
    int nResult;

    nResult = RtNetEnter();
    if (nResult != 0) {
        return nResult;
    }
    return RtNetLeave(RtDmeUpdate());
}

int rtDmeDisconnect(int connection) {
    int nResult;

    nResult = RtNetEnter();
    if (nResult != 0) {
        return nResult;
    }
    return RtNetLeave(RtDmeDisconnect(connection));
}
