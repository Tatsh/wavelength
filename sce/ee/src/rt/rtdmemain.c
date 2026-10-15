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
    // The size of each queue of the datagram service.
    kServiceQueueSize = 8192,
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
    // The milliseconds a connection waits for its result.
    kConnectTimeout = 30000,
    // The keep-alive interval of the channel of a connection.
    kChannelKeepalive = 15000,
    // The milliseconds rtDmeDisconnect() pumps the channels to send the farewell.
    kDisconnectDrain = 500,
    // The milliseconds a latency probe or a listener poll waits for its answer.
    kPingTimeout = 30000,
    kPollTimeout = 10000,
    // The milliseconds between latency probes of the clients, in session time.
    kLatencyProbeInterval = 3000,
    // The milliseconds between probes of the unresponsive clients of connection 0.
    kPeerProbeInterval = 4000,
    // The milliseconds between probes of the addresses of a client, and the probes before it is
    // given up.
    kAddressProbeInterval = 300,
    kAddressProbeLimit = 85,
    // The silence that drops the host candidate this console watches.
    kWatchedClientTimeout = 30000,
    // The bytes of the session and access keys in a channel request.
    kSessionKeyBytes = 17,
    kRequestBufferSize = 256,
    // The size of a client record, its addresses alone.
    kClientAddressBytes = 48,
    // The size of the parameters of the public entry points.
    kConnectParamsSize = 276,
    kHostParamsSize = 104,
    kAddressQuerySize = 32,
    kAddressReportSize = 52,
    // The default number of clients of a hosted session.
    kDefaultHostClients = 8,
    // The largest chunk of a stream, and the number of bytes the size counts per second.
    kMaxStreamChunk = 255,
    kStreamBytesScale = 1000,
    // The send flags of an object.
    kObjectSendFlags = RT_DME_SEND_GUARANTEED,
    kStreamSendFlags = RT_DME_SEND_ORDERED | RT_DME_SEND_GUARANTEED,
    kPingSendFlags = RT_DME_SEND_BASE,
    // The roles of a client that finished joining the objects.
    kRoleObjectsJoined = 3,
    kRoleObjectsJoinedHost = 4,
    // The kind of the connection of RtDmeConnect().
    kConnectChannel = 1,
    kConnectDatagram = 2,
    kConnectChannelDatagram = 3,
    // The mode of the channel of each kind.
    kChannelModeStream = 0,
    kChannelModeDatagram = 1,
    // The kind of an address the gateway is reached at.
    kGatewayAddress = 3,
    // The states of a listener poll.
    kPollFree = 0,
    kPollStarted = 1,
    kPollWaiting = 2,
    kPollSucceeded = 3,
    kPollFailed = 4,
    kPollStateCount = 5,
};

// The headroom a stream is allowed over its rate.
#define STREAM_RATE_HEADROOM 1.02

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

// A latency probe, and its answer.
typedef struct {
    unsigned int uTime; // The session time of the sender, or 0.
    unsigned char uSlot;
    unsigned char bRequest;
    unsigned char abReserved06[2]; // Yes, the binary sends these bytes uninitialised.
} RtDmePingMessage;

// The addresses of this console, sent to probe the addresses of a client.
typedef struct {
    unsigned char abPrivateAddress[kAddressBytes];
    unsigned char abPublicAddress[kAddressBytes];
    unsigned short uPrivatePort;
    unsigned short uPublicPort;
} RtDmeAddressProbe;

// The size of the buffers of the object and stream messages.
enum {
    kObjectMessageSize = 512,
    kObjectHeaderSize = 26,
    kObjectUpdateHeaderSize = 4,
    kStreamHeaderSize = 12,
};

// A whole object.
typedef union {
    struct {
        unsigned char uOwner;
        unsigned char uType;
        unsigned char uFilter;
        unsigned char uFlags;
        unsigned char uLifetime;
        unsigned char uReserved05; // Yes, the binary sends this byte uninitialised.
        unsigned short uLockClient;
        unsigned short uObject;
        char szName[RT_DME_ADDRESS_SIZE];
    } header;
    unsigned char abBytes[kObjectMessageSize];
} RtDmeObjectMessage;

// The changed fields of an object, each an index byte before its data.
typedef union {
    struct {
        unsigned char uType;
        unsigned char uCount;
        unsigned short uObject;
    } header;
    unsigned char abBytes[kObjectMessageSize];
} RtDmeObjectUpdateMessage;

// A part of a stream.
typedef union {
    struct {
        unsigned char uStream;
        unsigned char bToHost;
        unsigned char uRoute;
        unsigned char uKind;
        unsigned char bOwnsData;
        unsigned char uReserved05; // Yes, the binary sends this byte uninitialised.
        unsigned short uStart;
        unsigned short uEnd;
        unsigned short uSize;
    } header;
    unsigned char abBytes[kObjectMessageSize];
} RtDmeStreamMessage;

// The results of the last pump of the channels.
typedef struct {
    int nSize;
    int anConnections[RT_DME_CONNECTION_COUNT];
    int nDatagrams;
} RtDmePumpResults;

// What a client matched in RtDmeFindClientByAddress().
enum {
    kMatchNone = 0,
    kMatchWorking = 1, // The address the client works at, which ends the search.
    kMatchOther = 2,   // Another address, which the search records and goes on past.
};

// NTSC-U/C: 0x003b6bec, PAL: 0x00435aac
int g_nDmeActiveConnection = RT_DME_CONNECTION_COUNT; // No connection is active.
// NTSC-U/C: 0x003b6bf0, PAL: 0x00435ab0
unsigned char g_bDmeObjectsShared;
// NTSC-U/C: 0x003b6bf4, PAL: 0x00435ab4
int g_nDmeRunning;
// The session time the clients were last probed for their latency.
// NTSC-U/C: 0x003b6bf8, PAL: 0x00435ab8
static unsigned int g_uDmeLastLatencyProbe;
// NTSC-U/C: 0x003b6bfc, PAL: 0x00435abc
int g_nDmeObjectCount;
// NTSC-U/C: 0x003b6c00, PAL: 0x00435ac0
RtDmeConnectFunc g_pfnDmeConnect;
// NTSC-U/C: 0x003b6c04, PAL: 0x00435ac4
RtDmeConnectFunc g_pfnDmeStatus;
// NTSC-U/C: 0x003b6c08, PAL: 0x00435ac8
int g_nDmeSessionReadyArgument;
// NTSC-U/C: 0x003b6c10, PAL: 0x00435ad0
rtDmeHostChangeCallback g_pfnDmeHostChange;
// NTSC-U/C: 0x003b6c14, PAL: 0x00435ad4
int g_nDmeHostChangeArgument;
// NTSC-U/C: 0x003b6c0c, PAL: 0x00435acc
int g_nDmeObjectRangeArgument;
// NTSC-U/C: 0x003b6c18, PAL: 0x00435ad8
rtDmeHostChangeCallback g_pfnDmeObjectHost;
// NTSC-U/C: 0x003b6c1c, PAL: 0x00435adc
int g_nDmeObjectHostArgument;
// NTSC-U/C: 0x003b6c20, PAL: 0x00435ae0
unsigned char g_bDmeServiceStarted;
// The time of this console at the last update of the engine.
// NTSC-U/C: 0x003b6c24, PAL: 0x00435ae4
static unsigned int g_uDmeLastUpdateTime;
// NTSC-U/C: 0x003b6c28, PAL: 0x00435ae8
unsigned char g_bDmeReceiveError;
// NTSC-U/C: 0x003b6c29, PAL: 0x00435ae9
unsigned char g_bDmeConnectWaiting;
// NTSC-U/C: 0x003b6c2a, PAL: 0x00435aea
unsigned char g_bDmeConnectReported;
// The sizes of the queues of the datagram service. Yes, the binary only reads them.
// NTSC-U/C: 0x003b6c2c, PAL: 0x00435aec
static unsigned int g_uDmeServiceSendSize = kServiceQueueSize;
// NTSC-U/C: 0x003b6c30, PAL: 0x00435af0
static unsigned int g_uDmeServiceReceiveSize = kServiceQueueSize;
// NTSC-U/C: 0x003b6c34, PAL: 0x00435af4
unsigned char g_bDmeListening;
// NTSC-U/C: 0x003b6c38, PAL: 0x00435af8
RtDmeStream *g_pDmeStreams;
// Written at start-up and never read.
// NTSC-U/C: 0x00435720
static int g_anDmeUnreadWords[4];
// NTSC-U/C: 0x00435730
unsigned char g_auDmeMessageTypeCount[RT_DME_MESSAGE_CLASS_COUNT];
// NTSC-U/C: 0x00435738
int g_nDmeVoicePeerMessage;
// NTSC-U/C: 0x0043573c
int g_nDmeObjectRangeMessage;
// NTSC-U/C: 0x00435740
unsigned int g_uDmeSessionTime;
// NTSC-U/C: 0x00435744
RtDmeObjectRangeFunc g_pfnDmeObjectRange;
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
// NTSC-U/C: 0x00435760
RtDmeSessionReadyFunc g_pfnDmeSessionReady;
// NTSC-U/C: 0x00435788
int g_nDmeClientLeftMessage;
// NTSC-U/C: 0x0043578c
int g_nDmePingMessage;
// NTSC-U/C: 0x00435790
int g_nDmeObjectMessage;
// NTSC-U/C: 0x004357a0
int g_nDmeObjectUpdateMessage;
// NTSC-U/C: 0x004357a4
int g_nDmeStreamClosedMessage;
// NTSC-U/C: 0x004357a8
int g_nDmeStreamDataMessage;
// NTSC-U/C: 0x004357ac
int g_nDmeAddressProbeMessage;
// NTSC-U/C: 0x004357b0
int g_nDmeDisconnectMessage;
// NTSC-U/C: 0x00519f28
static RtDmeMessage g_dmeDatagram;
// NTSC-U/C: 0x0051a128
static RtDmeMessage g_dmeChannelMessage;
// NTSC-U/C: 0x0051a328
static rtDmeMessage g_dmeReceived;
// NTSC-U/C: 0x0051ab28
static RtDmeObjectUpdateMessage g_dmeObjectUpdate;
// NTSC-U/C: 0x0051ad28
static RtDmeObjectMessage g_dmeObjectMessage;
// NTSC-U/C: 0x0051af28
static RtDmeStreamMessage g_dmeStreamMessage;
// NTSC-U/C: 0x0051b128
RtDmeConnection *g_apDmeConnections[RT_DME_CONNECTION_COUNT];
// NTSC-U/C: 0x0051b138
RtDmeClientStats *g_apDmeClientStats[RT_DME_CLIENT_COUNT];
// NTSC-U/C: 0x0051b538
rtNetAddress g_dmeGatewayAddress;
// NTSC-U/C: 0x0051b550
rtDmeAddressResult g_dmeMapping;
// NTSC-U/C: 0x0051b588
rtDmeAddressCallback g_pfnDmeMapping;
// When the unresponsive clients of connection 0 were last probed.
// NTSC-U/C: 0x0051b590
static RtTime g_dmeLastPeerProbe;
// NTSC-U/C: 0x0051b598
static RtDmePumpResults g_dmePumpResults;
// NTSC-U/C: 0x0051b5b0
RtDmePoll g_aDmePolls[RT_DME_POLL_COUNT];
// NTSC-U/C: 0x0053cda0
RtDmeObjectType g_aDmeObjectTypes[RT_DME_OBJECT_TYPE_COUNT];
// NTSC-U/C: 0x0053f020
int g_anDmeStreamRelay[RT_DME_STREAM_KIND_COUNT];
// NTSC-U/C: 0x0053f0e0
RtDmeObjectFilterFunc g_apfnDmeObjectFilter[RT_DME_OBJECT_TYPE_COUNT];
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

// The receiver of the peers of the channel of a connection.
// NTSC-U/C: 0x002b3448, PAL: 0x002bd3b8
static void RtDmeOnChannelPeer(unsigned short uChannel,
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

// The receiver of the messages of the channel of a connection.
// NTSC-U/C: 0x002b3a18, PAL: 0x002bd988
static void RtDmeOnChannelData(unsigned short uChannel,
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

int RtDmeSetHostChangeHook(rtDmeHostChangeCallback pfnHook, int nArgument) {
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
            g_pDmeStreams[i].uState = RT_DME_STREAM_FREE;
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
        if (!(nClientFlags & RT_DME_CLIENT_FLAG_DATAGRAMS) && bForce != 1) {
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

int rtDmeSendClassMessage(signed char nFlags,
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
        if (rtDmeSendClassMessage(nFlags, nConnection, nClient, nClass, nType, nSize, pData) != 0) {
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
    return rtDmeSendClassMessage(
        nFlags, nConnection, nClient, RT_DME_CLASS_APPLICATION, nType, nSize, pData);
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
    return rtDmeSendClassMessage(
        nFlags, nConnection, nClient, RT_DME_CLASS_SYSTEM, nType, nSize, pData);
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
    rtDmeHostChange change;
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
                        change.connection = 0;
                        change.host = nCandidate;
                        change.argument = g_nDmeHostChangeArgument;
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
        g_aDmePings[i].done.pfnDone = NULL;
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
        pClient->anState[1] = 0;
        pClient->anState[0] = 0;
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

// NTSC-U/C: 0x002b8430, PAL: 0x002c23a0
static int RtDmeCreateConnection(unsigned int nConnection, RtDmeConnection **ppConnection) {
    if (ppConnection == NULL) {
        return RT_DME_RESULT_INVALID;
    }
    *ppConnection = NULL;
    if (nConnection >= RT_DME_CONNECTION_COUNT) {
        return RT_DME_RESULT_INVALID;
    }
    if (g_apDmeConnections[nConnection] == NULL) {
        g_apDmeConnections[nConnection] = RtDmeAlloc(sizeof(RtDmeConnection));
        if (g_apDmeConnections[nConnection] == NULL) {
            return RT_DME_RESULT_NO_MEMORY;
        }
    }
    *ppConnection = g_apDmeConnections[nConnection];
    return 0;
}

// NTSC-U/C: 0x002b84c0, PAL: 0x002c2430
static int RtDmeFreeConnection(unsigned int nConnection) {
    if (nConnection >= RT_DME_CONNECTION_COUNT) {
        return RT_DME_RESULT_INVALID;
    }
    if (g_apDmeConnections[nConnection] != NULL) {
        RtDmeFree((void **)&g_apDmeConnections[nConnection]);
    }
    g_apDmeConnections[nConnection] = NULL;
    return 0;
}

// NTSC-U/C: 0x002b8658, PAL: 0x002c25c8
static int RtDmeFreeClient(unsigned int nConnection, unsigned int nClient) {
    RtDmeConnection *pConnection;
    int nResult;

    pConnection = NULL;
    if (nConnection >= RT_DME_CONNECTION_COUNT || nClient >= RT_DME_CLIENT_COUNT) {
        return RT_DME_RESULT_INVALID;
    }
    nResult = RtDmeGetConnection(nConnection, &pConnection);
    if (nResult != 0) {
        return nResult;
    }
    if (pConnection != NULL) {
        if (pConnection->apClients[nClient] != NULL) {
            RtDmeFree((void **)&pConnection->apClients[nClient]);
        }
        pConnection->apClients[nClient] = NULL;
    }
    return 0;
}

// NTSC-U/C: 0x002b8398, PAL: 0x002c2308
static int RtDmeFreeAllClientStats(void) {
    unsigned int i;

    for (i = 0; i < RT_DME_CLIENT_COUNT; ++i) {
        if (g_apDmeClientStats[i] != NULL) {
            RtDmeFree((void **)&g_apDmeClientStats[i]);
            g_apDmeClientStats[i] = NULL;
        }
    }
    return 0;
}

// NTSC-U/C: 0x002b8720, PAL: 0x002c2690
static int RtDmeSetServerName(unsigned int nConnection, const char *pszName) {
    RtDmeConnection *pConnection;
    int nResult;

    pConnection = NULL;
    if (nConnection >= RT_DME_CONNECTION_COUNT) {
        return RT_DME_RESULT_OUT_OF_RANGE;
    }
    if (pszName == NULL) {
        return RT_DME_RESULT_INVALID;
    }
    nResult = RtDmeGetConnection(nConnection, &pConnection);
    if (nResult != 0) {
        return nResult;
    }
    if (pConnection == NULL) {
        return nResult;
    }
    strncpy(pConnection->szServerName, pszName, RT_DME_ADDRESS_SIZE);
    return 0;
}

// NTSC-U/C: 0x002b5720, PAL: 0x002bf690
static int RtDmeFindFreePing(void) {
    int i;

    for (i = 0; i < RT_DME_PING_COUNT; ++i) {
        if (g_aDmePings[i].done.pfnDone == NULL) {
            return i;
        }
    }
    return RT_DME_PING_COUNT;
}

// NTSC-U/C: 0x002b9a10, PAL: 0x002c3980
static int RtDmeFreeStreams(void) {
    RtDmeStream *pStream;
    RtRingBuffer *pQueue;
    unsigned int i;

    if (g_pDmeStreams == NULL) {
        return 0;
    }
    for (i = 0; i < RT_DME_STREAM_COUNT; ++i) {
        pStream = &g_pDmeStreams[i];
        if (pStream->pbData != NULL) {
            RtDmeFree((void **)&pStream->pbData);
        }
        pQueue = &g_pDmeStreams[i].queue;
        // Yes, the binary tests the address of the queue against null.
        if (pQueue != NULL) {
            RtDestroyRingBuffer(pQueue);
        }
    }
    return 0;
}

// NTSC-U/C: 0x002b9ab8, PAL: 0x002c3a28
static int RtDmeCloseFailedConnections(void) {
    RtDmeConnection *pConnection;
    unsigned int i;

    pConnection = NULL;
    for (i = 0; i < RT_DME_CONNECTION_COUNT; ++i) {
        if (RtDmeGetConnection(i, &pConnection) != 0 || pConnection == NULL) {
            continue;
        }
        if (pConnection->nState == RT_DME_CONNECTION_FAILED) {
            RtDmeDisconnect(i);
        }
    }
    return 0;
}

// NTSC-U/C: 0x002b4dc0, PAL: 0x002bed30
static int RtDmeSetGatewayAddress(const rtNetAddress *pAddress) {
    if (pAddress == NULL) {
        return RT_DME_RESULT_INVALID;
    }
    g_dmeGatewayAddress = *pAddress;
    return 0;
}

// The receiver of the probes of the unresponsive clients, which only spoils the address text.
// NTSC-U/C: 0x002b8ce0, PAL: 0x002c2c50
static void RtDmeIgnorePing(int nResult, char *pszAddress, int nLatency, unsigned int uRemoteTime) {
    (void)nResult;
    (void)nLatency;
    (void)uRemoteTime;
    if (pszAddress != NULL) {
        *pszAddress = '\0';
    }
}

int RtDmeConvertResult(int nResult, int nDefault) {
    // The results of the engine for the results of the channels, -1 where the default applies.
    static const signed char kChannelResults[] = {
        0,  -1, 23, 33, -1, 18, 35, -1, 22, 10, -1, 15, 3,  14, 13,
        13, 34, 26, 37, 13, 36, 36, 37, 16, -1, 2,  -1, 32, 32,
    };

    if ((unsigned int)nResult >= sizeof(kChannelResults) || kChannelResults[nResult] < 0) {
        return nDefault;
    }
    return kChannelResults[nResult];
}

int RtDmeGetAddressIndex(int nConnection, int nClient) {
    RtDmeClient *pClient;

    pClient = NULL;
    if ((unsigned int)nConnection >= RT_DME_CONNECTION_COUNT) {
        return RT_DME_ADDRESS_INDEX_NONE;
    }
    if (RtDmeIsClientIndex(nClient) != 1) {
        return RT_DME_ADDRESS_INDEX_NONE;
    }
    if (RtDmeGetClient(nConnection, nClient, &pClient) != 0 || pClient == NULL) {
        return RT_DME_ADDRESS_INDEX_NONE;
    }
    return pClient->uAddressIndex;
}

void RtDmeSetAddressIndex(int nConnection, int nClient, unsigned char uIndex) {
    RtDmeClient *pClient;

    pClient = NULL;
    if ((unsigned int)nConnection >= RT_DME_CONNECTION_COUNT) {
        return;
    }
    if (RtDmeIsClientIndex(nClient) != 1 || uIndex >= RT_DME_ADDRESS_COUNT) {
        return;
    }
    if (RtDmeGetClient(nConnection, nClient, &pClient) != 0 || pClient == NULL) {
        return;
    }
    pClient->uAddressIndex = uIndex;
}

char *RtDmeGetClientAddress(int nConnection, int nClient, unsigned char uIndex) {
    RtDmeClient *pClient;

    pClient = NULL;
    if ((unsigned int)nConnection >= RT_DME_CONNECTION_COUNT) {
        return NULL;
    }
    if (RtDmeIsClientIndex(nClient) != 1 || uIndex >= RT_DME_ADDRESS_COUNT) {
        return NULL;
    }
    if (RtDmeGetClient(nConnection, nClient, &pClient) != 0 || pClient == NULL) {
        return NULL;
    }
    return pClient->addresses[uIndex].address;
}

void RtDmeSetClientAddress(int nConnection,
                           int nClient,
                           unsigned char uIndex,
                           const char *pszAddress) {
    RtDmeClient *pClient;

    pClient = NULL;
    if ((unsigned int)nConnection >= RT_DME_CONNECTION_COUNT) {
        return;
    }
    if (RtDmeIsClientIndex(nClient) != 1 || uIndex >= RT_DME_ADDRESS_COUNT || pszAddress == NULL) {
        return;
    }
    if (RtDmeGetClient(nConnection, nClient, &pClient) != 0 || pClient == NULL) {
        return;
    }
    strncpy(pClient->addresses[uIndex].address, pszAddress, RT_DME_ADDRESS_SIZE);
}

unsigned char RtDmeGetClientAddressKind(int nConnection, int nClient, unsigned char uIndex) {
    RtDmeClient *pClient;

    pClient = NULL;
    if ((unsigned int)nConnection >= RT_DME_CONNECTION_COUNT) {
        return 0;
    }
    if (RtDmeIsClientIndex(nClient) != 1 || uIndex >= RT_DME_ADDRESS_COUNT) {
        return 0;
    }
    if (RtDmeGetClient(nConnection, nClient, &pClient) != 0 || pClient == NULL) {
        return 0;
    }
    return pClient->addresses[uIndex].type;
}

void RtDmeSetClientAddressKind(int nConnection,
                               int nClient,
                               unsigned char uIndex,
                               unsigned char uKind) {
    RtDmeClient *pClient;

    pClient = NULL;
    if ((unsigned int)nConnection >= RT_DME_CONNECTION_COUNT) {
        return;
    }
    if (RtDmeIsClientIndex(nClient) != 1 || uIndex >= RT_DME_ADDRESS_COUNT) {
        return;
    }
    if (RtDmeGetClient(nConnection, nClient, &pClient) != 0 || pClient == NULL) {
        return;
    }
    pClient->addresses[uIndex].type = uKind;
}

void RtDmeSetClientPort(int nConnection, int nClient, unsigned char uIndex, unsigned short uPort) {
    RtDmeClient *pClient;

    pClient = NULL;
    if ((unsigned int)nConnection >= RT_DME_CONNECTION_COUNT) {
        return;
    }
    if (RtDmeIsClientIndex(nClient) != 1 || uIndex >= RT_DME_ADDRESS_COUNT) {
        return;
    }
    if (RtDmeGetClient(nConnection, nClient, &pClient) != 0 || pClient == NULL) {
        return;
    }
    pClient->addresses[uIndex].port = uPort;
}

unsigned short RtDmeGetClientPort(int nConnection, int nClient, unsigned char uIndex) {
    RtDmeClient *pClient;

    pClient = NULL;
    if ((unsigned int)nConnection >= RT_DME_CONNECTION_COUNT) {
        return 0;
    }
    if (RtDmeIsClientIndex(nClient) != 1 || uIndex >= RT_DME_ADDRESS_COUNT) {
        return 0;
    }
    if (RtDmeGetClient(nConnection, nClient, &pClient) != 0 || pClient == NULL) {
        return 0;
    }
    return pClient->addresses[uIndex].port;
}

int RtDmeGetClientStats(unsigned int uClient, RtDmeClientStats **ppStats) {
    if (ppStats == NULL) {
        return RT_DME_RESULT_INVALID;
    }
    *ppStats = NULL;
    if (uClient >= RT_DME_CLIENT_COUNT) {
        return RT_DME_RESULT_INVALID;
    }
    if (g_apDmeClientStats[uClient] == NULL) {
        g_apDmeClientStats[uClient] = RtDmeAlloc(sizeof(RtDmeClientStats));
        if (g_apDmeClientStats[uClient] == NULL) {
            return RT_DME_RESULT_NO_MEMORY;
        }
    }
    *ppStats = g_apDmeClientStats[uClient];
    return 0;
}

int RtDmeGetConnection(unsigned int nConnection, RtDmeConnection **ppConnection) {
    if (ppConnection == NULL) {
        return RT_DME_RESULT_INVALID;
    }
    *ppConnection = NULL;
    if (nConnection >= RT_DME_CONNECTION_COUNT) {
        return RT_DME_RESULT_BAD_CONNECTION;
    }
    *ppConnection = g_apDmeConnections[nConnection];
    return 0;
}

int RtDmeGetClient(unsigned int nConnection, unsigned int nClient, RtDmeClient **ppClient) {
    RtDmeConnection *pConnection;
    int nResult;

    pConnection = NULL;
    if (ppClient == NULL) {
        return RT_DME_RESULT_INVALID;
    }
    if (nConnection >= RT_DME_CONNECTION_COUNT) {
        return RT_DME_RESULT_INVALID;
    }
    *ppClient = NULL;
    if (nClient >= RT_DME_CLIENT_COUNT) {
        return RT_DME_RESULT_INVALID;
    }
    nResult = RtDmeGetConnection(nConnection, &pConnection);
    if (nResult != 0) {
        return nResult;
    }
    if (pConnection == NULL) {
        return RT_DME_RESULT_NO_RECORD;
    }
    *ppClient = pConnection->apClients[nClient];
    return 0;
}

int RtDmeCreateClient(unsigned int nConnection, unsigned int nClient, RtDmeClient **ppClient) {
    RtDmeConnection *pConnection;
    int nResult;

    pConnection = NULL;
    if (ppClient == NULL) {
        return RT_DME_RESULT_INVALID;
    }
    if (nConnection >= RT_DME_CONNECTION_COUNT) {
        return RT_DME_RESULT_INVALID;
    }
    *ppClient = NULL;
    if (nClient >= RT_DME_CLIENT_COUNT) {
        return RT_DME_RESULT_INVALID;
    }
    nResult = RtDmeGetConnection(nConnection, &pConnection);
    if (nResult != 0) {
        return nResult;
    }
    if (pConnection == NULL) {
        return RT_DME_RESULT_NO_RECORD;
    }
    if (pConnection->apClients[nClient] == NULL) {
        pConnection->apClients[nClient] = RtDmeAlloc(sizeof(RtDmeClient));
        if (pConnection->apClients[nClient] == NULL) {
            return RT_DME_RESULT_NO_MEMORY;
        }
    }
    *ppClient = pConnection->apClients[nClient];
    return 0;
}

char *RtDmeGetServerName(unsigned int nConnection) {
    RtDmeConnection *pConnection;

    pConnection = NULL;
    if (nConnection >= RT_DME_CONNECTION_COUNT) {
        return NULL;
    }
    if (RtDmeGetConnection(nConnection, &pConnection) != 0 || pConnection == NULL) {
        return NULL;
    }
    return pConnection->szServerName;
}

int RtDmeGetClientFlags(unsigned int nConnection, int nClient, signed char *pnFlags) {
    RtDmeClient *pClient;
    int nResult;

    pClient = NULL;
    if (pnFlags == NULL) {
        return RT_DME_RESULT_INVALID;
    }
    *pnFlags = 0;
    if (nConnection >= RT_DME_CONNECTION_COUNT || RtDmeIsClientIndex(nClient) != 1) {
        return RT_DME_RESULT_OUT_OF_RANGE;
    }
    nResult = RtDmeGetClient(nConnection, nClient, &pClient);
    if (nResult != 0 || pClient == NULL) {
        return nResult;
    }
    *pnFlags = pClient->nFlags;
    return nResult;
}

int RtDmeSetClientFlags(unsigned int nConnection, int nClient, signed char nFlags) {
    RtDmeClient *pClient;
    int nResult;

    pClient = NULL;
    if (nConnection >= RT_DME_CONNECTION_COUNT || RtDmeIsClientIndex(nClient) != 1) {
        return RT_DME_RESULT_OUT_OF_RANGE;
    }
    nResult = RtDmeGetClient(nConnection, nClient, &pClient);
    if (nResult == 0 && pClient != NULL) {
        pClient->nFlags = nFlags;
    }
    return nResult;
}

int RtDmeSetHostClient(unsigned int nConnection, int nClient) {
    RtDmeConnection *pConnection;
    int nResult;

    pConnection = NULL;
    if (nConnection >= RT_DME_CONNECTION_COUNT) {
        return RT_DME_RESULT_OUT_OF_RANGE;
    }
    nResult = RtDmeGetConnection(nConnection, &pConnection);
    if (nResult == 0 && pConnection != NULL) {
        pConnection->nHostClient = nClient;
    }
    return nResult;
}

int RtDmeIsClientIndex(unsigned int nClient) {
    return nClient < RT_DME_CLIENT_COUNT;
}

unsigned char RtDmeClientToByte(int nClient) {
    return nClient;
}

int RtDmeIsConnectionIndex(unsigned int nConnection) {
    return nConnection < RT_DME_CONNECTION_COUNT;
}

int RtDmeGetGatewayAddress(rtNetAddress **ppAddress) {
    if (ppAddress == NULL) {
        return RT_DME_RESULT_INVALID;
    }
    *ppAddress = &g_dmeGatewayAddress;
    return 0;
}

int RtDmeIsLocalHost(unsigned char *pbHost) {
    RtDmeConnection *pConnection;
    int nResult;

    pConnection = NULL;
    if (pbHost == NULL) {
        return RT_DME_RESULT_INVALID;
    }
    *pbHost = 0;
    nResult = RtDmeGetConnection(0, &pConnection);
    if (nResult != 0 || pConnection == NULL) {
        return nResult;
    }
    if (pConnection->nState != RT_DME_CONNECTION_CONNECTED &&
        pConnection->nState != RT_DME_CONNECTION_CONNECTING) {
        return 0;
    }
    if (pConnection->nLocalClient == RT_DME_NO_CLIENT) {
        return 0;
    }
    if (g_nDmeHostClient == pConnection->nLocalClient) {
        *pbHost = 1;
    }
    return 0;
}

// The receiver of the result of the channel of a connection.
// NTSC-U/C: 0x002b3d50, PAL: 0x002bdcc0
static void RtDmeOnChannelConnect(unsigned short uChannel,
                                  int nConnected,
                                  unsigned short uClient,
                                  unsigned short uClientCount,
                                  int nReason) {
    RtDmeConnection *pConnection;
    RtDmeClient *pClient;
    int nConnection;
    int nStatus;
    int nResult;
    int nState;
    int bValid;

    pConnection = NULL;
    pClient = NULL;
    nStatus = 0;
    nConnection = RtDmeChannelToConnection(uChannel);
    if ((unsigned int)nConnection >= RT_DME_CONNECTION_COUNT) {
        return;
    }
    if (RtDmeIsClientIndex(uClient) != 1) {
        return;
    }
    if (RtDmeGetConnection(nConnection, &pConnection) != 0 || pConnection == NULL) {
        return;
    }
    if (RtDmeCreateClient(nConnection, uClient, &pClient) != 0 || pClient == NULL) {
        return;
    }
    bValid = nConnection < RT_DME_CONNECTION_COUNT;
    if ((unsigned char)nConnected == 1) {
        if (bValid && uClient < pConnection->nMaxClients) {
            if (RtDmeVoiceIsOpen(nConnection) == 1 && RtDmeVoiceStart(nConnection, uClient) != 0) {
                return;
            }
            // Yes, the binary checks the connection again.
            if (pConnection == NULL) {
                return;
            }
            pConnection->nLocalClient = uClient;
            pConnection->nPendingJoins = uClientCount - 1;
            pConnection->nState = RT_DME_CONNECTION_CONNECTED;
            pClient->anState[1] = 0;
            pClient->nRole = RT_DME_ROLE_LOCAL;
            pClient->anState[0] = 0;
            nResult = 0;
            nStatus = 0;
            if (strncmp(g_dmeLocalAddresses[RT_DME_ADDRESS_INDEX_PUBLIC].address,
                        g_dmeLocalAddresses[RT_DME_ADDRESS_INDEX_PRIVATE].address,
                        RT_DME_ADDRESS_SIZE) == 0) {
                RtChannelApiGetServerName(uChannel,
                                          g_dmeLocalAddresses[RT_DME_ADDRESS_INDEX_PUBLIC].address);
            }
            memcpy(pClient->addresses, g_dmeLocalAddresses, sizeof(pClient->addresses));
            pClient->bJoined = 1;
        } else {
            if (RtDmeVoiceIsOpen(nConnection) == 1) {
                RtDmeVoiceClose(nConnection);
            }
            if (bValid && pConnection != NULL) {
                pConnection->nState = RT_DME_CONNECTION_FAILED;
                pConnection->nLocalClient = RT_DME_NO_CLIENT;
            }
            nStatus = 1;
            nResult =
                (uClient < pConnection->nMaxClients) ? RT_DME_RESULT_FAILED : RT_DME_RESULT_FULL;
        }
    } else {
        if (RtDmeVoiceIsOpen(nConnection) == 1) {
            RtDmeVoiceClose(nConnection);
        }
        bValid = nConnection < RT_DME_CONNECTION_COUNT;
        if (bValid && pConnection != NULL) {
            pConnection->nState = RT_DME_CONNECTION_FAILED;
        }
        nResult = RT_DME_RESULT_FAILED;
        if (RtDmeConvertStatus(&nStatus, nReason) != 0) {
            nStatus = 1;
        }
    }
    if (g_pfnDmeConnect != NULL) {
        nState = RT_DME_CONNECTION_FAILED;
        if (bValid && pConnection != NULL) {
            nState = pConnection->nState;
        }
        g_pfnDmeConnect(nConnection, nState, nStatus, nResult);
        g_pfnDmeConnect = NULL;
    }
}

// The receiver of the statuses the server of a connection sends.
// NTSC-U/C: 0x002b4038, PAL: 0x002bdfa8
static void RtDmeOnChannelStatus(unsigned short uChannel, int nStatus) {
    RtDmeConnection *pConnection;
    int nConnection;

    (void)nStatus;
    pConnection = NULL;
    nConnection = RtDmeChannelToConnection(uChannel);
    if ((unsigned int)nConnection >= RT_DME_CONNECTION_COUNT) {
        return;
    }
    if (RtDmeGetConnection(nConnection, &pConnection) != 0 || pConnection == NULL) {
        return;
    }
    if (g_pfnDmeStatus != NULL) {
        g_pfnDmeStatus(nConnection, pConnection->nState, 1, 0);
    }
}

int RtDmeInitConnectParams(rtDmeConnectParams *pParams) {
    int nResult;

    if (pParams == NULL) {
        return RT_DME_RESULT_INVALID;
    }
    memset(pParams, 0, kConnectParamsSize);
    pParams->maxClients = RT_DME_CLIENT_COUNT;
    pParams->info.type = 0;
    pParams->onConnect = NULL;
    pParams->onStatus = NULL;
    pParams->onClientJoin = NULL;
    pParams->onClientLeave = NULL;
    pParams->onHostChange = NULL;
    pParams->port = g_uDmePort;
    pParams->hostChangeArgument = 0;
    nResult = RtDmeVoiceInitSettings(&pParams->voice);
    if (nResult != 0) {
        return nResult;
    }
    pParams->version = RT_NET_SETTINGS_VERSION;
    return nResult;
}

// Close the channel of a connection that failed to open, and report the failure.
static inline void RtDmeAbandonChannel(unsigned short uChannel) {
    RtChannelApiClose(uChannel);
    if (g_pfnDmeConnect != NULL) {
        g_pfnDmeConnect(-1, RT_DME_CONNECTION_FAILED, 1, RT_DME_RESULT_FAILED);
    }
}

// Connect to a session over a channel.
// NTSC-U/C: 0x002b4220, PAL: 0x002be190
static int RtDmeConnectChannel(const rtDmeConnectParams *pParams,
                               const rtNetConnectionInfo *pInfo,
                               rtDmeConnectStatus *pStatus) {
    RtChannelConfig config;
    char abRequest[kRequestBufferSize];
    RtDmeConnection *pConnection;
    RtTime start;
    void *pRequest;
    unsigned int uElapsed;
    unsigned short uRequestSize;
    unsigned short uChannel;
    int nConnection;
    int nResult;

    pRequest = NULL;
    uRequestSize = 0;
    uElapsed = 0;
    pConnection = NULL;
    if (pStatus == NULL) {
        return RT_DME_RESULT_INVALID;
    }
    pStatus->connection = -1;
    pStatus->result = RT_DME_RESULT_FAILED;
    pStatus->version = RT_NET_SETTINGS_VERSION;
    if (pParams == NULL || pInfo == NULL) {
        pStatus->result = RT_DME_RESULT_INVALID;
        return RT_DME_RESULT_INVALID;
    }
    if (pParams->version != RT_NET_SETTINGS_VERSION) {
        pStatus->result = RT_DME_RESULT_BAD_VERSION;
        return RT_DME_RESULT_BAD_VERSION;
    }
    if (pParams->maxClients <= 0 ||
        (pInfo->type != kConnectChannel && pInfo->type != kConnectChannelDatagram)) {
        pStatus->result = RT_DME_RESULT_INVALID;
        return RT_DME_RESULT_INVALID;
    }
    if (pInfo->addresses[RT_DME_ADDRESS_INDEX_PRIVATE].type == kGatewayAddress) {
        RtDmeSetGatewayAddress(&pInfo->addresses[RT_DME_ADDRESS_INDEX_PRIVATE]);
    }
    g_pfnDmeConnect = pParams->onConnect;
    g_pfnDmeStatus = pParams->onStatus;
    if (strlen(pInfo->sessionKey) != 0 && strlen(pInfo->accessKey) != 0) {
        memset(abRequest, 0, sizeof(abRequest));
        strncpy(abRequest, pInfo->sessionKey, kSessionKeyBytes);
        strncpy(&abRequest[kSessionKeyBytes], pInfo->accessKey, kSessionKeyBytes);
        uRequestSize = kSessionKeyBytes * 2;
        pRequest = abRequest;
    }
    nResult = RtDmeConvertResult(RtChannelApiDefaultConfig(&config), RT_DME_RESULT_SERVICE);
    if (nResult != 0) {
        return nResult;
    }
    if (pInfo->type == kConnectChannel) {
        config.nMode = kChannelModeStream;
    } else if (pInfo->type == kConnectChannelDatagram) {
        config.nMode = kChannelModeDatagram;
        config.uLocalPort = pParams->port;
    } else {
        pStatus->result = RT_DME_RESULT_INVALID;
        return RT_DME_RESULT_INVALID;
    }
    strcpy(config.szAddress, pInfo->addresses[RT_DME_ADDRESS_INDEX_PUBLIC].address);
    config.uPort = pInfo->addresses[RT_DME_ADDRESS_INDEX_PUBLIC].port;
    config.uClientValue = pInfo->worldId;
    config.pServerKey = (RtKey *)pInfo->serverKey;
    config.uKeepaliveInterval = kChannelKeepalive;
    config.uSendSize = g_uDmeServiceSendSize;
    config.uReceiveSize = g_uDmeServiceReceiveSize;
    config.pRequestData = pRequest;
    config.uRequestSize = uRequestSize;
    config.pfnConnect = RtDmeOnChannelConnect;
    config.pfnStatus = RtDmeOnChannelStatus;
    config.pfnPeer = RtDmeOnChannelPeer;
    config.pfnData = RtDmeOnChannelData;
    nResult = RtChannelApiOpen(&uChannel, &config);
    if (nResult != 0) {
        RtDmeAbandonChannel(uChannel);
        pStatus->result = RtDmeConvertResult(nResult, RT_DME_RESULT_FAILED);
        return pStatus->result;
    }
    nConnection = RtDmeChannelToConnection(uChannel);
    pStatus->connection = nConnection;
    pStatus->result = RtDmeCreateConnection(nConnection, &pConnection);
    if (pStatus->result == 0 && pParams->voice.enabled == 1) {
        pStatus->result = RtDmeVoiceOpen(nConnection, pParams->maxClients, &pParams->voice);
    }
    if (pStatus->result != 0 || pConnection == NULL) {
        RtDmeAbandonChannel(uChannel);
        return pStatus->result;
    }
    RtDmeSetHostClient(nConnection, RT_DME_NO_CLIENT);
    pConnection->pfnClientJoin = pParams->onClientJoin;
    pConnection->pfnClientLeave = pParams->onClientLeave;
    pConnection->nState = RT_DME_CONNECTION_CONNECTING;
    pConnection->nWorldId = pInfo->worldId;
    pConnection->nMaxClients = pParams->maxClients;
    pStatus->result =
        RtDmeSetServerName(nConnection, pInfo->addresses[RT_DME_ADDRESS_INDEX_PUBLIC].address);
    if (pStatus->result != 0) {
        pConnection->nState = RT_DME_CONNECTION_FAILED;
        return pStatus->result;
    }
    if (g_pfnDmeConnect != NULL) {
        pStatus->result = 0;
        return 0;
    }
    RtReadTime(&start);
    while (pConnection->nState == RT_DME_CONNECTION_CONNECTING) {
        nResult = RtChannelApiPumpOne(uChannel, 1);
        if (nResult != 0) {
            pConnection->nState = RT_DME_CONNECTION_FAILED;
            pStatus->result = RtDmeConvertResult(nResult, RT_DME_RESULT_FAILED);
            return pStatus->result;
        }
        RtGetMillisecondsSince(&start, &uElapsed);
        if (uElapsed > kConnectTimeout) {
            pStatus->result = RT_DME_RESULT_FAILED;
            pConnection->nState = RT_DME_CONNECTION_FAILED;
            return RT_DME_RESULT_FAILED;
        }
    }
    if (pConnection->nState != RT_DME_CONNECTION_CONNECTED) {
        pStatus->result = RT_DME_RESULT_FAILED;
        pConnection->nState = RT_DME_CONNECTION_FAILED;
        return RT_DME_RESULT_FAILED;
    }
    pStatus->result = 0;
    return 0;
}

// Start the voice of connection 0 and the datagram service, or report the failure.
static inline int RtDmeStartPeers(RtDmeConnection *pConnection,
                                  int nResult,
                                  int nMaxClients,
                                  const rtDmeVoiceSettings *pVoice) {
    if (nResult == 0 && !g_bDmeServiceStarted) {
        nResult = RtDmeStartDatagramService(g_uDmeServiceSendSize, g_uDmeServiceReceiveSize);
    }
    if (nResult == 0 && pVoice->enabled == 1) {
        nResult = RtDmeVoiceOpen(0, nMaxClients, pVoice);
    }
    if (nResult != 0) {
        RtDmeVoiceClose(0);
        pConnection->nState = RT_DME_CONNECTION_FAILED;
        if (g_pfnDmeConnect != NULL) {
            g_pfnDmeConnect(0, RT_DME_CONNECTION_FAILED, 1, nResult);
        }
    }
    return nResult;
}

// Join a session of peers on connection 0 through the addresses of its host.
// NTSC-U/C: 0x002b4688, PAL: 0x002be5f8
static int RtDmeConnectPeers(const rtDmeConnectParams *pParams,
                             const rtNetConnectionInfo *pInfo,
                             rtDmeConnectStatus *pStatus) {
    RtDmeConnection *pConnection;
    rtDmeAddressReport report;
    rtDmeConnectStatus peerStatus;
    unsigned int uNow;
    unsigned int uDeadline;
    int nResult;

    pConnection = NULL;
    if (pStatus == NULL) {
        return RT_DME_RESULT_INVALID;
    }
    pStatus->result = RT_DME_RESULT_FAILED;
    pStatus->version = RT_NET_SETTINGS_VERSION;
    if (pParams == NULL || pInfo == NULL) {
        pStatus->result = RT_DME_RESULT_INVALID;
        return RT_DME_RESULT_INVALID;
    }
    if (pParams->version != RT_NET_SETTINGS_VERSION) {
        pStatus->result = RT_DME_RESULT_BAD_VERSION;
        return RT_DME_RESULT_BAD_VERSION;
    }
    if (pParams->maxClients <= 0 || pInfo->type != kConnectDatagram) {
        pStatus->result = RT_DME_RESULT_INVALID;
        return RT_DME_RESULT_INVALID;
    }
    pStatus->result = RtDmeCreateConnection(0, &pConnection);
    if (pStatus->result != 0) {
        return pStatus->result;
    }
    if (pConnection == NULL) {
        return 0;
    }
    RtDmeSetHostClient(0, RT_DME_NO_CLIENT);
    // Yes, the binary marks the session of the peers with the version of the parameters.
    pConnection->nWorldId = RT_NET_SETTINGS_VERSION;
    pConnection->nMaxClients = pParams->maxClients;
    g_nDmeHostClient = 0;
    pConnection->nLocalClient = RT_DME_NO_CLIENT;
    g_pfnDmeConnect = pParams->onConnect;
    g_pfnDmeStatus = pParams->onStatus;
    pStatus->connection = 0;
    pConnection->pfnClientJoin = pParams->onClientJoin;
    pConnection->pfnClientLeave = pParams->onClientLeave;
    nResult = RtDmeSetHostChangeHook(pParams->onHostChange, pParams->hostChangeArgument);
    nResult = RtDmeStartPeers(pConnection, nResult, pParams->maxClients, &pParams->voice);
    if (nResult != 0) {
        pStatus->result = nResult;
        return nResult;
    }
    pConnection->nState = RT_DME_CONNECTION_CONNECTING;
    pStatus->result = RtDmeInitAddressReport(&report);
    if (pStatus->result != 0) {
        return pStatus->result;
    }
    memcpy(report.addresses, pInfo->addresses, sizeof(report.addresses));
    if (report.addresses[RT_DME_ADDRESS_INDEX_PRIVATE].type == 0) {
        report.addresses[RT_DME_ADDRESS_INDEX_PRIVATE] =
            report.addresses[RT_DME_ADDRESS_INDEX_PUBLIC];
        if (report.addresses[RT_DME_ADDRESS_INDEX_PUBLIC].type == RT_DME_ADDRESS_PUBLIC) {
            report.addresses[RT_DME_ADDRESS_INDEX_PRIVATE].type = RT_DME_ADDRESS_PRIVATE;
        } else if (report.addresses[RT_DME_ADDRESS_INDEX_PUBLIC].type == RT_DME_ADDRESS_PRIVATE) {
            report.addresses[RT_DME_ADDRESS_INDEX_PRIVATE].type = RT_DME_ADDRESS_PUBLIC;
        }
    }
    if (report.addresses[RT_DME_ADDRESS_INDEX_PRIVATE].port == 0) {
        report.addresses[RT_DME_ADDRESS_INDEX_PRIVATE].port = g_uDmePort;
    }
    if (report.addresses[RT_DME_ADDRESS_INDEX_PUBLIC].port == 0) {
        report.addresses[RT_DME_ADDRESS_INDEX_PUBLIC].port = g_uDmePort;
    }
    pStatus->result = RtDmeAddReportedPeer(&report, &peerStatus);
    if (pStatus->result != 0) {
        return pStatus->result;
    }
    if (peerStatus.version != RT_NET_SETTINGS_VERSION) {
        pStatus->result = RT_DME_RESULT_MAPPING;
        return RT_DME_RESULT_MAPPING;
    }
    RtDmeGetLocalTime(&uNow);
    if (pParams->onConnect == NULL) {
        uDeadline = uNow + kConnectTimeout;
        g_bDmeConnectWaiting = 1;
        while (pConnection->nState != RT_DME_CONNECTION_CONNECTED) {
            RtDmeGetLocalTime(&uNow);
            if (uDeadline < uNow) {
                pStatus->result = RT_DME_RESULT_TIMED_OUT;
                pConnection->nState = RT_DME_CONNECTION_IDLE;
                g_bDmeConnectWaiting = 0;
                return pStatus->result;
            }
            RtDmeUpdate();
        }
        g_bDmeConnectReported = 1;
        g_bDmeConnectWaiting = 0;
    }
    RtReadTime(&g_dmeLastPeerProbe);
    pStatus->result = 0;
    return 0;
}

int RtDmeConnect(const rtDmeConnectParams *pParams, rtDmeConnectStatus *pStatus) {
    int nResult;

    if (pStatus == NULL) {
        return RT_DME_RESULT_INVALID;
    }
    pStatus->result = RT_DME_RESULT_FAILED;
    pStatus->version = RT_NET_SETTINGS_VERSION;
    nResult = RtDmeCloseFailedConnections();
    if (nResult != 0) {
        return nResult;
    }
    if (pParams == NULL) {
        pStatus->result = RT_DME_RESULT_INVALID;
        return RT_DME_RESULT_INVALID;
    }
    if (pParams->version != RT_NET_SETTINGS_VERSION) {
        pStatus->result = RT_DME_RESULT_BAD_VERSION;
        return RT_DME_RESULT_BAD_VERSION;
    }
    switch (pParams->info.type) {
    case kConnectChannel:
    case kConnectChannelDatagram:
        pStatus->result = RtDmeConnectChannel(pParams, &pParams->info, pStatus);
        break;
    case kConnectDatagram:
        pStatus->result = RtDmeConnectPeers(pParams, &pParams->info, pStatus);
        break;
    default:
        pStatus->result = RT_DME_RESULT_INVALID;
        break;
    }
    return pStatus->result;
}

int RtDmeInitHostParams(rtDmeHostParams *pParams) {
    int nResult;

    if (pParams == NULL) {
        return RT_DME_RESULT_INVALID;
    }
    memset(pParams, 0, kHostParamsSize);
    pParams->onConnect = NULL;
    pParams->onClientJoin = NULL;
    pParams->maxClients = kDefaultHostClients;
    pParams->onClientLeave = NULL;
    nResult = RtDmeVoiceInitSettings(&pParams->voice);
    if (nResult != 0) {
        return nResult;
    }
    pParams->version = RT_NET_SETTINGS_VERSION;
    return nResult;
}

int RtDmeHost(const rtDmeHostParams *pParams, rtDmeConnectStatus *pStatus) {
    RtDmeConnection *pConnection;
    RtDmeClient *pClient;
    int nResult;

    pConnection = NULL;
    pClient = NULL;
    if (pStatus == NULL) {
        return RT_DME_RESULT_INVALID;
    }
    pStatus->result = RT_DME_RESULT_HOST_FAILED;
    pStatus->version = RT_NET_SETTINGS_VERSION;
    nResult = RtDmeCloseFailedConnections();
    if (nResult != 0) {
        return nResult;
    }
    if (pParams == NULL) {
        pStatus->result = RT_DME_RESULT_INVALID;
        return RT_DME_RESULT_INVALID;
    }
    if (pParams->version != RT_NET_SETTINGS_VERSION) {
        pStatus->result = RT_DME_RESULT_BAD_VERSION;
        return RT_DME_RESULT_BAD_VERSION;
    }
    if (pParams->maxClients == 0) {
        pStatus->result = RT_DME_RESULT_INVALID;
        return RT_DME_RESULT_INVALID;
    }
    pStatus->result = RtDmeCreateConnection(0, &pConnection);
    if (pStatus->result != 0 || pConnection == NULL) {
        return pStatus->result;
    }
    pStatus->result = RtDmeCreateClient(0, 0, &pClient);
    if (pStatus->result != 0 || pClient == NULL) {
        return pStatus->result;
    }
    RtDmeSetHostClient(0, RT_DME_NO_CLIENT);
    // Yes, the binary marks the session of the peers with the version of the parameters.
    pConnection->nWorldId = pParams->version;
    pConnection->nMaxClients = pParams->maxClients;
    g_pfnDmeConnect = pParams->onConnect;
    pStatus->connection = 0;
    pConnection->pfnClientJoin = pParams->onClientJoin;
    pConnection->pfnClientLeave = pParams->onClientLeave;
    pConnection->nState = RT_DME_CONNECTION_CONNECTING;
    g_nDmeHostClient = RT_DME_NO_CLIENT;
    pConnection->nLocalClient = RT_DME_NO_CLIENT;
    nResult = RtDmeStartPeers(pConnection, 0, pParams->maxClients, &pParams->voice);
    if (nResult != 0) {
        pStatus->result = nResult;
        return nResult;
    }
    if (g_dmeLocalAddresses[RT_DME_ADDRESS_INDEX_PRIVATE].type != 0 &&
        g_dmeLocalAddresses[RT_DME_ADDRESS_INDEX_PUBLIC].type != 0) {
        memcpy(pClient->addresses, g_dmeLocalAddresses, sizeof(pClient->addresses));
        pConnection->nLocalClient = 0;
        g_nDmeHostClient = pConnection->nLocalClient;
        RtDmeSetAddressIndex(0, pConnection->nLocalClient, RT_DME_ADDRESS_INDEX_PRIVATE);
        RtDmeSetConnectResult(0, pConnection->nLocalClient, 1, 0);
        pStatus->result = 0;
    } else {
        RtDmeSetConnectResult(0, pConnection->nLocalClient, 0, 1);
        pStatus->result = RT_DME_RESULT_FAILED;
    }
    RtReadTime(&g_dmeLastPeerProbe);
    return pStatus->result;
}

int RtDmeInitAddressQuery(rtDmeAddressQuery *pQuery) {
    if (pQuery == NULL) {
        return RT_DME_RESULT_INVALID;
    }
    memset(pQuery, 0, kAddressQuerySize);
    pQuery->version = RT_NET_SETTINGS_VERSION;
    return 0;
}

int RtDmeQueryAddresses(const rtDmeAddressQuery *pQuery, rtDmeConnectStatus *pStatus) {
    int nResult;

    if (pStatus == NULL) {
        return RT_DME_RESULT_INVALID;
    }
    pStatus->result = RT_DME_RESULT_MAPPING;
    pStatus->version = RT_NET_SETTINGS_VERSION;
    if (pQuery == NULL) {
        pStatus->result = RT_DME_RESULT_INVALID;
        return RT_DME_RESULT_INVALID;
    }
    if (g_nDmeRunning != 1) {
        pStatus->result = RT_DME_RESULT_NOT_STARTED;
        return RT_DME_RESULT_NOT_STARTED;
    }
    if (pQuery->version != RT_NET_SETTINGS_VERSION) {
        pStatus->result = RT_DME_RESULT_BAD_VERSION;
        return RT_DME_RESULT_BAD_VERSION;
    }
    if (pQuery->onResult == NULL) {
        pStatus->result = RT_DME_RESULT_INVALID;
        return RT_DME_RESULT_INVALID;
    }
    if (g_bDmeServiceStarted == 1 && g_dmeMapping.complete == 1 && g_dmeMapping.error == 0) {
        pQuery->onResult(&g_dmeMapping);
        pStatus->result = 0;
        return 0;
    }
    if (!g_bDmeServiceStarted) {
        pStatus->result =
            RtDmeStartDatagramService(g_uDmeServiceSendSize, g_uDmeServiceReceiveSize);
        if (pStatus->result != 0) {
            return pStatus->result;
        }
    }
    if (pQuery->server.type == kGatewayAddress) {
        RtDmeSetGatewayAddress(&pQuery->server);
    }
    if (g_dmeGatewayAddress.type != kGatewayAddress) {
        pStatus->result = RT_DME_RESULT_INVALID;
        return RT_DME_RESULT_INVALID;
    }
    g_pfnDmeMapping = pQuery->onResult;
    nResult = RtDmeConvertResult(RtChannelApiMapPort(g_dmeGatewayAddress.address,
                                                     g_dmeGatewayAddress.port,
                                                     RtDmeOnPortMapped),
                                 RT_DME_RESULT_SERVICE);
    if (nResult != 0) {
        return nResult;
    }
    pStatus->result = 0;
    return 0;
}

int RtDmeInitAddressReport(rtDmeAddressReport *pReport) {
    if (pReport == NULL) {
        return RT_DME_RESULT_INVALID;
    }
    memset(pReport, 0, kAddressReportSize);
    pReport->version = RT_NET_SETTINGS_VERSION;
    return 0;
}

int RtDmeReportAddresses(const rtDmeAddressReport *pReport, rtDmeConnectStatus *pStatus) {
    unsigned char bHost;

    bHost = 0;
    if (pStatus == NULL) {
        return RT_DME_RESULT_INVALID;
    }
    pStatus->result = RT_DME_RESULT_MAPPING;
    pStatus->version = RT_NET_SETTINGS_VERSION;
    if (pReport == NULL) {
        pStatus->result = RT_DME_RESULT_INVALID;
        return RT_DME_RESULT_INVALID;
    }
    if (g_nDmeRunning != 1) {
        pStatus->result = RT_DME_RESULT_NOT_STARTED;
        return RT_DME_RESULT_NOT_STARTED;
    }
    if (pReport->version != RT_NET_SETTINGS_VERSION) {
        pStatus->result = RT_DME_RESULT_BAD_VERSION;
        return RT_DME_RESULT_BAD_VERSION;
    }
    if (g_bDmeServiceStarted != 1) {
        pStatus->result = RT_DME_RESULT_NO_SERVICE;
        return RT_DME_RESULT_NO_SERVICE;
    }
    pStatus->result = RtDmeIsLocalHost(&bHost);
    if (pStatus->result != 0) {
        return pStatus->result;
    }
    if (bHost == 1) {
        RtDmeAddReportedPeer(pReport, pStatus);
    }
    return pStatus->result;
}

int RtDmeAddReportedPeer(const rtDmeAddressReport *pReport, rtDmeConnectStatus *pStatus) {
    RtDmeClient *pClient;
    const char *pszPrivate;
    const char *pszPublic;
    unsigned short uPrivatePort;
    unsigned short uPublicPort;
    unsigned char bFound;
    int nSlot;
    int i;

    nSlot = 0;
    bFound = 0;
    pszPrivate = NULL;
    pszPublic = NULL;
    uPrivatePort = 0;
    uPublicPort = 0;
    pClient = NULL;
    if (pStatus == NULL) {
        return RT_DME_RESULT_INVALID;
    }
    pStatus->result = RT_DME_RESULT_MAPPING;
    pStatus->version = RT_NET_SETTINGS_VERSION;
    if (pReport == NULL) {
        pStatus->result = RT_DME_RESULT_INVALID;
        return RT_DME_RESULT_INVALID;
    }
    if (g_nDmeRunning != 1) {
        pStatus->result = RT_DME_RESULT_NOT_STARTED;
        return RT_DME_RESULT_NOT_STARTED;
    }
    if (pReport->version != RT_NET_SETTINGS_VERSION) {
        pStatus->result = RT_DME_RESULT_BAD_VERSION;
        return RT_DME_RESULT_BAD_VERSION;
    }
    if (g_bDmeServiceStarted != 1) {
        pStatus->result = RT_DME_RESULT_NO_SERVICE;
        return RT_DME_RESULT_NO_SERVICE;
    }
    for (i = 0; i < RT_DME_ADDRESS_COUNT; ++i) {
        if (pReport->addresses[i].type == RT_DME_ADDRESS_PRIVATE) {
            pszPrivate = pReport->addresses[i].address;
            uPrivatePort = pReport->addresses[i].port;
        } else if (pReport->addresses[i].type == RT_DME_ADDRESS_PUBLIC) {
            pszPublic = pReport->addresses[i].address;
            uPublicPort = pReport->addresses[i].port;
        }
    }
    if (pszPublic == NULL || pszPrivate == NULL) {
        pStatus->result = RT_DME_RESULT_INVALID;
        return RT_DME_RESULT_INVALID;
    }
    for (i = 0; i < RT_DME_CLIENT_COUNT; ++i) {
        if (RtDmeGetClientAddressKind(0, i, RT_DME_ADDRESS_INDEX_PUBLIC) == 0 &&
            RtDmeGetClientAddressKind(0, i, RT_DME_ADDRESS_INDEX_PRIVATE) == 0) {
            if (!bFound) {
                bFound = 1;
                nSlot = i;
            }
            continue;
        }
        pClient = NULL;
        if (RtDmeGetClient(0, i, &pClient) != 0 || pClient == NULL) {
            continue;
        }
        if (pClient->addresses[RT_DME_ADDRESS_INDEX_PRIVATE].port != uPrivatePort ||
            pClient->addresses[RT_DME_ADDRESS_INDEX_PUBLIC].port != uPublicPort) {
            continue;
        }
        if (strncmp(pClient->addresses[RT_DME_ADDRESS_INDEX_PRIVATE].address,
                    pszPrivate,
                    RT_DME_ADDRESS_SIZE) != 0 ||
            strncmp(pClient->addresses[RT_DME_ADDRESS_INDEX_PUBLIC].address,
                    pszPublic,
                    RT_DME_ADDRESS_SIZE) != 0) {
            continue;
        }
        // The peer is known under another index. Drop that client and take its place.
        bFound = 1;
        RtDmeRemoveClient(0, i, RtDmeGetClientAddress(0, i, RT_DME_ADDRESS_INDEX_PUBLIC), 0);
        nSlot = i;
    }
    if (bFound != 1) {
        pStatus->result = RT_DME_RESULT_NO_MEMORY;
        return RT_DME_RESULT_NO_MEMORY;
    }
    pStatus->result = RtDmeCreateClient(0, nSlot, &pClient);
    if (pStatus->result != 0 || pClient == NULL) {
        return pStatus->result;
    }
    if (pszPrivate != NULL) {
        RtDmeSetClientAddressKind(0, nSlot, RT_DME_ADDRESS_INDEX_PRIVATE, RT_DME_ADDRESS_PRIVATE);
        RtDmeSetClientAddress(0, nSlot, RT_DME_ADDRESS_INDEX_PRIVATE, pszPrivate);
        RtDmeSetClientPort(0, nSlot, RT_DME_ADDRESS_INDEX_PRIVATE, uPrivatePort);
    }
    if (pszPublic != NULL) {
        RtDmeSetClientAddressKind(0, nSlot, RT_DME_ADDRESS_INDEX_PUBLIC, RT_DME_ADDRESS_PUBLIC);
        RtDmeSetClientAddress(0, nSlot, RT_DME_ADDRESS_INDEX_PUBLIC, pszPublic);
        RtDmeSetClientPort(0, nSlot, RT_DME_ADDRESS_INDEX_PUBLIC, uPublicPort);
    }
    pStatus->result = RtDmeGetClient(0, nSlot, &pClient);
    if (pStatus->result != 0 || pClient == NULL) {
        return pStatus->result;
    }
    pClient->uAddressIndex = RT_DME_ADDRESS_INDEX_UNRESOLVED;
    pClient->nProbeCount = 0;
    RtReadClock(&pClient->lastProbe);
    pStatus->result = RtDmeDropClientPeers(0, nSlot);
    if (pStatus->result != 0) {
        return pStatus->result;
    }
    pStatus->result = 0;
    return 0;
}

int RtDmeEnableConnection(int nConnection, int bEnable) {
    RtDmeConnection *pConnection;
    signed char nFlags;
    int nResult;

    pConnection = NULL;
    nFlags = 0;
    if ((unsigned int)nConnection >= RT_DME_CONNECTION_COUNT) {
        return RT_DME_RESULT_INVALID;
    }
    nResult = RtDmeGetConnection(nConnection, &pConnection);
    if (nResult != 0 || pConnection == NULL) {
        return nResult;
    }
    nResult = RtDmeGetClientFlags(nConnection, pConnection->nLocalClient, &nFlags);
    if (nResult != 0) {
        return nResult;
    }
    if (bEnable) {
        nFlags |= RT_DME_CLIENT_FLAG_DATAGRAMS;
    } else {
        nFlags &= ~RT_DME_CLIENT_FLAG_DATAGRAMS;
    }
    nResult = RtDmeSetClientFlags(nConnection, pConnection->nLocalClient, nFlags);
    if (nResult != 0) {
        return nResult;
    }
    RtDmeSendClientFlags(nConnection, RT_DME_ALL_CLIENTS);
    if (nConnection == 0 || nConnection == RT_DME_SERVER_CLIENT) {
        return 0;
    }
    nResult = RtDmeConvertResult(
        RtChannelApiSendFlag(RtDmeConnectionToChannel(nConnection), (unsigned char)bEnable),
        RT_DME_RESULT_SERVICE);
    if (nResult != 0) {
        return nResult;
    }
    return 0;
}

int RtDmeLeaveSession(int nConnection) {
    RtDmeConnection *pConnection;
    RtDmeClient *pClient;
    unsigned char uClient;
    int nResult;
    int i;

    pConnection = NULL;
    pClient = NULL;
    if ((unsigned int)nConnection >= RT_DME_CONNECTION_COUNT) {
        return RT_DME_RESULT_INVALID;
    }
    nResult = RtDmeGetConnection(nConnection, &pConnection);
    if (nResult != 0 || pConnection == NULL) {
        return nResult;
    }
    RtDmeEnableConnection(nConnection, 0);
    uClient = RtDmeClientToByte(pConnection->nLocalClient);
    rtDmeSendClassMessage(kObjectSendFlags,
                          nConnection,
                          RT_DME_ALL_CLIENTS,
                          RT_DME_CLASS_SYSTEM,
                          g_nDmeClientLeftMessage,
                          sizeof(uClient),
                          &uClient);
    RtDmeFreeStreams();
    RtDmeDestroyAllObjects();
    RtDmeFreeConnectionObjectRanges(nConnection);
    g_nDmeActiveConnection = RT_DME_CONNECTION_COUNT;
    g_bDmeObjectsShared = 0;
    g_bDmeObjectRangesSent = 0;
    nResult = RtDmeGetClient(nConnection, pConnection->nLocalClient, &pClient);
    if (nResult != 0 || pClient == NULL) {
        return nResult;
    }
    pClient->anState[1] = 0;
    pClient->nRole = RT_DME_ROLE_LOCAL;
    pClient->anState[0] = 0;
    RtDmeSendClientState(nConnection, RT_DME_ALL_CLIENTS);
    for (i = 0; i < RT_DME_CONNECTION_COUNT; ++i) {
        if (RtDmeGetConnection(i, &pConnection) == 0 && pConnection != NULL) {
            RtDmeSetHostClient(i, RT_DME_NO_CLIENT);
        }
    }
    g_pfnDmeSessionReady = NULL;
    g_nDmeSessionReadyArgument = 0;
    g_pfnDmeObjectRange = NULL;
    g_nDmeObjectRangeArgument = 0;
    g_pfnDmeObjectHost = NULL;
    g_nDmeObjectHostArgument = 0;
    g_nDmeObjectCount = 0;
    return 0;
}

// The receiver of the latency probes of the clients.
// NTSC-U/C: 0x002b5768, PAL: 0x002bf6d8
static void RtDmeOnLatency(int nResult,
                           char *pszAddress,
                           int nLatency,
                           unsigned int uRemoteTime,
                           int nConnection,
                           int nClient) {
    RtDmeConnection *pConnection;
    RtDmeClientStats *pStats;
    unsigned int uHalfLatency;
    unsigned int uSessionTime;
    unsigned int uClientLatency;
    int nHost;
    int nNext;

    (void)pszAddress;
    pConnection = NULL;
    if (nConnection == RT_DME_CONNECTION_COUNT || nClient == RT_DME_NO_CLIENT) {
        RtDmeFindClientByAddress(&nConnection, &nClient, g_szDmeSenderAddress, g_uDmeSenderPort);
    }
    if (RtDmeGetConnection(nConnection, &pConnection) != 0 || pConnection == NULL) {
        return;
    }
    if (nConnection != g_nDmeActiveConnection || nClient >= pConnection->nMaxClients ||
        nResult != 0) {
        return;
    }
    nHost = RT_DME_NO_CLIENT;
    pStats = NULL;
    RtDmeGetClientStats(nClient, &pStats);
    if (pStats == NULL) {
        return;
    }
    uHalfLatency = (unsigned int)nLatency >> 1;
    nNext = pStats->nNextSample;
    pStats->auSamples[nNext] = uHalfLatency;
    if ((unsigned int)pStats->nSampleCount < RT_DME_LATENCY_SAMPLES) {
        ++pStats->nSampleCount;
    }
    ++nNext;
    RtDmeGetHostClient(&nHost, nConnection);
    if (nConnection == g_nDmeActiveConnection && nClient == nHost && uRemoteTime != 0) {
        (void)RtDmeGetSessionTime(&uSessionTime); // Yes, the binary discards the time.
        RtDmeGetClientLatency(&uClientLatency, nClient);
        if (uClientLatency >= uHalfLatency) {
            RtDmeSetSessionTime(uRemoteTime + uHalfLatency);
        }
    }
    if (nNext >= RT_DME_LATENCY_SAMPLES) {
        nNext = 0;
    }
    pStats->nNextSample = nNext;
    RtReadClock(&pStats->lastProbe);
}

void RtDmeProbeClient(unsigned int nClient) {
    RtDmePingMessage message;
    RtDmePing *pPing;
    int nIndex;
    unsigned char uSlot;

    if (nClient >= RT_DME_CLIENT_COUNT) {
        return;
    }
    message.bRequest = 1;
    uSlot = RtDmeFindFreePing();
    if ((signed char)uSlot == RT_DME_PING_COUNT) {
        return;
    }
    message.uSlot = uSlot;
    nIndex = RtDmeGetAddressIndex(g_nDmeActiveConnection, nClient);
    if ((unsigned int)nIndex >= RT_DME_ADDRESS_COUNT) {
        return;
    }
    message.uTime = g_uDmeSessionTime;
    rtDmeSendClassMessage(kPingSendFlags,
                          g_nDmeActiveConnection,
                          nClient,
                          RT_DME_CLASS_SYSTEM,
                          g_nDmePingMessage,
                          sizeof(message),
                          &message);
    pPing = &g_aDmePings[uSlot];
    RtDmeGetLocalTime(&pPing->uSentTime);
    RtDmeParseAddress(pPing->abAddress,
                      RtDmeGetClientAddress(g_nDmeActiveConnection, nClient, nIndex));
    pPing->bClientProbe = 1;
    pPing->uPort = RtDmeGetClientPort(g_nDmeActiveConnection, nClient, nIndex);
    pPing->nConnection = g_nDmeActiveConnection;
    pPing->done.pfnClientDone = RtDmeOnLatency;
    pPing->nClient = nClient;
}

// Probe the latency of a client or of the server for the game.
// NTSC-U/C: 0x002b5a60, PAL: 0x002bf9d0
static int RtDmeProbe(unsigned int nConnection, unsigned int nClient, RtDmePingFunc pfnDone) {
    RtDmePingMessage message;
    RtDmePing *pPing;
    int nIndex;
    int nSlot;
    int nResult;

    if (nConnection >= RT_DME_CONNECTION_COUNT) {
        return RT_DME_RESULT_INVALID;
    }
    if (nClient >= RT_DME_CLIENT_COUNT && nClient != RT_DME_SERVER_CLIENT) {
        return RT_DME_RESULT_INVALID;
    }
    if (pfnDone == NULL) {
        return RT_DME_RESULT_INVALID;
    }
    // Yes, the binary tests for every client after the range check rejected it.
    if ((int)nClient == RT_DME_ALL_CLIENTS) {
        return RT_DME_RESULT_OUT_OF_RANGE;
    }
    nSlot = RtDmeFindFreePing();
    if (nSlot == RT_DME_PING_COUNT) {
        return RT_DME_RESULT_NO_PROBE;
    }
    message.bRequest = 1;
    message.uTime = 0;
    message.uSlot = nSlot;
    rtDmeSendClassMessage(kPingSendFlags,
                          nConnection,
                          nClient,
                          RT_DME_CLASS_SYSTEM,
                          g_nDmePingMessage,
                          sizeof(message),
                          &message);
    pPing = &g_aDmePings[nSlot];
    if (nClient == RT_DME_SERVER_CLIENT) {
        nResult = RtDmeParseAddress(pPing->abAddress, RtDmeGetServerName(nConnection));
        if (nResult != 0) {
            return nResult;
        }
        pPing->uPort = g_uDmePort;
        pPing->nConnection = RT_DME_CONNECTION_COUNT;
        pPing->nClient = RT_DME_NO_CLIENT;
    } else {
        nIndex = RtDmeGetAddressIndex(nConnection, nClient);
        nResult = RtDmeParseAddress(pPing->abAddress,
                                    RtDmeGetClientAddress(nConnection, nClient, nIndex));
        if (nResult != 0) {
            return nResult;
        }
        pPing->uPort = RtDmeGetClientPort(nConnection, nClient, nIndex);
        pPing->nConnection = nConnection;
        pPing->nClient = nClient;
    }
    pPing->bClientProbe = 0;
    pPing->done.pfnDone = pfnDone;
    RtDmeGetLocalTime(&pPing->uSentTime);
    return 0;
}

// Add the clients that see an object to a set.
// NTSC-U/C: 0x002b5c78, PAL: 0x002bfbe8
static void RtDmeBuildObjectClientSet(RtDmeClientSet *pSet, unsigned int nObject) {
    RtDmeConnection *pConnection;
    RtDmeObject *pObject;
    RtDmeObjectRange *pRange;
    RtDmeObjectFilterFunc pfnFilter;
    int i;

    pObject = NULL;
    pConnection = NULL;
    if (pSet == NULL || nObject >= RT_DME_OBJECT_COUNT) {
        return;
    }
    if ((unsigned int)g_nDmeActiveConnection >= RT_DME_CONNECTION_COUNT) {
        return;
    }
    if (RtDmeGetConnection(g_nDmeActiveConnection, &pConnection) != 0 || pConnection == NULL) {
        return;
    }
    RtDmeGetObject(&pObject, nObject);
    if (pObject == NULL) {
        return;
    }
    pSet->nLast = -1;
    pSet->nFirst = pConnection->nMaxClients;
    pfnFilter = g_apfnDmeObjectFilter[pObject->uFilter];
    for (i = sizeof(pSet->abBits) - 1; i >= 0; --i) {
        pSet->abBits[i] = 0;
    }
    for (i = 0; i < pConnection->nMaxClients; ++i) {
        pRange = NULL;
        RtDmeGetObjectRange(&pRange, g_nDmeActiveConnection, i);
        if (pRange == NULL || i == pConnection->nLocalClient) {
            continue;
        }
        // Yes, the binary calls the filter without checking it.
        if (pfnFilter(i, nObject)) {
            RtDmeAddToClientSet(pSet, i);
        }
    }
}

// Report whether two values of a field differ enough to send the field.
// NTSC-U/C: 0x002b64a8, PAL: 0x002c0418
static int RtDmeFieldChanged(const RtDmeObjectField *pField,
                             const unsigned char *pbNew,
                             const unsigned char *pbOld) {
    double dDifference;
    double dDistance;
    float fX;
    float fY;
    float fZ;
    int nX;
    int nY;
    int nZ;

    dDifference = 0.0;
    if (pField == NULL || pbNew == NULL || pbOld == NULL) {
        return 0;
    }
    switch (pField->nCompare) {
    case RT_DME_COMPARE_S8:
        dDifference = (double)*(const signed char *)pbNew - (double)*(const signed char *)pbOld;
        break;
    case RT_DME_COMPARE_U8:
        dDifference = (double)*pbNew - (double)*pbOld;
        break;
    case RT_DME_COMPARE_S16:
        dDifference = (double)*(const short *)pbNew - (double)*(const short *)pbOld;
        break;
    case RT_DME_COMPARE_U16:
        dDifference =
            (double)*(const unsigned short *)pbNew - (double)*(const unsigned short *)pbOld;
        break;
    case RT_DME_COMPARE_S32:
        dDifference = (double)*(const int *)pbNew - (double)*(const int *)pbOld;
        break;
    case RT_DME_COMPARE_U32:
        dDifference = (double)*(const unsigned int *)pbNew - (double)*(const unsigned int *)pbOld;
        break;
    case RT_DME_COMPARE_FLOAT:
        dDifference = (double)*(const float *)pbNew - (double)*(const float *)pbOld;
        break;
    case RT_DME_COMPARE_DOUBLE:
        dDifference = *(const double *)pbNew - *(const double *)pbOld;
        break;
    case RT_DME_COMPARE_S16_2D:
        nX = ((const short *)pbNew)[0] - ((const short *)pbOld)[0];
        nY = ((const short *)pbNew)[1] - ((const short *)pbOld)[1];
        dDistance = nX * nX + nY * nY;
        return dDistance > pField->test.dThreshold * pField->test.dThreshold;
    case RT_DME_COMPARE_S16_3D:
        nX = ((const short *)pbNew)[0] - ((const short *)pbOld)[0];
        nY = ((const short *)pbNew)[1] - ((const short *)pbOld)[1];
        nZ = ((const short *)pbNew)[2] - ((const short *)pbOld)[2];
        dDistance = nX * nX + nY * nY + nZ * nZ;
        return dDistance > pField->test.dThreshold * pField->test.dThreshold;
    case RT_DME_COMPARE_S32_2D:
        nX = ((const int *)pbNew)[0] - ((const int *)pbOld)[0];
        nY = ((const int *)pbNew)[1] - ((const int *)pbOld)[1];
        dDistance = nX * nX + nY * nY;
        return dDistance > pField->test.dThreshold * pField->test.dThreshold;
    case RT_DME_COMPARE_S32_3D:
        nX = ((const int *)pbNew)[0] - ((const int *)pbOld)[0];
        nY = ((const int *)pbNew)[1] - ((const int *)pbOld)[1];
        nZ = ((const int *)pbNew)[2] - ((const int *)pbOld)[2];
        dDistance = nX * nX + nY * nY + nZ * nZ;
        return dDistance > pField->test.dThreshold * pField->test.dThreshold;
    case RT_DME_COMPARE_FLOAT_2D:
        fX = ((const float *)pbNew)[0] - ((const float *)pbOld)[0];
        fY = ((const float *)pbNew)[1] - ((const float *)pbOld)[1];
        dDistance = fX * fX + fY * fY;
        return dDistance > pField->test.dThreshold * pField->test.dThreshold;
    case RT_DME_COMPARE_FLOAT_3D:
        fX = ((const float *)pbNew)[0] - ((const float *)pbOld)[0];
        fY = ((const float *)pbNew)[1] - ((const float *)pbOld)[1];
        fZ = ((const float *)pbNew)[2] - ((const float *)pbOld)[2];
        dDistance = fX * fX + fY * fY + fZ * fZ;
        return dDistance > pField->test.dThreshold * pField->test.dThreshold;
    case RT_DME_COMPARE_DOUBLE_2D:
        dDistance = (((const double *)pbNew)[0] - ((const double *)pbOld)[0]) *
                        (((const double *)pbNew)[0] - ((const double *)pbOld)[0]) +
                    (((const double *)pbNew)[1] - ((const double *)pbOld)[1]) *
                        (((const double *)pbNew)[1] - ((const double *)pbOld)[1]);
        return dDistance > pField->test.dThreshold * pField->test.dThreshold;
    case RT_DME_COMPARE_DOUBLE_3D:
        dDistance = (((const double *)pbNew)[0] - ((const double *)pbOld)[0]) *
                        (((const double *)pbNew)[0] - ((const double *)pbOld)[0]) +
                    (((const double *)pbNew)[1] - ((const double *)pbOld)[1]) *
                        (((const double *)pbNew)[1] - ((const double *)pbOld)[1]) +
                    (((const double *)pbNew)[2] - ((const double *)pbOld)[2]) *
                        (((const double *)pbNew)[2] - ((const double *)pbOld)[2]);
        return dDistance > pField->test.dThreshold * pField->test.dThreshold;
    default:
        break;
    }
    if (dDifference < 0.0) {
        dDifference = 0.0 - dDifference;
    }
    if (pField->nMode == RT_DME_FIELD_CHANGED) {
        return dDifference != 0.0;
    }
    return dDifference > pField->test.dThreshold;
}

// Report whether a field of an object changed enough to send it.
// NTSC-U/C: 0x002b69a0, PAL: 0x002c0910
static unsigned char
RtDmeFieldNeedsSend(const RtDmeObject *pObject, int nObject, unsigned char uField) {
    const RtDmeObjectType *pType;
    const RtDmeObjectField *pField;
    const unsigned char *pbNew;
    const unsigned char *pbOld;
    RtDmeFieldTest test;
    int i;

    if (pObject == NULL) {
        return 0;
    }
    pType = &g_aDmeObjectTypes[pObject->uType];
    pField = pType->apFields[uField];
    if (pField == NULL) {
        return 0;
    }
    pbNew = &pObject->pbData[pType->anFieldOffsets[uField]];
    if (pbNew == NULL) {
        return 0;
    }
    pbOld = &pObject->pbShadow[pType->anFieldOffsets[uField]];
    if (pbOld == NULL) {
        return 0;
    }
    if (pField->nMode == RT_DME_FIELD_CUSTOM) {
        test.nObject = nObject;
        test.nField = uField;
        test.pbNew = pbNew;
        test.nElementSize = pField->nElementSize;
        test.pbOld = pbOld;
        test.nElementCount = pField->nElementCount;
        if (pField->test.pfnTest == NULL) {
            return 0;
        }
        return pField->test.pfnTest(&test);
    }
    for (i = 0; i < pField->nElementCount; ++i) {
        if (RtDmeFieldChanged(pField, pbNew, pbOld)) {
            return 1;
        }
        pbOld = &pbOld[pField->nElementSize];
        pbNew = &pbNew[pField->nElementSize];
    }
    return 0;
}

// Write the fields of an object that are due and changed, each an index byte before its data.
// NTSC-U/C: 0x002b5dd8, PAL: 0x002bfd48
static unsigned char RtDmeGatherChangedFields(RtDmeObject *pObject,
                                              int nObject,
                                              signed char *pnSendFlags,
                                              unsigned char *pbOut,
                                              int *pnSize) {
    const RtDmeObjectType *pType;
    const RtDmeObjectField *pField;
    unsigned char *pbData;
    unsigned char *pbShadow;
    unsigned char uCount;
    unsigned char uField;
    int nBytes;

    uCount = 0;
    if (pbOut == NULL || pnSize == NULL || pObject == NULL ||
        pObject->uType >= RT_DME_OBJECT_TYPE_COUNT) {
        return 0;
    }
    pType = &g_aDmeObjectTypes[pObject->uType];
    for (uField = 0; uField < pType->nFieldCount; ++uField) {
        if ((unsigned int)*pnSize >= RT_DME_MAX_MESSAGE_SIZE + 1) {
            continue;
        }
        pField = pType->apFields[uField];
        if (pField == NULL || pField->nMode == RT_DME_FIELD_NEVER) {
            continue;
        }
        if (pObject->auFieldSendTime[uField] + pField->uInterval >= g_uDmeSessionTime) {
            continue;
        }
        pbData = &pObject->pbData[pType->anFieldOffsets[uField]];
        pbShadow = &pObject->pbShadow[pType->anFieldOffsets[uField]];
        if (!RtDmeFieldNeedsSend(pObject, nObject, uField)) {
            continue;
        }
        *pbOut++ = uField;
        ++*pnSize;
        memcpy(pbOut, pbData, pField->nElementSize * pField->nElementCount);
        nBytes = pField->nElementSize * pField->nElementCount;
        ++uCount;
        *pnSize += nBytes;
        pbOut = &pbOut[nBytes];
        *pnSendFlags |= pField->nSendFlags;
        memcpy(pbShadow, pbData, pField->nElementCount * pField->nElementSize);
        pObject->auFieldSendTime[uField] = g_uDmeSessionTime;
    }
    return uCount;
}

// Send the changed fields of an object to the clients that see it.
// NTSC-U/C: 0x002b5ff0, PAL: 0x002bff60
static int RtDmeSendObjectUpdate(unsigned int nObject) {
    RtDmeObject *pObject;
    RtDmeClientSet set;
    signed char nSendFlags;
    int nSize;
    int nResult;

    pObject = NULL;
    nSize = 0;
    nSendFlags = 0;
    if (nObject >= RT_DME_OBJECT_COUNT) {
        return RT_DME_RESULT_INVALID;
    }
    nResult = RtDmeGetObject(&pObject, nObject);
    if (nResult != 0 || pObject == NULL) {
        return nResult;
    }
    if (pObject->uFilter != 0) {
        RtDmeBuildObjectClientSet(&set, nObject);
    }
    g_dmeObjectUpdate.header.uType = pObject->uType;
    g_dmeObjectUpdate.header.uObject = nObject;
    nSize = kObjectUpdateHeaderSize;
    g_dmeObjectUpdate.header.uCount = RtDmeGatherChangedFields(
        pObject, nObject, &nSendFlags, &g_dmeObjectUpdate.abBytes[kObjectUpdateHeaderSize], &nSize);
    if (g_dmeObjectUpdate.header.uCount == 0) {
        return 0;
    }
    if (pObject->uFilter != 0) {
        RtDmeSendToClientSet(nSendFlags,
                             g_nDmeActiveConnection,
                             RT_DME_CLASS_SYSTEM,
                             g_nDmeObjectUpdateMessage,
                             nSize,
                             &g_dmeObjectUpdate,
                             &set);
    } else {
        RtDmeSendSystemMessage(nSendFlags,
                               g_nDmeActiveConnection,
                               RT_DME_ALL_CLIENTS,
                               g_nDmeObjectUpdateMessage,
                               nSize,
                               &g_dmeObjectUpdate);
    }
    return 0;
}

// Send an object whole to a client, or to every client that sees it.
// NTSC-U/C: 0x002b6118, PAL: 0x002c0088
static int RtDmeSendObject(int nClient, unsigned int nObject) {
    RtDmeConnection *pConnection;
    RtDmeObject *pObject;
    RtDmeObjectRange *pRange;
    const RtDmeObjectType *pType;
    const RtDmeObjectField *pField;
    RtDmeClientSet set;
    int nSize;
    int nBytes;
    int nResult;
    int i;

    pObject = NULL;
    pConnection = NULL;
    if (nObject >= RT_DME_OBJECT_COUNT) {
        return RT_DME_RESULT_INVALID;
    }
    RtDmeGetObject(&pObject, nObject);
    if (pObject == NULL) {
        return RT_DME_RESULT_NO_RECORD;
    }
    nResult = RtDmeGetConnection(g_nDmeActiveConnection, &pConnection);
    if (nResult != 0 || pConnection == NULL) {
        return nResult;
    }
    if (pObject->uFilter != 0) {
        if (nClient == RT_DME_ALL_CLIENTS) {
            RtDmeBuildObjectClientSet(&set, nObject);
        } else if (!g_apfnDmeObjectFilter[pObject->uFilter](nClient, nObject)) {
            return 0;
        }
    }
    pType = &g_aDmeObjectTypes[pObject->uType];
    if (pObject->nLockClient != RT_DME_ALL_CLIENTS &&
        pObject->nLockClient != pConnection->nLocalClient) {
        return RT_DME_RESULT_OUT_OF_RANGE;
    }
    nSize = kObjectHeaderSize;
    g_dmeObjectMessage.header.uOwner = RtDmeClientToByte(pObject->nOwner);
    g_dmeObjectMessage.header.uObject = nObject;
    g_dmeObjectMessage.header.uLockClient = pObject->nLockClient;
    g_dmeObjectMessage.header.uType = pObject->uType;
    g_dmeObjectMessage.header.uFilter = pObject->uFilter;
    g_dmeObjectMessage.header.uLifetime = pObject->uLifetime;
    g_dmeObjectMessage.header.uFlags = pObject->uFlags;
    strcpy(g_dmeObjectMessage.header.szName, pObject->szName);
    for (i = 0; i < pType->nFieldCount; ++i) {
        pField = pType->apFields[i];
        if (pField == NULL) {
            return RT_DME_RESULT_NO_RECORD;
        }
        nBytes = pField->nElementSize * pField->nElementCount;
        memcpy(
            &g_dmeObjectMessage.abBytes[nSize], &pObject->pbData[pType->anFieldOffsets[i]], nBytes);
        nSize += nBytes;
        pObject->auFieldSendTime[i] = g_uDmeSessionTime;
    }
    pObject->uLastFullTime = g_uDmeSessionTime;
    if (nClient == RT_DME_ALL_CLIENTS) {
        memcpy(pObject->pbShadow, pObject->pbData, pType->nDataSize);
        for (i = 0; i < pConnection->nMaxClients; ++i) {
            RtDmeGetObjectRange(&pRange, g_nDmeActiveConnection, i);
            if (pRange != NULL) {
                pObject->auClientSendTime[i] = g_uDmeSessionTime;
            }
        }
    } else {
        pObject->auClientSendTime[nClient] = g_uDmeSessionTime;
    }
    if (pObject->uFilter != 0 && nClient == RT_DME_ALL_CLIENTS) {
        RtDmeSendToClientSet(kObjectSendFlags,
                             g_nDmeActiveConnection,
                             RT_DME_CLASS_SYSTEM,
                             g_nDmeObjectMessage,
                             nSize,
                             &g_dmeObjectMessage,
                             &set);
    } else {
        RtDmeSendSystemMessage(kObjectSendFlags,
                               g_nDmeActiveConnection,
                               nClient,
                               g_nDmeObjectMessage,
                               nSize,
                               &g_dmeObjectMessage);
    }
    return 0;
}

// Write the next part of a stream after its header, and report the size of the message.
// NTSC-U/C: 0x002b6ae0, PAL: 0x002c0a50
static int RtDmePackStreamPart(RtDmeStreamMessage *pMessage, unsigned int nStream, int nSize) {
    RtDmeStream *pStream;
    unsigned char *pbRead;
    unsigned char bWrapped;

    pStream = NULL;
    pbRead = NULL;
    bWrapped = 0;
    if (pMessage == NULL || nSize <= 0 || nStream >= RT_DME_STREAM_COUNT) {
        return 0;
    }
    if (RtDmeGetStream(nStream, &pStream) != 0 || pStream == NULL) {
        return 0;
    }
    pMessage->header.uStream = nStream;
    pMessage->header.bToHost = pStream->bToHost;
    pMessage->header.uRoute = pStream->uRoute;
    pMessage->header.uKind = pStream->uKind;
    pMessage->header.bOwnsData = pStream->bOwnsData;
    pMessage->header.uSize = nSize;
    pMessage->header.uStart = pStream->pbStart - pStream->pbData;
    pMessage->header.uEnd = pStream->pbEnd - pStream->pbData;
    if (RtIsRingBufferWrapped(&pStream->queue, &bWrapped) != 0) {
        return 0;
    }
    if (bWrapped) {
        if (RtCompactRingBuffer(&pStream->queue, 1) != 0) {
            return 0;
        }
        memcpy(pStream->pbData, pStream->queue.pbData, pStream->queue.uSize);
    }
    if (RtGetRingBufferReadPointer(&pStream->queue, &pbRead) != 0) {
        return 0;
    }
    memcpy(&pMessage->abBytes[kStreamHeaderSize], pbRead, nSize);
    return nSize + kStreamHeaderSize;
}

// Announce a stream that closed to its receiver.
// NTSC-U/C: 0x002b6c38, PAL: 0x002c0ba8
static void RtDmeSendStreamClosed(int nClient, unsigned int nStream) {
    RtDmeConnection *pConnection;
    RtDmeStream *pStream;
    unsigned char uStream;
    int nHost;

    pStream = NULL;
    pConnection = NULL;
    if (nStream >= RT_DME_OBJECT_COUNT) {
        return;
    }
    if ((unsigned int)g_nDmeActiveConnection >= RT_DME_CONNECTION_COUNT) {
        return;
    }
    if (RtDmeGetConnection(g_nDmeActiveConnection, &pConnection) != 0 || pConnection == NULL) {
        return;
    }
    if (RtDmeFindStream(&pStream, nStream) != 0 || pStream == NULL) {
        return;
    }
    if (RtDmeGetHostClient(&nHost, g_nDmeActiveConnection) != 0) {
        return;
    }
    if (nHost < 0 || nHost >= RT_DME_CLIENT_COUNT) {
        return;
    }
    if (g_anDmeStreamRelay[pStream->uRoute] != 0 && pConnection->nLocalClient != nHost &&
        nClient == RT_DME_ALL_CLIENTS) {
        nClient = nHost;
    }
    uStream = nStream;
    RtDmeSendSystemMessage(kObjectSendFlags,
                           g_nDmeActiveConnection,
                           nClient,
                           g_nDmeStreamClosedMessage,
                           sizeof(uStream),
                           &uStream);
}

// Send the next parts of the streams of the local client, within their rates.
// NTSC-U/C: 0x002b6d50, PAL: 0x002c0cc0
static void RtDmeFlushStreams(void) {
    RtDmeConnection *pConnection;
    RtDmeObjectRange *pRange;
    RtDmeStream *pStream;
    RtRingBuffer *pQueue;
    unsigned int uUsed;
    unsigned int uChunk;
    unsigned int uElapsed;
    int nReceiver;
    int nHost;
    int nStream;
    int nSize;
    unsigned int i;

    nHost = RT_DME_NO_CLIENT;
    pStream = NULL;
    pRange = NULL;
    pConnection = NULL;
    if ((unsigned int)g_nDmeActiveConnection >= RT_DME_CONNECTION_COUNT) {
        return;
    }
    if (RtDmeGetConnection(g_nDmeActiveConnection, &pConnection) != 0 || pConnection == NULL) {
        return;
    }
    if (RtDmeGetObjectRange(&pRange, g_nDmeActiveConnection, pConnection->nLocalClient) != 0 ||
        pRange == NULL) {
        return;
    }
    if (pRange->uFirstStream >= RT_DME_STREAM_COUNT) {
        return;
    }
    if (RtDmeGetHostClient(&nHost, g_nDmeActiveConnection) != 0) {
        return;
    }
    if (nHost < 0 || nHost >= RT_DME_CLIENT_COUNT) {
        return;
    }
    for (i = 0; i < pRange->uStreamCount; ++i) {
        nStream = i + pRange->uFirstStream;
        if (RtDmeGetStream(nStream, &pStream) != 0 || pStream == NULL) {
            continue;
        }
        if (pStream->uState != RT_DME_STREAM_OPEN) {
            continue;
        }
        nReceiver = pStream->nReceiver;
        if (g_anDmeStreamRelay[pStream->uRoute] != 0 && pConnection->nLocalClient != nHost &&
            pStream->bToHost) {
            nReceiver = nHost;
        }
        pQueue = &pStream->queue;
        // Yes, the binary tests the address of the queue against null.
        if (pQueue == NULL) {
            continue;
        }
        uUsed = 0;
        if (RtGetRingBufferUsed(pQueue, &uUsed) != 0) {
            continue;
        }
        if (uUsed != 0) {
            uChunk = pStream->uMaxChunk;
            if (uChunk >= uUsed) {
                uChunk = uUsed;
            }
            if (uChunk >= kMaxStreamChunk + 1) {
                uChunk = kMaxStreamChunk;
            }
            uUsed = uChunk;
            uElapsed = g_uDmeSessionTime - pStream->uLastSendTime;
            if ((double)(uChunk * kStreamBytesScale) >=
                (double)(int)(pStream->nRate * uElapsed) * STREAM_RATE_HEADROOM) {
                continue;
            }
            nSize = RtDmePackStreamPart(&g_dmeStreamMessage, nStream, uChunk);
            if (RtDmeSendSystemMessage(kStreamSendFlags | RT_DME_SEND_ORDERED,
                                       g_nDmeActiveConnection,
                                       nReceiver,
                                       g_nDmeStreamDataMessage,
                                       nSize,
                                       &g_dmeStreamMessage) != 0) {
                continue;
            }
            pStream->uLastSendTime = g_uDmeSessionTime;
            RtConsumeRingBuffer(&pStream->queue, uUsed);
            RtCompactRingBuffer(&pStream->queue, 1);
            memcpy(pStream->pbData, pStream->queue.pbData, pStream->queue.uSize);
        } else if (pStream->bClosing) {
            RtDmeFree((void **)&pStream->pbData);
            RtDestroyRingBuffer(&pStream->queue);
            pStream->pbData = NULL;
            pStream->uState = RT_DME_STREAM_RESERVED;
            RtDmeSendStreamClosed(nReceiver, nStream);
        }
    }
}

// Send the objects of the local client of the active connection that are due.
// NTSC-U/C: 0x002b7088, PAL: 0x002c0ff8
static void RtDmeSendObjects(void) {
    RtDmeConnection *pConnection;
    RtDmeObject *pObject;
    RtDmeObjectRange *pRange;
    RtDmeClient *pClient;
    int nLocalClient;
    int nObject;
    int i;

    pObject = NULL;
    pConnection = NULL;
    pClient = NULL;
    if ((unsigned int)g_nDmeActiveConnection >= RT_DME_CONNECTION_COUNT) {
        return;
    }
    if (RtDmeGetConnection(g_nDmeActiveConnection, &pConnection) != 0 || pConnection == NULL) {
        return;
    }
    nLocalClient = pConnection->nLocalClient;
    if ((unsigned int)nLocalClient >= RT_DME_CLIENT_COUNT) {
        return;
    }
    for (nObject = 0; nObject < g_nDmeObjectCount; ++nObject) {
        if (g_anDmeObjectState[nObject] != RT_DME_OBJECT_USED) {
            continue;
        }
        if (RtDmeGetObject(&pObject, nObject) != 0 || pObject == NULL) {
            continue;
        }
        if (pObject->nLockClient != nLocalClient &&
            (pObject->nLockClient != RT_DME_ALL_CLIENTS || pObject->nOwner != nLocalClient)) {
            continue;
        }
        if (pObject->uLastFullTime == 0 ||
            (pObject->uFullInterval != 0 &&
             pObject->uLastFullTime + pObject->uFullInterval < g_uDmeSessionTime)) {
            RtDmeSendObject(RT_DME_ALL_CLIENTS, nObject);
            continue;
        }
        for (i = 0; i < RT_DME_CLIENT_COUNT; ++i) {
            pRange = NULL;
            RtDmeGetObjectRange(&pRange, g_nDmeActiveConnection, i);
            RtDmeGetClient(g_nDmeActiveConnection, i, &pClient);
            if (pRange != NULL && pClient != NULL && i != nLocalClient &&
                pClient->bObjectsJoined == 1) {
                RtDmeSendObject(i, nObject);
            }
        }
        RtDmeSendObjectUpdate(nObject);
    }
    for (i = 0; i < RT_DME_CLIENT_COUNT; ++i) {
        RtDmeGetClient(g_nDmeActiveConnection, i, &pClient);
        if (pClient != NULL && i != nLocalClient && pClient->bObjectsJoined == 1) {
            pClient->bObjectsJoined = 0;
        }
    }
}

// Probe the addresses of the clients of connection 0 that are not resolved, and give up on a
// client whose probes go unanswered.
// NTSC-U/C: 0x002b72c0, PAL: 0x002c1230
static int RtDmeProbeAddresses(void) {
    RtDmeConnection *pConnection;
    RtDmeClient *pClient;
    RtDmeAddressProbe probe;
    unsigned int uElapsed;
    int nResult;
    int i;

    pConnection = NULL;
    pClient = NULL;
    if (!g_bDmeServiceStarted) {
        return 0;
    }
    nResult = RtDmeGetConnection(0, &pConnection);
    if (pConnection == NULL) {
        return 0;
    }
    if (nResult != 0) {
        return nResult;
    }
    for (i = 0; i < RT_DME_CLIENT_COUNT; ++i) {
        if (pConnection->nLocalClient != RT_DME_NO_CLIENT && i == pConnection->nLocalClient) {
            continue;
        }
        nResult = RtDmeGetClient(0, i, &pClient);
        if (nResult != 0) {
            return nResult;
        }
        if (pClient == NULL) {
            continue;
        }
        if (RtDmeGetAddressIndex(0, i) != RT_DME_ADDRESS_INDEX_UNRESOLVED && pClient->bJoined) {
            continue;
        }
        if (RtDmeGetClientAddressKind(0, i, RT_DME_ADDRESS_INDEX_PRIVATE) == 0 ||
            RtDmeGetClientAddressKind(0, i, RT_DME_ADDRESS_INDEX_PUBLIC) == 0) {
            continue;
        }
        RtGetMillisecondsSince(&pClient->lastProbe, &uElapsed);
        if (uElapsed < kAddressProbeInterval + 1) {
            continue;
        }
        RtDmeParseAddress(probe.abPrivateAddress,
                          g_dmeLocalAddresses[RT_DME_ADDRESS_INDEX_PRIVATE].address);
        RtDmeParseAddress(probe.abPublicAddress,
                          g_dmeLocalAddresses[RT_DME_ADDRESS_INDEX_PUBLIC].address);
        probe.uPublicPort = g_dmeLocalAddresses[RT_DME_ADDRESS_INDEX_PUBLIC].port;
        probe.uPrivatePort = g_dmeLocalAddresses[RT_DME_ADDRESS_INDEX_PRIVATE].port;
        RtDmeSendDatagram(kPingSendFlags,
                          RtDmeGetClientAddress(0, i, RT_DME_ADDRESS_INDEX_PRIVATE),
                          RtDmeGetClientPort(0, i, RT_DME_ADDRESS_INDEX_PRIVATE),
                          RT_DME_CLASS_SYSTEM,
                          g_nDmeAddressProbeMessage,
                          sizeof(probe),
                          &probe);
        RtDmeSendDatagram(kPingSendFlags,
                          RtDmeGetClientAddress(0, i, RT_DME_ADDRESS_INDEX_PUBLIC),
                          RtDmeGetClientPort(0, i, RT_DME_ADDRESS_INDEX_PUBLIC),
                          RT_DME_CLASS_SYSTEM,
                          g_nDmeAddressProbeMessage,
                          sizeof(probe),
                          &probe);
        RtReadClock(&pClient->lastProbe);
        ++pClient->nProbeCount;
        if ((unsigned int)pClient->nProbeCount < kAddressProbeLimit) {
            continue;
        }
        pClient->uAddressIndex = RT_DME_ADDRESS_INDEX_NONE;
        nResult = RtDmeDropClientPeers(0, i);
        if (nResult != 0) {
            return nResult;
        }
        memset(pClient, 0, kClientAddressBytes);
        nResult = RtDmeGetClient(0, pConnection->nLocalClient, &pClient);
        if (nResult != 0 || pClient == NULL) {
            return nResult;
        }
        if (pClient->nRole == RT_DME_ROLE_HOST) {
            RtDmeElectHost(0, 0, 0, 0);
        }
    }
    return 0;
}

// Pump the channels of the connections, their voice, and the datagram service.
// NTSC-U/C: 0x002b7550, PAL: 0x002c14c0
static int RtDmePumpChannels(unsigned char bProcess) {
    RtDmeConnection *pConnection;
    int nLastResult;
    int nResult;
    unsigned char uConnection;

    pConnection = NULL;
    nLastResult = 0;
    memset(&g_dmePumpResults, 0, sizeof(g_dmePumpResults));
    g_dmePumpResults.nSize = sizeof(g_dmePumpResults);
    for (uConnection = 0; uConnection < RT_DME_CONNECTION_COUNT; ++uConnection) {
        nResult = RtDmeGetConnection(uConnection, &pConnection);
        if (nResult == 0) {
            if (pConnection == NULL) {
                g_dmePumpResults.anConnections[uConnection] = 0;
                continue;
            }
            if (uConnection != 0 && (pConnection->nState == RT_DME_CONNECTION_CONNECTED ||
                                     pConnection->nState == RT_DME_CONNECTION_CONNECTING)) {
                nResult = RtDmeConvertResult(
                    RtChannelApiPumpOne(RtDmeConnectionToChannel(uConnection), bProcess),
                    RT_DME_RESULT_SERVICE);
            }
            if (nResult == 0 && RtDmeVoiceIsOpen(uConnection) == 1) {
                nResult = RtDmeVoiceUpdate(uConnection, bProcess);
            }
            if (nResult == 0) {
                continue;
            }
        }
        nLastResult = nResult;
        g_dmePumpResults.anConnections[uConnection] = nResult;
    }
    if (g_bDmeServiceStarted != 1) {
        return nLastResult;
    }
    nResult = RtDmeConvertResult(RtChannelApiReceiveDatagrams(bProcess), RT_DME_RESULT_SERVICE);
    if (nResult != 0) {
        g_dmePumpResults.nDatagrams = nResult;
        return nResult;
    }
    nResult = RtDmeGetConnection(0, &pConnection);
    if (pConnection == NULL || nResult != 0) {
        return nLastResult;
    }
    if (pConnection->nState == RT_DME_CONNECTION_CONNECTED ||
        pConnection->nState == RT_DME_CONNECTION_CONNECTING) {
        g_dmePumpResults.anConnections[0] = g_dmePumpResults.nDatagrams;
    }
    return nLastResult;
}

// Expire the latency probes left unanswered.
// NTSC-U/C: 0x002b8bb0, PAL: 0x002c2b20
static int RtDmeExpirePings(void) {
    char szAddress[RT_DME_ADDRESS_SIZE];
    RtDmePing *pPing;
    unsigned int uNow;
    unsigned int i;

    RtGetMilliseconds(&uNow);
    for (i = 0; i < RT_DME_PING_COUNT; ++i) {
        pPing = &g_aDmePings[i];
        if (pPing->done.pfnDone == NULL || (int)(uNow - pPing->uSentTime - kPingTimeout) <= 0) {
            continue;
        }
        RtDmeFormatAddress(szAddress, pPing->abAddress);
        RtDmeFormatAddress(g_szDmeSenderAddress, pPing->abAddress);
        g_uDmeSenderPort = pPing->uPort;
        if (pPing->bClientProbe == 1) {
            pPing->done.pfnClientDone(
                RT_DME_RESULT_TIMED_OUT, szAddress, 0, 0, pPing->nConnection, pPing->nClient);
        } else {
            pPing->done.pfnDone(RT_DME_RESULT_TIMED_OUT, szAddress, 0, 0);
        }
        pPing->done.pfnDone = NULL;
    }
    return 0;
}

// Find the joined client of connection 0 that this console watches, the one before it.
// NTSC-U/C: 0x002b89b0, PAL: 0x002c2920
static int RtDmeFindWatchedClient(int *pnClient) {
    RtDmeConnection *pConnection;
    RtDmeClient *pClient;
    int nLocalClient;
    int nResult;
    int i;

    pConnection = NULL;
    pClient = NULL;
    if (pnClient == NULL) {
        return RT_DME_RESULT_INVALID;
    }
    *pnClient = RT_DME_NO_CLIENT;
    nResult = RtDmeGetConnection(0, &pConnection);
    if (nResult != 0 || pConnection == NULL) {
        return nResult;
    }
    if (pConnection->nState != RT_DME_CONNECTION_CONNECTED) {
        return 0;
    }
    nLocalClient = pConnection->nLocalClient;
    if (nLocalClient == RT_DME_NO_CLIENT) {
        return 0;
    }
    for (i = nLocalClient - 1; i >= 0; --i) {
        nResult = RtDmeGetClient(0, i, &pClient);
        if (nResult != 0) {
            return nResult;
        }
        if (pClient != NULL && pClient->bJoined == 1) {
            *pnClient = i;
            break;
        }
    }
    if (*pnClient != RT_DME_NO_CLIENT) {
        return 0;
    }
    for (i = RT_DME_CLIENT_COUNT - 1; i >= 0 && i != nLocalClient; --i) {
        nResult = RtDmeGetClient(0, i, &pClient);
        if (nResult != 0) {
            return nResult;
        }
        if (pClient != NULL && pClient->bJoined == 1) {
            *pnClient = i;
            break;
        }
    }
    return 0;
}

// Drop the client this console watches when it went silent.
// NTSC-U/C: 0x002b8910, PAL: 0x002c2880
static int RtDmeExpireWatchedClient(void) {
    RtDmeConnection *pConnection;
    int nClient;
    int nResult;

    pConnection = NULL;
    nResult = RtDmeGetConnection(0, &pConnection);
    if (nResult != 0 || pConnection == NULL) {
        return nResult;
    }
    if (pConnection->nState != RT_DME_CONNECTION_CONNECTED ||
        pConnection->nLocalClient == RT_DME_NO_CLIENT) {
        return 0;
    }
    nResult = RtDmeFindWatchedClient(&nClient);
    if (nResult != 0) {
        return nResult;
    }
    if (nClient == RT_DME_NO_CLIENT || nClient == pConnection->nLocalClient) {
        return 0;
    }
    nResult = RtDmeExpireClient(nClient, kWatchedClientTimeout, 1);
    if (nResult != 0) {
        return nResult;
    }
    return 0;
}

// Probe the clients of connection 0 that do not take datagrams, every few seconds.
// NTSC-U/C: 0x002b8cf0, PAL: 0x002c2c60
static int RtDmeProbeSilentClients(void) {
    RtDmeConnection *pConnection;
    RtDmeClient *pClient;
    unsigned int uElapsed;
    signed char nFlags;
    int nResult;
    int i;

    pConnection = NULL;
    pClient = NULL;
    nResult = RtDmeGetConnection(0, &pConnection);
    if (nResult != 0 || pConnection == NULL) {
        return nResult;
    }
    if (pConnection->nLocalClient == RT_DME_NO_CLIENT ||
        pConnection->nState != RT_DME_CONNECTION_CONNECTED) {
        return 0;
    }
    uElapsed = 0;
    RtGetMillisecondsSince(&g_dmeLastPeerProbe, &uElapsed);
    if (uElapsed < kPeerProbeInterval) {
        return 0;
    }
    if (g_nDmeActiveConnection < RT_DME_CONNECTION_COUNT) {
        nFlags = 0;
        for (i = nResult; i < pConnection->nMaxClients; ++i) {
            nResult = RtDmeGetClient(0, i, &pClient);
            if (nResult != 0) {
                return nResult;
            }
            if (pClient == NULL || RtDmeGetClientFlags(0, i, &nFlags) != 0) {
                continue;
            }
            if (pClient->bJoined != 1 || i == pConnection->nLocalClient ||
                (nFlags & RT_DME_CLIENT_FLAG_DATAGRAMS)) {
                continue;
            }
            nResult = RtDmeProbe(0, i, RtDmeIgnorePing);
            if (nResult != 0) {
                return nResult;
            }
        }
    } else {
        for (i = 0; i < pConnection->nMaxClients; ++i) {
            nResult = RtDmeGetClient(0, i, &pClient);
            if (nResult != 0) {
                return nResult;
            }
            if (pClient == NULL || pClient->bJoined != 1 || i == pConnection->nLocalClient) {
                continue;
            }
            nResult = RtDmeProbe(0, i, RtDmeIgnorePing);
            if (nResult != 0) {
                return nResult;
            }
        }
    }
    RtReadTime(&g_dmeLastPeerProbe);
    return 0;
}

// Run the polls of the listener for peers, reporting their results when bNotify is 1.
// NTSC-U/C: 0x002b8f78, PAL: 0x002c2ee8
static int RtDmePumpPolls(unsigned char bNotify) {
    RtDmePoll *pPoll;
    unsigned int uElapsed;
    bool bReady;
    int nResult;
    unsigned int i;

    if (g_bDmeListening != 1) {
        return 0;
    }
    nResult = RtDmeConvertResult(RtChannelApiAcceptPeers(), RT_DME_RESULT_SERVICE);
    if (nResult != 0) {
        return nResult;
    }
    for (i = 0; i < RT_DME_POLL_COUNT; ++i) {
        pPoll = &g_aDmePolls[i];
        switch (pPoll->uState) {
        case kPollFree:
            break;
        case kPollStarted:
            bReady = false;
            if (RtChannelApiPollListener(&bReady, pPoll->szAddress, pPoll->uPort) != 0) {
                break;
            }
            if (!bReady) {
                break;
            }
            RtReadClock(&pPoll->startTime);
            pPoll->uState = kPollWaiting;
            break;
        case kPollWaiting:
            RtGetMillisecondsSince(&pPoll->startTime, &uElapsed);
            if (uElapsed < kPollTimeout + 1) {
                break;
            }
            pPoll->uState = kPollFailed;
            if (bNotify != 1) {
                break;
            }
            if (pPoll->pfnDone != NULL) {
                pPoll->pfnDone(RT_DME_RESULT_TIMED_OUT, pPoll->szAddress, 0);
                pPoll->pfnDone = NULL;
            }
            pPoll->uState = kPollFree;
            break;
        case kPollSucceeded:
            if (bNotify != 1) {
                break;
            }
            if (pPoll->pfnDone != NULL) {
                pPoll->pfnDone(0, pPoll->szAddress, pPoll->nValue);
                pPoll->pfnDone = NULL;
            }
            pPoll->uState = kPollFree;
            break;
        case kPollFailed:
            if (bNotify != 1) {
                break;
            }
            if (pPoll->pfnDone != NULL) {
                pPoll->pfnDone(RT_DME_RESULT_TIMED_OUT, pPoll->szAddress, 0);
                pPoll->pfnDone = NULL;
            }
            pPoll->uState = kPollFree;
            break;
        default:
            pPoll->uState = kPollFree;
            break;
        }
    }
    return 0;
}

// Fail the pending polls of the listener and close it.
// NTSC-U/C: 0x002b91c0, PAL: 0x002c3130
static int RtDmeStopPolls(void) {
    RtDmePoll *pPoll;
    int nResult;
    unsigned int i;

    for (i = 0; i < RT_DME_POLL_COUNT; ++i) {
        pPoll = &g_aDmePolls[i];
        if (pPoll->uState == kPollStarted || pPoll->uState == kPollWaiting) {
            pPoll->uState = kPollFailed;
        }
        if (pPoll->uState != kPollFailed) {
            continue;
        }
        if (pPoll->pfnDone != NULL) {
            pPoll->pfnDone(RT_DME_RESULT_TIMED_OUT, pPoll->szAddress, 0);
            pPoll->pfnDone = NULL;
        }
        pPoll->uState = kPollFree;
    }
    if (g_bDmeListening == 1) {
        nResult = RtDmeConvertResult(RtChannelApiCloseListener(), RT_DME_RESULT_SERVICE);
        if (nResult != 0) {
            return nResult;
        }
        g_bDmeListening = 0;
    }
    memset(g_aDmePolls, 0, sizeof(g_aDmePolls));
    return 0;
}

// Report connection 0 connected once every client has a working address, or failed when a
// client has none.
// NTSC-U/C: 0x002b92d8, PAL: 0x002c3248
static int RtDmeCheckPeersResolved(void) {
    RtDmeConnection *pConnection;
    RtDmeClient *pClient;
    int nUnresolved;
    int nResult;
    int i;

    pConnection = NULL;
    pClient = NULL;
    nResult = RtDmeGetConnection(0, &pConnection);
    if (nResult != 0 || pConnection == NULL) {
        return nResult;
    }
    if (pConnection->nLocalClient == RT_DME_NO_CLIENT) {
        return 0;
    }
    nResult = RtDmeGetClient(0, pConnection->nLocalClient, &pClient);
    if (nResult != 0 || pClient == NULL) {
        return nResult;
    }
    if (pClient->bJoined) {
        return 0;
    }
    nUnresolved = 0;
    for (i = 0; i < RT_DME_CLIENT_COUNT; ++i) {
        nResult = RtDmeGetClient(0, i, &pClient);
        if (nResult != 0) {
            return nResult;
        }
        if (pClient == NULL || i == pConnection->nLocalClient) {
            continue;
        }
        if (RtDmeGetAddressIndex(0, i) == RT_DME_ADDRESS_INDEX_NONE) {
            RtDmeSetConnectResult(0, 0, 0, 1);
            RtDmeDisconnect(0);
            return 0;
        }
        if (RtDmeGetClientAddressKind(0, i, RT_DME_ADDRESS_INDEX_PRIVATE) == 0 ||
            RtDmeGetClientAddressKind(0, i, RT_DME_ADDRESS_INDEX_PUBLIC) == 0) {
            continue;
        }
        if (RtDmeGetAddressIndex(0, i) == RT_DME_ADDRESS_INDEX_UNRESOLVED) {
            ++nUnresolved;
        }
    }
    if (nUnresolved == 0) {
        RtDmeSetConnectResult(0, pConnection->nLocalClient, 1, 0);
    }
    return 0;
}

// Register the clients of connection 0 whose addresses resolved, after a connection result.
// NTSC-U/C: 0x002b9450, PAL: 0x002c33c0
static int RtDmeJoinResolvedPeers(void) {
    RtDmeConnection *pConnection;
    RtDmeClient *pClient;
    int nIndex;
    int nResult;
    int i;

    pConnection = NULL;
    pClient = NULL;
    nResult = RtDmeGetConnection(0, &pConnection);
    if (nResult != 0 || pConnection == NULL) {
        return nResult;
    }
    if (pConnection->nLocalClient == RT_DME_NO_CLIENT) {
        return 0;
    }
    nResult = RtDmeGetClient(0, pConnection->nLocalClient, &pClient);
    if (nResult != 0 || pClient == NULL) {
        return nResult;
    }
    if (g_bDmeConnectWaiting || !g_bDmeConnectReported) {
        return 0;
    }
    for (i = 0; i < RT_DME_CLIENT_COUNT; ++i) {
        nResult = RtDmeGetClient(0, i, &pClient);
        if (nResult != 0) {
            return nResult;
        }
        if (pClient == NULL || i == pConnection->nLocalClient) {
            continue;
        }
        if (pClient->bJoined) {
            pClient->bJoined = 0;
        }
        nIndex = RtDmeGetAddressIndex(0, i);
        // Yes, the binary tests the joined flag it has just cleared.
        if ((unsigned int)nIndex >= RT_DME_ADDRESS_COUNT || pClient->bJoined) {
            continue;
        }
        if (RtDmeGetClientAddressKind(0, i, RT_DME_ADDRESS_INDEX_PRIVATE) == 0 ||
            RtDmeGetClientAddressKind(0, i, RT_DME_ADDRESS_INDEX_PUBLIC) == 0) {
            continue;
        }
        RtDmeAddClient(0, i, RtDmeGetClientAddress(0, i, nIndex));
    }
    g_bDmeConnectReported = 0;
    return 0;
}

// Report the session of the active connection ready once every joined client joined its objects.
// NTSC-U/C: 0x002b98a0, PAL: 0x002c3810
static int RtDmeCheckSessionReady(void) {
    RtDmeConnection *pConnection;
    RtDmeClient *pClient;
    RtDmeSessionReady ready;
    int nResult;
    int i;

    pConnection = NULL;
    pClient = NULL;
    if (g_nDmeActiveConnection < 0) {
        return RT_DME_RESULT_OUT_OF_RANGE;
    }
    if (g_nDmeActiveConnection >= RT_DME_CONNECTION_COUNT || g_bDmeObjectRangesSent) {
        return 0;
    }
    nResult = RtDmeGetConnection(g_nDmeActiveConnection, &pConnection);
    if (nResult != 0 || pConnection == NULL) {
        return nResult;
    }
    if ((unsigned int)pConnection->nLocalClient >= RT_DME_CLIENT_COUNT) {
        return RT_DME_RESULT_OUT_OF_RANGE;
    }
    nResult = RtDmeGetClient(g_nDmeActiveConnection, pConnection->nLocalClient, &pClient);
    if (nResult != 0 || pClient == NULL) {
        return nResult;
    }
    if (pClient->nRole != kRoleObjectsJoined && pClient->nRole != kRoleObjectsJoinedHost) {
        return 0;
    }
    for (i = 0; i < RT_DME_CLIENT_COUNT; ++i) {
        if (i == pConnection->nLocalClient) {
            continue;
        }
        nResult = RtDmeGetClient(g_nDmeActiveConnection, i, &pClient);
        if (nResult != 0) {
            return nResult;
        }
        if (pClient != NULL && pClient->bJoined == 1 && pClient->nRole == 0) {
            return 0;
        }
    }
    g_bDmeObjectRangesSent = 1;
    if (g_pfnDmeSessionReady != NULL) {
        ready.nEvent = 0;
        ready.nArgument = g_nDmeSessionReadyArgument;
        g_pfnDmeSessionReady(&ready);
        g_pfnDmeSessionReady = NULL;
        g_nDmeSessionReadyArgument = 0;
    }
    return 0;
}

int RtDmeUpdate(void) {
    RtDmeConnection *pConnection;
    RtDmeClient *pClient;
    unsigned int uStart;
    unsigned int uNow;
    int nResult;
    int i;

    if (!g_nDmeRunning) {
        return RT_DME_RESULT_NOT_STARTED;
    }
    RtGetMilliseconds(&uStart); // Yes, the binary discards this time.
    nResult = RtDmeCloseFailedConnections();
    if (nResult != 0) {
        return nResult;
    }
    g_bDmeReceiveError = 0;
    nResult = RtDmePumpChannels(1);
    if (nResult != 0) {
        return nResult;
    }
    if (g_bDmeReceiveError == 1) {
        return RT_DME_RESULT_ERROR_RECEIVED;
    }
    RtGetMilliseconds(&uNow);
    g_uDmeSessionTime += uNow - g_uDmeLastUpdateTime;
    g_uDmeLastUpdateTime = uNow;
    RtDmeProbeAddresses();
    if (g_nDmeActiveConnection < RT_DME_CONNECTION_COUNT) {
        pConnection = NULL;
        pClient = NULL;
        nResult = RtDmeGetConnection(g_nDmeActiveConnection, &pConnection);
        if (nResult != 0 || pConnection == NULL) {
            return nResult;
        }
        if (g_uDmeLastLatencyProbe + kLatencyProbeInterval < g_uDmeSessionTime) {
            if (pConnection->nState != RT_DME_CONNECTION_CONNECTED) {
                goto expire;
            }
            if (pConnection->nLocalClient < pConnection->nMaxClients) {
                for (i = 0; i < pConnection->nMaxClients; ++i) {
                    nResult = RtDmeGetClient(g_nDmeActiveConnection, i, &pClient);
                    if (nResult != 0) {
                        return nResult;
                    }
                    if (pClient != NULL && i != pConnection->nLocalClient &&
                        pClient->bJoined == 1) {
                        RtDmeProbeClient(i);
                    }
                }
                g_uDmeLastLatencyProbe = g_uDmeSessionTime;
            }
        }
        if (pConnection->nState == RT_DME_CONNECTION_CONNECTED &&
            pConnection->nLocalClient < pConnection->nMaxClients && g_bDmeObjectsShared == 1) {
            RtDmeSendObjects();
            RtDmeFlushStreams();
        }
    }
expire:
    nResult = RtDmeExpirePings();
    if (nResult != 0) {
        return nResult;
    }
    nResult = RtDmeExpireWatchedClient();
    if (nResult != 0) {
        return nResult;
    }
    nResult = RtDmeProbeSilentClients();
    if (nResult != 0) {
        return nResult;
    }
    nResult = RtDmePumpPolls(1);
    if (nResult != 0) {
        return nResult;
    }
    nResult = RtDmeCheckPeersResolved();
    if (nResult != 0) {
        return nResult;
    }
    nResult = RtDmeJoinResolvedPeers();
    if (nResult != 0) {
        return nResult;
    }
    nResult = RtDmeCheckSessionReady();
    if (nResult != 0) {
        return nResult;
    }
    return RtDmeExpireReassembly();
}

int RtDmeDisconnect(int nConnection) {
    RtDmeConnection *pConnection;
    RtDmeClient *pClient;
    RtTime start;
    unsigned int uElapsed;
    unsigned char abFarewell[2];
    int nResult;
    int i;

    pConnection = NULL;
    pClient = NULL;
    if ((unsigned int)nConnection >= RT_DME_CONNECTION_COUNT) {
        return RT_DME_RESULT_INVALID;
    }
    nResult = RtDmeGetConnection(nConnection, &pConnection);
    if (nResult != 0 || pConnection == NULL) {
        return nResult;
    }
    if (RtDmeVoiceIsOpen(nConnection) == 1) {
        RtDmeVoiceClose(nConnection);
    }
    if (pConnection->nState >= RT_DME_CONNECTION_CONNECTED &&
        pConnection->nState <= RT_DME_CONNECTION_FAILED) {
        if (nConnection == 0) {
            abFarewell[0] = 0;
            abFarewell[1] = RtDmeClientToByte(pConnection->nLocalClient);
            RtDmeBroadcastDatagram(0,
                                   1,
                                   kStreamSendFlags,
                                   0,
                                   RT_DME_CLASS_SYSTEM,
                                   g_nDmeDisconnectMessage,
                                   sizeof(abFarewell),
                                   abFarewell);
            RtReadTime(&start);
            do {
                nResult = RtDmeConvertResult(RtChannelApiPumpAll(0), RT_DME_RESULT_SERVICE);
                if (nResult != 0) {
                    return nResult;
                }
                RtGetMillisecondsSince(&start, &uElapsed);
            } while (uElapsed < kDisconnectDrain);
            RtDmeSetHostClient(nConnection, RT_DME_NO_CLIENT);
            for (i = 0; i < RT_DME_CLIENT_COUNT; ++i) {
                if (i != pConnection->nLocalClient) {
                    RtDmeDropClientPeers(nConnection, i);
                }
            }
            g_nDmeHostClient = RT_DME_NO_CLIENT;
        } else {
            if (g_bDmeServiceStarted) {
                for (i = 0; i < RT_DME_CLIENT_COUNT; ++i) {
                    if (i != pConnection->nLocalClient) {
                        RtDmeDropClientPeers(nConnection, i);
                    }
                }
            }
            RtChannelApiClose(RtDmeConnectionToChannel(nConnection));
        }
    }
    for (i = 0; i < RT_DME_CLIENT_COUNT; ++i) {
        RtDmeGetClient(nConnection, i, &pClient);
        if (pClient != NULL) {
            memset(pClient, 0, sizeof(RtDmeClient));
        }
    }
    pConnection->nState = RT_DME_CONNECTION_IDLE;
    pConnection->nLocalClient = RT_DME_NO_CLIENT;
    pConnection->nHostClient = RT_DME_NO_CLIENT;
    pConnection->pfnClientJoin = NULL;
    pConnection->pfnClientLeave = NULL;
    pConnection->nPendingJoins = 0;
    pConnection->nWorldId = -1;
    return 0;
}

int RtDmeShutdown(void) {
    RtDmeConnection *pConnection;
    int nResult;
    int nClass;
    int nType;
    int i;

    pConnection = NULL;
    if (g_nDmeRunning != 1) {
        return RT_DME_RESULT_NOT_STARTED;
    }
    RtDmeStopPolls();
    for (i = 0; i < RT_DME_CONNECTION_COUNT; ++i) {
        nResult = RtDmeGetConnection(i, &pConnection);
        if (nResult != 0) {
            return nResult;
        }
        if (pConnection != NULL && pConnection->nState == RT_DME_CONNECTION_CONNECTED) {
            RtDmeDisconnect(i);
        }
    }
    RtDmeDestroyAllObjects();
    RtDmeFreeAllObjectRanges();
    RtDmeFreeObjectRangeTable();
    for (nClass = 0; nClass < RT_DME_MESSAGE_CLASS_COUNT; ++nClass) {
        for (nType = 0; nType < RT_DME_MESSAGE_TYPE_COUNT; ++nType) {
            if (g_apDmeMessageTypes[nClass][nType] != NULL) {
                RtDmeFree((void **)&g_apDmeMessageTypes[nClass][nType]);
            }
        }
    }
    g_bDmeServiceStarted = 0;
    if (g_pDmeStreams != NULL) {
        RtDmeFreeStreams();
        RtDmeFree((void **)&g_pDmeStreams);
        g_pDmeStreams = NULL;
    }
    for (i = 0; i < RT_DME_CONNECTION_COUNT; ++i) {
        for (nType = 0; nType < RT_DME_CLIENT_COUNT; ++nType) {
            RtDmeFreeClient(i, nType);
        }
        RtDmeFreeConnection(i);
    }
    RtDmeFreeAllClientStats();
    RtChannelApiShutdown();
    RtDmeVoiceShutdown();
    g_nDmeRunning = 0;
    return 0;
}

int RtDmeExpireClient(int nClient, unsigned int uTimeout, unsigned char bAnnounce) {
    RtDmeClient *pClient;
    unsigned int uElapsed;
    const char *pszAddress;
    int nResult;

    pClient = NULL;
    uElapsed = 0;
    if (nClient == RT_DME_NO_CLIENT || nClient >= RT_DME_CLIENT_COUNT) {
        return RT_DME_RESULT_INVALID;
    }
    nResult = RtDmeGetClient(0, nClient, &pClient);
    if (nResult != 0 || pClient == NULL) {
        return nResult;
    }
    RtGetMillisecondsSince(&pClient->lastHeard, &uElapsed);
    if (uTimeout < uElapsed) {
        pszAddress = RtDmeGetClientAddress(0, nClient, RtDmeGetAddressIndex(0, nClient));
        if (pszAddress != NULL) {
            nResult = RtDmeRemoveClient(0, nClient, pszAddress, bAnnounce);
            if (nResult != 0) {
                return nResult;
            }
        }
    }
    return 0;
}

int RtDmeFinishDeparture(int nConnection, int nClient) {
    RtDmeConnection *pConnection;
    RtDmeClient *pClient;
    int nResult;

    pConnection = NULL;
    pClient = NULL;
    if ((unsigned int)nConnection >= RT_DME_CONNECTION_COUNT) {
        return RT_DME_RESULT_INVALID;
    }
    if (RtDmeIsClientIndex(nClient) != 1) {
        return RT_DME_RESULT_INVALID;
    }
    if (nConnection != 0) {
        return RT_DME_RESULT_INVALID;
    }
    nResult = RtDmeGetConnection(0, &pConnection);
    if (nResult != 0 || pConnection == NULL) {
        return nResult;
    }
    if (pConnection->nLocalClient == RT_DME_NO_CLIENT) {
        RtDmeSetConnectResult(0, 0, 0, 1);
        RtDmeDisconnect(0);
        return 0;
    }
    nResult = RtDmeGetClient(0, pConnection->nLocalClient, &pClient);
    if (nResult != 0 || pClient == NULL) {
        return nResult;
    }
    // While this console still joins, losing the host fails the connection.
    if (pClient->bJoined || g_nDmeHostClient != nClient) {
        return 0;
    }
    RtDmeSetConnectResult(0, 0, 0, 1);
    RtDmeDisconnect(0);
    return 0;
}

int RtDmeDropClientPeers(int nConnection, int nClient) {
    const char *pszAddress;
    const char *pszOther;
    unsigned char uKind;
    int nShared;
    int i;

    nShared = 0;
    if ((unsigned int)nConnection >= RT_DME_CONNECTION_COUNT) {
        return RT_DME_RESULT_INVALID;
    }
    if (RtDmeIsClientIndex(nClient) != 1) {
        return RT_DME_RESULT_INVALID;
    }
    if (g_bDmeServiceStarted != 1) {
        return 0;
    }
    if (RtDmeGetClientAddressKind(nConnection, nClient, RT_DME_ADDRESS_INDEX_PUBLIC) != 0) {
        RtChannelApiDropPeer(
            RtDmeGetClientAddress(nConnection, nClient, RT_DME_ADDRESS_INDEX_PUBLIC),
            RtDmeGetClientPort(nConnection, nClient, RT_DME_ADDRESS_INDEX_PUBLIC));
    }
    for (i = 0; i < RT_DME_CLIENT_COUNT; ++i) {
        if (i == nClient) {
            continue;
        }
        pszAddress = RtDmeGetClientAddress(nConnection, nClient, RT_DME_ADDRESS_INDEX_PRIVATE);
        pszOther = RtDmeGetClientAddress(nConnection, i, RT_DME_ADDRESS_INDEX_PRIVATE);
        if (pszAddress == NULL || pszOther == NULL) {
            continue;
        }
        if (RtDmeGetClientAddressKind(nConnection, i, RT_DME_ADDRESS_INDEX_PRIVATE) == 0) {
            continue;
        }
        if (strncmp(RtDmeGetClientAddress(nConnection, nClient, RT_DME_ADDRESS_INDEX_PRIVATE),
                    RtDmeGetClientAddress(nConnection, i, RT_DME_ADDRESS_INDEX_PRIVATE),
                    RT_DME_ADDRESS_SIZE) != 0) {
            continue;
        }
        if (RtDmeGetClientPort(nConnection, nClient, RT_DME_ADDRESS_INDEX_PRIVATE) ==
            RtDmeGetClientPort(nConnection, i, RT_DME_ADDRESS_INDEX_PRIVATE)) {
            ++nShared;
        }
    }
    uKind = RtDmeGetClientAddressKind(nConnection, nClient, RT_DME_ADDRESS_INDEX_PRIVATE);
    if (uKind == 0) {
        return 0;
    }
    // Another client shares the private address. Yes, the binary returns the kind here.
    if (nShared != 0) {
        return uKind;
    }
    RtChannelApiDropPeer(RtDmeGetClientAddress(nConnection, nClient, RT_DME_ADDRESS_INDEX_PRIVATE),
                         RtDmeGetClientPort(nConnection, nClient, RT_DME_ADDRESS_INDEX_PRIVATE));
    return 0;
}

// The system messages and their receivers.

enum {
    // The send flags of the introductions of the clients and of the grants of the host.
    kIntroductionSendFlags = RT_DME_SEND_ORDERED | RT_DME_SEND_GUARANTEED,
    kLockSendFlags = RT_DME_SEND_GUARANTEED,
    // The answers of a host to a console that asked to join.
    kSessionFull = 4,
    // The sizes of the system messages that the receivers report consumed.
    kClientFlagsSize = 1,
    kDisconnectSize = 2,
    kObjectIndexSize = 2,
    kWordSize = 4,
    kObjectRequestSize = 6,
    kObjectGrantSize = 8,
    kPingSize = 8,
    kHostReplySize = 8,
    kAddressProbeSize = 12,
    kClientStateSize = 12,
    kClientAddressesSize = 16,
    kClientInfoSize = 16,
    kObjectRangeSize = 36,
    kIgnoredSize = 68,
    kJoinAnnouncementSize = 72,
    kVoicePeerSize = 28,
    // The stream rate a received stream starts with.
    kStreamStartRate = 90000,
    // The milliseconds a long message waits for its next part.
    kReassemblyTimeout = 30000,
    // The number of long messages reassembled at once.
    kReassemblyCount = 256,
    // The states of an entry of the reassembly table.
    kReassemblyFree = 0,
    kReassemblyUsed = 2,
    // The rank that wraps the ranks of the candidate hosts back to 1.
    kHostRankLimit = 0x0fffffff,
    // The state of an object reserved for a client.
    kObjectReserved = 1,
    // The roles a client announces.
    kRoleNone = 0,
    kRoleObjectsReleased = 1,
    // The events of the receiver of a ready session.
    kSessionLeft = 7,
    kSessionNoRange = 22,
    // The number of probes a client gets when it is admitted.
    kAdmitProbeCount = 3,
    // The results of the helpers of the introductions and of the reassembly.
    kReserveTooMany = 256,
    kReassemblyTableFull = 25,
    kClientInfoTooManyClients = 30,
};

// A long message being reassembled from its parts. 44 bytes.
typedef struct {
    int nState;
    int nPartCount;
    int nTotalSize;
    int nReceived;
    int nConnection;
    int nSender;
    int nMessageId;
    unsigned char uClass;
    unsigned char uType;
    unsigned char abReserved1e[2];
    unsigned int uLastTime;
    unsigned char *pbParts;
    unsigned char *pbData;
} RtDmeReassembly;

// The addresses of a client and its place in the session, sent by the host.
typedef struct {
    RtDmeAddressProbe addresses;
    unsigned char uConnection;
    unsigned char uClient;
    unsigned char uHost;
    unsigned char uMaxClients;
} RtDmeClientInfo;

// The request of a client for its objects and its stream slots.
typedef struct {
    unsigned char uClient;
    unsigned char uReserved01;
    unsigned short uObjectCount;
    unsigned short uStreamCount;
} RtDmeObjectRequest;

// The grant of the host of the objects and the stream slots of a client.
typedef struct {
    unsigned int uTime;
    unsigned short uFirstObject;
    unsigned short uFirstStream;
} RtDmeObjectGrant;

// The client that holds an object.
typedef struct {
    unsigned char uClient;
    unsigned char uReserved01; // Yes, the binary sends this byte uninitialised.
    unsigned short uObject;
} RtDmeObjectLock;

// A client that reports itself ready to the host.
typedef struct {
    unsigned char uReserved00;
    unsigned char uClient;
    unsigned char auState[2];
} RtDmeClientReady;

// The answer of a host to a console that asked for it.
typedef struct {
    int nPort;
    unsigned char abAddress[kAddressBytes];
} RtDmeHostReply;

// A system message the engine receives and ignores.
typedef struct {
    unsigned char abHeader[kAddressBytes];
    RtKey body;
} RtDmeIgnoredMessage;

// The words of an address the engine passes by value.
typedef union {
    unsigned int uWord;
    unsigned char abBytes[kAddressBytes];
} RtDmeAddressWord;

// NTSC-U/C: 0x00435758
unsigned char g_abDmeFixedAddress[kAddressBytes];
// NTSC-U/C: 0x0043575c
RtDmeHostReplyFunc g_pfnDmeHostReply;
// NTSC-U/C: 0x00435764
int g_nDmeObjectMode;
// NTSC-U/C: 0x00435768
int g_nDmeClientStateMessage;
// NTSC-U/C: 0x0043576c
int g_nDmeClientFlagsMessage;
// NTSC-U/C: 0x00435770
int g_nDmeClientJoinMessage;
// NTSC-U/C: 0x00435774
int g_nDmeFragmentMessage;
// NTSC-U/C: 0x00435778
int g_nDmeObjectDestroyMessage;
// NTSC-U/C: 0x0043577c
int g_nDmeClientAddressesMessage;
// NTSC-U/C: 0x00435780
int g_nDmeHostQueryMessage;
// NTSC-U/C: 0x00435784
int g_nDmeClientReadyMessage;
// NTSC-U/C: 0x00435794
int g_nDmeObjectLockMessage;
// NTSC-U/C: 0x00435798
int g_nDmeObjectLockedMessage;
// NTSC-U/C: 0x0043579c
int g_nDmeObjectUnlockMessage;
// NTSC-U/C: 0x004357b4
int g_nDmeHostReplyMessage;
// NTSC-U/C: 0x004357b8
int g_nDmeSessionFullMessage;
// NTSC-U/C: 0x004357bc
int g_nDmeClientInfoMessage;
// NTSC-U/C: 0x004357c0
int g_nDmeSessionNameMessage;
// NTSC-U/C: 0x004357c4
int g_nDmeClockMessage;
// NTSC-U/C: 0x004357c8
int g_nDmeObjectRequestMessage;
// NTSC-U/C: 0x004357cc
int g_nDmeIgnoredMessage;
// NTSC-U/C: 0x004357d0
int g_nDmeObjectGrantMessage;
// NTSC-U/C: 0x004357d4
int g_nDmeHostRankMessage;
// NTSC-U/C: 0x0051d9b0
static RtDmeReassembly g_aDmeReassembly[kReassemblyCount];
// NTSC-U/C: 0x0053f060
RtDmeStreamDataFunc g_apfnDmeStreamData[RT_DME_STREAM_KIND_COUNT];
// NTSC-U/C: 0x0053f0a0
RtDmeStreamClosedFunc g_apfnDmeStreamClosed[RT_DME_STREAM_KIND_COUNT];

// Reserve a run of free objects.
// NTSC-U/C: 0x002b9b68, PAL: 0x002c3ad8
static int RtDmeReserveObjects(int nCount) {
    unsigned char bFree;
    int nLast;
    int i;
    int j;

    if ((unsigned int)nCount >= RT_DME_OBJECT_COUNT) {
        return kReserveTooMany; // Yes, the binary reports 256, not -1, for a count out of range.
    }
    nLast = RT_DME_OBJECT_COUNT - nCount;
    i = 0;
    while (i < nLast) {
        if (g_anDmeObjectState[i] != RT_DME_OBJECT_FREE) {
            ++i;
            continue;
        }
        bFree = 1;
        for (j = i + 1; j < i + nCount; ++j) {
            if (g_anDmeObjectState[j] != RT_DME_OBJECT_FREE) {
                bFree = 0;
                break;
            }
        }
        if (!bFree) {
            ++i;
            continue;
        }
        for (j = i; j < i + nCount; ++j) {
            g_anDmeObjectState[j] = kObjectReserved;
        }
        return i;
    }
    return -1;
}

// Find a run of free stream slots.
// NTSC-U/C: 0x002b9c60, PAL: 0x002c3bd0
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
static int RtDmeFindFreeStreams(unsigned int nCount) {
    RtDmeStream *pStream;
    unsigned char bFree;
    int nLast;
    int i;
    int j;

    if (nCount >= RT_DME_STREAM_COUNT) {
        return -1;
    }
    nLast = RT_DME_STREAM_COUNT - nCount;
    i = 0;
    while (i < nLast) {
        // Yes, the binary reads the slot through a pointer a failed lookup leaves unset.
        RtDmeFindStream(&pStream, i);
        if (pStream->uState != RT_DME_STREAM_FREE) {
            ++i;
            continue;
        }
        bFree = 1;
        for (j = i + 1; j < i + (int)nCount; ++j) {
            RtDmeFindStream(&pStream, j);
            if (pStream->uState != RT_DME_STREAM_FREE) {
                bFree = 0;
                break;
            }
        }
        if (bFree) {
            return i;
        }
        ++i;
    }
    return -1;
}

#pragma GCC diagnostic pop

// Pick the next rank of a candidate host, one past the highest a client of a connection has.
// NTSC-U/C: 0x002bd9d0, PAL: 0x002c7940
static int RtDmeGetNextHostRank(unsigned int nConnection, int *pnRank) {
    RtDmeClient *pClient;
    int nResult;
    int i;

    pClient = NULL;
    if (pnRank == NULL) {
        return RT_DME_RESULT_INVALID;
    }
    *pnRank = 0;
    if (nConnection >= RT_DME_CONNECTION_COUNT) {
        return RT_DME_RESULT_INVALID;
    }
    for (i = 0; i < RT_DME_CLIENT_COUNT; ++i) {
        nResult = RtDmeGetClient(nConnection, i, &pClient);
        if (nResult != 0) {
            return nResult;
        }
        if (pClient != NULL && *pnRank < pClient->nHostRank) {
            *pnRank = pClient->nHostRank;
        }
    }
    ++*pnRank;
    if (*pnRank == 0 || *pnRank == kHostRankLimit) {
        *pnRank = 1;
    }
    return 0;
}

// Record which address of a client of connection 0 the last datagram came from.
// NTSC-U/C: 0x002bda98, PAL: 0x002c7a08
static void RtDmeResolveSenderAddress(unsigned int nConnection, int nClient) {
    if (nConnection >= RT_DME_CONNECTION_COUNT || RtDmeIsClientIndex(nClient) != 1 ||
        nConnection != 0) {
        return;
    }
    if (strcmp(g_szDmeSenderAddress,
               RtDmeGetClientAddress(0, nClient, RT_DME_ADDRESS_INDEX_PRIVATE)) == 0 &&
        g_uDmeSenderPort == RtDmeGetClientPort(0, nClient, RT_DME_ADDRESS_INDEX_PRIVATE)) {
        RtDmeSetAddressIndex(0, nClient, RT_DME_ADDRESS_INDEX_PRIVATE);
        return;
    }
    if (strcmp(g_szDmeSenderAddress,
               RtDmeGetClientAddress(nConnection, nClient, RT_DME_ADDRESS_INDEX_PUBLIC)) == 0 &&
        g_uDmeSenderPort == RtDmeGetClientPort(nConnection, nClient, RT_DME_ADDRESS_INDEX_PUBLIC)) {
        RtDmeSetAddressIndex(nConnection, nClient, RT_DME_ADDRESS_INDEX_PUBLIC);
    }
}

// Grant a client that joined its objects and stream slots, when this console is the host.
// NTSC-U/C: 0x002bd7e0, PAL: 0x002c7750
static void RtDmeAdmitClient(unsigned int nConnection,
                             int nClient,
                             unsigned int nObjectCount,
                             unsigned int nStreamCount) {
    RtDmeConnection *pConnection;
    RtDmeClient *pClient;
    RtDmeObjectRange *pRange;
    RtDmeObjectGrant grant;
    int nHost;
    int nRank;
    int i;

    pConnection = NULL;
    pClient = NULL;
    if (nConnection >= RT_DME_CONNECTION_COUNT || RtDmeIsClientIndex(nClient) != 1) {
        return;
    }
    RtDmeGetHostClient(&nHost, nConnection);
    if (nHost == RT_DME_NO_CLIENT || (unsigned int)nHost >= RT_DME_CLIENT_COUNT) {
        return;
    }
    if (RtDmeGetConnection(nConnection, &pConnection) != 0 || pConnection == NULL) {
        return;
    }
    if (RtDmeGetClient(nConnection, nClient, &pClient) != 0 || pClient == NULL) {
        return;
    }
    if (pConnection->nLocalClient != nHost) {
        return;
    }
    pRange = NULL;
    RtDmeGetObjectRange(&pRange, nConnection, nClient);
    if (pRange != NULL) {
        return;
    }
    pRange = RtDmeCreateObjectRange(nConnection, nClient);
    if (pRange == NULL) {
        return;
    }
    if (nObjectCount != 0) {
        pRange->uFirstObject = RtDmeReserveObjects(nObjectCount);
    }
    if (nStreamCount != 0) {
        pRange->uFirstStream = RtDmeFindFreeStreams(nStreamCount);
    }
    grant.uFirstObject = pRange->uFirstObject;
    grant.uTime = g_uDmeSessionTime;
    grant.uFirstStream = pRange->uFirstStream;
    RtDmeSendSystemMessage(kIntroductionSendFlags,
                           nConnection,
                           nClient,
                           g_nDmeObjectGrantMessage,
                           sizeof(grant),
                           &grant);
    RtDmeGetNextHostRank(nConnection, &nRank);
    if (RtDmeGetClient(nConnection, nClient, &pClient) == 0 && pClient != NULL) {
        pClient->nHostRank = nRank;
    }
    RtDmeSendSystemMessage(
        kIntroductionSendFlags, nConnection, nClient, g_nDmeHostRankMessage, sizeof(nRank), &nRank);
    for (i = 0; i < kAdmitProbeCount; ++i) {
        RtDmeProbeClient(nClient);
    }
}

// Take a peer that probed this host into connection 0, or refuse it when the session is full.
// NTSC-U/C: 0x002bdbe0, PAL: 0x002c7b50
static void RtDmeAcceptPeer(unsigned int uPrivateAddress,
                            unsigned int uPublicAddress,
                            unsigned short uPrivatePort,
                            unsigned short uPublicPort) {
    RtDmeConnection *pConnection;
    rtDmeAddressReport report;
    rtDmeConnectStatus status;
    RtDmeAddressWord privateAddress;
    RtDmeAddressWord publicAddress;
    unsigned char bHost;
    int nCount;
    int nReply;

    privateAddress.uWord = uPrivateAddress;
    publicAddress.uWord = uPublicAddress;
    bHost = 0;
    pConnection = NULL;
    if (RtDmeIsLocalHost(&bHost) != 0 || bHost != 1) {
        return;
    }
    if (RtDmeGetConnection(0, &pConnection) != 0 || pConnection == NULL) {
        return;
    }
    RtDmeCountJoinedClients(&nCount, 0);
    if (nCount >= pConnection->nMaxClients) {
        nReply = kSessionFull;
        RtDmeSendDatagram(RT_DME_SEND_BASE,
                          g_szDmeSenderAddress,
                          g_uDmeSenderPort,
                          RT_DME_CLASS_SYSTEM,
                          g_nDmeSessionFullMessage,
                          sizeof(nReply),
                          &nReply);
        return;
    }
    if (RtDmeInitAddressReport(&report) != 0) {
        return;
    }
    report.addresses[RT_DME_ADDRESS_INDEX_PUBLIC].type = bHost;
    report.addresses[RT_DME_ADDRESS_INDEX_PRIVATE].type = RT_DME_ADDRESS_PRIVATE;
    RtDmeFormatAddress(report.addresses[RT_DME_ADDRESS_INDEX_PRIVATE].address,
                       privateAddress.abBytes);
    RtDmeFormatAddress(report.addresses[RT_DME_ADDRESS_INDEX_PUBLIC].address,
                       publicAddress.abBytes);
    report.addresses[RT_DME_ADDRESS_INDEX_PRIVATE].port = uPrivatePort;
    report.addresses[RT_DME_ADDRESS_INDEX_PUBLIC].port = uPublicPort;
    RtDmeAddReportedPeer(&report, &status);
}

// Describe a client of a connection, for the introductions of the host.
// NTSC-U/C: 0x002bdd08, PAL: 0x002c7c78
static int RtDmeBuildClientInfo(RtDmeClientInfo *pInfo, unsigned int nConnection, int nClient) {
    RtDmeConnection *pConnection;
    int nResult;

    pConnection = NULL;
    if (pInfo == NULL || nConnection >= RT_DME_CONNECTION_COUNT ||
        RtDmeIsClientIndex(nClient) != 1) {
        return RT_DME_RESULT_INVALID;
    }
    nResult = RtDmeGetConnection(nConnection, &pConnection);
    if (nResult != 0 || pConnection == NULL) {
        return nResult;
    }
    RtDmeParseAddress(pInfo->addresses.abPrivateAddress,
                      RtDmeGetClientAddress(nConnection, nClient, RT_DME_ADDRESS_INDEX_PRIVATE));
    RtDmeParseAddress(pInfo->addresses.abPublicAddress,
                      RtDmeGetClientAddress(nConnection, nClient, RT_DME_ADDRESS_INDEX_PUBLIC));
    pInfo->addresses.uPrivatePort =
        RtDmeGetClientPort(nConnection, nClient, RT_DME_ADDRESS_INDEX_PRIVATE);
    pInfo->addresses.uPublicPort =
        RtDmeGetClientPort(nConnection, nClient, RT_DME_ADDRESS_INDEX_PUBLIC);
    pInfo->uConnection = nConnection;
    pInfo->uClient = RtDmeClientToByte(nClient);
    pInfo->uHost = RtDmeClientToByte(g_nDmeHostClient);
    if (pConnection->nMaxClients >= RT_DME_CLIENT_COUNT) {
        return kClientInfoTooManyClients;
    }
    pInfo->uMaxClients = pConnection->nMaxClients;
    return 0;
}

// Find the reassembly of a long message, or start one. Stale entries met on the way are freed.
// NTSC-U/C: 0x002bde30, PAL: 0x002c7da0
static int RtDmeGetReassembly(int nConnection,
                              int nSender,
                              const RtDmeFragmentHeader *pHeader,
                              RtDmeReassembly **ppReassembly) {
    RtDmeReassembly *pReassembly;
    int nFree;
    int i;

    nFree = -1;
    if (ppReassembly == NULL) {
        return RT_DME_RESULT_INVALID;
    }
    *ppReassembly = NULL;
    if (pHeader == NULL || nConnection < 0) {
        return RT_DME_RESULT_INVALID;
    }
    if (nConnection >= RT_DME_CONNECTION_COUNT && nConnection != RT_DME_SERVER_CLIENT) {
        return RT_DME_RESULT_INVALID;
    }
    if ((unsigned int)(nSender + 1) > RT_DME_CLIENT_COUNT && nSender != RT_DME_SERVER_CLIENT) {
        return RT_DME_RESULT_INVALID;
    }
    for (i = 0; i < kReassemblyCount; ++i) {
        pReassembly = &g_aDmeReassembly[i];
        if (pReassembly->nState == kReassemblyFree) {
            if (nFree == -1) {
                nFree = i;
            }
            continue;
        }
        if (pReassembly->nState == kReassemblyUsed) {
            // Yes, the binary goes on after a match, and a later match replaces it.
            if (nConnection == pReassembly->nConnection && nSender == pReassembly->nSender &&
                pHeader->uCount == pReassembly->nPartCount &&
                pHeader->nTotalSize == pReassembly->nTotalSize &&
                pHeader->uMessageId == pReassembly->nMessageId &&
                pHeader->uClass == pReassembly->uClass && pHeader->uType == pReassembly->uType) {
                *ppReassembly = pReassembly;
            }
            continue;
        }
        if (pReassembly->pbData != NULL) {
            RtDmeFree((void **)&pReassembly->pbData);
            pReassembly->pbData = NULL;
        }
        if (pReassembly->pbParts != NULL) {
            RtDmeFree((void **)&pReassembly->pbParts);
            pReassembly->pbParts = NULL;
        }
        pReassembly->nState = kReassemblyFree;
        if (nFree == -1) {
            nFree = i;
        }
    }
    if (*ppReassembly != NULL) {
        return 0;
    }
    if (nFree == -1) {
        return kReassemblyTableFull;
    }
    pReassembly = &g_aDmeReassembly[nFree];
    pReassembly->nPartCount = pHeader->uCount;
    pReassembly->nState = kReassemblyUsed;
    pReassembly->nConnection = nConnection;
    pReassembly->nTotalSize = pHeader->nTotalSize;
    pReassembly->nReceived = 0;
    pReassembly->nSender = nSender;
    pReassembly->nMessageId = pHeader->uMessageId;
    pReassembly->uClass = pHeader->uClass;
    pReassembly->uType = pHeader->uType;
    pReassembly->pbData = RtDmeAlloc(pHeader->nTotalSize);
    pReassembly->pbParts = RtDmeAlloc(pHeader->uCount);
    if (pReassembly->pbData == NULL || pReassembly->pbParts == NULL) {
        if (pReassembly->pbData != NULL) {
            RtDmeFree((void **)&pReassembly->pbData);
            pReassembly->pbData = NULL;
        }
        if (pReassembly->pbParts != NULL) {
            RtDmeFree((void **)&pReassembly->pbParts);
            pReassembly->pbParts = NULL;
        }
        pReassembly->nState = kReassemblyFree;
        return RT_DME_RESULT_NO_MEMORY;
    }
    *ppReassembly = pReassembly;
    return 0;
}

// The flags a client announces.
// NTSC-U/C: 0x002b9d50, PAL: 0x002c3cc0
static int RtDmeOnClientFlags(int nConnection, int nWorldId, int nSender, rtDmeMessage *pMessage) {
    const signed char *pnFlags;

    (void)nWorldId;
    pnFlags = (const signed char *)pMessage;
    if (pnFlags == NULL) {
        return -1;
    }
    if ((unsigned int)nConnection >= RT_DME_CONNECTION_COUNT ||
        (unsigned int)nSender >= RT_DME_CLIENT_COUNT) {
        return kClientFlagsSize;
    }
    // Yes, the binary checks both ranges a second time, signed.
    if (nConnection >= RT_DME_CONNECTION_COUNT || nSender >= RT_DME_CLIENT_COUNT) {
        return kClientFlagsSize;
    }
    if (RtDmeSetClientFlags(nConnection, nSender, *pnFlags) != 0 || nConnection != 0) {
        return kClientFlagsSize;
    }
    if ((*pnFlags & RT_DME_CLIENT_FLAG_DATAGRAMS) && g_bDmeObjectsShared == 1) {
        RtDmeSendObjectRange(0, nSender);
    }
    return kClientFlagsSize;
}

// The announcement of a client to its peers, with its public key.
// NTSC-U/C: 0x002b9e10, PAL: 0x002c3d80
static int RtDmeOnClientJoin(int nConnection, int nWorldId, int nSender, rtDmeMessage *pMessage) {
    const RtDmeJoinAnnouncement *pJoin;
    RtDmeConnection *pConnection;
    char szAddress[RT_DME_ADDRESS_SIZE];
    int nTarget;

    (void)nWorldId;
    (void)nSender;
    pJoin = (const RtDmeJoinAnnouncement *)pMessage;
    pConnection = NULL;
    if (pJoin == NULL) {
        return -1;
    }
    if (RtDmeGetConnection(nConnection, &pConnection) != 0 || pConnection == NULL) {
        return -1;
    }
    RtDmeFormatAddress(szAddress, pJoin->abAddress);
    nTarget = nConnection;
    if ((signed char)pJoin->uConnection == 0) {
        nTarget = 0;
    }
    if (strcmp(szAddress, g_dmeLocalAddresses[RT_DME_ADDRESS_INDEX_PUBLIC].address) != 0 ||
        pJoin->uClient != pConnection->nLocalClient) {
        if (nConnection == RT_DME_SERVER_CLIENT) {
            RtStorePeerPublicKey(RT_SECURITY_SERVER_ID, pJoin->uClient, &pJoin->modulus);
        } else if (nConnection == 0) {
            RtStorePeerPublicKey(0, pJoin->uClient, &pJoin->modulus);
        } else {
            RtStorePeerPublicKey(
                RtDmeConnectionToChannel(nConnection), pJoin->uClient, &pJoin->modulus);
        }
    }
    if (RtDmeGetAddressIndex((signed char)pJoin->uConnection, pJoin->uClient) ==
            RT_DME_ADDRESS_INDEX_UNRESOLVED &&
        (signed char)pJoin->uConnection == 0) {
        RtDmeResolveSenderAddress(0, pJoin->uClient);
        if ((unsigned int)RtDmeGetAddressIndex((signed char)pJoin->uConnection, pJoin->uClient) >=
            RT_DME_ADDRESS_INDEX_UNRESOLVED) {
            return kJoinAnnouncementSize;
        }
    }
    RtDmeAddClient(nTarget, pJoin->uClient, szAddress);
    return kJoinAnnouncementSize;
}

// A console that disconnects.
// NTSC-U/C: 0x002b9f50, PAL: 0x002c3ec0
static int RtDmeOnDisconnect(int nConnection, int nWorldId, int nSender, rtDmeMessage *pMessage) {
    const unsigned char *pbFarewell;
    RtDmeConnection *pConnection;
    RtDmeClient *pClient;
    int nIndex;

    (void)nWorldId;
    pbFarewell = (const unsigned char *)pMessage;
    pConnection = NULL;
    pClient = NULL;
    if (pbFarewell == NULL) {
        return -1;
    }
    if (pbFarewell[0] >= RT_DME_CONNECTION_COUNT) {
        return kDisconnectSize;
    }
    nIndex = RtDmeGetAddressIndex((signed char)pbFarewell[0], pbFarewell[1]);
    if (RtDmeGetClient((signed char)pbFarewell[0], pbFarewell[1], &pClient) != 0 ||
        pClient == NULL) {
        return kDisconnectSize;
    }
    if (RtDmeGetConnection((signed char)pbFarewell[0], &pConnection) != 0) {
        return kDisconnectSize;
    }
    if (pConnection == NULL) {
        return kDisconnectSize;
    }
    if ((unsigned int)nIndex >= RT_DME_ADDRESS_INDEX_UNRESOLVED && pClient->bJoined != 1) {
        return kDisconnectSize;
    }
    if (nConnection != 0 || pConnection->nLocalClient == pbFarewell[1] ||
        nSender != pbFarewell[1]) {
        return kDisconnectSize;
    }
    RtDmeRemoveClient((signed char)pbFarewell[0],
                      pbFarewell[1],
                      RtDmeGetClientAddress((signed char)pbFarewell[0], nSender, nIndex),
                      1);
    return kDisconnectSize;
}

// The name the server gave the session.
// NTSC-U/C: 0x002ba068, PAL: 0x002c3fd8
static int RtDmeOnSessionName(int nConnection, int nWorldId, int nSender, rtDmeMessage *pMessage) {
    RtDmeConnection *pConnection;
    int nResult;

    (void)nWorldId;
    (void)nSender;
    pConnection = NULL;
    if (pMessage == NULL) {
        return -1;
    }
    if ((unsigned int)nConnection >= RT_DME_CONNECTION_COUNT) {
        return RT_DME_ADDRESS_SIZE;
    }
    nResult = RtDmeGetConnection(nConnection, &pConnection);
    if (nResult != 0) {
        return -1;
    }
    if (pConnection == NULL) {
        return nResult; // Yes, the binary reports 0 bytes consumed.
    }
    strncpy(pConnection->szSessionName, (const char *)pMessage, RT_DME_ADDRESS_SIZE);
    return RT_DME_ADDRESS_SIZE;
}

// A client that left, whose objects are released.
// NTSC-U/C: 0x002ba0d8, PAL: 0x002c4048
static int RtDmeOnClientLeft(int nConnection, int nWorldId, int nSender, rtDmeMessage *pMessage) {
    (void)nWorldId;
    if (pMessage == NULL) {
        return -1;
    }
    if ((unsigned int)nConnection < RT_DME_CONNECTION_COUNT &&
        (unsigned int)nSender < RT_DME_CLIENT_COUNT) {
        RtDmeReleaseClientObjects(nConnection, nSender);
    }
    return kClientFlagsSize;
}

// The object ranges a client announces.
// NTSC-U/C: 0x002ba120, PAL: 0x002c4090
static int RtDmeOnObjectRange(int nConnection, int nWorldId, int nSender, rtDmeMessage *pMessage) {
    const RtDmeObjectRangeMessage *pRangeMessage;
    RtDmeConnection *pConnection;
    RtDmeClient *pClient;
    RtDmeObjectRange *pRange;
    unsigned int nClient;

    (void)nWorldId;
    (void)nSender;
    pRangeMessage = (const RtDmeObjectRangeMessage *)pMessage;
    pRange = NULL;
    pConnection = NULL;
    pClient = NULL;
    if (pRangeMessage == NULL) {
        return -1;
    }
    if (RtDmeGetConnection(nConnection, &pConnection) != 0 || pConnection == NULL) {
        return kObjectRangeSize;
    }
    if (RtDmeGetClient(nConnection, pConnection->nLocalClient, &pClient) != 0 || pClient == NULL) {
        return kObjectRangeSize;
    }
    if (pClient->nRole != kRoleObjectsJoined && pClient->nRole != kRoleObjectsJoinedHost) {
        return kObjectRangeSize;
    }
    if ((unsigned int)nConnection >= RT_DME_CONNECTION_COUNT) {
        return kObjectRangeSize;
    }
    nClient = pRangeMessage->uClient;
    if (nClient >= RT_DME_CLIENT_COUNT) {
        return kObjectRangeSize;
    }
    RtDmeGetObjectRange(&pRange, nConnection, nClient);
    if (pRange == NULL) {
        pRange = RtDmeCreateObjectRange(nConnection, nClient);
        if (pRange == NULL) {
            return kObjectRangeSize;
        }
    }
    pRange->uObjectLimit = pRangeMessage->uObjectLimit;
    pRange->uFirstObject = pRangeMessage->uFirstObject;
    pRange->uObjectCount = pRangeMessage->uObjectCount;
    pRange->uFirstStream = pRangeMessage->uFirstStream;
    pRange->uStreamCount = pRangeMessage->uStreamCount;
    strcpy(pRange->szName, pRangeMessage->szName);
    if (pRange->uJoinTime == 0) {
        RtDmeJoinObjects(nConnection, nClient);
    }
    return kObjectRangeSize;
}

// The clock of the host.
// NTSC-U/C: 0x002ba270, PAL: 0x002c41e0
static int RtDmeOnClock(int nConnection, int nWorldId, int nSender, rtDmeMessage *pMessage) {
    const unsigned int *puTime;

    (void)nWorldId;
    puTime = (const unsigned int *)pMessage;
    if (puTime == NULL) {
        return -1;
    }
    if ((unsigned int)nConnection >= RT_DME_CONNECTION_COUNT ||
        (unsigned int)nSender >= RT_DME_CLIENT_COUNT) {
        return kWordSize;
    }
    if (RtDmeSetHostClient(nConnection, nSender) != 0) {
        return kWordSize;
    }
    RtDmeSetSessionTime(*puTime);
    return kWordSize;
}

// A console that asks whether this console hosts a session.
// NTSC-U/C: 0x002ba2d8, PAL: 0x002c4248
static int RtDmeOnHostQuery(int nConnection, int nWorldId, int nSender, rtDmeMessage *pMessage) {
    const int *pnApplicationId;
    RtDmeClient *pClient;
    RtDmeHostReply reply;
    char szSender[RT_DME_ADDRESS_SIZE];
    unsigned char abSender[kAddressBytes];
    unsigned char bHost;

    (void)nWorldId;
    pnApplicationId = (const int *)pMessage;
    pClient = NULL;
    bHost = 0;
    if (pnApplicationId == NULL) {
        return -1;
    }
    if (RtDmeIsLocalHost(&bHost) != 0) {
        return kWordSize;
    }
    rtDmeGetSenderAddress(abSender);
    RtDmeFormatAddress(szSender, abSender);
    if (!g_bDmeUseFixedAddress && !bHost) {
        return kWordSize;
    }
    if (*pnApplicationId != g_nDmeApplicationId) {
        return kWordSize;
    }
    if (nSender != RT_DME_NO_CLIENT && RtDmeGetClient(nConnection, nSender, &pClient) == 0 &&
        pClient != NULL &&
        ((unsigned int)RtDmeGetAddressIndex(nConnection, nSender) <
             RT_DME_ADDRESS_INDEX_UNRESOLVED ||
         pClient->bJoined == 1)) {
        RtDmeRemoveClient(nConnection, nSender, szSender, 1);
    }
    if (g_bDmeUseFixedAddress) {
        memcpy(reply.abAddress, g_abDmeFixedAddress, kAddressBytes);
    } else {
        RtDmeParseAddress(reply.abAddress,
                          g_dmeLocalAddresses[RT_DME_ADDRESS_INDEX_PRIVATE].address);
    }
    if (bHost == 1) {
        reply.nPort = g_uDmePort;
    } else {
        // Yes, the binary answers with the fixed-address flag in place of a port.
        reply.nPort = g_bDmeUseFixedAddress;
    }
    RtDmeSendDatagramToBytes(
        0, abSender, RT_DME_CLASS_SYSTEM, g_nDmeHostReplyMessage, sizeof(reply), &reply);
    return kWordSize;
}

// The answer of a host to the query of this console.
// NTSC-U/C: 0x002ba470, PAL: 0x002c43e0
static int RtDmeOnHostReply(int nConnection, int nWorldId, int nSender, rtDmeMessage *pMessage) {
    const RtDmeHostReply *pReply;
    char szAddress[RT_DME_ADDRESS_SIZE];

    (void)nConnection;
    (void)nWorldId;
    (void)nSender;
    pReply = (const RtDmeHostReply *)pMessage;
    if (pReply == NULL) {
        return -1;
    }
    if (g_pfnDmeHostReply != NULL) {
        RtDmeFormatAddress(szAddress, pReply->abAddress);
        g_pfnDmeHostReply(szAddress, pReply->nPort);
    }
    return kHostReplySize;
}

// A system message the engine ignores.
// NTSC-U/C: 0x002ba4d8, PAL: 0x002c4448
static int RtDmeOnIgnored(int nConnection, int nWorldId, int nSender, rtDmeMessage *pMessage) {
    (void)nConnection;
    (void)nWorldId;
    (void)nSender;
    (void)pMessage;
    return kIgnoredSize;
}

// The request of a client for its objects and its stream slots.
// NTSC-U/C: 0x002ba4e0, PAL: 0x002c4450
static int
RtDmeOnObjectRequest(int nConnection, int nWorldId, int nSender, rtDmeMessage *pMessage) {
    const RtDmeObjectRequest *pRequest;

    (void)nWorldId;
    (void)nSender;
    pRequest = (const RtDmeObjectRequest *)pMessage;
    if (pRequest == NULL) {
        return -1;
    }
    RtDmeAdmitClient(
        nConnection, pRequest->uClient, pRequest->uObjectCount, pRequest->uStreamCount);
    return kObjectRequestSize;
}

// The grant of the host of the objects and the stream slots of this console.
// NTSC-U/C: 0x002ba518, PAL: 0x002c4488
static int RtDmeOnObjectGrant(int nConnection, int nWorldId, int nSender, rtDmeMessage *pMessage) {
    const RtDmeObjectGrant *pGrant;
    RtDmeConnection *pConnection;
    RtDmeClient *pClient;
    RtDmeObjectRange *pRange;
    RtDmeSessionReady ready;
    int nResult;
    int nLocal;

    (void)nWorldId;
    pGrant = (const RtDmeObjectGrant *)pMessage;
    pRange = NULL;
    pConnection = NULL;
    pClient = NULL;
    if (pGrant == NULL) {
        return -1;
    }
    if ((unsigned int)nConnection >= RT_DME_CONNECTION_COUNT ||
        (unsigned int)nSender >= RT_DME_CLIENT_COUNT) {
        return -1;
    }
    if (g_bDmeObjectsShared == 1) {
        return kObjectGrantSize;
    }
    nResult = RtDmeGetConnection(nConnection, &pConnection);
    if (nResult != 0) {
        return -1;
    }
    // Yes, the binary reports 0 bytes consumed for a connection that is not open.
    if (pConnection == NULL) {
        return nResult;
    }
    nLocal = pConnection->nLocalClient;
    if ((unsigned int)nLocal >= RT_DME_CLIENT_COUNT) {
        return nResult;
    }
    RtDmeGetObjectRange(&pRange, nConnection, nLocal);
    if (pRange == NULL) {
        return -1;
    }
    if (g_nDmeObjectMode == 1) {
        RtDmeLeaveSession(nConnection);
        if (g_pfnDmeSessionReady != NULL) {
            ready.nEvent = kSessionLeft;
            ready.nArgument = g_nDmeSessionReadyArgument;
            g_pfnDmeSessionReady(&ready);
            g_pfnDmeSessionReady = NULL;
            g_nDmeSessionReadyArgument = 0;
        }
        return kObjectGrantSize;
    }
    nResult = RtDmeGetClient(nConnection, nLocal, &pClient);
    if (nResult != 0) {
        return -1;
    }
    if (pClient == NULL) {
        return nResult;
    }
    pClient->nRole = kRoleObjectsJoinedHost;
    RtDmeSendClientState(nConnection, RT_DME_ALL_CLIENTS);
    RtDmeSetHostClient(nConnection, nSender);
    g_bDmeObjectsShared = 1;
    RtDmeSetSessionTime(pGrant->uTime);
    pRange->uJoinTime = g_uDmeSessionTime;
    pRange->uFirstObject = pGrant->uFirstObject;
    pRange->uFirstStream = pGrant->uFirstStream;
    RtDmeSendObjectRange(nConnection, RT_DME_ALL_CLIENTS);
    RtDmeOpenObjectRange(nConnection, nLocal);
    RtDmeEnableConnection(nConnection, 1);
    return kObjectGrantSize;
}

// A latency probe of a peer, or the answer to one of this console.
// NTSC-U/C: 0x002ba6f0, PAL: 0x002c4660
static int RtDmeOnPing(int nConnection, int nWorldId, int nSender, rtDmeMessage *pMessage) {
    const RtDmePingMessage *pPing;
    RtDmePingMessage reply;
    RtDmeClient *pClient;
    RtDmePing *pSlot;
    RtDmePingFunc pfnDone;
    char szAddress[RT_DME_ADDRESS_SIZE];
    unsigned int uNow;
    int nLatency;

    (void)nWorldId;
    pPing = (const RtDmePingMessage *)pMessage;
    if (pPing == NULL) {
        return -1;
    }
    if ((signed char)pPing->bRequest != 0) {
        if ((unsigned int)nConnection >= RT_DME_CONNECTION_COUNT || nSender == RT_DME_NO_CLIENT) {
            return kPingSize;
        }
        pClient = NULL;
        if (RtDmeGetClient(nConnection, nSender, &pClient) != 0 || pClient == NULL) {
            return -1;
        }
        if (pClient->bJoined != 1) {
            return kPingSize;
        }
        RtDmeGetSessionTime(&reply.uTime);
        reply.uSlot = pPing->uSlot;
        reply.bRequest = 0;
        rtDmeSendClassMessage(RT_DME_SEND_BASE,
                              nConnection,
                              nSender,
                              RT_DME_CLASS_SYSTEM,
                              g_nDmePingMessage,
                              sizeof(reply),
                              &reply);
        return kPingSize;
    }
    RtDmeGetLocalTime(&uNow); // Yes, the binary reads the time twice and drops the first.
    pSlot = &g_aDmePings[pPing->uSlot];
    pfnDone = pSlot->done.pfnDone;
    if (pfnDone == NULL) {
        return kPingSize;
    }
    RtDmeGetLocalTime(&uNow);
    nLatency = uNow - pSlot->uSentTime;
    RtDmeFormatAddress(szAddress, pSlot->abAddress);
    RtDmeFormatAddress(g_szDmeSenderAddress, pSlot->abAddress);
    g_uDmeSenderPort = pSlot->uPort;
    if (pSlot->bClientProbe == 1) {
        pSlot->done.pfnClientDone(
            0, szAddress, nLatency, pPing->uTime, pSlot->nConnection, pSlot->nClient);
    } else {
        pfnDone(0, szAddress, nLatency, pPing->uTime);
    }
    g_aDmePings[pPing->uSlot].done.pfnDone = NULL;
    return kPingSize;
}

// A part of a long message. The whole message goes to its receiver once every part arrived.
// NTSC-U/C: 0x002ba908, PAL: 0x002c4878
static int RtDmeOnFragment(int nConnection, int nWorldId, int nSender, rtDmeMessage *pMessage) {
    const RtDmeOutgoing *pFragment;
    RtDmeReassembly *pReassembly;
    RtDmeMessageType *pType;
    unsigned char bComplete;
    int i;

    pFragment = (const RtDmeOutgoing *)pMessage;
    pReassembly = NULL;
    bComplete = 1;
    if (pFragment == NULL) {
        return -1;
    }
    if (RtDmeGetReassembly(nConnection, nSender, &pFragment->fragment.header, &pReassembly) != 0) {
        return pFragment->fragment.header.uSize + sizeof(RtDmeFragmentHeader);
    }
    if (pReassembly == NULL) {
        return pFragment->fragment.header.uSize + sizeof(RtDmeFragmentHeader);
    }
    memcpy(&pReassembly->pbData[pFragment->fragment.header.nOffset],
           pFragment->fragment.abData,
           pFragment->fragment.header.uSize);
    pReassembly->nReceived += pFragment->fragment.header.uSize;
    RtGetMilliseconds(&pReassembly->uLastTime);
    pReassembly->pbParts[pFragment->fragment.header.uIndex] = 1;
    for (i = 0; i < pReassembly->nPartCount; ++i) {
        if (pReassembly->pbParts[i] == 0) {
            bComplete = 0;
            break;
        }
    }
    if (!bComplete) {
        return pFragment->fragment.header.uSize + sizeof(RtDmeFragmentHeader);
    }
    pType =
        RtDmeGetMessageType(pFragment->fragment.header.uClass, pFragment->fragment.header.uType);
    pType->pfnHandler(nConnection, nWorldId, nSender, (rtDmeMessage *)pReassembly->pbData);
    if (pReassembly->pbData != NULL) {
        RtDmeFree((void **)&pReassembly->pbData);
        pReassembly->pbData = NULL;
    }
    if (pReassembly->pbParts != NULL) {
        RtDmeFree((void **)&pReassembly->pbParts);
        pReassembly->pbParts = NULL;
    }
    pReassembly->nState = kReassemblyFree;
    return pFragment->fragment.header.uSize + sizeof(RtDmeFragmentHeader);
}

// An object to destroy.
// NTSC-U/C: 0x002baa98, PAL: 0x002c4a08
static int
RtDmeOnObjectDestroy(int nConnection, int nWorldId, int nSender, rtDmeMessage *pMessage) {
    (void)nConnection;
    (void)nWorldId;
    (void)nSender;
    if (pMessage == NULL) {
        return -1;
    }
    RtDmeDestroyObject(*(const unsigned short *)pMessage);
    return kObjectIndexSize;
}

// A client that asks to hold an object of this console.
// NTSC-U/C: 0x002baac0, PAL: 0x002c4a30
static int RtDmeOnObjectLock(int nConnection, int nWorldId, int nSender, rtDmeMessage *pMessage) {
    RtDmeConnection *pConnection;
    RtDmeObject *pObject;
    RtDmeObjectLock lock;
    unsigned int nObject;

    (void)nWorldId;
    pObject = NULL;
    pConnection = NULL;
    if (pMessage == NULL) {
        return -1;
    }
    if ((unsigned int)g_nDmeActiveConnection >= RT_DME_CONNECTION_COUNT) {
        return -1;
    }
    nObject = *(const unsigned short *)pMessage;
    if (nObject >= RT_DME_OBJECT_COUNT) {
        return -1;
    }
    RtDmeGetObject(&pObject, nObject);
    if (pObject == NULL) {
        return -1;
    }
    if (RtDmeGetConnection(g_nDmeActiveConnection, &pConnection) != 0 || pConnection == NULL) {
        return -1;
    }
    if (pObject->nOwner != pConnection->nLocalClient || pObject->nLockClient != -1) {
        return kObjectIndexSize;
    }
    pObject->nLockClient = nSender;
    RtDmeNotifyObjectChanged(pObject->nLockClient, nObject, kObjectChangedWhole);
    lock.uClient = RtDmeClientToByte(nSender);
    lock.uObject = nObject;
    RtDmeSendSystemMessage(kLockSendFlags,
                           nConnection,
                           RT_DME_ALL_CLIENTS,
                           g_nDmeObjectLockedMessage,
                           sizeof(lock),
                           &lock);
    return kObjectIndexSize;
}

// The client that holds an object.
// NTSC-U/C: 0x002babe0, PAL: 0x002c4b50
static int RtDmeOnObjectLocked(int nConnection, int nWorldId, int nSender, rtDmeMessage *pMessage) {
    const RtDmeObjectLock *pLock;
    RtDmeObject *pObject;
    int nClient;

    (void)nConnection;
    (void)nWorldId;
    (void)nSender;
    pLock = (const RtDmeObjectLock *)pMessage;
    if (pLock == NULL) {
        return -1;
    }
    nClient = pLock->uClient;
    // Yes, the binary does not clear the pointer before the lookup.
    RtDmeGetObject(&pObject, pLock->uObject);
    if (pObject != NULL) {
        pObject->nLockClient = nClient;
        RtDmeNotifyObjectChanged(pObject->nLockClient, pLock->uObject, kObjectChangedWhole);
    }
    return kWordSize;
}

// An object nobody holds.
// NTSC-U/C: 0x002bac48, PAL: 0x002c4bb8
static int RtDmeOnObjectUnlock(int nConnection, int nWorldId, int nSender, rtDmeMessage *pMessage) {
    const unsigned short *puObject;
    RtDmeObject *pObject;

    (void)nConnection;
    (void)nWorldId;
    (void)nSender;
    puObject = (const unsigned short *)pMessage;
    if (puObject == NULL) {
        return -1;
    }
    RtDmeGetObject(&pObject, *puObject);
    if (pObject == NULL) {
        return kObjectIndexSize;
    }
    pObject->nLockClient = -1;
    RtDmeNotifyObjectChanged(pObject->nLockClient, *puObject, kObjectChangedWhole);
    return kObjectIndexSize;
}

// Measure the fields of an object type in a message, for an object that is skipped.
static inline int RtDmeObjectFieldsSize(const RtDmeObjectType *pType) {
    const RtDmeObjectField *pField;
    int nSize;
    int i;

    nSize = kObjectHeaderSize;
    for (i = 0; i < pType->nFieldCount; ++i) {
        pField = pType->apFields[i];
        nSize += pField->nElementSize * pField->nElementCount;
    }
    return nSize;
}

// A whole object, new or changed.
// NTSC-U/C: 0x002baca8, PAL: 0x002c4c18
static int RtDmeOnWholeObject(int nConnection, int nWorldId, int nSender, rtDmeMessage *pMessage) {
    const RtDmeObjectMessage *pObjectMessage;
    const RtDmeObjectType *pType;
    const RtDmeObjectField *pField;
    const unsigned char *pbField;
    RtDmeConnection *pConnection;
    RtDmeObject *pObject;
    signed char *pnState;
    signed char nOldState;
    unsigned char bNew;
    int nOldCount;
    int nObject;
    int nSize;
    int nPosition;
    int i;

    (void)nWorldId;
    pObjectMessage = (const RtDmeObjectMessage *)pMessage;
    pObject = NULL;
    pConnection = NULL;
    bNew = 0;
    if (pObjectMessage == NULL) {
        return -1;
    }
    if ((unsigned int)g_nDmeActiveConnection >= RT_DME_CONNECTION_COUNT ||
        (unsigned int)nConnection >= RT_DME_CONNECTION_COUNT ||
        (unsigned int)nSender >= RT_DME_CLIENT_COUNT) {
        return -1;
    }
    if (RtDmeGetConnection(g_nDmeActiveConnection, &pConnection) != 0) {
        return -1;
    }
    if (pConnection == NULL) {
        return 0; // Yes, the binary reports 0 bytes consumed.
    }
    pType = &g_aDmeObjectTypes[pObjectMessage->header.uType];
    nObject = pObjectMessage->header.uObject;
    if (pObjectMessage->header.uOwner == pConnection->nLocalClient) {
        if (g_anDmeObjectState[nObject] == RT_DME_OBJECT_USED) {
            RtDmeGetObject(&pObject, nObject);
            goto update;
        }
        pObject = NULL;
        if (RtDmeFindObjectByName(
                pObjectMessage->header.szName, nConnection, pObjectMessage->header.uOwner) >= 0) {
            return RtDmeObjectFieldsSize(pType);
        }
        nObject = RtDmeFindFreeObject();
        if (nObject >= 0) {
            RtDmeGetObject(&pObject, nObject);
        }
    }
    // Yes, the binary indexes the states with -1 when no object is free, and with the object of
    // the message unchecked.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Warray-bounds"
    pnState = &g_anDmeObjectState[nObject];
#pragma GCC diagnostic pop
    nOldState = *pnState;
    if (nOldState == RT_DME_OBJECT_USED) {
        RtDmeGetObject(&pObject, nObject);
        goto update;
    }
    nOldCount = g_nDmeObjectCount;
    pObject = RtDmeAlloc(sizeof(RtDmeObject));
    if (pObject == NULL) {
        return kObjectHeaderSize;
    }
    *pnState = RT_DME_OBJECT_USED;
    g_apDmeObjects[nObject] = pObject;
    if (g_nDmeObjectCount < nObject + 1) {
        g_nDmeObjectCount = nObject + 1;
    }
    pObject->nOwner = pObjectMessage->header.uOwner;
    pObject->nReserved24 = 0;
    pObject->uFullInterval = 0;
    pObject->uLastFullTime = 0;
    pObject->pbData = RtDmeAlloc(pType->nDataSize);
    pObject->pbShadow = RtDmeAlloc(pType->nDataSize);
    if (pObject->pbData == NULL || pObject->pbShadow == NULL) {
        if (pObject->pbData != NULL) {
            RtDmeFree((void **)&pObject->pbData);
            pObject->pbData = NULL;
        }
        if (pObject->pbShadow != NULL) {
            RtDmeFree((void **)&pObject->pbShadow);
            pObject->pbShadow = NULL;
        }
        RtDmeFree((void **)&pObject);
        g_nDmeObjectCount = nOldCount;
        *pnState = nOldState;
        g_apDmeObjects[nObject] = NULL;
        return kObjectHeaderSize;
    }
    for (i = 0; i < pConnection->nMaxClients; ++i) {
        pObject->auClientSendTime[i] = 0;
    }
    bNew = 1;
update:
    if (pObject == NULL) {
        return kObjectHeaderSize;
    }
    pObject->nLockClient = (short)pObjectMessage->header.uLockClient;
    pObject->uType = pObjectMessage->header.uType;
    strcpy(pObject->szName, pObjectMessage->header.szName);
    pObject->uFilter = pObjectMessage->header.uFilter;
    pObject->uLifetime = pObjectMessage->header.uLifetime;
    pObject->uFlags = pObjectMessage->header.uFlags;
    nPosition = kObjectHeaderSize;
    pbField = &pObjectMessage->abBytes[kObjectHeaderSize];
    for (i = 0; i < pType->nFieldCount; ++i) {
        pField = pType->apFields[i];
        nSize = pField->nElementSize * pField->nElementCount;
        if (pObject != NULL) {
            memcpy(&pObject->pbData[pType->anFieldOffsets[i]], pbField, nSize);
        }
        pObject->auFieldSendTime[i] = g_uDmeSessionTime;
        nPosition += nSize;
        pbField += nSize;
    }
    memcpy(pObject->pbShadow, pObject->pbData, pType->nDataSize);
    pObject->uLastFullTime = g_uDmeSessionTime;
    if (bNew) {
        RtDmeNotifyObjectAdded(pObject->nOwner, nObject);
    } else {
        RtDmeNotifyObjectChanged(pObject->nOwner, nObject, kObjectChangedAll);
    }
    return nPosition;
}

// The changed fields of an object.
// NTSC-U/C: 0x002bb0d0, PAL: 0x002c5040
static int RtDmeOnObjectUpdate(int nConnection, int nWorldId, int nSender, rtDmeMessage *pMessage) {
    const RtDmeObjectUpdateMessage *pUpdate;
    const RtDmeObjectType *pType;
    const RtDmeObjectField *pField;
    const unsigned char *pbField;
    RtDmeObject *pObject;
    unsigned int uField;
    unsigned int uDone;
    int nSize;
    int nPosition;

    (void)nConnection;
    (void)nWorldId;
    pUpdate = (const RtDmeObjectUpdateMessage *)pMessage;
    uDone = 0;
    pObject = NULL;
    if (pUpdate == NULL) {
        return -1;
    }
    if ((unsigned int)nSender >= RT_DME_CLIENT_COUNT ||
        pUpdate->header.uObject >= RT_DME_OBJECT_COUNT) {
        return -1;
    }
    nPosition = kObjectUpdateHeaderSize;
    pbField = &pUpdate->abBytes[kObjectUpdateHeaderSize];
    if (g_anDmeObjectState[pUpdate->header.uObject] != RT_DME_OBJECT_USED) {
        // Skip the fields of an object this console does not have.
        pType = &g_aDmeObjectTypes[pUpdate->header.uType];
        if (pUpdate->header.uCount == 0) {
            return nPosition;
        }
        do {
            uField = *pbField++;
            if ((int)uField >= pType->nFieldCount) {
                return -1;
            }
            pField = pType->apFields[uField];
            if (pField == NULL) {
                return -1;
            }
            ++nPosition;
            ++uDone;
            nSize = pField->nElementSize * pField->nElementCount;
            nPosition += nSize;
            pbField += nSize;
        } while (uDone < pUpdate->header.uCount);
        return nPosition;
    }
    if (RtDmeGetObject(&pObject, pUpdate->header.uObject) != 0 || pObject == NULL) {
        return -1;
    }
    pType = &g_aDmeObjectTypes[pObject->uType];
    if (pUpdate->header.uCount == 0) {
        return nPosition;
    }
    do {
        uField = *pbField++;
        if ((int)uField >= pType->nFieldCount) {
            return -1;
        }
        pField = pType->apFields[uField];
        if (pField == NULL) {
            return -1;
        }
        ++nPosition;
        ++uDone;
        nSize = pField->nElementSize * pField->nElementCount;
        memcpy(&pObject->pbData[pType->anFieldOffsets[uField]], pbField, nSize);
        memcpy(&pObject->pbShadow[pType->anFieldOffsets[uField]],
               pbField,
               pField->nElementSize * pField->nElementCount);
        pObject->auFieldSendTime[uField] = g_uDmeSessionTime;
        nSize = pField->nElementSize * pField->nElementCount;
        nPosition += nSize;
        pbField += nSize;
        RtDmeNotifyObjectChanged(pObject->nOwner, pUpdate->header.uObject, uField);
    } while (uDone < pUpdate->header.uCount);
    return nPosition;
}

// A part of a stream.
// NTSC-U/C: 0x002bb338, PAL: 0x002c52a8
static int RtDmeOnStreamData(int nConnection, int nWorldId, int nSender, rtDmeMessage *pMessage) {
    const RtDmeStreamMessage *pStreamMessage;
    const unsigned char *pbData;
    RtDmeStream *pStream;
    RtDmeObjectRange *pRange;
    RtDmeStreamDataFunc pfnData;
    unsigned int uFree;
    unsigned int uSize;
    int nHost;
    int i;

    (void)nWorldId;
    pStreamMessage = (const RtDmeStreamMessage *)pMessage;
    nHost = RT_DME_NO_CLIENT;
    pStream = NULL;
    pRange = NULL;
    if (pStreamMessage == NULL) {
        return -1;
    }
    if (RtDmeGetStream(pStreamMessage->header.uStream, &pStream) != 0 || pStream == NULL) {
        return pStreamMessage->header.uSize + kStreamHeaderSize;
    }
    pbData = &pStreamMessage->abBytes[kStreamHeaderSize];
    if (pStream->uState != RT_DME_STREAM_OPEN) {
        if (pStreamMessage->header.bOwnsData) {
            pStream->pbData = RtDmeAlloc(pStreamMessage->header.uStart);
            if (pStream->pbData == NULL) {
                pStream->uState = RT_DME_STREAM_FREE;
                return pStreamMessage->header.uSize + kStreamHeaderSize;
            }
            if (RtInitRingBuffer(&pStream->queue, pStreamMessage->header.uStart, pStream->pbData) !=
                0) {
                RtDmeFree((void **)&pStream->pbData);
                pStream->pbData = NULL;
                return pStreamMessage->header.uSize + kStreamHeaderSize;
            }
            pStream->pbStart = &pStream->pbData[pStreamMessage->header.uStart];
            pStream->uRoute = pStreamMessage->header.uRoute;
            pStream->bToHost = pStreamMessage->header.bToHost;
            pStream->pbEnd = pStream->pbData;
            pStream->nReserved1c = 0;
            pStream->uKind = pStreamMessage->header.uKind;
            pStream->bOwnsData = pStreamMessage->header.bOwnsData;
            pStream->bClosing = 0;
            pStream->nRate = kStreamStartRate;
            pStream->uLastSendTime = 0;
        }
        pStream->uState = RT_DME_STREAM_OPEN;
        RtDmeGetHostClient(&nHost, nConnection);
        if (nSender != nHost) {
            pStream->nReserved0c = nSender;
        } else {
            for (i = 0; i < RT_DME_CLIENT_COUNT; ++i) {
                if (RtDmeGetObjectRange(&pRange, nConnection, i) != 0 || pRange == NULL) {
                    continue;
                }
                // Yes, the binary compares signed, so a stream below the range matches too.
                if ((int)pStreamMessage->header.uStream - pRange->uFirstStream <
                    pRange->uStreamCount) {
                    pStream->nReserved0c = i;
                    break;
                }
            }
        }
    }
    if (pStreamMessage->header.bOwnsData) {
        uFree = 0;
        if (RtGetRingBufferFree(&pStream->queue, &uFree) == 0) {
            if (pStreamMessage->header.uKind) {
                if (RtConsumeRingBuffer(&pStream->queue, pStreamMessage->header.uSize - uFree) ==
                    0) {
                    RtWriteRingBuffer(&pStream->queue, pbData, pStreamMessage->header.uSize);
                    RtCompactRingBuffer(&pStream->queue, 1);
                    memcpy(pStream->pbData, pStream->queue.pbData, pStream->queue.uSize);
                }
            } else if (uFree != 0) {
                uSize = pStreamMessage->header.uSize;
                if (uSize >= uFree) {
                    uSize = uFree;
                }
                uFree = uSize;
                RtWriteRingBuffer(&pStream->queue, pbData, uSize);
                memcpy(pStream->pbData, pStream->queue.pbData, pStream->queue.uSize);
            }
        }
    }
    pfnData = g_apfnDmeStreamData[pStreamMessage->header.uRoute];
    if (pfnData != NULL) {
        pfnData(nSender, pStreamMessage->header.uStream, pbData, pStreamMessage->header.uSize);
    }
    pStream->uLastSendTime = g_uDmeSessionTime;
    return pStreamMessage->header.uSize + kStreamHeaderSize;
}

// A stream that closed.
// NTSC-U/C: 0x002bb640, PAL: 0x002c55b0
static int RtDmeOnStreamClosed(int nConnection, int nWorldId, int nSender, rtDmeMessage *pMessage) {
    const unsigned char *puStream;
    RtDmeStream *pStream;
    RtDmeConnection *pConnection;
    RtDmeStreamClosedFunc pfnClosed;
    int nHost;

    (void)nWorldId;
    puStream = (const unsigned char *)pMessage;
    nHost = RT_DME_NO_CLIENT;
    pStream = NULL;
    pConnection = NULL;
    if (puStream == NULL) {
        return -1;
    }
    if (RtDmeGetStream(*puStream, &pStream) != 0 || pStream == NULL) {
        return kClientFlagsSize;
    }
    if (pStream->uState != RT_DME_STREAM_OPEN) {
        return kClientFlagsSize;
    }
    pfnClosed = g_apfnDmeStreamClosed[pStream->uRoute];
    if (pfnClosed != NULL) {
        pfnClosed(nSender, *puStream);
    }
    if (RtDmeGetConnection(g_nDmeActiveConnection, &pConnection) != 0 || pConnection == NULL) {
        return kClientFlagsSize;
    }
    RtDmeGetHostClient(&nHost, nConnection);
    if (pConnection->nLocalClient == nHost && pStream->bToHost &&
        g_anDmeStreamRelay[pStream->uRoute]) {
        pStream->bClosing = 1;
        return kClientFlagsSize;
    }
    pStream->uState = RT_DME_STREAM_RESERVED;
    if (pStream->bOwnsData) {
        RtDmeFree((void **)&pStream->pbData);
        RtDestroyRingBuffer(&pStream->queue);
        pStream->pbStart = NULL;
        pStream->pbData = NULL;
    }
    return kClientFlagsSize;
}

// Copy the addresses of a client info into a pair of addresses.
static inline void RtDmeFormatClientAddresses(rtNetAddress *pAddresses,
                                              const RtDmeAddressProbe *pProbe) {
    RtDmeFormatAddress(pAddresses[RT_DME_ADDRESS_INDEX_PRIVATE].address, pProbe->abPrivateAddress);
    pAddresses[RT_DME_ADDRESS_INDEX_PRIVATE].port = pProbe->uPrivatePort;
    pAddresses[RT_DME_ADDRESS_INDEX_PRIVATE].type = RT_DME_ADDRESS_PRIVATE;
    RtDmeFormatAddress(pAddresses[RT_DME_ADDRESS_INDEX_PUBLIC].address, pProbe->abPublicAddress);
    pAddresses[RT_DME_ADDRESS_INDEX_PUBLIC].type = RT_DME_ADDRESS_PUBLIC;
    pAddresses[RT_DME_ADDRESS_INDEX_PUBLIC].port = pProbe->uPublicPort;
}

// The introduction of a client by the host.
// NTSC-U/C: 0x002bb798, PAL: 0x002c5708
static int RtDmeOnClientInfo(int nConnection, int nWorldId, int nSender, rtDmeMessage *pMessage) {
    const RtDmeClientInfo *pInfo;
    RtDmeConnection *pConnection;
    RtDmeClient *pClient;
    RtDmeClient *pOld;
    rtNetAddress addresses[RT_DME_ADDRESS_COUNT];
    const char *pszPrivate;
    const char *pszPublic;
    unsigned short uPrivatePort;
    unsigned short uPublicPort;
    int nIndex;

    (void)nWorldId;
    pInfo = (const RtDmeClientInfo *)pMessage;
    pConnection = NULL;
    pClient = NULL;
    if (pInfo == NULL) {
        return -1;
    }
    if (nConnection != 0 || g_nDmeHostClient != nSender) {
        return kClientInfoSize;
    }
    if (RtDmeGetAddressIndex(nConnection, nSender) != RT_DME_ADDRESS_INDEX_PRIVATE) {
        RtDmeResolveSenderAddress(0, nSender);
    }
    if (RtDmeCreateClient(pInfo->uConnection, pInfo->uClient, &pClient) != 0 || pClient == NULL) {
        return kClientInfoSize;
    }
    if (pClient->bJoined == 1 && g_nDmeHostClient != pInfo->uClient) {
        if (RtDmeGetConnection(0, &pConnection) != 0 || pConnection == NULL) {
            return kClientInfoSize;
        }
        if (pInfo->uClient != pConnection->nLocalClient) {
            nIndex = RtDmeGetAddressIndex(pInfo->uConnection, pInfo->uClient);
            if ((unsigned int)nIndex < RT_DME_ADDRESS_INDEX_UNRESOLVED) {
                RtDmeRemoveClient(pInfo->uConnection,
                                  pInfo->uClient,
                                  RtDmeGetClientAddress(
                                      pInfo->uConnection, pInfo->uClient, (unsigned char)nIndex),
                                  0);
            }
        }
    }
    RtDmeFormatClientAddresses(addresses, &pInfo->addresses);
    if (RtDmeGetConnection(pInfo->uConnection, &pConnection) != 0 || pConnection == NULL) {
        return kClientInfoSize;
    }
    if (RtDmeGetClient(pInfo->uConnection, pInfo->uClient, &pClient) != 0 || pClient == NULL) {
        return kClientInfoSize;
    }
    pConnection->nMaxClients = pInfo->uMaxClients;
    if (strncmp(g_dmeLocalAddresses[RT_DME_ADDRESS_INDEX_PUBLIC].address,
                addresses[RT_DME_ADDRESS_INDEX_PUBLIC].address,
                RT_DME_ADDRESS_SIZE) == 0 &&
        strncmp(g_dmeLocalAddresses[RT_DME_ADDRESS_INDEX_PRIVATE].address,
                addresses[RT_DME_ADDRESS_INDEX_PRIVATE].address,
                RT_DME_ADDRESS_SIZE) == 0 &&
        g_dmeLocalAddresses[RT_DME_ADDRESS_INDEX_PUBLIC].port ==
            addresses[RT_DME_ADDRESS_INDEX_PUBLIC].port &&
        g_dmeLocalAddresses[RT_DME_ADDRESS_INDEX_PRIVATE].port ==
            addresses[RT_DME_ADDRESS_INDEX_PRIVATE].port) {
        // The introduction is of this console.
        if (pConnection->nLocalClient == RT_DME_NO_CLIENT) {
            memcpy(pClient->addresses, addresses, sizeof(addresses));
            pClient->uAddressIndex = RT_DME_ADDRESS_INDEX_PRIVATE;
            pConnection->nLocalClient = pInfo->uClient;
            return kClientInfoSize;
        }
        RtDmeSetConnectResult(0, 0, 0, 1);
        RtDmeDisconnect(0);
        return kClientInfoSize;
    }
    if (g_nDmeHostClient != pInfo->uHost) {
        // The host moved. Its record moves to the new index when the addresses match.
        pszPrivate = RtDmeGetClientAddress(0, g_nDmeHostClient, RT_DME_ADDRESS_INDEX_PRIVATE);
        pszPublic = RtDmeGetClientAddress(0, g_nDmeHostClient, RT_DME_ADDRESS_INDEX_PUBLIC);
        uPrivatePort = RtDmeGetClientPort(0, g_nDmeHostClient, RT_DME_ADDRESS_INDEX_PRIVATE);
        uPublicPort = RtDmeGetClientPort(0, g_nDmeHostClient, RT_DME_ADDRESS_INDEX_PUBLIC);
        if (pszPrivate == NULL || pszPublic == NULL) {
            return kClientInfoSize;
        }
        if (strncmp(pszPrivate, pszPublic, RT_DME_ADDRESS_SIZE) != 0 ||
            uPrivatePort != uPublicPort) {
            if (strncmp(pszPrivate,
                        addresses[RT_DME_ADDRESS_INDEX_PRIVATE].address,
                        RT_DME_ADDRESS_SIZE) != 0 ||
                strncmp(pszPublic,
                        addresses[RT_DME_ADDRESS_INDEX_PUBLIC].address,
                        RT_DME_ADDRESS_SIZE) != 0 ||
                uPrivatePort != addresses[RT_DME_ADDRESS_INDEX_PRIVATE].port ||
                uPublicPort != addresses[RT_DME_ADDRESS_INDEX_PUBLIC].port) {
                return kClientInfoSize;
            }
        }
        pOld = NULL;
        if (RtDmeGetClient(pInfo->uConnection, g_nDmeHostClient, &pOld) != 0 || pClient == NULL) {
            return kClientInfoSize;
        }
        g_nDmeHostClient = pInfo->uHost;
        memcpy(pClient->addresses, addresses, sizeof(addresses));
        pClient->uAddressIndex = pOld->uAddressIndex;
        memset(pOld, 0, kClientRecordSize);
        return kClientInfoSize;
    }
    if (g_nDmeHostClient == pInfo->uClient) {
        pszPrivate = RtDmeGetClientAddress(0, g_nDmeHostClient, RT_DME_ADDRESS_INDEX_PRIVATE);
        pszPublic = RtDmeGetClientAddress(0, g_nDmeHostClient, RT_DME_ADDRESS_INDEX_PUBLIC);
        uPrivatePort = RtDmeGetClientPort(0, g_nDmeHostClient, RT_DME_ADDRESS_INDEX_PRIVATE);
        uPublicPort = RtDmeGetClientPort(0, g_nDmeHostClient, RT_DME_ADDRESS_INDEX_PUBLIC);
        if (pszPrivate == NULL || pszPublic == NULL) {
            return kClientInfoSize;
        }
        if (strncmp(pszPrivate, pszPublic, RT_DME_ADDRESS_SIZE) != 0 ||
            uPrivatePort != uPublicPort) {
            return kClientInfoSize;
        }
        memcpy(pClient->addresses, addresses, sizeof(addresses));
        return kClientInfoSize;
    }
    memcpy(pClient->addresses, addresses, sizeof(addresses));
    pClient->uAddressIndex = RT_DME_ADDRESS_INDEX_UNRESOLVED;
    pClient->nProbeCount = 0;
    RtReadClock(&pClient->lastProbe);
    nIndex = RtDmeDropClientPeers(pInfo->uConnection, pInfo->uClient);
    if (nIndex != 0) {
        return nIndex;
    }
    return kClientInfoSize;
}

// Send the introduction of a client to the working address of another.
static inline void
RtDmeSendClientInfo(unsigned int nConnection, int nClient, const RtDmeClientInfo *pInfo) {
    RtDmeSendDatagram(
        kIntroductionSendFlags,
        RtDmeGetClientAddress(nConnection, nClient, RtDmeGetAddressIndex(nConnection, nClient)),
        RtDmeGetClientPort(nConnection, nClient, RtDmeGetAddressIndex(nConnection, nClient)),
        RT_DME_CLASS_SYSTEM,
        g_nDmeClientInfoMessage,
        sizeof(*pInfo),
        pInfo);
}

// A probe of the addresses of this console by a peer. The host introduces a new peer to the others.
// NTSC-U/C: 0x002bbd80, PAL: 0x002c5cf0
static int RtDmeOnAddressProbe(int nConnection, int nWorldId, int nSender, rtDmeMessage *pMessage) {
    const RtDmeAddressProbe *pProbe;
    RtDmeConnection *pConnection;
    RtDmeClient *pClient;
    RtDmeClientInfo hostInfo;
    RtDmeClientInfo info;
    rtNetAddress addresses[RT_DME_ADDRESS_COUNT];
    const char *pszPrivate;
    const char *pszPublic;
    unsigned short uPrivatePort;
    unsigned short uPublicPort;
    RtDmeAddressWord privateAddress;
    RtDmeAddressWord publicAddress;
    int nReply;
    int nIndex;
    int i;

    (void)nWorldId;
    pProbe = (const RtDmeAddressProbe *)pMessage;
    if (pProbe == NULL) {
        return -1;
    }
    if (nConnection != 0 || nSender == RT_DME_NO_CLIENT) {
        if (nSender == RT_DME_NO_CLIENT) {
            memcpy(privateAddress.abBytes, pProbe->abPrivateAddress, kAddressBytes);
            memcpy(publicAddress.abBytes, pProbe->abPublicAddress, kAddressBytes);
            RtDmeAcceptPeer(privateAddress.uWord,
                            publicAddress.uWord,
                            pProbe->uPrivatePort,
                            pProbe->uPublicPort);
        }
        return kAddressProbeSize;
    }
    nIndex = RtDmeGetAddressIndex(nConnection, nSender);
    if (nIndex != RT_DME_ADDRESS_INDEX_UNRESOLVED) {
        return kAddressProbeSize;
    }
    pConnection = NULL;
    if (nSender == g_nDmeHostClient) {
        RtDmeFormatAddress(addresses[RT_DME_ADDRESS_INDEX_PRIVATE].address,
                           pProbe->abPrivateAddress);
        addresses[RT_DME_ADDRESS_INDEX_PRIVATE].port = pProbe->uPrivatePort;
        addresses[RT_DME_ADDRESS_INDEX_PRIVATE].type = nIndex;
        RtDmeFormatAddress(addresses[RT_DME_ADDRESS_INDEX_PUBLIC].address, pProbe->abPublicAddress);
        addresses[RT_DME_ADDRESS_INDEX_PUBLIC].port = pProbe->uPublicPort;
        addresses[RT_DME_ADDRESS_INDEX_PUBLIC].type = RT_DME_ADDRESS_PUBLIC;
        pszPrivate = RtDmeGetClientAddress(0, g_nDmeHostClient, RT_DME_ADDRESS_INDEX_PRIVATE);
        pszPublic = RtDmeGetClientAddress(0, g_nDmeHostClient, RT_DME_ADDRESS_INDEX_PUBLIC);
        uPrivatePort = RtDmeGetClientPort(0, g_nDmeHostClient, RT_DME_ADDRESS_INDEX_PRIVATE);
        uPublicPort = RtDmeGetClientPort(0, g_nDmeHostClient, RT_DME_ADDRESS_INDEX_PUBLIC);
        if (pszPrivate != NULL && pszPublic != NULL &&
            strncmp(pszPrivate, pszPublic, RT_DME_ADDRESS_SIZE) != 0) {
            if (strncmp(pszPrivate,
                        addresses[RT_DME_ADDRESS_INDEX_PRIVATE].address,
                        RT_DME_ADDRESS_SIZE) != 0 ||
                strncmp(pszPublic,
                        addresses[RT_DME_ADDRESS_INDEX_PUBLIC].address,
                        RT_DME_ADDRESS_SIZE) != 0 ||
                uPrivatePort != addresses[RT_DME_ADDRESS_INDEX_PRIVATE].port ||
                uPublicPort != addresses[RT_DME_ADDRESS_INDEX_PUBLIC].port) {
                return kAddressProbeSize;
            }
        }
    }
    RtDmeResolveSenderAddress(nConnection, nSender);
    if ((unsigned int)RtDmeGetAddressIndex(nConnection, nSender) >=
        RT_DME_ADDRESS_INDEX_UNRESOLVED) {
        return kAddressProbeSize;
    }
    if (RtDmeGetConnection(nConnection, &pConnection) != 0) {
        return -1;
    }
    if (pConnection == NULL) {
        return 0; // Yes, the binary reports 0 bytes consumed.
    }
    if (pConnection->nLocalClient != g_nDmeHostClient) {
        return kAddressProbeSize;
    }
    if (nSender >= pConnection->nMaxClients) {
        pClient = NULL;
        if (RtDmeGetClient(nConnection, nSender, &pClient) != 0 || pClient == NULL) {
            return -1;
        }
        nReply = kSessionFull;
        RtDmeSendDatagram(RT_DME_SEND_BASE,
                          g_szDmeSenderAddress,
                          g_uDmeSenderPort,
                          RT_DME_CLASS_SYSTEM,
                          g_nDmeSessionFullMessage,
                          sizeof(nReply),
                          &nReply);
        memset(pClient, 0, kClientRecordSize);
        return kAddressProbeSize;
    }
    RtDmeBuildClientInfo(&hostInfo, nConnection, pConnection->nLocalClient);
    RtDmeSendClientInfo(nConnection, nSender, &hostInfo);
    RtDmeBuildClientInfo(&info, nConnection, nSender);
    for (i = 0; i < RT_DME_CLIENT_COUNT; ++i) {
        nIndex = RtDmeGetAddressIndex(nConnection, i);
        if ((unsigned int)nIndex >= RT_DME_ADDRESS_INDEX_UNRESOLVED) {
            continue;
        }
        if (RtDmeGetClientAddressKind(nConnection, i, nIndex) == 0 || i == nSender ||
            pConnection->nLocalClient == i) {
            continue;
        }
        RtDmeBuildClientInfo(&hostInfo, nConnection, i);
        RtDmeSendClientInfo(nConnection, nSender, &hostInfo);
        RtDmeSendDatagram(kIntroductionSendFlags,
                          RtDmeGetClientAddress(nConnection, i, nIndex),
                          RtDmeGetClientPort(nConnection, i, nIndex),
                          RT_DME_CLASS_SYSTEM,
                          g_nDmeClientInfoMessage,
                          sizeof(info),
                          &info);
    }
    RtDmeSendClientInfo(nConnection, nSender, &info);
    return kAddressProbeSize;
}

// A host that refused this console a place in the session.
// NTSC-U/C: 0x002bc220, PAL: 0x002c6190
static int RtDmeOnSessionFull(int nConnection, int nWorldId, int nSender, rtDmeMessage *pMessage) {
    const int *pnReason;
    RtDmeConnection *pConnection;

    (void)nConnection;
    (void)nWorldId;
    (void)nSender;
    pnReason = (const int *)pMessage;
    pConnection = NULL;
    if (pnReason == NULL) {
        return -1;
    }
    if (g_nDmeHostClient == RT_DME_NO_CLIENT) {
        return kWordSize;
    }
    if (RtDmeGetConnection(0, &pConnection) != 0 || pConnection == NULL) {
        return kWordSize;
    }
    if (RtDmeGetClientAddressKind(0, g_nDmeHostClient, RT_DME_ADDRESS_INDEX_PRIVATE) != 0 &&
        strcmp(g_szDmeSenderAddress,
               RtDmeGetClientAddress(0, g_nDmeHostClient, RT_DME_ADDRESS_INDEX_PRIVATE)) == 0 &&
        g_uDmeSenderPort == RtDmeGetClientPort(0, g_nDmeHostClient, RT_DME_ADDRESS_INDEX_PRIVATE)) {
        RtDmeSetConnectResult(0, 0, 0, *pnReason);
        RtDmeDisconnect(0);
    }
    if (RtDmeGetClientAddressKind(0, g_nDmeHostClient, RT_DME_ADDRESS_INDEX_PUBLIC) == 0) {
        return kWordSize;
    }
    if (strcmp(g_szDmeSenderAddress,
               RtDmeGetClientAddress(0, g_nDmeHostClient, RT_DME_ADDRESS_INDEX_PUBLIC)) != 0) {
        return kWordSize;
    }
    if (g_uDmeSenderPort != RtDmeGetClientPort(0, g_nDmeHostClient, RT_DME_ADDRESS_INDEX_PUBLIC)) {
        return kWordSize;
    }
    RtDmeSetConnectResult(0, 0, 0, *pnReason);
    RtDmeDisconnect(0);
    return kWordSize;
}

// The addresses of a client that left, from another client.
// NTSC-U/C: 0x002bc398, PAL: 0x002c6308
static int
RtDmeOnClientAddresses(int nConnection, int nWorldId, int nSender, rtDmeMessage *pMessage) {
    const RtDmeClientAddresses *pAddresses;
    RtDmeClient *pClient;
    rtNetAddress addresses[RT_DME_ADDRESS_COUNT];

    (void)nWorldId;
    pAddresses = (const RtDmeClientAddresses *)pMessage;
    pClient = NULL;
    if (pAddresses == NULL) {
        return -1;
    }
    if ((unsigned int)nConnection >= RT_DME_CONNECTION_COUNT ||
        (unsigned int)nSender >= RT_DME_CLIENT_COUNT || nConnection != 0 ||
        pAddresses->uConnection != 0) {
        return -1;
    }
    if (RtDmeGetClient(pAddresses->uConnection, pAddresses->uClient, &pClient) != 0 ||
        pClient == NULL) {
        return -1;
    }
    if (pClient->bJoined != 1) {
        return kClientAddressesSize;
    }
    RtDmeFormatAddress(addresses[RT_DME_ADDRESS_INDEX_PUBLIC].address, pAddresses->abPublicAddress);
    RtDmeFormatAddress(addresses[RT_DME_ADDRESS_INDEX_PRIVATE].address,
                       pAddresses->abPrivateAddress);
    addresses[RT_DME_ADDRESS_INDEX_PUBLIC].port = pAddresses->uPublicPort;
    addresses[RT_DME_ADDRESS_INDEX_PRIVATE].port = pAddresses->uPrivatePort;
    if (strcmp(addresses[RT_DME_ADDRESS_INDEX_PUBLIC].address,
               RtDmeGetClientAddress(pAddresses->uConnection,
                                     pAddresses->uClient,
                                     RT_DME_ADDRESS_INDEX_PUBLIC)) != 0) {
        return kClientAddressesSize;
    }
    if (strcmp(addresses[RT_DME_ADDRESS_INDEX_PRIVATE].address,
               RtDmeGetClientAddress(pAddresses->uConnection,
                                     pAddresses->uClient,
                                     RT_DME_ADDRESS_INDEX_PRIVATE)) != 0) {
        return kClientAddressesSize;
    }
    if (addresses[RT_DME_ADDRESS_INDEX_PUBLIC].port !=
        RtDmeGetClientPort(
            pAddresses->uConnection, pAddresses->uClient, RT_DME_ADDRESS_INDEX_PUBLIC)) {
        return kClientAddressesSize;
    }
    if (addresses[RT_DME_ADDRESS_INDEX_PRIVATE].port !=
        RtDmeGetClientPort(
            pAddresses->uConnection, pAddresses->uClient, RT_DME_ADDRESS_INDEX_PRIVATE)) {
        return kClientAddressesSize;
    }
    RtDmeRemoveClient(
        pAddresses->uConnection,
        pAddresses->uClient,
        RtDmeGetClientAddress(pAddresses->uConnection,
                              pAddresses->uClient,
                              RtDmeGetAddressIndex(pAddresses->uConnection, pAddresses->uClient)),
        0);
    return kClientAddressesSize;
}

// A client that reports itself ready to the host.
// NTSC-U/C: 0x002bc520, PAL: 0x002c6490
static int RtDmeOnClientReady(int nConnection, int nWorldId, int nSender, rtDmeMessage *pMessage) {
    const RtDmeClientReady *pReady;
    RtDmeConnection *pConnection;
    RtDmeClient *pClient;
    int nLocal;

    (void)nWorldId;
    pReady = (const RtDmeClientReady *)pMessage;
    pConnection = NULL;
    pClient = NULL;
    if (pReady == NULL) {
        return -1;
    }
    if ((unsigned int)nConnection >= RT_DME_CONNECTION_COUNT ||
        (unsigned int)nSender >= RT_DME_CLIENT_COUNT) {
        return -1;
    }
    if (RtDmeGetConnection(nConnection, &pConnection) != 0 || pConnection == NULL) {
        return -1;
    }
    if (RtDmeGetClient(nConnection, pReady->uClient, &pClient) != 0 || pClient == NULL) {
        return -1;
    }
    if (pClient->bJoined != 1) {
        return -1;
    }
    nLocal = pConnection->nLocalClient;
    if ((unsigned int)nLocal >= RT_DME_CLIENT_COUNT) {
        return -1;
    }
    RtDmeSendClientState(nConnection, pReady->uClient);
    if (pConnection->apClients[nLocal]->nRole != kRoleObjectsJoined) {
        return kWordSize;
    }
    RtDmeAdmitClient(nConnection, pReady->uClient, pReady->auState[0], pReady->auState[1]);
    return kWordSize;
}

// The state a client announces.
// NTSC-U/C: 0x002bc628, PAL: 0x002c6598
static int RtDmeOnClientState(int nConnection, int nWorldId, int nSender, rtDmeMessage *pMessage) {
    const RtDmeClientStateMessage *pState;
    RtDmeConnection *pConnection;
    RtDmeClient *pClient;
    RtDmeClient *pLocal;

    (void)nWorldId;
    pState = (const RtDmeClientStateMessage *)pMessage;
    pConnection = NULL;
    pClient = NULL;
    pLocal = NULL;
    if (pState == NULL) {
        return -1;
    }
    if ((unsigned int)nConnection >= RT_DME_CONNECTION_COUNT ||
        (unsigned int)nSender >= RT_DME_CLIENT_COUNT) {
        return kClientStateSize;
    }
    if (RtDmeGetConnection(nConnection, &pConnection) != 0 || pConnection == NULL) {
        return -1;
    }
    if (RtDmeGetClient(nConnection, pState->uClient, &pClient) != 0 || pClient == NULL) {
        return -1;
    }
    if (pClient->bJoined != 1) {
        return kClientStateSize;
    }
    if ((unsigned int)pConnection->nLocalClient >= RT_DME_CLIENT_COUNT) {
        return kClientStateSize;
    }
    // Yes, the binary checks the record of the sender where it fetched the local client.
    if (RtDmeGetClient(nConnection, pConnection->nLocalClient, &pLocal) != 0 || pClient == NULL) {
        return kClientStateSize;
    }
    pClient->nRole = pState->uRole;
    pClient->anState[0] = pState->auState[0];
    pClient->anState[1] = pState->auState[1];
    pClient->uObjectCount = pState->uObjectCount;
    if (pState->nHostRank != 0) {
        pClient->nHostRank = pState->nHostRank;
    }
    switch (pLocal->nRole) {
    case RT_DME_ROLE_HOST:
        RtDmeElectHost(nConnection, pState->uClient, pState->auState[0], pState->auState[1]);
        break;
    case kRoleObjectsJoined:
        if (pState->uRole == RT_DME_ROLE_HOST) {
            RtDmeAdmitClient(nConnection, pState->uClient, pState->auState[0], pState->auState[1]);
        }
        break;
    default:
        break;
    }
    return kClientStateSize;
}

// The rank the host gave this console as a candidate host.
// NTSC-U/C: 0x002bc7b8, PAL: 0x002c6728
static int RtDmeOnHostRank(int nConnection, int nWorldId, int nSender, rtDmeMessage *pMessage) {
    const int *pnRank;
    RtDmeConnection *pConnection;
    RtDmeClient *pLocal;
    int nHost;

    (void)nWorldId;
    pnRank = (const int *)pMessage;
    pConnection = NULL;
    pLocal = NULL;
    nHost = RT_DME_NO_CLIENT;
    if (pnRank == NULL) {
        return -1;
    }
    if ((unsigned int)nConnection >= RT_DME_CONNECTION_COUNT ||
        (unsigned int)nSender >= RT_DME_CLIENT_COUNT ||
        (unsigned int)g_nDmeActiveConnection >= RT_DME_CONNECTION_COUNT) {
        return kWordSize;
    }
    if (RtDmeGetConnection(g_nDmeActiveConnection, &pConnection) != 0 || pConnection == NULL) {
        return kWordSize;
    }
    if (RtDmeGetClient(g_nDmeActiveConnection, pConnection->nLocalClient, &pLocal) != 0 ||
        pLocal == NULL) {
        return kWordSize;
    }
    if (RtDmeGetHostClient(&nHost, g_nDmeActiveConnection) != 0 || nHost == RT_DME_NO_CLIENT) {
        return kWordSize;
    }
    pLocal->nHostRank = *pnRank;
    RtDmeSendClientState(g_nDmeActiveConnection, RT_DME_ALL_CLIENTS);
    return kWordSize;
}

// The voice address a client announces.
// NTSC-U/C: 0x002bc8a8, PAL: 0x002c6818
static int RtDmeOnVoicePeer(int nConnection, int nWorldId, int nSender, rtDmeMessage *pMessage) {
    int bValid;

    (void)nWorldId;
    if (pMessage == NULL) {
        return -1;
    }
    bValid = RtDmeIsConnectionIndex(nConnection);
    if (bValid != 1 || RtDmeIsClientIndex(nSender) != bValid) {
        return -1;
    }
    if (RtDmeVoiceAddPeer((const RtDmeVoicePeerMessage *)pMessage) != 0) {
        return -1;
    }
    return kVoicePeerSize;
}

void RtDmeElectHost(int nConnection, int nClient, int nState0, int nState1) {
    RtDmeConnection *pConnection;
    RtDmeClient *pClient;
    RtDmeObjectRange *pRange;
    RtDmeSessionReady ready;
    int nCandidate;
    int i;

    (void)nClient;
    (void)nState0;
    (void)nState1;
    nCandidate = RT_DME_NO_CLIENT;
    pConnection = NULL;
    pClient = NULL;
    if ((unsigned int)nConnection >= RT_DME_CONNECTION_COUNT) {
        return;
    }
    if (RtDmeGetConnection(nConnection, &pConnection) != 0 || pConnection == NULL) {
        return;
    }
    if (pConnection->nPendingJoins != 0) {
        return;
    }
    if ((unsigned int)(g_nDmeObjectMode - 1) >= 2) {
        return;
    }
    for (i = 0; i < RT_DME_CLIENT_COUNT; ++i) {
        if (RtDmeGetClient(nConnection, i, &pClient) != 0) {
            return;
        }
        if (pClient == NULL || pClient->bJoined != 1) {
            continue;
        }
        if (pClient->nRole == kRoleNone || pClient->nRole == kRoleObjectsJoined) {
            return;
        }
        if (pClient->nRole == RT_DME_ROLE_HOST && nCandidate == RT_DME_NO_CLIENT) {
            nCandidate = i;
        }
    }
    if (nCandidate != pConnection->nLocalClient) {
        return;
    }
    // Yes, the binary does not clear the range before the lookup.
    RtDmeGetObjectRange(&pRange, nConnection, nCandidate);
    if (pRange == NULL) {
        pConnection->apClients[nCandidate]->nRole = kRoleObjectsReleased;
        RtDmeSendClientState(nConnection, RT_DME_ALL_CLIENTS);
        if (g_pfnDmeSessionReady != NULL) {
            ready.nEvent = kSessionNoRange;
            ready.nArgument = g_nDmeSessionReadyArgument;
            g_pfnDmeSessionReady(&ready);
            g_pfnDmeSessionReady = NULL;
            g_nDmeSessionReadyArgument = 0;
        }
        return;
    }
    pConnection->apClients[nCandidate]->nRole = kRoleObjectsJoined;
    pConnection->apClients[nCandidate]->nHostRank = 1;
    RtDmeSetHostClient(nConnection, nCandidate);
    RtDmeSendClientState(nConnection, RT_DME_ALL_CLIENTS);
    pRange->uFirstObject = 0;
    pRange->uFirstStream = 0;
    g_bDmeObjectsShared = 1;
    RtDmeOpenObjectRange(nConnection, nCandidate);
    RtDmeEnableConnection(nConnection, 1);
    if (g_pfnDmeSessionReady != NULL) {
        ready.nEvent = 0;
        ready.nArgument = g_nDmeSessionReadyArgument;
        g_pfnDmeSessionReady(&ready);
        g_pfnDmeSessionReady = NULL;
        g_nDmeSessionReadyArgument = 0;
    }
    for (i = 0; i < RT_DME_CLIENT_COUNT; ++i) {
        if (RtDmeGetClient(nConnection, i, &pClient) != 0 || pClient == NULL) {
            continue;
        }
        if (pClient->bJoined != 1 || pClient->nRole != RT_DME_ROLE_HOST || i == nCandidate) {
            continue;
        }
        RtDmeAdmitClient(nConnection, i, pClient->anState[0], pClient->anState[1]);
    }
}

int RtDmeRegisterSystemMessages(void) {
    RtDmeOutgoing fragment;
    RtDmeObjectUpdateMessage update;
    RtDmeObjectMessage object;
    RtDmeObjectRequest request;
    RtDmeIgnoredMessage ignored;
    RtDmeObjectGrant grant;
    RtDmeObjectLock lock;
    RtDmeStreamMessage stream;
    RtDmeObjectRangeMessage range;
    RtDmePingMessage ping;
    RtDmeJoinAnnouncement join;
    unsigned char abFarewell[kDisconnectSize];
    char szSessionName[RT_DME_ADDRESS_SIZE];
    RtDmeHostReply hostReply;
    RtDmeClientInfo info;
    RtDmeAddressProbe probe;
    RtDmeClientAddresses addresses;
    RtDmeClientReady ready;
    RtDmeClientStateMessage state;
    RtDmeVoicePeerMessage voicePeer;
    unsigned char uStream;
    unsigned int uClock;
    unsigned short uDestroyed;
    unsigned short uLocked;
    unsigned short uUnlocked;
    unsigned char uLeft;
    int nHostQuery;
    signed char nFlags;
    int nSessionFull;
    int nHostRank;

    // Yes, the binary drops the result of every registration but the last.
    RtDmeBeginFields();
    RtDmeAddField(szSessionName, szSessionName, 1, RT_DME_ADDRESS_SIZE);
    RtDmeRegisterMessageType(&g_nDmeSessionNameMessage, RT_DME_CLASS_SYSTEM, RtDmeOnSessionName);

    RtDmeBeginFields();
    RtDmeAddField(&ping, &ping.uTime, sizeof(ping.uTime), 1);
    RtDmeAddField(&ping, &ping.uSlot, sizeof(ping.uSlot), 1);
    RtDmeAddField(&ping, &ping.bRequest, sizeof(ping.bRequest), 1);
    RtDmeRegisterMessageType(&g_nDmePingMessage, RT_DME_CLASS_SYSTEM, RtDmeOnPing);

    RtDmeBeginFields();
    RtDmeAddField(&fragment,
                  &fragment.fragment.header.uMessageId,
                  sizeof(fragment.fragment.header.uMessageId),
                  1);
    RtDmeAddField(
        &fragment, &fragment.fragment.header.nOffset, sizeof(fragment.fragment.header.nOffset), 1);
    RtDmeAddField(&fragment,
                  &fragment.fragment.header.nTotalSize,
                  sizeof(fragment.fragment.header.nTotalSize),
                  1);
    RtDmeAddField(
        &fragment, &fragment.fragment.header.uCount, sizeof(fragment.fragment.header.uCount), 1);
    RtDmeAddField(
        &fragment, &fragment.fragment.header.uIndex, sizeof(fragment.fragment.header.uIndex), 1);
    RtDmeAddField(
        &fragment, &fragment.fragment.header.uSize, sizeof(fragment.fragment.header.uSize), 1);
    RtDmeRegisterMessageType(&g_nDmeFragmentMessage, RT_DME_CLASS_SYSTEM, RtDmeOnFragment);

    RtDmeBeginFields();
    RtDmeAddField(&update, &update.header.uCount, sizeof(update.header.uCount), 1);
    RtDmeAddField(&update, &update.header.uObject, sizeof(update.header.uObject), 1);
    RtDmeAddField(&update, &update.header.uType, sizeof(update.header.uType), 1);
    RtDmeRegisterMessageType(&g_nDmeObjectUpdateMessage, RT_DME_CLASS_SYSTEM, RtDmeOnObjectUpdate);

    RtDmeBeginFields();
    RtDmeAddField(&object, &object.header.uOwner, sizeof(object.header.uOwner), 1);
    RtDmeAddField(&object, &object.header.uFilter, sizeof(object.header.uFilter), 1);
    RtDmeAddField(&object, &object.header.uFlags, sizeof(object.header.uFlags), 1);
    RtDmeAddField(&object, &object.header.uLifetime, sizeof(object.header.uLifetime), 1);
    RtDmeAddField(&object, object.header.szName, 1, RT_DME_ADDRESS_SIZE);
    RtDmeAddField(&object, &object.header.uObject, sizeof(object.header.uObject), 1);
    RtDmeAddField(&object, &object.header.uLockClient, sizeof(object.header.uLockClient), 1);
    RtDmeAddField(&object, &object.header.uType, sizeof(object.header.uType), 1);
    RtDmeRegisterMessageType(&g_nDmeObjectMessage, RT_DME_CLASS_SYSTEM, RtDmeOnWholeObject);

    RtDmeBeginFields();
    RtDmeAddField(&uStream, &uStream, sizeof(uStream), 1);
    RtDmeRegisterMessageType(&g_nDmeStreamClosedMessage, RT_DME_CLASS_SYSTEM, RtDmeOnStreamClosed);

    RtDmeBeginFields();
    RtDmeAddField(&uClock, &uClock, sizeof(uClock), 1);
    RtDmeRegisterMessageType(&g_nDmeClockMessage, RT_DME_CLASS_SYSTEM, RtDmeOnClock);

    RtDmeBeginFields();
    RtDmeAddField(&request, &request.uClient, sizeof(request.uClient), 1);
    RtDmeAddField(&request, &request.uObjectCount, sizeof(request.uObjectCount), 1);
    RtDmeAddField(&request, &request.uStreamCount, sizeof(request.uStreamCount), 1);
    RtDmeRegisterMessageType(
        &g_nDmeObjectRequestMessage, RT_DME_CLASS_SYSTEM, RtDmeOnObjectRequest);

    RtDmeBeginFields();
    RtDmeAddField(&ignored, ignored.abHeader, 1, kAddressBytes);
    RtDmeAddField(&ignored, &ignored.body, sizeof(ignored.body), 1);
    RtDmeRegisterMessageType(&g_nDmeIgnoredMessage, RT_DME_CLASS_SYSTEM, RtDmeOnIgnored);

    RtDmeBeginFields();
    RtDmeAddField(&grant, &grant.uTime, sizeof(grant.uTime), 1);
    RtDmeAddField(&grant, &grant.uFirstObject, sizeof(grant.uFirstObject), 1);
    RtDmeAddField(&grant, &grant.uFirstStream, sizeof(grant.uFirstStream), 1);
    RtDmeRegisterMessageType(&g_nDmeObjectGrantMessage, RT_DME_CLASS_SYSTEM, RtDmeOnObjectGrant);

    RtDmeBeginFields();
    RtDmeAddField(&uDestroyed, &uDestroyed, sizeof(uDestroyed), 1);
    RtDmeRegisterMessageType(
        &g_nDmeObjectDestroyMessage, RT_DME_CLASS_SYSTEM, RtDmeOnObjectDestroy);

    RtDmeBeginFields();
    RtDmeAddField(&uLocked, &uLocked, sizeof(uLocked), 1);
    RtDmeRegisterMessageType(&g_nDmeObjectLockMessage, RT_DME_CLASS_SYSTEM, RtDmeOnObjectLock);

    RtDmeBeginFields();
    RtDmeAddField(&lock, &lock.uClient, sizeof(lock.uClient), 1);
    RtDmeAddField(&lock, &lock.uObject, sizeof(lock.uObject), 1);
    RtDmeRegisterMessageType(&g_nDmeObjectLockedMessage, RT_DME_CLASS_SYSTEM, RtDmeOnObjectLocked);

    RtDmeBeginFields();
    RtDmeAddField(&uUnlocked, &uUnlocked, sizeof(uUnlocked), 1);
    RtDmeRegisterMessageType(&g_nDmeObjectUnlockMessage, RT_DME_CLASS_SYSTEM, RtDmeOnObjectUnlock);

    RtDmeBeginFields();
    RtDmeAddField(&stream, &stream.header.uStart, sizeof(stream.header.uStart), 1);
    RtDmeAddField(&stream, &stream.header.uSize, sizeof(stream.header.uSize), 1);
    RtDmeAddField(&stream, &stream.header.uStream, sizeof(stream.header.uStream), 1);
    RtDmeAddField(&stream, &stream.header.uKind, sizeof(stream.header.uKind), 1);
    RtDmeAddField(&stream, &stream.header.bOwnsData, sizeof(stream.header.bOwnsData), 1);
    RtDmeAddField(&stream, &stream.header.bToHost, sizeof(stream.header.bToHost), 1);
    RtDmeAddField(&stream, &stream.header.uRoute, sizeof(stream.header.uRoute), 1);
    RtDmeAddField(&stream, &stream.header.uEnd, sizeof(stream.header.uEnd), 1);
    RtDmeRegisterMessageType(&g_nDmeStreamDataMessage, RT_DME_CLASS_SYSTEM, RtDmeOnStreamData);

    RtDmeBeginFields();
    RtDmeAddField(&range, &range.uClient, sizeof(range.uClient), 1);
    RtDmeAddField(&range, &range.uReserved01, sizeof(range.uReserved01), 1);
    RtDmeAddField(&range, &range.uReserved04, sizeof(range.uReserved04), 1);
    RtDmeAddField(&range, &range.uObjectLimit, sizeof(range.uObjectLimit), 1);
    RtDmeAddField(&range, &range.uFirstObject, sizeof(range.uFirstObject), 1);
    RtDmeAddField(&range, &range.uObjectCount, sizeof(range.uObjectCount), 1);
    RtDmeAddField(&range, &range.uFirstStream, sizeof(range.uFirstStream), 1);
    RtDmeAddField(&range, &range.uStreamCount, sizeof(range.uStreamCount), 1);
    // Yes, the binary registers 12 bytes of the 14 of the name.
    RtDmeAddField(&range, range.szName, 1, RT_DME_RANGE_NAME_SIZE - 2);
    RtDmeRegisterMessageType(&g_nDmeObjectRangeMessage, RT_DME_CLASS_SYSTEM, RtDmeOnObjectRange);

    RtDmeBeginFields();
    RtDmeAddField(&join, &join.uConnection, sizeof(join.uConnection), 1);
    RtDmeAddField(&join, &join.uClient, sizeof(join.uClient), 1);
    RtDmeAddField(&join, join.abAddress, 1, kAddressBytes);
    RtDmeAddField(&join, &join.modulus, sizeof(join.modulus), 1);
    RtDmeRegisterMessageType(&g_nDmeClientJoinMessage, RT_DME_CLASS_SYSTEM, RtDmeOnClientJoin);

    RtDmeBeginFields();
    RtDmeAddField(abFarewell, &abFarewell[0], 1, 1);
    RtDmeAddField(abFarewell, &abFarewell[1], 1, 1);
    RtDmeRegisterMessageType(&g_nDmeDisconnectMessage, RT_DME_CLASS_SYSTEM, RtDmeOnDisconnect);

    RtDmeBeginFields();
    RtDmeAddField(&uLeft, &uLeft, sizeof(uLeft), 1);
    RtDmeRegisterMessageType(&g_nDmeClientLeftMessage, RT_DME_CLASS_SYSTEM, RtDmeOnClientLeft);

    RtDmeBeginFields();
    RtDmeAddField(&nHostQuery, &nHostQuery, sizeof(nHostQuery), 1);
    RtDmeRegisterMessageType(&g_nDmeHostQueryMessage, RT_DME_CLASS_SYSTEM, RtDmeOnHostQuery);

    RtDmeBeginFields();
    RtDmeAddField(&hostReply, hostReply.abAddress, 1, kAddressBytes);
    RtDmeAddField(&hostReply, &hostReply.nPort, sizeof(hostReply.nPort), 1);
    RtDmeRegisterMessageType(&g_nDmeHostReplyMessage, RT_DME_CLASS_SYSTEM, RtDmeOnHostReply);

    RtDmeBeginFields();
    RtDmeAddField(&nFlags, &nFlags, sizeof(nFlags), 1);
    RtDmeRegisterMessageType(&g_nDmeClientFlagsMessage, RT_DME_CLASS_SYSTEM, RtDmeOnClientFlags);

    RtDmeBeginFields();
    RtDmeAddField(&info, info.addresses.abPrivateAddress, kAddressBytes, 1);
    RtDmeAddField(&info, info.addresses.abPublicAddress, kAddressBytes, 1);
    RtDmeAddField(&info, &info.addresses.uPrivatePort, sizeof(info.addresses.uPrivatePort), 1);
    RtDmeAddField(&info, &info.addresses.uPublicPort, sizeof(info.addresses.uPublicPort), 1);
    RtDmeAddField(&info, &info.uConnection, sizeof(info.uConnection), 1);
    RtDmeAddField(&info, &info.uClient, sizeof(info.uClient), 1);
    RtDmeAddField(&info, &info.uHost, sizeof(info.uHost), 1);
    RtDmeAddField(&info, &info.uMaxClients, sizeof(info.uMaxClients), 1);
    RtDmeRegisterMessageType(&g_nDmeClientInfoMessage, RT_DME_CLASS_SYSTEM, RtDmeOnClientInfo);

    RtDmeBeginFields();
    RtDmeAddField(&probe, probe.abPrivateAddress, kAddressBytes, 1);
    RtDmeAddField(&probe, probe.abPublicAddress, kAddressBytes, 1);
    RtDmeAddField(&probe, &probe.uPrivatePort, sizeof(probe.uPrivatePort), 1);
    RtDmeAddField(&probe, &probe.uPublicPort, sizeof(probe.uPublicPort), 1);
    RtDmeRegisterMessageType(&g_nDmeAddressProbeMessage, RT_DME_CLASS_SYSTEM, RtDmeOnAddressProbe);

    RtDmeBeginFields();
    RtDmeAddField(&nSessionFull, &nSessionFull, sizeof(nSessionFull), 1);
    RtDmeRegisterMessageType(&g_nDmeSessionFullMessage, RT_DME_CLASS_SYSTEM, RtDmeOnSessionFull);

    RtDmeBeginFields();
    RtDmeAddField(&addresses, &addresses.uConnection, sizeof(addresses.uConnection), 1);
    RtDmeAddField(&addresses, &addresses.uClient, sizeof(addresses.uClient), 1);
    RtDmeAddField(&addresses, addresses.abPrivateAddress, kAddressBytes, 1);
    RtDmeAddField(&addresses, addresses.abPublicAddress, kAddressBytes, 1);
    RtDmeAddField(&addresses, &addresses.uPrivatePort, sizeof(addresses.uPrivatePort), 1);
    RtDmeAddField(&addresses, &addresses.uPublicPort, sizeof(addresses.uPublicPort), 1);
    RtDmeRegisterMessageType(
        &g_nDmeClientAddressesMessage, RT_DME_CLASS_SYSTEM, RtDmeOnClientAddresses);

    RtDmeBeginFields();
    RtDmeAddField(&ready, &ready.uReserved00, sizeof(ready.uReserved00), 1);
    RtDmeAddField(&ready, &ready.uClient, sizeof(ready.uClient), 1);
    RtDmeAddField(&ready, &ready.auState[0], sizeof(ready.auState[0]), 1);
    RtDmeAddField(&ready, &ready.auState[1], sizeof(ready.auState[1]), 1);
    RtDmeRegisterMessageType(&g_nDmeClientReadyMessage, RT_DME_CLASS_SYSTEM, RtDmeOnClientReady);

    RtDmeBeginFields();
    RtDmeAddField(&state, &state.uRole, sizeof(state.uRole), 1);
    RtDmeAddField(&state, &state.uClient, sizeof(state.uClient), 1);
    RtDmeAddField(&state, &state.auState[0], sizeof(state.auState[0]), 1);
    RtDmeAddField(&state, &state.auState[1], sizeof(state.auState[1]), 1);
    RtDmeAddField(&state, &state.uObjectCount, sizeof(state.uObjectCount), 1);
    RtDmeAddField(&state, &state.nHostRank, sizeof(state.nHostRank), 1);
    RtDmeRegisterMessageType(&g_nDmeClientStateMessage, RT_DME_CLASS_SYSTEM, RtDmeOnClientState);

    RtDmeBeginFields();
    RtDmeAddField(&nHostRank, &nHostRank, sizeof(nHostRank), 1);
    RtDmeRegisterMessageType(&g_nDmeHostRankMessage, RT_DME_CLASS_SYSTEM, RtDmeOnHostRank);

    RtDmeBeginFields();
    RtDmeAddField(&voicePeer, &voicePeer.nConnection, sizeof(voicePeer.nConnection), 1);
    RtDmeAddField(&voicePeer, &voicePeer.nClient, sizeof(voicePeer.nClient), 1);
    RtDmeAddField(&voicePeer, &voicePeer.uLocalAddress, sizeof(voicePeer.uLocalAddress), 1);
    RtDmeAddField(&voicePeer, &voicePeer.nLocalPort, sizeof(voicePeer.nLocalPort), 1);
    RtDmeAddField(&voicePeer, &voicePeer.uAddress, sizeof(voicePeer.uAddress), 1);
    RtDmeAddField(&voicePeer, &voicePeer.nPort, sizeof(voicePeer.nPort), 1);
    return RtDmeRegisterMessageType(&g_nDmeVoicePeerMessage, RT_DME_CLASS_SYSTEM, RtDmeOnVoicePeer);
}

int RtDmeExpireReassembly(void) {
    RtDmeReassembly *pReassembly;
    unsigned int uNow;
    int i;

    RtGetMilliseconds(&uNow);
    for (i = 0; i < kReassemblyCount; ++i) {
        pReassembly = &g_aDmeReassembly[i];
        if (pReassembly->nState != kReassemblyUsed) {
            continue;
        }
        if (uNow - pReassembly->uLastTime <= kReassemblyTimeout) {
            continue;
        }
        if (pReassembly->pbData != NULL) {
            RtDmeFree((void **)&pReassembly->pbData);
            pReassembly->pbData = NULL;
        }
        if (pReassembly->pbParts != NULL) {
            RtDmeFree((void **)&pReassembly->pbParts);
            pReassembly->pbParts = NULL;
        }
        pReassembly->nState = kReassemblyFree;
    }
    return 0;
}

int RtDmeClearReassembly(void) {
    memset(g_aDmeReassembly, 0, sizeof(g_aDmeReassembly));
    return 0;
}
