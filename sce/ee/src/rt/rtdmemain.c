#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "rt/rtchannelapi.h"
#include "rt/rtdme.h"
#include "rt/rtmemory.h"
#include "rt/rtrandom.h"
#include "rt/rtsecurity.h"
#include "rt/rttime.h"

enum {
    // The flags the engine sends its own messages with.
    kSystemSendFlags = RT_DME_SEND_ORDERED | RT_DME_SEND_GUARANTEED,
    // The peer index the channels take for every peer.
    kChannelAllPeers = 0xfffe,
    // The size of a client record.
    kClientRecordSize = 104,
    // The latency samples per percentile step.
    kLatencySamplesPerStep = 20,
    // The milliseconds of silence that drop a client of the datagram service.
    kPeerTimeout = 45000,
    // The milliseconds of silence after which a timed-out client is dropped.
    kClientExpiry = 30000,
    // The bytes of the address of a client in the messages.
    kAddressBytes = 4,
    // The address separator and the parts of an address.
    kAddressParts = 4,
    // The bytes of a message header.
    kMessageHeaderSize = 2,
    // The statuses of the channels.
    kChannelStatusCount = 10,
    // The fields of an object, and the field values that stand for every field and the object.
    kObjectFieldCount = 64,
    kObjectChangedAll = -1,
    kObjectChangedWhole = -2,
};

// The strncpy() bounds are the sizes of their destinations, as in the binary.
#pragma GCC diagnostic ignored "-Wstringop-truncation"

// A message as it travels, its class and type before its fields.
typedef struct {
    unsigned char uClass;
    unsigned char uType;
    unsigned char abData[RT_DME_MAX_MESSAGE_SIZE];
} RtDmeMessage;

// The header of a fragment of a long message.
typedef struct {
    unsigned char uClass;
    unsigned char uType;
    unsigned short uSize;
    unsigned short uCount;
    unsigned short uIndex;
    unsigned char uMessageId;
    unsigned char abReserved09[3]; // Yes, the binary sends these bytes uninitialised.
    int nTotalSize;
    int nOffset;
} RtDmeFragmentHeader;

// A message to send, whole or as a fragment.
typedef union {
    unsigned char abData[RT_DME_MAX_MESSAGE_SIZE + kMessageHeaderSize];
    struct {
        RtDmeFragmentHeader header;
        unsigned char abData[RT_DME_FRAGMENT_DATA_SIZE];
    } fragment;
} RtDmeOutgoing;

// The announcement of this console to a client that joined.
typedef struct {
    unsigned char uConnection;
    unsigned char uClient;
    unsigned char abAddress[kAddressBytes];
    unsigned char abReserved06[2]; // Yes, the binary sends these bytes uninitialised.
    RtKey modulus;
} RtDmeJoinAnnouncement;

// The addresses of a client that left, sent to the others.
typedef struct {
    unsigned char uConnection;
    unsigned char uClient;
    unsigned char abReserved02[2]; // Yes, the binary sends these bytes uninitialised.
    unsigned char abPrivateAddress[kAddressBytes];
    unsigned char abPublicAddress[kAddressBytes];
    unsigned short uPrivatePort;
    unsigned short uPublicPort;
} RtDmeClientAddresses;

// What a client matched in RtDmeFindClientByAddress().
enum {
    kMatchNone = 0,
    kMatchWorking = 1, // The address the client works at, which ends the search.
    kMatchOther = 2,   // Another address, which the search records and goes on past.
};

// NTSC-U/C: 0x003b6bec
int g_nDmeActiveConnection;
// NTSC-U/C: 0x003b6bf4
int g_nDmeRunning;
// NTSC-U/C: 0x003b6bfc
int g_nDmeObjectCount;
// NTSC-U/C: 0x003b6c00
RtDmeConnectFunc g_pfnDmeConnect;
// NTSC-U/C: 0x003b6c10
RtDmeHostChangeFunc g_pfnDmeHostChange;
// NTSC-U/C: 0x003b6c14
int g_nDmeHostChangeArgument;
// NTSC-U/C: 0x003b6c20
unsigned char g_bDmeServiceStarted;
// NTSC-U/C: 0x003b6c28
unsigned char g_bDmeReceiveError;
// NTSC-U/C: 0x003b6c2a
unsigned char g_bDmeConnectReported;
// NTSC-U/C: 0x003b6c34
unsigned char g_bDmeListening;
// NTSC-U/C: 0x003b6c38
RtDmeStream *g_pDmeStreams;
// Written at start-up and never read.
// NTSC-U/C: 0x00435720
static int g_anDmeUnreadWords[4];
// NTSC-U/C: 0x00435730
unsigned char g_auDmeMessageTypeCount[RT_DME_MESSAGE_CLASS_COUNT];
// NTSC-U/C: 0x00435740
unsigned int g_uDmeSessionTime;
// NTSC-U/C: 0x00435748
int g_nDmeHostClient;
// NTSC-U/C: 0x0043574c
unsigned short g_uDmePort;
// NTSC-U/C: 0x0043574e
unsigned short g_uDmeSenderPort;
// NTSC-U/C: 0x00435750
int g_bDmeUseFixedAddress;
// NTSC-U/C: 0x00435754
int g_nDmeApplicationId;
// NTSC-U/C: 0x00519f28
static RtDmeMessage g_dmeDatagram;
// NTSC-U/C: 0x0051a128
static RtDmeMessage g_dmeChannelMessage;
// NTSC-U/C: 0x0051a328
static rtDmeMessage g_dmeReceived;
// NTSC-U/C: 0x0051b128
RtDmeConnection *g_apDmeConnections[RT_DME_CONNECTION_COUNT];
// NTSC-U/C: 0x0051b138
RtDmeClientStats *g_apDmeClientStats[RT_DME_CLIENT_COUNT];
// NTSC-U/C: 0x0051b550
RtDmeMapping g_dmeMapping;
// NTSC-U/C: 0x0051b5b0
RtDmePoll g_aDmePolls[RT_DME_POLL_COUNT];
// NTSC-U/C: 0x0053f1e0
RtDmeMessageType *g_apDmeMessageTypes[RT_DME_MESSAGE_CLASS_COUNT][RT_DME_MESSAGE_TYPE_COUNT];
// NTSC-U/C: 0x005409e0
RtDmeObjectRemovedFunc g_apfnDmeObjectRemoved[RT_DME_OBJECT_TYPE_COUNT];
// NTSC-U/C: 0x00540a20
RtDmeObjectAddedFunc g_apfnDmeObjectAdded[RT_DME_OBJECT_TYPE_COUNT];
// NTSC-U/C: 0x00540a60
RtDmeObjectChangedFunc g_apfnDmeObjectChanged[RT_DME_OBJECT_TYPE_COUNT];
// NTSC-U/C: 0x00540aa0
signed char g_anDmeObjectState[RT_DME_OBJECT_COUNT];
// NTSC-U/C: 0x00541aa0
rtNetAddress g_dmeLocalAddresses[RT_DME_ADDRESS_COUNT];
// NTSC-U/C: 0x00541ad0
char g_szDmeSenderAddress[RT_DME_ADDRESS_SIZE];
// NTSC-U/C: 0x00541ae0
RtDmeObject *g_apDmeObjects[RT_DME_OBJECT_COUNT];
// NTSC-U/C: 0x00545ae0
RtDmePing g_aDmePings[RT_DME_PING_COUNT];

// Test one client of a connection against an address and a port.
static inline int
RtDmeMatchClient(int nConnection, int nClient, const char *pszAddress, unsigned short uPort) {
    int nIndex;
    int nOther;

    nIndex = RtDmeGetAddressIndex(nConnection, nClient);
    if (nIndex == RT_DME_ADDRESS_INDEX_NONE) {
        return kMatchNone;
    }
    if (nIndex == RT_DME_ADDRESS_INDEX_UNRESOLVED) {
        if (RtDmeGetClientAddressKind(nConnection, nClient, RT_DME_ADDRESS_INDEX_PRIVATE) != 0 &&
            strcmp(pszAddress,
                   RtDmeGetClientAddress(nConnection, nClient, RT_DME_ADDRESS_INDEX_PRIVATE)) ==
                0 &&
            uPort == RtDmeGetClientPort(nConnection, nClient, RT_DME_ADDRESS_INDEX_PRIVATE)) {
            return kMatchWorking;
        }
        if (RtDmeGetClientAddressKind(nConnection, nClient, RT_DME_ADDRESS_INDEX_PUBLIC) != 0 &&
            strcmp(pszAddress,
                   RtDmeGetClientAddress(nConnection, nClient, RT_DME_ADDRESS_INDEX_PUBLIC)) == 0 &&
            uPort == RtDmeGetClientPort(nConnection, nClient, RT_DME_ADDRESS_INDEX_PUBLIC)) {
            return kMatchWorking;
        }
        return kMatchNone;
    }
    if (RtDmeGetClientAddressKind(nConnection, nClient, nIndex) != 0 &&
        strcmp(pszAddress, RtDmeGetClientAddress(nConnection, nClient, nIndex)) == 0 &&
        uPort == RtDmeGetClientPort(nConnection, nClient, nIndex)) {
        return kMatchWorking;
    }
    nOther = (nIndex == RT_DME_ADDRESS_INDEX_PRIVATE) ? RT_DME_ADDRESS_INDEX_PUBLIC :
                                                        RT_DME_ADDRESS_INDEX_PRIVATE;
    if (RtDmeGetClientAddressKind(nConnection, nClient, nOther) != 0 &&
        strcmp(pszAddress, RtDmeGetClientAddress(nConnection, nClient, nOther)) == 0 &&
        uPort == RtDmeGetClientPort(nConnection, nClient, nOther)) {
        return kMatchOther;
    }
    return kMatchNone;
}

// Search the clients of one connection. Report non-zero when the search is over.
static inline int RtDmeSearchConnection(int nConnection,
                                        int *pnConnection,
                                        int *pnClient,
                                        const char *pszAddress,
                                        unsigned short uPort) {
    int nClient;

    for (nClient = 0; nClient < RT_DME_CLIENT_COUNT; ++nClient) {
        switch (RtDmeMatchClient(nConnection, nClient, pszAddress, uPort)) {
        case kMatchWorking:
            *pnConnection = nConnection;
            *pnClient = nClient;
            return 1;
        case kMatchOther:
            *pnConnection = nConnection;
            *pnClient = nClient;
            break;
        default:
            break;
        }
    }
    return 0;
}

// The session of the active connection, or -1.
// NTSC-U/C: 0x002b11d0, PAL: 0x002bb140
static int RtDmeGetActiveWorldId(void) {
    RtDmeConnection *pConnection;

    if ((unsigned int)g_nDmeActiveConnection >= RT_DME_CONNECTION_COUNT) {
        return -1;
    }
    pConnection = NULL;
    if (RtDmeGetConnection(g_nDmeActiveConnection, &pConnection) != 0) {
        return -1;
    }
    if (pConnection == NULL) {
        return -1;
    }
    return pConnection->nWorldId;
}

// NTSC-U/C: 0x002b17b0, PAL: 0x002bb720
static void RtDmeClearClient(int nConnection, int nClient) {
    RtDmeConnection *pConnection;
    RtDmeClient *pClient;

    pConnection = NULL;
    pClient = NULL;
    if ((unsigned int)nConnection >= RT_DME_CONNECTION_COUNT ||
        (unsigned int)nClient >= RT_DME_CLIENT_COUNT) {
        return;
    }
    if (RtDmeGetConnection(nConnection, &pConnection) != 0 || pConnection == NULL) {
        return;
    }
    if (RtDmeGetClient(nConnection, nClient, &pClient) != 0 || pClient == NULL) {
        return;
    }
    memset(pClient, 0, kClientRecordSize);
    pClient->bCleared = 1;
}

// NTSC-U/C: 0x002b1bf0, PAL: 0x002bbb60
static int RtDmeSendOnChannel(
    signed char nFlags, int nConnection, int nClient, unsigned short uSize, const void *pData) {
    unsigned short uPeer;
    int nResult;

    uPeer = (unsigned short)nClient;
    if (nClient == RT_DME_SERVER_CLIENT) {
        uPeer = RT_DME_SERVER_CLIENT;
    }
    if (nClient == RT_DME_ALL_CLIENTS) {
        uPeer = kChannelAllPeers;
    }
    nResult = RtChannelApiSend(RtDmeConnectionToChannel(nConnection), uPeer, nFlags, pData, uSize);
    if (nResult != 0) {
        printf("SendPacket error sending message, msg error code = %d\n", nResult);
        return RT_DME_RESULT_SEND_FAILED;
    }
    return 0;
}

// Send a message as a datagram to a client, the server, or every client.
// NTSC-U/C: 0x002b2010, PAL: 0x002bbf80
static int RtDmeSendDatagramToClient(signed char nFlags,
                                     int nConnection,
                                     int nClient,
                                     unsigned int nClass,
                                     unsigned int nType,
                                     unsigned int nSize,
                                     const void *pData) {
    RtDmeConnection *pConnection;
    const char *pszAddress;
    unsigned short uPort;
    int nIndex;
    int nResult;

    nResult = 0;
    pConnection = NULL;
    if (nClient == RT_DME_ALL_CLIENTS) {
        return RtDmeBroadcastDatagram(0, 0, nFlags, nConnection, nClass, nType, nSize, pData);
    }
    if (nClient == RT_DME_SERVER_CLIENT) {
        if (RtDmeSendDatagram(
                nFlags, RtDmeGetServerName(nConnection), g_uDmePort, nClass, nType, nSize, pData) !=
            0) {
            nResult = RT_DME_RESULT_SEND_FAILED;
        }
        return nResult;
    }
    nIndex = RtDmeGetAddressIndex(nConnection, nClient);
    nResult = RtDmeGetConnection(nConnection, &pConnection);
    if (nResult != 0 || pConnection == NULL) {
        return nResult;
    }
    if ((unsigned int)nIndex >= RT_DME_ADDRESS_COUNT) {
        return nResult;
    }
    if (RtDmeGetClientAddressKind(nConnection, nClient, nIndex) == 0) {
        return nResult;
    }
    if (nClient == pConnection->nLocalClient) {
        return nResult;
    }
    pszAddress = RtDmeGetClientAddress(nConnection, nClient, nIndex);
    uPort = RtDmeGetClientPort(nConnection, nClient, nIndex);
    if (RtDmeSendDatagram(nFlags, pszAddress, uPort, nClass, nType, nSize, pData) != 0) {
        nResult = RT_DME_RESULT_SEND_FAILED;
    }
    return nResult;
}

// Send a message over the channel of a connection.
// NTSC-U/C: 0x002b21c0, PAL: 0x002bc130
static int RtDmeSendChannelMessage(signed char nFlags,
                                   unsigned int nConnection,
                                   int nClient,
                                   unsigned int nClass,
                                   unsigned int nType,
                                   int nSize,
                                   const void *pData) {
    RtDmeConnection *pConnection;
    int nResult;

    pConnection = NULL;
    if (nConnection >= RT_DME_CONNECTION_COUNT || nClass >= RT_DME_MESSAGE_CLASS_COUNT ||
        nType >= RT_DME_MESSAGE_TYPE_COUNT || nSize <= 0) {
        return RT_DME_RESULT_INVALID;
    }
    if (nSize > RT_DME_MAX_MESSAGE_SIZE) {
        return RT_DME_RESULT_TOO_LARGE;
    }
    if (pData == NULL) {
        return RT_DME_RESULT_INVALID;
    }
    nResult = RtDmeGetConnection(nConnection, &pConnection);
    if (nResult != 0 || pConnection == NULL) {
        return nResult;
    }
    if (nClient == pConnection->nLocalClient) {
        return nResult;
    }
    g_dmeChannelMessage.uClass = nClass;
    g_dmeChannelMessage.uType = nType;
    memcpy(g_dmeChannelMessage.abData, pData, nSize);
    return RtDmeSendOnChannel(
        nFlags, nConnection, nClient, nSize + kMessageHeaderSize, &g_dmeChannelMessage);
}

// NTSC-U/C: 0x002b2850, PAL: 0x002bc7c0
static void RtDmeNotifyObjectRemoved(unsigned int nOwner, unsigned int nObject) {
    RtDmeObject *pObject;
    RtDmeObjectRemovedFunc pfnRemoved;

    if (nOwner >= RT_DME_CLIENT_COUNT || nObject >= RT_DME_OBJECT_COUNT) {
        return;
    }
    (void)RtDmeGetObject(&pObject, nObject); // Yes, the binary discards the result.
    if (pObject == NULL) {
        return;
    }
    pfnRemoved = g_apfnDmeObjectRemoved[pObject->uType];
    if (pfnRemoved != NULL) {
        pfnRemoved(nOwner, nObject);
    }
}

// Hand each message of a packet to the handler of its type.
// NTSC-U/C: 0x002b2c00, PAL: 0x002bcb70
static void RtDmeDispatchMessages(int nWorldId,
                                  int nConnection,
                                  unsigned short uClient,
                                  const unsigned char *pbData,
                                  unsigned short uSize) {
    const RtDmeMessage *pMessage;
    RtDmeMessageType *pType;
    rtDmeMessageHandler pfnHandler;
    int nClient;
    int nHost;
    int nConsumed;
    int nPosition;

    if (pbData == NULL || uSize == 0) {
        return;
    }
    nPosition = 0;
    do {
        pMessage = (const RtDmeMessage *)&pbData[nPosition];
        nPosition += kMessageHeaderSize;
        nHost = g_nDmeHostClient;
        if (pMessage->uClass >= RT_DME_MESSAGE_CLASS_COUNT ||
            pMessage->uType >= g_auDmeMessageTypeCount[pMessage->uClass]) {
            g_bDmeReceiveError = 1;
            return;
        }
        pType = g_apDmeMessageTypes[pMessage->uClass][pMessage->uType];
        pfnHandler = pType->pfnHandler;
        if (pfnHandler == NULL) {
            return;
        }
        memcpy(&g_dmeReceived, &pbData[nPosition], uSize - nPosition);
        nConsumed = pfnHandler(nConnection, nWorldId, uClient, &g_dmeReceived);
        nPosition += nConsumed;
        if (nConsumed == -1) {
            return;
        }
        if (uSize < nPosition) {
            g_bDmeReceiveError = 1;
            return;
        }
        // The handler may have moved the host, so the sender is looked up again.
        if ((nConnection == RT_DME_SERVER_CLIENT && uClient == RT_DME_NO_CLIENT) ||
            nHost != g_nDmeHostClient) {
            RtDmeFindClientByAddress(
                &nConnection, &nClient, g_szDmeSenderAddress, g_uDmeSenderPort);
            uClient = nClient;
        }
    } while (nPosition < uSize);
}

// The receiver of the datagrams of the peers.
// NTSC-U/C: 0x002b2da0, PAL: 0x002bcd10
static void RtDmeOnDatagram(const char *pszAddress,
                            unsigned short uPort,
                            const void *pData,
                            unsigned short uSize) {
    RtDmeClient *pClient;
    int nConnection;
    int nClient;

    nConnection = RT_DME_SERVER_CLIENT;
    nClient = RT_DME_NO_CLIENT;
    pClient = NULL;
    if (pszAddress == NULL || strlen(pszAddress) == 0 || uPort == 0 || pData == NULL ||
        uSize == 0) {
        return;
    }
    if (strcmp(pszAddress, g_dmeLocalAddresses[RT_DME_ADDRESS_INDEX_PUBLIC].address) == 0 &&
        uPort == g_dmeLocalAddresses[RT_DME_ADDRESS_INDEX_PUBLIC].port) {
        return;
    }
    if (strcmp(pszAddress, g_dmeLocalAddresses[RT_DME_ADDRESS_INDEX_PRIVATE].address) == 0 &&
        uPort == g_dmeLocalAddresses[RT_DME_ADDRESS_INDEX_PRIVATE].port) {
        return;
    }
    strcpy(g_szDmeSenderAddress, pszAddress);
    g_uDmeSenderPort = uPort;
    RtDmeFindClientByAddress(&nConnection, &nClient, g_szDmeSenderAddress, uPort);
    if (nConnection == 0 && nClient != RT_DME_NO_CLIENT) {
        if (RtDmeGetClient(0, nClient, &pClient) != 0 || pClient == NULL) {
            return;
        }
        RtReadTime(&pClient->lastHeard);
    }
    RtDmeDispatchMessages(RtDmeGetActiveWorldId(), nConnection, nClient, pData, uSize);
}

// Announce the addresses of a client that left to the other clients.
// NTSC-U/C: 0x002b3258, PAL: 0x002bd1c8
static int RtDmeAnnounceDeparture(unsigned int nConnection, int nDeparted) {
    RtDmeConnection *pConnection;
    RtDmeClient *pClient;
    RtDmeClientAddresses addresses;
    const char *pszAddress;
    unsigned short uPort;
    int nIndex;
    int nClient;
    int nResult;

    pConnection = NULL;
    pClient = NULL;
    if (nConnection >= RT_DME_CONNECTION_COUNT || nDeparted == RT_DME_NO_CLIENT) {
        return RT_DME_RESULT_INVALID;
    }
    addresses.uConnection = nConnection;
    addresses.uClient = RtDmeClientToByte(nDeparted);
    RtDmeParseAddress(addresses.abPrivateAddress,
                      RtDmeGetClientAddress(nConnection, nDeparted, RT_DME_ADDRESS_INDEX_PRIVATE));
    RtDmeParseAddress(addresses.abPublicAddress,
                      RtDmeGetClientAddress(nConnection, nDeparted, RT_DME_ADDRESS_INDEX_PUBLIC));
    addresses.uPrivatePort =
        RtDmeGetClientPort(nConnection, nDeparted, RT_DME_ADDRESS_INDEX_PRIVATE);
    addresses.uPublicPort = RtDmeGetClientPort(nConnection, nDeparted, RT_DME_ADDRESS_INDEX_PUBLIC);
    nResult = RtDmeGetConnection(nConnection, &pConnection);
    if (nResult != 0 || pConnection == NULL) {
        return nResult;
    }
    for (nClient = 0; nClient < RT_DME_CLIENT_COUNT; ++nClient) {
        nResult = RtDmeGetClient(nConnection, nClient, &pClient);
        if (nResult != 0) {
            return nResult;
        }
        if (pClient == NULL) {
            continue;
        }
        nIndex = RtDmeGetAddressIndex(nConnection, nClient);
        if ((unsigned int)nIndex >= RT_DME_ADDRESS_COUNT && pClient->bJoined != 1) {
            continue;
        }
        if (RtDmeGetClientAddressKind(nConnection, nClient, RT_DME_ADDRESS_INDEX_PRIVATE) == 0 ||
            RtDmeGetClientAddressKind(nConnection, nClient, RT_DME_ADDRESS_INDEX_PUBLIC) == 0) {
            continue;
        }
        if (nClient == pConnection->nLocalClient || nClient == nDeparted) {
            continue;
        }
        pszAddress = RtDmeGetClientAddress(nConnection, nClient, nIndex);
        uPort = RtDmeGetClientPort(nConnection, nClient, nIndex);
        // Yes, the binary discards the result of each send.
        (void)RtDmeSendDatagram(kSystemSendFlags,
                                pszAddress,
                                uPort,
                                RT_DME_CLASS_SYSTEM,
                                g_nDmeClientAddressesMessage,
                                sizeof(addresses),
                                &addresses);
    }
    return 0;
}

void RtDmeOnChannelPeer(unsigned short uChannel,
                        unsigned short uPeer,
                        const RtChannelPeerInfo *pInfo,
                        int bHasKey) {
    RtDmeConnection *pConnection;
    int nConnection;

    pConnection = NULL;
    nConnection = RtDmeChannelToConnection(uChannel);
    if ((unsigned int)nConnection >= RT_DME_CONNECTION_COUNT || pInfo == NULL) {
        return;
    }
    if (RtDmeGetConnection(nConnection, &pConnection) != 0 || pConnection == NULL) {
        return;
    }
    if ((unsigned char)bHasKey) {
        ++pConnection->nPendingJoins;
        RtDmeAddClient(nConnection, uPeer, pInfo->szName);
    } else {
        RtDmeRemoveClient(nConnection, uPeer, pInfo->szName, 0);
    }
}

// The receiver of the peers of the datagram service that went silent.
// NTSC-U/C: 0x002b3508, PAL: 0x002bd478
static void RtDmeOnPeerTimeout(const char *pszAddress, unsigned short uPort) {
    char szAddress[RT_DME_ADDRESS_SIZE];
    int nConnection;
    int nClient;

    nClient = RT_DME_NO_CLIENT;
    if (pszAddress == NULL || strlen(pszAddress) == 0 || uPort == 0) {
        return;
    }
    strcpy(szAddress, pszAddress);
    RtDmeFindClientByAddress(&nConnection, &nClient, szAddress, uPort);
    if (nClient != RT_DME_NO_CLIENT && nConnection == 0) {
        RtDmeExpireClient(nClient, kClientExpiry, 0);
        return;
    }
    RtChannelApiDropPeer(szAddress, uPort);
}

void RtDmeOnChannelData(unsigned short uChannel,
                        unsigned short uPeer,
                        const void *pData,
                        unsigned short uSize) {
    int nConnection;

    nConnection = RtDmeChannelToConnection(uChannel);
    if ((unsigned int)nConnection >= RT_DME_CONNECTION_COUNT || pData == NULL || uSize == 0) {
        return;
    }
    RtDmeDispatchMessages(RtDmeGetActiveWorldId(), nConnection, uPeer, pData, uSize);
}

unsigned short RtDmeConnectionToChannel(int nConnection) {
    if (nConnection < RT_DME_CONNECTION_COUNT + 1) {
        return nConnection - 1;
    }
    return nConnection;
}

int RtDmeChannelToConnection(unsigned short uChannel) {
    return uChannel + (uChannel < RT_DME_CONNECTION_COUNT + 1);
}

void RtDmeFindClientByAddress(int *pnConnection,
                              int *pnClient,
                              const char *pszAddress,
                              unsigned short uPort) {
    RtDmeConnection *pConnection;
    int nConnection;

    *pnClient = RT_DME_NO_CLIENT;
    pConnection = NULL;
    nConnection = g_nDmeActiveConnection;
    if (nConnection < RT_DME_CONNECTION_COUNT) {
        if (RtDmeGetConnection(nConnection, &pConnection) != 0 || pConnection == NULL) {
            return;
        }
        if (RtDmeSearchConnection(nConnection, pnConnection, pnClient, pszAddress, uPort)) {
            return;
        }
    }
    if (*pnClient != RT_DME_NO_CLIENT) {
        return;
    }
    for (nConnection = 0; nConnection < RT_DME_CONNECTION_COUNT; ++nConnection) {
        if (RtDmeGetConnection(nConnection, &pConnection) != 0) {
            return;
        }
        if (pConnection == NULL || nConnection == g_nDmeActiveConnection) {
            continue;
        }
        if (RtDmeSearchConnection(nConnection, pnConnection, pnClient, pszAddress, uPort)) {
            return;
        }
    }
}

int RtDmeGetClientLatency(unsigned int *puLatency, unsigned int uClient) {
    RtDmeClientStats *pStats;
    unsigned int auSamples[RT_DME_LATENCY_SAMPLES];
    unsigned int uSwap;
    unsigned int uIndex;
    int nPass;
    int i;
    int nResult;

    pStats = NULL;
    if (uClient >= RT_DME_CLIENT_COUNT || puLatency == NULL) {
        return RT_DME_RESULT_INVALID;
    }
    nResult = RtDmeGetClientStats(uClient, &pStats);
    if (nResult != 0) {
        return nResult;
    }
    if (pStats == NULL) {
        return RT_DME_RESULT_NO_MEMORY;
    }
    for (i = 0; i < RT_DME_LATENCY_SAMPLES; ++i) {
        auSamples[i] = pStats->auSamples[i];
    }
    for (nPass = pStats->nSampleCount - 1; nPass > 0; --nPass) {
        for (i = 0; i < nPass; ++i) {
            if (auSamples[i + 1] < auSamples[i]) {
                uSwap = auSamples[i];
                auSamples[i] = auSamples[i + 1];
                auSamples[i + 1] = uSwap;
            }
        }
    }
    uIndex = (unsigned int)pStats->nSampleCount / kLatencySamplesPerStep;
    if ((int)uIndex <= 0) {
        uIndex = 1;
    }
    *puLatency = auSamples[uIndex];
    return 0;
}

int RtDmeSendClientFlags(int nConnection, int nClient) {
    RtDmeConnection *pConnection;
    signed char nFlags;
    int nResult;

    pConnection = NULL;
    if ((unsigned int)nConnection >= RT_DME_CONNECTION_COUNT) {
        return 0;
    }
    nResult = RtDmeGetConnection(nConnection, &pConnection);
    if (nResult != 0 || pConnection == NULL) {
        return nResult;
    }
    nResult = RtDmeGetClientFlags(nConnection, pConnection->nLocalClient, &nFlags);
    if (nResult != 0) {
        return nResult;
    }
    if (nConnection == 0 && nClient == RT_DME_ALL_CLIENTS) {
        return RtDmeBroadcastDatagram(0,
                                      1,
                                      kSystemSendFlags,
                                      0,
                                      RT_DME_CLASS_SYSTEM,
                                      g_nDmeClientFlagsMessage,
                                      sizeof(nFlags),
                                      &nFlags);
    }
    return RtDmeSendSystemMessage(
        kSystemSendFlags, nConnection, nClient, g_nDmeClientFlagsMessage, sizeof(nFlags), &nFlags);
}

int RtDmeGetLocalClient(int *pnClient, int nConnection) {
    RtDmeConnection *pConnection;
    int nResult;

    pConnection = NULL;
    if (!g_nDmeRunning) {
        return RT_DME_RESULT_NOT_STARTED;
    }
    if ((unsigned int)nConnection >= RT_DME_CONNECTION_COUNT || pnClient == NULL) {
        return RT_DME_RESULT_INVALID;
    }
    *pnClient = RT_DME_NO_CLIENT;
    nResult = RtDmeGetConnection(nConnection, &pConnection);
    if (nResult != 0 || pConnection == NULL) {
        return nResult;
    }
    *pnClient = pConnection->nLocalClient;
    return nResult;
}

int RtDmeSetHostChangeHook(RtDmeHostChangeFunc pfnHook, int nArgument) {
    g_pfnDmeHostChange = pfnHook;
    g_nDmeHostChangeArgument = nArgument;
    return 0;
}

void RtDmeSetSessionTime(unsigned int uTime) {
    g_uDmeSessionTime = uTime;
}

void *RtDmeAlloc(unsigned int uSize) {
    void *pBlock;

    pBlock = NULL;
    if (RtAllocZeroedBlock(&pBlock, uSize) != 0) {
        printf("rt_memory_allocate failed \n");
    }
    return pBlock;
}

void RtDmeFree(void **ppBlock) {
    if (*ppBlock == NULL) {
        return;
    }
    if (RtFreeBlock(ppBlock) != 0) {
        printf("rt_memory_free failed \n");
    }
}

int RtDmeSetMemoryFunctions(rtAllocFunc pfnAlloc, rtFreeFunc pfnFree) {
    if (RtSetAllocHooks(pfnAlloc, pfnFree) != 0) {
        return RT_DME_RESULT_NO_MEMORY;
    }
    return 0;
}

int RtDmeParseAddress(unsigned char *pbAddress, const char *pszAddress) {
    char szAddress[RT_DME_ADDRESS_SIZE];
    char *pszPart;
    char *pszDot;
    int i;

    pszPart = szAddress;
    if (pszAddress == NULL || pbAddress == NULL) {
        return RT_DME_RESULT_NO_RECORD;
    }
    strncpy(szAddress, pszAddress, sizeof(szAddress));
    if (strlen(szAddress) == 0) {
        return RT_DME_RESULT_INVALID;
    }
    for (i = 0; i < kAddressParts - 1; ++i) {
        pszDot = strstr(pszPart, ".");
        if (pszDot == NULL) {
            return RT_DME_RESULT_NO_RECORD;
        }
        *pszDot = '\0';
        pbAddress[i] = atoi(pszPart);
        pszPart = pszDot + 1;
    }
    pbAddress[kAddressParts - 1] = atoi(pszPart);
    return 0;
}

int RtDmeFormatAddress(char *pszAddress, const unsigned char *pbAddress) {
    if (pszAddress == NULL || pbAddress == NULL) {
        return RT_DME_RESULT_NO_RECORD;
    }
    sprintf(pszAddress, "%u.%u.%u.%u", pbAddress[0], pbAddress[1], pbAddress[2], pbAddress[3]);
    return 0;
}

int RtDmeGetLocalAddress(char *pszAddress) {
    if (pszAddress == NULL) {
        return RT_DME_RESULT_INVALID;
    }
    memset(pszAddress, 0, RT_DME_ADDRESS_SIZE);
    if (!g_nDmeRunning) {
        return RT_DME_RESULT_NOT_STARTED;
    }
    strncpy(
        pszAddress, g_dmeLocalAddresses[RT_DME_ADDRESS_INDEX_PUBLIC].address, RT_DME_ADDRESS_SIZE);
    return 0;
}

void RtDmeAddClient(int nConnection, int nClient, const char *pszAddress) {
    RtDmeConnection *pConnection;
    RtDmeClient *pClient;
    RtDmeClientStats *pStats;
    RtDmeJoinAnnouncement announcement;
    RtDmeClientJoinFunc pfnJoin;
    const char *pszLocalAddress;
    int nHost;
    int nIndex;

    nHost = RT_DME_NO_CLIENT;
    pConnection = NULL;
    pClient = NULL;
    if ((unsigned int)nClient >= RT_DME_CLIENT_COUNT ||
        (unsigned int)nConnection >= RT_DME_CONNECTION_COUNT) {
        return;
    }
    if (RtDmeGetConnection(nConnection, &pConnection) != 0 || pConnection == NULL) {
        return;
    }
    if (RtDmeCreateClient(nConnection, nClient, &pClient) != 0 || pClient == NULL) {
        return;
    }
    nIndex = RtDmeGetAddressIndex(nConnection, nClient);
    if (pClient->bJoined == 1) {
        RtDmeSendClientState(nConnection, nClient);
        return;
    }
    if (pConnection->nPendingJoins != 0) {
        --pConnection->nPendingJoins;
    }
    if (RtDmeGetClientAddressKind(nConnection, nClient, RT_DME_ADDRESS_INDEX_PRIVATE) == 0 &&
        RtDmeGetClientAddressKind(nConnection, nClient, RT_DME_ADDRESS_INDEX_PUBLIC) == 0) {
        RtDmeSetClientAddressKind(
            nConnection, nClient, RT_DME_ADDRESS_INDEX_PRIVATE, RT_DME_ADDRESS_PRIVATE);
        RtDmeSetClientAddressKind(
            nConnection, nClient, RT_DME_ADDRESS_INDEX_PUBLIC, RT_DME_ADDRESS_PUBLIC);
        RtDmeSetClientAddress(nConnection, nClient, RT_DME_ADDRESS_INDEX_PRIVATE, pszAddress);
        RtDmeSetClientAddress(nConnection, nClient, RT_DME_ADDRESS_INDEX_PUBLIC, pszAddress);
        RtDmeSetClientPort(nConnection, nClient, RT_DME_ADDRESS_INDEX_PRIVATE, g_uDmePort);
        RtDmeSetClientPort(nConnection, nClient, RT_DME_ADDRESS_INDEX_PUBLIC, g_uDmePort);
        RtDmeSetAddressIndex(nConnection, nClient, RT_DME_ADDRESS_INDEX_PRIVATE);
    }
    pClient->bJoined = 1;
    RtReadTime(&pClient->lastHeard);
    if (RtDmeGetClient(nConnection, pConnection->nLocalClient, &pClient) != 0 || pClient == NULL) {
        return;
    }
    if (pClient->bJoined && RtDmeVoiceIsOpen(nConnection) == 1 &&
        RtDmeVoiceAddClient(nConnection, nClient) != 0) {
        return;
    }
    pfnJoin = pConnection->pfnClientJoin;
    if (pfnJoin != NULL) {
        pfnJoin(nConnection, nClient, RtDmeGetClientAddress(nConnection, nClient, nIndex));
    }
    if (pConnection->nState != RT_DME_CONNECTION_CONNECTED) {
        return;
    }
    announcement.uClient = RtDmeClientToByte(pConnection->nLocalClient);
    // Yes, the binary takes the address of this console at the index of the client that joined.
    pszLocalAddress = RtDmeGetClientAddress(nConnection, announcement.uClient, nIndex);
    if (pszLocalAddress != NULL && strlen(pszLocalAddress) != 0) {
        RtDmeParseAddress(announcement.abAddress, pszLocalAddress);
    } else {
        memset(announcement.abAddress, 0, sizeof(announcement.abAddress));
    }
    announcement.uConnection = nConnection;
    RtReadKeyPairModulus(0, &announcement.modulus);
    RtDmeSendSystemMessage(kSystemSendFlags,
                           nConnection,
                           nClient,
                           g_nDmeClientJoinMessage,
                           sizeof(announcement),
                           &announcement);
    RtDmeSendClientFlags(nConnection, nClient);
    RtDmeSendClientState(nConnection, nClient);
    RtDmeGetHostClient(&nHost, nConnection);
    if (nConnection != 0 || pConnection->nLocalClient != nHost) {
        return;
    }
    pStats = NULL;
    RtDmeGetClientStats(nClient, &pStats);
    if (pStats != NULL) {
        RtReadClock(&pStats->lastProbe);
    }
}

int RtDmeGetSenderAddress(unsigned char *pbAddress) {
    if (pbAddress == NULL) {
        return RT_DME_RESULT_INVALID;
    }
    (void)RtDmeParseAddress(pbAddress, g_szDmeSenderAddress); // Yes, the binary discards this.
    return 0;
}

RtDmeMessageType *RtDmeGetMessageType(unsigned int nClass, unsigned int nType) {
    if (nClass >= RT_DME_MESSAGE_CLASS_COUNT || nType >= RT_DME_MESSAGE_TYPE_COUNT) {
        return NULL;
    }
    return g_apDmeMessageTypes[nClass][nType];
}

int RtDmeGetObject(RtDmeObject **ppObject, unsigned int nObject) {
    if (ppObject == NULL) {
        return RT_DME_RESULT_INVALID;
    }
    if (nObject >= RT_DME_OBJECT_COUNT) {
        *ppObject = NULL;
        return RT_DME_RESULT_OUT_OF_RANGE;
    }
    *ppObject = g_apDmeObjects[nObject];
    return 0;
}

int RtDmeFindStream(RtDmeStream **ppStream, unsigned int nStream) {
    RtDmeStream *pStream;
    int nResult;

    pStream = NULL;
    nResult = RtDmeGetStream(nStream, &pStream);
    if (nResult != 0) {
        return nResult;
    }
    if (pStream != NULL) {
        *ppStream = pStream;
    }
    return nResult;
}

int RtDmeGetStream(unsigned int nStream, RtDmeStream **ppStream) {
    int i;

    if (ppStream == NULL) {
        return RT_DME_RESULT_INVALID;
    }
    *ppStream = NULL;
    if (nStream >= RT_DME_STREAM_COUNT) {
        return RT_DME_RESULT_OUT_OF_RANGE;
    }
    if (g_pDmeStreams == NULL) {
        g_pDmeStreams = RtDmeAlloc(RT_DME_STREAM_COUNT * sizeof(RtDmeStream));
        if (g_pDmeStreams == NULL) {
            return RT_DME_RESULT_NO_MEMORY;
        }
        for (i = 0; i < RT_DME_STREAM_COUNT; ++i) {
            g_pDmeStreams[i].bUsed = 0;
        }
    }
    *ppStream = &g_pDmeStreams[nStream];
    return 0;
}

int RtDmeGetLocalTime(unsigned int *puTime) {
    unsigned int uTime;

    if (puTime == NULL) {
        return RT_DME_RESULT_INVALID;
    }
    (void)RtGetMilliseconds(&uTime); // Yes, the binary discards the result.
    *puTime = uTime;
    return 0;
}

int RtDmeGetSessionTime(unsigned int *puTime) {
    if (puTime == NULL) {
        return RT_DME_RESULT_INVALID;
    }
    *puTime = g_uDmeSessionTime;
    return 0;
}

int RtDmeGetHostClient(int *pnClient, int nConnection) {
    RtDmeConnection *pConnection;
    int nResult;

    pConnection = NULL;
    if ((unsigned int)nConnection >= RT_DME_CONNECTION_COUNT || pnClient == NULL) {
        return RT_DME_RESULT_INVALID;
    }
    *pnClient = RT_DME_NO_CLIENT;
    nResult = RtDmeGetConnection(nConnection, &pConnection);
    if (nResult != 0 || pConnection == NULL) {
        return nResult;
    }
    *pnClient = pConnection->nHostClient;
    return nResult;
}

int RtDmeCountJoinedClients(int *pnCount, int nConnection) {
    RtDmeConnection *pConnection;
    RtDmeClient *pClient;
    int nResult;
    int i;

    pConnection = NULL;
    if (pnCount == NULL) {
        return RT_DME_RESULT_INVALID;
    }
    *pnCount = 0;
    if ((unsigned int)nConnection >= RT_DME_CONNECTION_COUNT) {
        return RT_DME_RESULT_INVALID;
    }
    nResult = RtDmeGetConnection(nConnection, &pConnection);
    if (nResult != 0 || pConnection == NULL) {
        return nResult;
    }
    for (i = 0; i < RT_DME_CLIENT_COUNT; ++i) {
        pClient = pConnection->apClients[i];
        if (pClient != NULL && pClient->bJoined == 1) {
            ++*pnCount;
        }
    }
    return 0;
}

void RtDmeAddToClientSet(RtDmeClientSet *pSet, int nClient) {
    if (pSet == NULL || (unsigned int)nClient >= RT_DME_CLIENT_COUNT) {
        return;
    }
    if (nClient < pSet->nFirst) {
        pSet->nFirst = nClient;
    }
    if (pSet->nLast < nClient) {
        pSet->nLast = nClient;
    }
    pSet->abBits[nClient >> 3] |= 1 << (nClient & 7);
}

int RtDmeSendDatagram(signed char nFlags,
                      const char *pszAddress,
                      unsigned short uPort,
                      unsigned int nClass,
                      unsigned int nType,
                      unsigned int nSize,
                      const void *pData) {
    int nLength;

    if (pszAddress == NULL) {
        return RT_DME_RESULT_INVALID;
    }
    nLength = strlen(pszAddress);
    if (nLength == 0) {
        return RT_DME_RESULT_INVALID;
    }
    // Yes, the binary returns the length of the address for these missing arguments.
    if (uPort == 0 || pData == NULL || nSize == 0) {
        return nLength;
    }
    if (nSize > RT_DME_MAX_MESSAGE_SIZE) {
        return RT_DME_RESULT_TOO_LARGE;
    }
    if (nClass >= RT_DME_MESSAGE_CLASS_COUNT || nType >= RT_DME_MESSAGE_TYPE_COUNT) {
        return RT_DME_RESULT_INVALID;
    }
    if (strcmp(pszAddress, g_dmeLocalAddresses[RT_DME_ADDRESS_INDEX_PRIVATE].address) == 0 &&
        uPort == g_dmeLocalAddresses[RT_DME_ADDRESS_INDEX_PRIVATE].port) {
        return 0;
    }
    if (strcmp(pszAddress, g_dmeLocalAddresses[RT_DME_ADDRESS_INDEX_PUBLIC].address) == 0 &&
        uPort == g_dmeLocalAddresses[RT_DME_ADDRESS_INDEX_PUBLIC].port) {
        return 0;
    }
    g_dmeDatagram.uClass = nClass;
    g_dmeDatagram.uType = nType;
    memcpy(g_dmeDatagram.abData, pData, nSize);
    return RtDmeConvertResult(
        RtChannelApiSendDatagram(
            pszAddress, uPort, nFlags, &g_dmeDatagram, nSize + kMessageHeaderSize),
        RT_DME_RESULT_SERVICE);
}

int RtDmeSendDatagramToBytes(signed char nFlags,
                             const unsigned char *pbAddress,
                             unsigned int nClass,
                             unsigned int nType,
                             int nSize,
                             const void *pData) {
    char szAddress[RT_DME_ADDRESS_SIZE];

    if (pbAddress == NULL || nSize <= 0) {
        return RT_DME_RESULT_INVALID;
    }
    RtDmeFormatAddress(szAddress, pbAddress);
    return RtDmeSendDatagram(nFlags, szAddress, g_uDmePort, nClass, nType, nSize, pData);
}

int RtDmeBroadcastDatagram(unsigned char bIncludeSelf,
                           unsigned char bForce,
                           signed char nFlags,
                           int nConnection,
                           unsigned int nClass,
                           unsigned int nType,
                           unsigned int nSize,
                           const void *pData) {
    RtDmeConnection *pConnection;
    const char *pszAddress;
    unsigned short uPort;
    signed char nClientFlags;
    int nIndex;
    int nClient;
    int nResult;

    pConnection = NULL;
    nResult = RtDmeGetConnection(nConnection, &pConnection);
    if (nResult != 0 || pConnection == NULL) {
        return nResult;
    }
    for (nClient = nResult; nClient < pConnection->nMaxClients; ++nClient) {
        if (nClient == pConnection->nLocalClient && bIncludeSelf != 1) {
            continue;
        }
        nIndex = RtDmeGetAddressIndex(nConnection, nClient);
        if (RtDmeGetClientFlags(nConnection, nClient, &nClientFlags) != 0) {
            return nResult;
        }
        if ((unsigned int)nIndex >= RT_DME_ADDRESS_COUNT) {
            continue;
        }
        if (RtDmeGetClientAddressKind(nConnection, nClient, nIndex) == 0) {
            continue;
        }
        if (!(nClientFlags & 1) && bForce != 1) {
            continue;
        }
        pszAddress = RtDmeGetClientAddress(nConnection, nClient, nIndex);
        uPort = RtDmeGetClientPort(nConnection, nClient, nIndex);
        if (RtDmeSendDatagram(nFlags, pszAddress, uPort, nClass, nType, nSize, pData) != 0) {
            nResult = RT_DME_RESULT_SEND_FAILED;
        }
    }
    return nResult;
}

int RtDmeSend(signed char nFlags,
              int nConnection,
              int nClient,
              unsigned int nClass,
              unsigned int nType,
              int nSize,
              const void *pData) {
    RtDmeConnection *pConnection;
    RtDmeFragmentHeader header;
    RtDmeOutgoing outgoing;
    unsigned int uSendClass;
    unsigned int uSendType;
    signed char nSendFlags;
    int nCount;
    int nChunk;
    int nSent;
    int nOffset;
    int i;
    int nResult;

    nOffset = 0;
    nSent = 0;
    pConnection = NULL;
    if (nClass >= RT_DME_MESSAGE_CLASS_COUNT || nType >= RT_DME_MESSAGE_TYPE_COUNT || nSize <= 0 ||
        pData == NULL) {
        return RT_DME_RESULT_INVALID;
    }
    nResult = RtDmeGetConnection(nConnection, &pConnection);
    if (nResult != 0 || pConnection == NULL) {
        return nResult;
    }
    if (nSize > RT_DME_MAX_MESSAGE_SIZE) {
        nCount = (nSize + RT_DME_FRAGMENT_DATA_SIZE - 1) / RT_DME_FRAGMENT_DATA_SIZE;
        uSendClass = RT_DME_CLASS_SYSTEM;
        uSendType = g_nDmeFragmentMessage;
        nSendFlags = nFlags | kSystemSendFlags;
        header.uMessageId = RtRandom();
    } else {
        uSendClass = nClass;
        nSent = nSize;
        nSendFlags = nFlags;
        nOffset = nSent;
        uSendType = nType;
        memcpy(outgoing.abData, pData, nSent);
        nCount = 1;
    }
    for (i = 0; i < nCount; ++i) {
        if (nSent != nSize) {
            nChunk = nSize - nOffset;
            if (nChunk > RT_DME_FRAGMENT_DATA_SIZE) {
                nChunk = RT_DME_FRAGMENT_DATA_SIZE;
            }
            header.uIndex = i;
            header.uClass = nClass;
            header.nTotalSize = nSize;
            header.uType = nType;
            header.uCount = nCount;
            header.uSize = nChunk;
            nSent = nChunk + sizeof(RtDmeFragmentHeader);
            header.nOffset = nOffset;
            outgoing.fragment.header = header;
            memcpy(outgoing.fragment.abData, (const unsigned char *)pData + nOffset, nChunk);
            nOffset += nChunk;
        }
        if (nConnection == RT_DME_SERVER_CLIENT || nConnection == 0) {
            nResult = RtDmeSendDatagramToClient(
                nSendFlags, nConnection, nClient, uSendClass, uSendType, nSent, &outgoing);
        } else {
            nResult = RtDmeSendChannelMessage(
                nSendFlags, nConnection, nClient, uSendClass, uSendType, nSent, &outgoing);
        }
        if (nResult != 0) {
            return nResult;
        }
    }
    return 0;
}

int RtDmeSendToClientSet(signed char nFlags,
                         int nConnection,
                         unsigned int nClass,
                         unsigned int nType,
                         int nSize,
                         const void *pData,
                         const RtDmeClientSet *pSet) {
    RtDmeConnection *pConnection;
    RtDmeObjectRange *pRange;
    int nClient;
    int nResult;

    pConnection = NULL;
    pRange = NULL;
    if (nClass >= RT_DME_MESSAGE_CLASS_COUNT || nType >= RT_DME_MESSAGE_TYPE_COUNT || nSize <= 0 ||
        pData == NULL || pSet == NULL) {
        return RT_DME_RESULT_INVALID;
    }
    nResult = RtDmeGetConnection(nConnection, &pConnection);
    if (nResult != 0 || pConnection == NULL) {
        return nResult;
    }
    if (pSet->nLast < 0) {
        return nResult;
    }
    for (nClient = pSet->nFirst; nClient <= pSet->nLast; ++nClient) {
        if (nClient == pConnection->nLocalClient) {
            continue;
        }
        RtDmeGetObjectRange(&pRange, nConnection, nClient);
        if (pRange == NULL || ((pSet->abBits[nClient >> 3] >> (nClient & 7)) & 1) == 0) {
            continue;
        }
        if (RtDmeSend(nFlags, nConnection, nClient, nClass, nType, nSize, pData) != 0) {
            nResult = RT_DME_RESULT_SEND_FAILED;
        }
    }
    return nResult;
}

int RtDmeSendApplicationMessage(signed char nFlags,
                                int nConnection,
                                int nClient,
                                unsigned int nType,
                                int nSize,
                                const void *pData) {
    if (nType >= RT_DME_MESSAGE_TYPE_COUNT || nSize <= 0 || pData == NULL) {
        return RT_DME_RESULT_INVALID;
    }
    return RtDmeSend(nFlags, nConnection, nClient, RT_DME_CLASS_APPLICATION, nType, nSize, pData);
}

int RtDmeSendSystemMessage(signed char nFlags,
                           int nConnection,
                           int nClient,
                           unsigned int nType,
                           int nSize,
                           const void *pData) {
    if (nType >= RT_DME_MESSAGE_TYPE_COUNT || nSize <= 0 || pData == NULL) {
        return RT_DME_RESULT_INVALID;
    }
    return RtDmeSend(nFlags, nConnection, nClient, RT_DME_CLASS_SYSTEM, nType, nSize, pData);
}

void RtDmeNotifyObjectAdded(unsigned int nOwner, unsigned int nObject) {
    RtDmeObject *pObject;
    RtDmeObjectAddedFunc pfnAdded;

    if (nOwner >= RT_DME_CLIENT_COUNT || nObject >= RT_DME_OBJECT_COUNT) {
        return;
    }
    (void)RtDmeGetObject(&pObject, nObject); // Yes, the binary discards the result.
    if (pObject == NULL) {
        return;
    }
    pfnAdded = g_apfnDmeObjectAdded[pObject->uType];
    if (pfnAdded != NULL) {
        pfnAdded(nOwner, nObject);
    }
}

void RtDmeNotifyObjectChanged(int nOwner, unsigned int nObject, int nField) {
    RtDmeObject *pObject;
    RtDmeObjectChangedFunc pfnChanged;

    if (nObject >= RT_DME_OBJECT_COUNT) {
        return;
    }
    if ((unsigned int)nField >= kObjectFieldCount && nField != kObjectChangedWhole &&
        nField != kObjectChangedAll) {
        return;
    }
    (void)RtDmeGetObject(&pObject, nObject); // Yes, the binary discards the result.
    if (pObject == NULL) {
        return;
    }
    pfnChanged = g_apfnDmeObjectChanged[pObject->uType];
    if (pfnChanged != NULL) {
        pfnChanged(nOwner, nObject, nField);
    }
}

int RtDmeDestroyObject(unsigned int nObject) {
    RtDmeObject *pObject;
    RtDmeObjectRange *pRange;
    signed char *pnState;

    pRange = NULL;
    if (nObject >= RT_DME_OBJECT_COUNT) {
        return RT_DME_RESULT_INVALID;
    }
    if ((unsigned int)g_nDmeActiveConnection >= RT_DME_CONNECTION_COUNT) {
        return RT_DME_RESULT_OUT_OF_RANGE;
    }
    RtDmeGetObject(&pObject, nObject);
    if (pObject == NULL) {
        return RT_DME_RESULT_OUT_OF_RANGE;
    }
    RtDmeNotifyObjectRemoved(pObject->nOwner, nObject);
    RtDmeGetObject(&pObject, nObject);
    if (pObject == NULL) {
        return 0;
    }
    pnState = &g_anDmeObjectState[nObject];
    *pnState = RT_DME_OBJECT_CLOSING;
    if (g_nDmeActiveConnection < RT_DME_CONNECTION_COUNT) {
        RtDmeGetObjectRange(&pRange, g_nDmeActiveConnection, pObject->nOwner);
        if (pRange != NULL && ((int)nObject < pRange->uFirstObject ||
                               (int)nObject >= pRange->uFirstObject + pRange->uObjectCount)) {
            *pnState = RT_DME_OBJECT_FREE;
        }
    }
    if (pObject->pbShadow != NULL) {
        RtDmeFree((void **)&pObject->pbShadow);
        pObject->pbShadow = NULL;
    }
    if (pObject->pbData != NULL) {
        RtDmeFree((void **)&pObject->pbData);
        pObject->pbData = NULL;
    }
    g_apDmeObjects[nObject] = NULL;
    RtDmeFree((void **)&pObject);
    return 0;
}

int RtDmeFindObjectByName(const char *pszName, unsigned int nConnection, unsigned int nClient) {
    RtDmeObjectRange *pRange;
    RtDmeObject *pObject;
    const char *pszObjectName;
    int nObject;
    int i;

    if (pszName == NULL || nConnection >= RT_DME_CONNECTION_COUNT ||
        nClient >= RT_DME_CLIENT_COUNT) {
        return -1;
    }
    RtDmeGetObjectRange(&pRange, nConnection, nClient);
    if (pRange == NULL) {
        return -1;
    }
    for (i = 0; i < pRange->uObjectCount; ++i) {
        nObject = i + pRange->uFirstObject;
        if (g_anDmeObjectState[nObject] != RT_DME_OBJECT_USED) {
            continue;
        }
        RtDmeGetObject(&pObject, nObject);
        // Yes, the binary tests the address of the name, which is non-null even for no object.
        pszObjectName = pObject->szName;
        if (pszObjectName != NULL && strcmp(pszObjectName, pszName) == 0) {
            return nObject;
        }
    }
    return -1;
}

int RtDmeFindFreeObject(void) {
    RtDmeConnection *pConnection;
    RtDmeObjectRange *pRange;
    int nResult;
    int i;

    pConnection = NULL;
    pRange = NULL;
    if ((unsigned int)g_nDmeActiveConnection >= RT_DME_CONNECTION_COUNT) {
        return -1;
    }
    nResult = RtDmeGetConnection(g_nDmeActiveConnection, &pConnection);
    // Yes, the binary returns the result of the lookup as an object.
    if (nResult != 0 || pConnection == NULL) {
        return nResult;
    }
    RtDmeGetObjectRange(&pRange, g_nDmeActiveConnection, pConnection->nLocalClient);
    if (pRange == NULL) {
        return -1;
    }
    for (i = 0; i < pRange->uObjectCount; ++i) {
        if (g_anDmeObjectState[pRange->uFirstObject + i] != RT_DME_OBJECT_USED) {
            return i + pRange->uFirstObject;
        }
    }
    return -1;
}

int RtDmeRemoveClient(int nConnection,
                      int nClient,
                      const char *pszAddress,
                      unsigned char bAnnounce) {
    RtDmeConnection *pConnection;
    RtDmeClient *pClient;
    RtDmeObjectRange *pRange;
    RtDmeClientLeaveFunc pfnLeave;
    RtDmeHostChange change;
    int nLocalClient;
    int nIndex;
    int nCandidate;
    int nResult;

    pConnection = NULL;
    pClient = NULL;
    pRange = NULL;
    // Yes, the binary checks the connection against the client count.
    if (nClient < 0 && nConnection >= RT_DME_CLIENT_COUNT) {
        return RT_DME_RESULT_INVALID;
    }
    nResult = RtDmeGetConnection(nConnection, &pConnection);
    if (nResult != 0 || pConnection == NULL) {
        return nResult;
    }
    nResult = RtDmeGetClient(nConnection, nClient, &pClient);
    if (nResult != 0) {
        return nResult;
    }
    nLocalClient = pConnection->nLocalClient;
    if (pClient == NULL) {
        if (pConnection->nPendingJoins != 0) {
            --pConnection->nPendingJoins;
        }
    } else {
        nIndex = RtDmeGetAddressIndex(nConnection, nClient);
        if ((unsigned int)nIndex >= RT_DME_ADDRESS_COUNT && pClient->bJoined != 1) {
            return 0;
        }
        if (RtDmeGetClientAddressKind(nConnection, nClient, RT_DME_ADDRESS_INDEX_PRIVATE) == 0 ||
            RtDmeGetClientAddressKind(nConnection, nClient, RT_DME_ADDRESS_INDEX_PUBLIC) == 0) {
            return 0;
        }
        RtDmeGetObjectRange(&pRange, nConnection, nClient);
        if (pRange != NULL) {
            RtDmeReleaseClientObjects(nConnection, nClient);
        }
        if (nLocalClient != RT_DME_NO_CLIENT) {
            nResult = RtDmeGetClient(nConnection, nLocalClient, &pClient);
            if (nResult != 0) {
                return nResult;
            }
            // Yes, the binary checks the connection again, not the local client.
            if (pConnection == NULL) {
                return nResult;
            }
            if (pClient->bJoined == 1) {
                if (RtDmeVoiceIsOpen(nConnection) == 1) {
                    RtDmeVoiceRemoveClient(nConnection, nClient);
                }
                pfnLeave = pConnection->pfnClientLeave;
                if (pfnLeave != NULL) {
                    pfnLeave(nConnection, nClient, pszAddress);
                }
            }
        }
        if (g_bDmeServiceStarted == 1) {
            RtDmeDropClientPeers(nConnection, nClient);
        }
        if (bAnnounce == 1 && nConnection == 0) {
            RtDmeAnnounceDeparture(0, nClient);
        }
        RtDmeClearClient(nConnection, nClient);
        if (nConnection == 0) {
            nResult = RtDmeFinishDeparture(0, nClient);
            if (nResult != 0) {
                return nResult;
            }
            if (g_nDmeHostClient == nClient) {
                for (nCandidate = 0; nCandidate < pConnection->nMaxClients; ++nCandidate) {
                    nResult = RtDmeGetClient(0, nCandidate, &pClient);
                    if (nResult != 0) {
                        return nResult;
                    }
                    if (pClient == NULL) {
                        continue;
                    }
                    nIndex = RtDmeGetAddressIndex(0, nCandidate);
                    if ((unsigned int)nIndex >= RT_DME_ADDRESS_COUNT && pClient->bJoined != 1) {
                        continue;
                    }
                    if (RtDmeGetClientAddressKind(0, nCandidate, RT_DME_ADDRESS_INDEX_PRIVATE) ==
                            0 ||
                        RtDmeGetClientAddressKind(0, nCandidate, RT_DME_ADDRESS_INDEX_PUBLIC) ==
                            0) {
                        continue;
                    }
                    g_nDmeHostClient = nCandidate;
                    if (g_pfnDmeHostChange != NULL) {
                        change.nConnection = 0;
                        change.nHost = nCandidate;
                        change.nArgument = g_nDmeHostChangeArgument;
                        g_pfnDmeHostChange(&change);
                    }
                    break;
                }
            }
        }
    }
    nResult = RtDmeGetClient(nConnection, nLocalClient, &pClient);
    if (nResult != 0 || pClient == NULL) {
        return nResult;
    }
    if (pClient->nRole != RT_DME_ROLE_HOST) {
        return 0;
    }
    RtDmeElectHost(nConnection, 0, 0, 0);
    return 0;
}

int RtDmeStartup(const rtNetSettings *pSettings, rtNetInitStatus *pStatus) {
    int nResult;
    int nClass;
    int i;

    if (pStatus == NULL) {
        return RT_DME_RESULT_INVALID;
    }
    pStatus->result = RT_DME_RESULT_STARTUP_FAILED;
    pStatus->version = RT_NET_SETTINGS_VERSION;
    nResult = RtDmeGetVersion(pStatus->libraryVersion);
    if (nResult != 0) {
        pStatus->result = nResult;
        return nResult;
    }
    if (pSettings == NULL) {
        pStatus->result = RT_DME_RESULT_INVALID;
        return RT_DME_RESULT_INVALID;
    }
    if (pSettings->version != RT_NET_SETTINGS_VERSION) {
        pStatus->result = RT_DME_RESULT_BAD_VERSION;
        return RT_DME_RESULT_INVALID;
    }
    if (g_nDmeRunning) {
        pStatus->result = 0;
        return nResult;
    }
    g_nDmeRunning = pSettings->version;
    memset(g_apDmeConnections, 0, sizeof(g_apDmeConnections));
    memset(g_apDmeClientStats, 0, sizeof(g_apDmeClientStats));
    g_bDmeUseFixedAddress = 0;
    g_nDmeHostClient = RT_DME_NO_CLIENT;
    g_nDmeApplicationId = pSettings->applicationId;
    RtDmeSetSessionTime(0);
    for (nClass = 0; nClass < RT_DME_MESSAGE_CLASS_COUNT; ++nClass) {
        g_auDmeMessageTypeCount[nClass] = 0;
        for (i = RT_DME_MESSAGE_TYPE_COUNT - 1; i >= 0; --i) {
            g_apDmeMessageTypes[nClass][i] = NULL;
        }
    }
    for (i = RT_DME_PING_COUNT - 1; i >= 0; --i) {
        g_aDmePings[i].pfnDone = NULL;
    }
    g_anDmeUnreadWords[3] = 1;
    g_anDmeUnreadWords[0] = 0;
    g_anDmeUnreadWords[1] = 0;
    g_nDmeObjectCount = 0;
    g_anDmeUnreadWords[2] = 0;
    RtDmeRegisterSystemMessages();
    for (i = 0; i < RT_DME_OBJECT_COUNT; ++i) {
        g_anDmeObjectState[i] = RT_DME_OBJECT_FREE;
        g_apDmeObjects[i] = NULL;
    }
    for (i = 0; i < RT_DME_OBJECT_TYPE_COUNT; ++i) {
        g_apfnDmeObjectAdded[i] = NULL;
        g_apfnDmeObjectChanged[i] = NULL;
        g_apfnDmeObjectRemoved[i] = NULL;
    }
    RtDmeInitObjectRanges();
    nResult = RtDmeClearReassembly();
    if (nResult != 0) {
        return nResult;
    }
    nResult = RtChannelApiStartup(pSettings->keyPair);
    if (nResult != 0) {
        printf("msg client startup failed, msg error code = %d\n", nResult);
        pStatus->result = RT_DME_RESULT_STARTUP_FAILED;
        return RT_DME_RESULT_STARTUP_FAILED;
    }
    nResult =
        RtChannelApiGetLocalAddress(g_dmeLocalAddresses[RT_DME_ADDRESS_INDEX_PRIVATE].address);
    if (nResult != 0) {
        printf("get local IP failed, msg error code = %d\n", nResult);
    }
    strncpy(g_dmeLocalAddresses[RT_DME_ADDRESS_INDEX_PUBLIC].address,
            g_dmeLocalAddresses[RT_DME_ADDRESS_INDEX_PRIVATE].address,
            RT_DME_ADDRESS_SIZE);
    g_uDmePort = (pSettings->port != 0) ? (unsigned short)pSettings->port : RT_DME_DEFAULT_PORT;
    g_dmeLocalAddresses[RT_DME_ADDRESS_INDEX_PUBLIC].type = RT_DME_ADDRESS_PUBLIC;
    g_dmeLocalAddresses[RT_DME_ADDRESS_INDEX_PUBLIC].port = g_uDmePort;
    g_dmeLocalAddresses[RT_DME_ADDRESS_INDEX_PRIVATE].type = RT_DME_ADDRESS_PRIVATE;
    g_dmeLocalAddresses[RT_DME_ADDRESS_INDEX_PRIVATE].port = g_uDmePort;
    pStatus->result = 0;
    memset(&g_dmeMapping, 0, sizeof(g_dmeMapping));
    memset(g_aDmePolls, 0, sizeof(g_aDmePolls));
    g_bDmeListening = 0;
    return RtDmeVoiceStartup();
}

int RtDmeStartDatagramService(unsigned int uSendSize, unsigned int uReceiveSize) {
    RtChannelServiceConfig config;
    int nResult;

    if (g_bDmeServiceStarted) {
        return 0;
    }
    nResult = RtDmeConvertResult(RtChannelApiDefaultServiceConfig(&config), RT_DME_RESULT_SERVICE);
    if (nResult != 0) {
        return nResult;
    }
    config.pfnDatagram = RtDmeOnDatagram;
    config.uSendSize = uSendSize;
    config.uReceiveSize = uReceiveSize;
    config.pfnTimeout = RtDmeOnPeerTimeout;
    config.uPeerTimeout = kPeerTimeout;
    config.uPort = g_uDmePort;
    nResult = RtDmeConvertResult(RtChannelApiStartService(&config), RT_DME_RESULT_SERVICE);
    if (nResult != 0) {
        return nResult;
    }
    g_bDmeServiceStarted = 1;
    return 0;
}

int RtDmeDestroyAllObjects(void) {
    int i;

    for (i = 0; i < RT_DME_OBJECT_COUNT; ++i) {
        if (g_anDmeObjectState[i] == RT_DME_OBJECT_USED) {
            RtDmeDestroyObject(i);
        }
        g_anDmeObjectState[i] = RT_DME_OBJECT_FREE;
    }
    return 0;
}

int RtDmeConvertStatus(int *pnStatus, unsigned int nChannelStatus) {
    if (pnStatus == NULL) {
        return RT_DME_RESULT_INVALID;
    }
    // Yes, the binary maps each of the 10 statuses to itself through a table.
    if (nChannelStatus >= kChannelStatusCount) {
        return RT_DME_RESULT_INVALID;
    }
    *pnStatus = nChannelStatus;
    return 0;
}

void RtDmeSetConnectResult(int nConnection, int nClient, unsigned char bConnected, int nReason) {
    RtDmeConnection *pConnection;
    RtDmeClient *pClient;
    int nResult;

    pConnection = NULL;
    pClient = NULL;
    if ((unsigned int)nConnection >= RT_DME_CONNECTION_COUNT) {
        return;
    }
    if (RtDmeIsClientIndex(nClient) != 1) {
        return;
    }
    if (RtDmeGetConnection(nConnection, &pConnection) != 0 || pConnection == NULL) {
        return;
    }
    if (pConnection->nState == RT_DME_CONNECTION_CONNECTED) {
        return;
    }
    nResult = RtDmeGetClient(nConnection, nClient, &pClient);
    if (nResult != 0 || pClient == NULL) {
        return;
    }
    if (bConnected) {
        pClient->nRole = RT_DME_ROLE_LOCAL;
        pConnection->nLocalClient = nClient;
        pConnection->nState = RT_DME_CONNECTION_CONNECTED;
        pClient->anReserved34[1] = 0;
        pClient->anReserved34[0] = 0;
        RtSetConnectionPeer(RT_SECURITY_SERVER_ID, nClient);
        RtDmeSetAddressIndex(nConnection, nClient, RT_DME_ADDRESS_INDEX_PRIVATE);
        pClient->bJoined = 1;
        RtReadTime(&pClient->lastHeard);
        if (RtDmeVoiceIsOpen(nConnection) == 1) {
            nResult = RtDmeVoiceStart(nConnection, nClient);
            if (nResult != 0) {
                return;
            }
        }
    } else {
        pConnection->nState = RT_DME_CONNECTION_FAILED;
        pConnection->nLocalClient = RT_DME_NO_CLIENT;
        g_nDmeHostClient = RT_DME_NO_CLIENT;
        nResult = RT_DME_CONNECTION_FAILED;
        if (RtDmeVoiceIsOpen(nConnection) == 1) {
            RtDmeVoiceClose(nConnection);
        }
    }
    if (g_pfnDmeConnect != NULL) {
        g_pfnDmeConnect(nConnection, pConnection->nState, nReason, nResult);
        g_pfnDmeConnect = NULL;
        g_bDmeConnectReported = 1;
    }
}
