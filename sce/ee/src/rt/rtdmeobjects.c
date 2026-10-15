#include <string.h>

#include "rt/rtdme.h"

enum {
    // The flags the object ranges are sent with.
    kRangeSendFlags = RT_DME_SEND_ORDERED | RT_DME_SEND_GUARANTEED,
    // The result for a missing object range.
    kRangeMissing = 21,
    // The roles a client takes for its objects.
    kObjectRoleReleased = 1,
    kObjectRoleHost = 3,
    kObjectRoleCandidate = 4,
    // The event the receiver of the object ranges gets for a release.
    kRangeEventClaimed = 0,
    kRangeEventReleased = 1,
    // The rank no candidate host reaches.
    kNoRank = 0x0fffffff,
    // The value the object receivers get for a change of the owner of an object.
    kObjectChangedOwner = -2,
    // The client an object is held by when none holds it.
    kNoLockClient = -1,
    // The connection index that stands for none.
    kNoConnection = 4,
};

// NTSC-U/C: 0x003b6be8, PAL: 0x00435aa8
unsigned char g_bDmeObjectRangesSent;
// NTSC-U/C: 0x00519f20
static RtDmeObjectRange **g_ppDmeObjectRanges;

void RtDmeInitObjectRanges(void) {
    g_ppDmeObjectRanges = RtDmeAlloc(RT_DME_CLIENT_COUNT * sizeof(RtDmeObjectRange *));
    g_bDmeObjectRangesSent = 0;
    memset(g_ppDmeObjectRanges, 0, RT_DME_CLIENT_COUNT * sizeof(RtDmeObjectRange *));
}

int RtDmeGetObjectRange(RtDmeObjectRange **ppRange, int nConnection, int nClient) {
    if (ppRange == NULL) {
        return RT_DME_RESULT_INVALID;
    }
    *ppRange = NULL;
    if ((unsigned int)nClient >= RT_DME_CLIENT_COUNT ||
        (unsigned int)nConnection >= RT_DME_CONNECTION_COUNT) {
        return RT_DME_RESULT_INVALID;
    }
    if (nConnection != g_nDmeActiveConnection) {
        return 0;
    }
    if (g_ppDmeObjectRanges == NULL) {
        return kRangeMissing;
    }
    *ppRange = g_ppDmeObjectRanges[nClient];
    return 0;
}

RtDmeObjectRange *RtDmeCreateObjectRange(unsigned int nConnection, unsigned int nClient) {
    RtDmeObjectRange *pRange;

    if (RtDmeIsClientIndex(nClient) != 1 || nConnection >= RT_DME_CONNECTION_COUNT ||
        nClient >= RT_DME_CLIENT_COUNT) {
        return NULL;
    }
    pRange = RtDmeAlloc(sizeof(RtDmeObjectRange));
    // Yes, the binary fills the range without checking the allocation.
    pRange->uJoinTime = 0;
    pRange->uObjectLimit = 0;
    g_ppDmeObjectRanges[nClient] = pRange;
    pRange->uFirstObject = 0;
    pRange->uObjectCount = 0;
    pRange->uFirstStream = 0;
    pRange->uStreamCount = 0;
    pRange->szName[0] = '\0';
    return pRange;
}

// NTSC-U/C: 0x002afd88, PAL: 0x002b9cf8
static void RtDmeFreeObjectRange(unsigned int nConnection, unsigned int nClient) {
    if (RtDmeIsClientIndex(nClient) == 1 && nConnection < RT_DME_CONNECTION_COUNT &&
        nClient < RT_DME_CLIENT_COUNT) {
        RtDmeFree((void **)&g_ppDmeObjectRanges[nClient]);
    }
}

void RtDmeFreeConnectionObjectRanges(unsigned int nConnection) {
    int i;

    if (nConnection >= RT_DME_CONNECTION_COUNT) {
        return;
    }
    if (g_ppDmeObjectRanges != NULL) {
        for (i = 0; i < RT_DME_CLIENT_COUNT; ++i) {
            RtDmeFreeObjectRange(nConnection, i);
        }
    }
    g_bDmeObjectRangesSent = 0;
}

void RtDmeFreeAllObjectRanges(void) {
    int i;

    if (g_ppDmeObjectRanges != NULL) {
        for (i = 0; i < RT_DME_CONNECTION_COUNT; ++i) {
            RtDmeFreeConnectionObjectRanges(i);
        }
    }
    g_bDmeObjectRangesSent = 0;
}

void RtDmeFreeObjectRangeTable(void) {
    if (g_ppDmeObjectRanges != NULL) {
        RtDmeFree((void **)&g_ppDmeObjectRanges);
    }
}

// NTSC-U/C: 0x002afed8, PAL: 0x002b9e48
static void RtDmeClaimObjectRange(const RtDmeObjectRange *pRange) {
    RtDmeStream *pStream;
    int i;

    if (pRange == NULL) {
        return;
    }
    if (pRange->uFirstObject < RT_DME_OBJECT_COUNT) {
        for (i = 0; i < pRange->uObjectCount; ++i) {
            if (g_anDmeObjectState[i + pRange->uFirstObject] == RT_DME_OBJECT_FREE) {
                g_anDmeObjectState[i + pRange->uFirstObject] = RT_DME_OBJECT_CLOSING;
            }
        }
    }
    if (pRange->uFirstStream < RT_DME_STREAM_COUNT) {
        for (i = 0; i < pRange->uStreamCount; ++i) {
            // Yes, the binary leaves pStream as it was when the lookup fails.
            RtDmeFindStream(&pStream, i + pRange->uFirstStream);
            if (pStream != NULL) {
                pStream->uState = RT_DME_STREAM_RESERVED;
            }
        }
    }
}

int RtDmeSendObjectRange(unsigned int nConnection, int nClient) {
    RtDmeConnection *pConnection;
    RtDmeClient *pClient;
    RtDmeObjectRange *pRange;
    RtDmeObjectRangeMessage message;
    int nResult;

    pConnection = NULL;
    pClient = NULL;
    if ((unsigned int)(nClient + 1) > RT_DME_CLIENT_COUNT ||
        nConnection >= RT_DME_CONNECTION_COUNT) {
        return RT_DME_RESULT_INVALID;
    }
    nResult = RtDmeGetConnection(nConnection, &pConnection);
    if (nResult != 0) {
        return nResult;
    }
    if (pConnection == NULL) {
        return RT_DME_RESULT_OUT_OF_RANGE;
    }
    nResult = RtDmeGetClient(nConnection, pConnection->nLocalClient, &pClient);
    if (nResult != 0) {
        return nResult;
    }
    if (pClient == NULL) {
        return RT_DME_RESULT_OUT_OF_RANGE;
    }
    RtDmeGetObjectRange(&pRange, nConnection, pConnection->nLocalClient);
    if (pRange == NULL) {
        return kRangeMissing;
    }
    if ((unsigned int)(pClient->nRole - kObjectRoleHost) >= 2) {
        return kRangeMissing;
    }
    message.uClient = RtDmeClientToByte(pConnection->nLocalClient);
    message.uObjectLimit = pRange->uObjectLimit;
    message.uFirstObject = pRange->uFirstObject;
    message.uObjectCount = pRange->uObjectCount;
    message.uFirstStream = pRange->uFirstStream;
    message.uStreamCount = pRange->uStreamCount;
    strcpy(message.szName, pRange->szName);
    RtDmeSendSystemMessage(kRangeSendFlags,
                           g_nDmeActiveConnection,
                           nClient,
                           g_nDmeObjectRangeMessage,
                           sizeof(message),
                           &message);
    return 0;
}

void RtDmeOpenObjectRange(unsigned int nConnection, int nClient) {
    RtDmeObjectRange *pRange;

    if (RtDmeIsClientIndex(nClient) != 1 || nConnection >= RT_DME_CONNECTION_COUNT) {
        return;
    }
    RtDmeGetObjectRange(&pRange, nConnection, nClient);
    if (pRange == NULL) {
        pRange = RtDmeCreateObjectRange(nConnection, nClient);
        if (pRange == NULL) {
            return;
        }
    }
    pRange->uObjectLimit = RT_DME_OBJECT_COUNT;
    RtDmeSendObjectRange(nConnection, RT_DME_ALL_CLIENTS);
    RtDmeClaimObjectRange(pRange);
}

void RtDmeJoinObjects(unsigned int nConnection, int nClient) {
    RtDmeObjectRange *pRange;
    RtDmeClient *pClient;
    RtDmeObjectRangeEvent event;
    int bIsClient;

    pClient = NULL;
    pRange = NULL;
    bIsClient = RtDmeIsClientIndex(nClient);
    if (bIsClient != 1 || nConnection >= RT_DME_CONNECTION_COUNT) {
        return;
    }
    RtDmeGetObjectRange(&pRange, nConnection, nClient);
    if (pRange == NULL) {
        // Yes, the binary does not check the new range.
        pRange = RtDmeCreateObjectRange(nConnection, nClient);
    }
    if (RtDmeGetClient(nConnection, nClient, &pClient) != 0 || pClient == NULL) {
        return;
    }
    pRange->uJoinTime = g_uDmeSessionTime;
    pClient->bObjectsJoined = bIsClient;
    RtDmeClaimObjectRange(pRange);
    if (g_pfnDmeObjectRange != NULL) {
        event.nEvent = kRangeEventClaimed;
        event.nConnection = nConnection;
        event.nClient = nClient;
        event.nArgument = g_nDmeObjectRangeArgument;
        g_pfnDmeObjectRange(&event);
    }
    RtDmeSendObjectRange(nConnection, nClient);
}

// NTSC-U/C: 0x002b05a0, PAL: 0x002ba510
static int RtDmeElectObjectHost(unsigned int nConnection, int nDeparted, int *pnHost) {
    RtDmeClient *pClient;
    rtDmeHostChange change;
    int nBestRank;
    int nResult;
    int i;

    nBestRank = kNoRank;
    pClient = NULL;
    if (nConnection >= RT_DME_CONNECTION_COUNT || RtDmeIsClientIndex(nDeparted) != 1 ||
        pnHost == NULL) {
        return RT_DME_RESULT_INVALID;
    }
    *pnHost = RT_DME_NO_CLIENT;
    for (i = 0; i < RT_DME_CLIENT_COUNT; ++i) {
        if (nDeparted == i) {
            continue;
        }
        nResult = RtDmeGetClient(nConnection, i, &pClient);
        if (nResult != 0) {
            return nResult;
        }
        if (pClient != NULL && pClient->nRole == kObjectRoleCandidate && pClient->nHostRank > 0 &&
            pClient->nHostRank < nBestRank) {
            *pnHost = i;
            nBestRank = pClient->nHostRank;
        }
    }
    if (*pnHost == RT_DME_NO_CLIENT) {
        return 0;
    }
    nResult = RtDmeGetClient(nConnection, *pnHost, &pClient);
    if (nResult != 0 || pClient == NULL) {
        return nResult;
    }
    pClient->nRole = kObjectRoleHost;
    RtDmeSetHostClient(nConnection, *pnHost);
    if (g_pfnDmeObjectHost != NULL) {
        change.connection = nConnection;
        change.host = *pnHost;
        change.argument = g_nDmeObjectHostArgument;
        g_pfnDmeObjectHost(&change);
    }
    return 0;
}

void RtDmeReleaseClientObjects(unsigned int nConnection, int nClient) {
    RtDmeClient *pClient;
    RtDmeObjectRange *pRange;
    RtDmeObject *pObject;
    RtDmeStream *pStream;
    RtDmeObjectRangeEvent event;
    int nHost;
    int nNewHost;
    int i;

    nNewHost = RT_DME_NO_CLIENT;
    pClient = NULL;
    if (nConnection >= RT_DME_CONNECTION_COUNT || (unsigned int)nClient >= RT_DME_CLIENT_COUNT ||
        (int)nConnection != g_nDmeActiveConnection) {
        return;
    }
    if (RtDmeGetClient(nConnection, nClient, &pClient) != 0 || pClient == NULL) {
        return;
    }
    pClient->nRole = kObjectRoleReleased;
    pClient->anState[0] = 0;
    pClient->anState[1] = 0;
    RtDmeGetObjectRange(&pRange, nConnection, nClient);
    if (pRange == NULL) {
        return;
    }
    RtDmeGetHostClient(&nHost, nConnection);
    nNewHost = nHost;
    if (nClient == nHost) {
        if (RtDmeElectObjectHost(nConnection, nClient, &nNewHost) != 0 ||
            nNewHost == RT_DME_NO_CLIENT) {
            return;
        }
    }
    if (g_pfnDmeObjectRange != NULL) {
        event.nEvent = kRangeEventReleased;
        event.nConnection = nConnection;
        event.nClient = nClient;
        event.nArgument = g_nDmeObjectRangeArgument;
        g_pfnDmeObjectRange(&event);
    }
    for (i = 0; i < g_nDmeObjectCount; ++i) {
        if (g_anDmeObjectState[i] != RT_DME_OBJECT_USED) {
            continue;
        }
        RtDmeGetObject(&pObject, i);
        if (pObject == NULL) {
            continue;
        }
        if (pObject->nOwner == nClient) {
            if ((unsigned char)(pObject->uLifetime - RT_DME_OBJECT_LIFETIME_SESSION) < 2) {
                RtDmeGetHostClient(&pObject->nOwner, nConnection);
                if (pObject->nLockClient == nClient) {
                    pObject->nLockClient = kNoLockClient;
                    RtDmeNotifyObjectChanged(pObject->nLockClient, i, kObjectChangedOwner);
                }
            } else {
                RtDmeDestroyObject(i);
                g_anDmeObjectState[i] = RT_DME_OBJECT_FREE;
            }
        } else if (pObject->nLockClient == nClient) {
            pObject->nLockClient = kNoLockClient;
            RtDmeNotifyObjectChanged(pObject->nLockClient, i, kObjectChangedOwner);
        }
    }
    for (i = pRange->uFirstObject; i < pRange->uFirstObject + pRange->uObjectCount; ++i) {
        if (g_anDmeObjectState[i] == RT_DME_OBJECT_CLOSING) {
            g_anDmeObjectState[i] = RT_DME_OBJECT_FREE;
        }
    }
    if (pRange->uFirstStream < RT_DME_STREAM_COUNT) {
        for (i = 0; i < pRange->uStreamCount; ++i) {
            RtDmeFindStream(&pStream, i + pRange->uFirstStream);
            if (pStream == NULL) {
                continue;
            }
            if (pStream->uState == RT_DME_STREAM_OPEN && pStream->bOwnsData) {
                RtDmeFree((void **)&pStream->pbData);
            }
            pStream->uState = RT_DME_STREAM_FREE;
        }
    }
    RtDmeFreeObjectRange(nConnection, nClient);
}

// NTSC-U/C: 0x002b0888, PAL: 0x002ba7f8
static int RtDmeCountPersistentObjects(unsigned short *puCount) {
    RtDmeConnection *pConnection;
    RtDmeObject *pObject;
    int nResult;
    int i;

    pConnection = NULL;
    pObject = NULL;
    if (puCount == NULL) {
        return RT_DME_RESULT_INVALID;
    }
    *puCount = 0;
    if (g_nDmeActiveConnection == kNoConnection) {
        return 0;
    }
    if ((unsigned int)g_nDmeActiveConnection >= RT_DME_CONNECTION_COUNT) {
        return RT_DME_RESULT_OUT_OF_RANGE;
    }
    nResult = RtDmeGetConnection(g_nDmeActiveConnection, &pConnection);
    if (nResult != 0 || pConnection == NULL) {
        return nResult;
    }
    if ((unsigned int)pConnection->nLocalClient >= RT_DME_CLIENT_COUNT) {
        return RT_DME_RESULT_OUT_OF_RANGE;
    }
    for (i = 0; i < RT_DME_OBJECT_COUNT; ++i) {
        nResult = RtDmeGetObject(&pObject, i);
        if (nResult != 0) {
            return nResult;
        }
        if (pObject != NULL &&
            (unsigned char)(pObject->uLifetime - RT_DME_OBJECT_LIFETIME_SESSION) < 2 &&
            pConnection->nLocalClient == pObject->nOwner) {
            ++*puCount;
        }
    }
    return 0;
}

int RtDmeSendClientState(int nConnection, int nClient) {
    RtDmeConnection *pConnection;
    RtDmeClient *pClient;
    RtDmeClientStateMessage message;
    int nLocalClient;
    int nResult;
    int i;

    pConnection = NULL;
    pClient = NULL;
    if ((unsigned int)nConnection >= RT_DME_CONNECTION_COUNT) {
        return 0;
    }
    nResult = RtDmeGetConnection(nConnection, &pConnection);
    if (nResult != 0 || pConnection == NULL) {
        return nResult;
    }
    nLocalClient = pConnection->nLocalClient;
    if ((unsigned int)nLocalClient >= RT_DME_CLIENT_COUNT) {
        return 0;
    }
    nResult = RtDmeGetClient(nConnection, nLocalClient, &pClient);
    if (nResult != 0 || pClient == NULL) {
        return nResult;
    }
    message.uClient = RtDmeClientToByte(nLocalClient);
    message.uRole = pClient->nRole;
    message.auState[0] = pClient->anState[0];
    message.auState[1] = pClient->anState[1];
    message.nHostRank = pClient->nHostRank;
    nResult = RtDmeCountPersistentObjects(&message.uObjectCount);
    if (nResult != 0) {
        return nResult;
    }
    if (nClient != RT_DME_ALL_CLIENTS) {
        return RtDmeSendSystemMessage(kRangeSendFlags,
                                      nConnection,
                                      nClient,
                                      g_nDmeClientStateMessage,
                                      sizeof(message),
                                      &message);
    }
    for (i = 0; i < RT_DME_CLIENT_COUNT; ++i) {
        nResult = RtDmeGetClient(nConnection, i, &pClient);
        if (nResult != 0) {
            return nResult;
        }
        if (pClient != NULL && pClient->bJoined == 1 && i != nLocalClient) {
            RtDmeSendSystemMessage(kRangeSendFlags,
                                   nConnection,
                                   i,
                                   g_nDmeClientStateMessage,
                                   sizeof(message),
                                   &message);
        }
    }
    return 0;
}

void RtDmeOnPortMapped(const char *pszAddress, unsigned short uPort, int nResult) {
    RtDmeConnection *pConnection;

    pConnection = NULL;
    if (nResult == 0) {
        g_dmeLocalAddresses[0].port = uPort;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstringop-truncation"
        strncpy(g_dmeLocalAddresses[0].address, pszAddress, RT_NET_ADDRESS_SIZE);
#pragma GCC diagnostic pop
        if (RtDmeGetConnection(0, &pConnection) != 0) {
            return;
        }
        if (pConnection != NULL && pConnection->nLocalClient < RT_DME_CLIENT_COUNT) {
            RtDmeSetClientAddress(0, pConnection->nLocalClient, 0, pszAddress);
            RtDmeSetClientPort(0, pConnection->nLocalClient, 0, uPort);
        }
    }
    g_dmeMapping.complete = 1;
    g_dmeMapping.error = RtDmeConvertResult(nResult, RT_DME_RESULT_MAPPING);
    memcpy(g_dmeMapping.addresses, g_dmeLocalAddresses, sizeof(g_dmeMapping.addresses));
    g_pfnDmeMapping(&g_dmeMapping);
}
