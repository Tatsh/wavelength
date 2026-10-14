#include "rt/rtchannelapi.h"

#include <stdlib.h>
#include <string.h>

#include "rt/rtmemory.h"
#include "rt/rtmutex.h"
#include "rt/rtrecord.h"
#include "rt/rtsocket.h"
#include "rt/rttime.h"

enum {
    // The states of a channel the layer checks.
    kStateClosed = 1,
    kStateOpening = 2,
    kStateSentKey = 3,
    kStateSentRequest = 4,
    kStateConnecting = 6,
    kStateConnected = 8,
    kStateClosing = 9,
    kStateDisconnected = 10,
    kStateFailed = 11,
    // The record types the layer queues.
    kRecordKeepalive = 5,
    kRecordToServer = 2,
    kRecordToPeer = 3,
    kRecordToAll = 11,
    kRecordFlag = 13,
    // The data of a keep-alive record.
    kKeepaliveByte = 0xa5,
    // The first byte of a flag record.
    kFlagRecordTag = 2,
    kFlagRecordSize = 2,
    // The milliseconds the layer allows for the last bytes of a closing channel.
    kCloseTimeout = 10000,
    // The milliseconds a connection request waits before the datagram link repeats it.
    kConnectRetryInterval = 200,
    // The milliseconds of silence after which a connected channel fails.
    kSilenceTimeout = 30000,
    // The smallest queue of the datagram service.
    kMinServiceQueue = RT_CHANNEL_MAX_PACKET,
    // The flags that keep data on the stream link of a host.
    kStreamOnlyFlags = 0x50,
    // The flag that makes the link note the peer of the record.
    kTrackPeerShift = 7,
    // The defaults of the parameters.
    kDefaultPort = 10075,
    kDefaultQueueSize = 0x8000,
    kDefaultKeepaliveInterval = 15000,
    kDefaultFlushInterval = 30,
    kDefaultPeerTimeout = 45000,
    // The octets of an address.
    kAddressOctets = 4,
    kAddressSize = RT_CHANNEL_ADDRESS_SIZE,
    // The bits of an address that name its record of keys.
    kPeerMask = 0x1fffffff,
    // The number of channel modes.
    kModeCount = 2,
};

// The datagram service.
typedef struct {
    unsigned char bStarted;
    RtSocket *pStream;
    unsigned short uPort;
    unsigned int uFlushInterval;
    unsigned int uSendSize;
    unsigned int uReceiveSize;
    RtChannelDatagramFunc pfnDatagram;
    unsigned int uPeerTimeout;
    RtStreamTimeoutFunc pfnTimeout;
    RtChannelMapFunc pfnMap;
} RtChannelService;

// The listening socket. Nothing opens it in this program.
typedef struct {
    unsigned char bOpen;
    RtSocket *pSocket;
    RtChannelAcceptFunc pfnAccept;
} RtChannelListener;

// An address as four octets, read as one word for its record of keys.
typedef union {
    unsigned char abOctets[kAddressOctets];
    unsigned int uWord;
} RtChannelAddress;

// NTSC-U/C: 0x003b6cc6
static short g_nChannelApiRefCount;
// NTSC-U/C: 0x003b6cd8
static unsigned char g_bChannelApiStarted;
// NTSC-U/C: 0x003b6cdc
static int g_nChannelSecurityState;
// NTSC-U/C: 0x005205d0
static RtChannelService g_channelService;
// NTSC-U/C: 0x005205f8
static RtChannelListener g_channelListener;
// NTSC-U/C: 0x00520608
static RtMutex g_channelApiMutex;

// Take the lock of the layer, or report why not.
static inline int RtChannelApiEnter(void) {
    if (g_bChannelApiStarted != 1) {
        return RT_CHANNEL_API_RESULT_LOCK;
    }
    if (RtLockMutex(&g_channelApiMutex, NULL) != 0) {
        return RT_CHANNEL_API_RESULT_LOCK;
    }
    return 0;
}

// Release the lock of the layer and pass the result on.
static inline int RtChannelApiLeave(int nResult) {
    if (RtUnlockMutex(&g_channelApiMutex, NULL) != 0) {
        return RT_CHANNEL_API_RESULT_LOCK;
    }
    return nResult;
}

int RtChannelGetSecurityState(void) {
    if (g_nChannelSecurityState == 0) {
        if (RtReadKeyPairModulus(0, NULL) != 0) {
            g_nChannelSecurityState = RT_CHANNEL_SECURITY_NO_KEYS;
        } else {
            g_nChannelSecurityState = RT_CHANNEL_SECURITY_KEYS;
        }
    }
    return g_nChannelSecurityState;
}

// NTSC-U/C: 0x002c0348, PAL: 0x002ca2b8
static int RtChannelParseAddress(RtChannelAddress *pAddress, const char *pszAddress) {
    char szText[kAddressSize];
    char *pszPart;
    char *pszDot;
    int i;

    if (pszAddress == NULL || pAddress == NULL) {
        return 1;
    }
    strcpy(szText, pszAddress);
    pszPart = szText;
    // Yes, the binary stores the first number in the last octet.
    for (i = kAddressOctets - 1; i > 0; --i) {
        pszDot = strstr(pszPart, ".");
        *pszDot = '\0';
        pAddress->abOctets[i] = atoi(pszPart);
        pszPart = pszDot + 1;
    }
    pAddress->abOctets[0] = atoi(pszPart);
    return 0;
}

// NTSC-U/C: 0x002c1238, PAL: 0x002cb1a8
static int RtChannelDoCloseListener(void) {
    if (g_channelListener.bOpen != 1) {
        return RT_CHANNEL_RESULT_NOT_STARTED;
    }
    RtSocketClose(g_channelListener.pSocket);
    g_channelListener.bOpen = 0;
    return 0;
}

// NTSC-U/C: 0x002c1e50, PAL: 0x002cbdc0
static int RtChannelDoGetLocalAddress(char *pszAddress) {
    if (g_bChannelsStarted != 1) {
        return RT_CHANNEL_RESULT_NOT_STARTED;
    }
    if (pszAddress == NULL) {
        return RT_CHANNEL_RESULT_INVALID;
    }
    return RtChannelMapSocketResult(RtSocketGetLocalAddress(pszAddress));
}

// NTSC-U/C: 0x002c2218, PAL: 0x002cc188
static void RtChannelOnPeerTimeout(const char *pszAddress, unsigned short uPort) {
    if (g_channelService.pfnTimeout != NULL) {
        g_channelService.pfnTimeout(pszAddress, uPort);
    }
}

// NTSC-U/C: 0x002c1280, PAL: 0x002cb1f0
static int RtChannelDoStartService(const RtChannelServiceConfig *pConfig) {
    char szAddress[kAddressSize];
    RtChannelAddress address;
    int nResult;

    if (g_bChannelsStarted != 1) {
        return RT_CHANNEL_RESULT_NOT_STARTED;
    }
    if (pConfig == NULL || pConfig->uPort == 0) {
        return RT_CHANNEL_RESULT_INVALID;
    }
    if (g_channelService.bStarted == 1) {
        if (g_channelService.uPort != pConfig->uPort ||
            g_channelService.pfnDatagram != pConfig->pfnDatagram ||
            g_channelService.uReceiveSize != pConfig->uReceiveSize ||
            g_channelService.uSendSize != pConfig->uSendSize ||
            g_channelService.uFlushInterval != pConfig->uFlushInterval ||
            g_channelService.uPeerTimeout != pConfig->uPeerTimeout ||
            g_channelService.pfnTimeout != pConfig->pfnTimeout) {
            return RT_CHANNEL_RESULT_INVALID;
        }
        return 0;
    }
    memset(&g_channelService, 0, sizeof(g_channelService));
    g_channelService.uReceiveSize = pConfig->uReceiveSize;
    g_channelService.uSendSize = pConfig->uSendSize;
    if (g_channelService.uReceiveSize < kMinServiceQueue) {
        g_channelService.uReceiveSize = kMinServiceQueue;
    }
    if (g_channelService.uSendSize < kMinServiceQueue) {
        g_channelService.uSendSize = kMinServiceQueue;
    }
    nResult = RtChannelReserveScratch(RT_CHANNEL_MAX_PACKET);
    if (nResult != 0) {
        return nResult;
    }
    if (RtStreamStartup() != 0) {
        return RT_CHANNEL_API_RESULT_STREAM;
    }
    // Yes, the binary never stores the flush interval, so the stream gets 0.
    if (RtStreamOpen(&g_channelService.pStream,
                     pConfig->uPort,
                     pConfig->uPeerTimeout,
                     g_channelService.uSendSize,
                     g_channelService.uReceiveSize,
                     g_channelService.uFlushInterval,
                     RtChannelOnPeerTimeout) != 0) {
        return RT_CHANNEL_API_RESULT_STREAM;
    }
    g_channelService.pfnDatagram = pConfig->pfnDatagram;
    g_channelService.pfnTimeout = pConfig->pfnTimeout;
    address.uWord = 0;
    g_channelService.uPeerTimeout = pConfig->uPeerTimeout;
    g_channelService.uPort = pConfig->uPort;
    (void)RtChannelDoGetLocalAddress(szAddress); // Yes, the binary discards the result.
    (void)RtChannelParseAddress(&address, szAddress);
    RtCreateConnectionKeys(RT_SECURITY_SERVER_ID, address.uWord & kPeerMask, NULL);
    g_channelService.bStarted = 1;
    return 0;
}

// NTSC-U/C: 0x002c1450, PAL: 0x002cb3c0
static int RtChannelDoOpen(unsigned short *puId, const RtChannelConfig *pConfig) {
    RtSocketOptions options;
    RtChannel *pChannel;
    RtChannelLink *pLink;
    unsigned short uId;
    unsigned char bEncrypted;
    int nResult;
    int i;

    uId = 0;
    pChannel = NULL;
    bEncrypted = 0;
    if (g_bChannelsStarted != 1) {
        return RT_CHANNEL_RESULT_NOT_STARTED;
    }
    if (puId == NULL || pConfig == NULL || pConfig->uPort == 0 || pConfig->pfnConnect == NULL ||
        (unsigned int)pConfig->nMode >= kModeCount) {
        return RT_CHANNEL_RESULT_INVALID;
    }
    *puId = g_uChannelCount + 1;
    nResult = RtChannelCreate(&uId, pConfig->nMode, pConfig->uSendSize, pConfig->uReceiveSize);
    if (nResult != 0) {
        return nResult;
    }
    *puId = uId;
    RtDestroyConnectionKeys(uId);
    RtCreateConnectionKeys(uId, 0, pConfig->pServerKey);
    nResult = RtChannelGet(uId, &pChannel);
    if (nResult != 0) {
        return nResult;
    }
    if (pChannel == NULL) {
        return RT_CHANNEL_RESULT_NO_LINK;
    }
    if (pChannel->nState != kStateClosed) {
        return RT_CHANNEL_RESULT_BAD_STATE;
    }
    nResult = RtChannelSetState(pChannel, 0);
    if (nResult != 0) {
        return nResult;
    }
    pChannel->config = *pConfig;
    if (pConfig->pRequestData != NULL && pConfig->uRequestSize != 0) {
        if (RtAllocZeroedBlock(&pChannel->config.pRequestData, pConfig->uRequestSize) != 0) {
            return RT_CHANNEL_RESULT_NO_MEMORY;
        }
        memcpy(pChannel->config.pRequestData, pConfig->pRequestData, pConfig->uRequestSize);
        pChannel->config.uRequestSize = pConfig->uRequestSize;
    } else {
        pChannel->config.pRequestData = NULL;
        pChannel->config.uRequestSize = 0;
    }
    pChannel->szServerName[0] = '\0';
    pChannel->pfnConnect = pConfig->pfnConnect;
    pChannel->pfnData = pConfig->pfnData;
    pChannel->pfnPeer = pConfig->pfnPeer;
    pChannel->pfnStatus = pConfig->pfnStatus;
    for (i = 0; i < RT_CHANNEL_LINK_COUNT; ++i) {
        pLink = pChannel->apLinks[i];
        if (pLink == NULL) {
            continue;
        }
        pLink->packet.uSize = 0;
        pLink->packet.uCount = 0;
        if (RtReadTime(&pLink->lastSend) != 0) {
            return RT_CHANNEL_RESULT_CLOCK;
        }
        if (RtReadTime(&pLink->lastReceive) != 0) {
            return RT_CHANNEL_RESULT_CLOCK;
        }
        pLink->uKeepaliveInterval = pConfig->uKeepaliveInterval;
        pLink->uPending = 0;
        if (RtReadTime(&pLink->lastKeepalive) != 0) {
            return RT_CHANNEL_RESULT_CLOCK;
        }
        pLink->uFlushInterval = pConfig->uFlushInterval;
        pLink->bPeerNoted = 0;
        if (RtResetRingBuffer(&pLink->receive) != 0) {
            return RT_CHANNEL_RESULT_RING;
        }
        if (RtResetRingBuffer(&pLink->send) != 0) {
            return RT_CHANNEL_RESULT_RING;
        }
    }
    nResult = RtChannelSetState(pChannel, kStateOpening);
    if (nResult != 0) {
        return nResult;
    }
    if (pChannel->nMode == RT_CHANNEL_MODE_HOST) {
        pLink = pChannel->apLinks[RT_CHANNEL_LINK_DATAGRAM];
        if (pLink == NULL) {
            return RT_CHANNEL_RESULT_NO_LINK;
        }
        options.nType = RT_SOCKET_DATAGRAM;
        options.nBufferSize = RT_CHANNEL_MAX_PACKET;
        nResult = RtChannelMapSocketResult(
            RtSocketCreate(&pLink->pSocket, NULL, pConfig->uLocalPort, &options));
        if (nResult != 0) {
            return nResult;
        }
        pLink->bOpen = 0; // Yes, the binary leaves the datagram link closed here.
    }
    if ((unsigned int)pChannel->nMode < kModeCount) {
        pLink = pChannel->apLinks[RT_CHANNEL_LINK_STREAM];
        if (pLink == NULL) {
            return RT_CHANNEL_RESULT_NO_LINK;
        }
        strncpy(pLink->szAddress, pConfig->szAddress, RT_CHANNEL_ADDRESS_SIZE - 1);
        pLink->szAddress[RT_CHANNEL_ADDRESS_SIZE - 1] = '\0';
        pLink->uPort = pConfig->uPort;
        options.nType = RT_SOCKET_STREAM;
        options.nBufferSize = 0;
        nResult = RtChannelMapSocketResult(
            RtSocketCreate(&pLink->pSocket, pConfig->szAddress, pConfig->uPort, &options));
        if (nResult != 0) {
            return nResult;
        }
        pLink->bOpen = 1;
        pChannel->pActiveLink = pLink;
    }
    pLink = pChannel->pActiveLink;
    if (pLink == NULL) {
        return RT_CHANNEL_RESULT_NO_LINK;
    }
    if (pLink->uChannel != pChannel->uId) {
        return RT_CHANNEL_RESULT_SHORT;
    }
    nResult = RtChannelIsEncrypted(pChannel->uId, &bEncrypted);
    if (nResult != 0) {
        return nResult;
    }
    if (bEncrypted == 1) {
        nResult = RtChannelSendPublicKey(pChannel->pActiveLink);
        if (nResult != 0) {
            return nResult;
        }
        return RtChannelSetState(pChannel, kStateSentKey);
    }
    nResult = RtChannelSendRequest(pChannel->nMode,
                                   pChannel->pActiveLink,
                                   pChannel->config.uClientValue,
                                   pChannel->config.pRequestData,
                                   pChannel->config.uRequestSize);
    if (nResult != 0) {
        return nResult;
    }
    nResult = RtChannelSetState(pChannel, kStateSentRequest);
    if (nResult != 0) {
        return nResult;
    }
    return 0;
}

// NTSC-U/C: 0x002c1918, PAL: 0x002cb888
static int RtChannelDoClose(unsigned short uId) {
    RtChannel *pChannel;
    RtChannelLink *pLink;
    unsigned int uPending;
    int nResult;
    int i;

    pChannel = NULL;
    uPending = 0;
    if (g_bChannelsStarted != 1) {
        return RT_CHANNEL_RESULT_NOT_STARTED;
    }
    nResult = RtChannelGet(uId, &pChannel);
    if (nResult != 0) {
        return nResult;
    }
    if (pChannel == NULL) {
        return 0;
    }
    if (pChannel->nState != kStateClosed) {
        if (pChannel->nState == kStateConnected) {
            pLink = pChannel->pActiveLink;
            if (pLink != NULL && RtGetRingBufferUsed(&pLink->send, &uPending) == 0) {
                if (uPending != 0) {
                    (void)RtChannelReserveScratch(uPending); // Yes, the binary discards this.
                    if (RtPeekRingBuffer(g_pbChannelScratch, &pLink->send, uPending) == 0) {
                        (void)RtChannelSendBlocking(
                            pLink, g_pbChannelScratch, uPending, kCloseTimeout);
                    }
                }
                (void)RtChannelSetState(pChannel, kStateClosing);
                (void)RtChannelSendDisconnect(pLink);
            }
        }
        (void)RtChannelSetState(pChannel, kStateDisconnected);
        for (i = 0; i < RT_CHANNEL_LINK_COUNT; ++i) {
            pLink = pChannel->apLinks[i];
            if (pLink != NULL) {
                RtSocketClose(pLink->pSocket);
            }
        }
        (void)RtChannelSetState(pChannel, kStateClosed);
    }
    RtChannelDestroy(uId);
    return 0;
}

// NTSC-U/C: 0x002c24e8, PAL: 0x002cc458
static int RtChannelServiceSend(RtChannel *pChannel, RtChannelLink *pLink) {
    unsigned char bKeepalive;
    unsigned int uMilliseconds;
    unsigned int uPending;
    int bFlushed;
    int bFlushAgain;
    int nResult;

    bFlushed = 0;
    bFlushAgain = 0;
    bKeepalive = kKeepaliveByte;
    uPending = 0;
    uMilliseconds = 0;
    if (g_bChannelsStarted != 1) {
        return RT_CHANNEL_RESULT_NOT_STARTED;
    }
    if (RtChannelIsValid(pChannel) != 1 || RtChannelLinkIsValid(pLink) != 1 || pLink->bOpen != 1 ||
        pChannel->uId != pLink->uChannel) {
        return RT_CHANNEL_RESULT_INVALID;
    }
    if (pChannel->nState == kStateConnected) {
        if (pLink->uKeepaliveInterval != 0) {
            if (RtGetMillisecondsSince(&pLink->lastKeepalive, &uMilliseconds) != 0) {
                return RT_CHANNEL_RESULT_CLOCK;
            }
            if (pLink->uKeepaliveInterval < uMilliseconds) {
                nResult = RtChannelQueueRecord(pLink,
                                               kRecordKeepalive,
                                               RT_CHANNEL_ALL_PEERS,
                                               &bKeepalive,
                                               sizeof(bKeepalive),
                                               0);
                if (nResult != 0) {
                    return nResult;
                }
                if (RtReadTime(&pLink->lastKeepalive) != 0) {
                    return RT_CHANNEL_RESULT_CLOCK;
                }
                ++pLink->uPending;
            }
        }
    } else if (pChannel->nState == kStateConnecting &&
               pLink->nTransport == RT_CHANNEL_TRANSPORT_DATAGRAM) {
        if (RtGetMillisecondsSince(&pChannel->connectTime, &uMilliseconds) != 0) {
            return RT_CHANNEL_RESULT_CLOCK;
        }
        if (uMilliseconds > kConnectRetryInterval) {
            nResult = RtChannelSendConnect(pChannel, pLink);
            if (nResult != 0) {
                return nResult;
            }
        }
    }
    if (pLink->bPeerNoted == 1 && pLink->uPeer != RT_CHANNEL_ALL_PEERS) {
        nResult = RtChannelSendControl(pLink, pLink->uPeer);
        if (nResult != 0) {
            return nResult;
        }
    }
    if (RtGetMillisecondsSince(&pLink->lastSend, &uMilliseconds) != 0) {
        return RT_CHANNEL_RESULT_CLOCK;
    }
    if (pLink->uFlushInterval < uMilliseconds || pLink->uFlushInterval == 0 ||
        pLink->bPeerNoted == 1 || pChannel->nState != kStateConnected) {
        bFlushed = 1;
        if (RtChannelFlushPacket(pLink) != RT_CHANNEL_RESULT_NO_MEMORY) {
            pLink->bPeerNoted = 0;
        } else {
            bFlushAgain = 1;
        }
    }
    if (RtGetRingBufferUsed(&pLink->send, &uPending) != 0) {
        return RT_CHANNEL_RESULT_RING;
    }
    if (uPending == 0) {
        return 0;
    }
    if (bFlushed == 1) {
        nResult = RtChannelFlushSend(pLink);
        if (nResult != 0) {
            return nResult;
        }
        if (RtCompactRingBufferWith(&pLink->send, 0, NULL) != 0) {
            return RT_CHANNEL_RESULT_RING;
        }
    }
    if (bFlushAgain) {
        if (RtChannelFlushPacket(pLink) != RT_CHANNEL_RESULT_NO_MEMORY) {
            pLink->bPeerNoted = 0;
        }
    }
    return 0;
}

// NTSC-U/C: 0x002c2788, PAL: 0x002cc6f8
static int
RtChannelServiceReceive(RtChannel *pChannel, RtChannelLink *pLink, unsigned char bProcess) {
    int nResult;

    if (g_bChannelsStarted != 1) {
        return RT_CHANNEL_RESULT_NOT_STARTED;
    }
    if (RtChannelIsValid(pChannel) != 1 || RtChannelLinkIsValid(pLink) != 1 || pLink->bOpen != 1 ||
        pChannel->uId != pLink->uChannel) {
        return RT_CHANNEL_RESULT_INVALID;
    }
    nResult = RtChannelReceive(pLink);
    if (nResult != 0) {
        return nResult;
    }
    if (bProcess != 1) {
        return 0;
    }
    nResult = RtChannelProcessRecords(pChannel, pLink);
    if (nResult != 0) {
        return nResult;
    }
    return 0;
}

// NTSC-U/C: 0x002c2398, PAL: 0x002cc308
static int RtChannelServiceTimers(RtChannel *pChannel, unsigned char bProcess) {
    RtChannelLink *pLink;
    unsigned int uMilliseconds;
    int nResult;
    int i;

    uMilliseconds = 0;
    if (g_bChannelsStarted != 1) {
        return RT_CHANNEL_RESULT_NOT_STARTED;
    }
    if (RtChannelIsValid(pChannel) != 1) {
        return RT_CHANNEL_RESULT_INVALID;
    }
    for (i = 0; i < RT_CHANNEL_LINK_COUNT; ++i) {
        pLink = pChannel->apLinks[i];
        if (pLink != NULL && pLink->bOpen == 1) {
            nResult = RtChannelServiceSend(pChannel, pLink);
            if (nResult != 0) {
                return nResult;
            }
        }
    }
    for (i = 0; i < RT_CHANNEL_LINK_COUNT; ++i) {
        pLink = pChannel->apLinks[i];
        if (pLink != NULL && pLink->bOpen == 1) {
            nResult = RtChannelServiceReceive(pChannel, pLink, bProcess);
            if (nResult != 0) {
                return nResult;
            }
        }
    }
    if (pChannel->nState != kStateConnected) {
        return 0;
    }
    pLink = pChannel->pActiveLink;
    if (pLink->uKeepaliveInterval == 0) {
        return 0;
    }
    if (RtGetMillisecondsSince(&pLink->lastReceive, &uMilliseconds) != 0) {
        return RT_CHANNEL_RESULT_CLOCK;
    }
    if (uMilliseconds > kSilenceTimeout) {
        return RT_CHANNEL_API_RESULT_SILENT;
    }
    return 0;
}

// NTSC-U/C: 0x002c22c8, PAL: 0x002cc238
static int RtChannelUpdate(unsigned short uId, unsigned char bProcess) {
    RtChannel *pChannel;
    unsigned short uSelf;
    int nResult;
    int nError;

    pChannel = NULL;
    nResult = RtChannelGetValid(uId, &pChannel);
    if (nResult != 0) {
        return nResult;
    }
    nError = RtChannelServiceTimers(pChannel, bProcess);
    if (nError == 0) {
        return 0;
    }
    if (bProcess != 1) {
        return nError;
    }
    // Yes, the binary passes the state of the channel to the check of a record type.
    if (RtChannelIsDataRecord(pChannel->nState) != bProcess) {
        return nError;
    }
    if (pChannel->pfnConnect != NULL) {
        uSelf = pChannel->uId;
        pChannel->pfnConnect(uId, 0, 0, 0, 1);
        if (!RtChannelIsValidId(uSelf)) {
            return RT_CHANNEL_RESULT_CLOSED;
        }
    }
    (void)RtChannelSetState(pChannel, kStateFailed);
    return nError;
}

// NTSC-U/C: 0x002c1a80, PAL: 0x002cb9f0
static int RtChannelDoPumpOne(unsigned short uId, unsigned char bProcess) {
    return RtChannelUpdate(uId, bProcess);
}

// NTSC-U/C: 0x002c2860, PAL: 0x002cc7d0
static int RtChannelDoReceiveDatagrams(unsigned char bDeliver) {
    char szAddress[kAddressSize];
    int nPort;
    unsigned short uReceived;

    uReceived = 0;
    if (g_bChannelsStarted != 1) {
        return RT_CHANNEL_RESULT_NOT_STARTED;
    }
    if (g_channelService.bStarted != 1) {
        return RT_CHANNEL_API_RESULT_NO_SERVICE;
    }
    if (RtStreamTick(g_channelService.pStream) != 0) {
        return RT_CHANNEL_API_RESULT_STREAM;
    }
    if (bDeliver != 1) {
        return 0;
    }
    do {
        if (RtStreamReceive(g_channelService.pStream,
                            szAddress,
                            &nPort,
                            g_pbChannelScratch,
                            RT_CHANNEL_MAX_PACKET,
                            &uReceived) != 0) {
            return RT_CHANNEL_API_RESULT_STREAM;
        }
        if (uReceived == 0) {
            return 0;
        }
        if (uReceived > RT_CHANNEL_MAX_PACKET) {
            return RT_CHANNEL_API_RESULT_STREAM;
        }
        if (g_channelService.pfnDatagram != NULL) {
            g_channelService.pfnDatagram(
                szAddress, (unsigned short)nPort, g_pbChannelScratch, uReceived);
        }
    } while (uReceived != 0);
    return 0;
}

// NTSC-U/C: 0x002c2950, PAL: 0x002cc8c0
static int RtChannelDoAcceptPeers(void) {
    char szAddress[kAddressSize];
    unsigned short uPort;
    bool bAccepted;
    int nResult;

    uPort = 0;
    bAccepted = false;
    if (g_bChannelsStarted != 1 || g_channelListener.bOpen != 1) {
        return RT_CHANNEL_RESULT_NOT_STARTED;
    }
    do {
        nResult = RtChannelMapSocketResult(
            RtSocketAccept(g_channelListener.pSocket, &bAccepted, szAddress, &uPort));
        if (nResult != 0) {
            return nResult;
        }
        if (bAccepted != 1) {
            return 0;
        }
        if (g_channelListener.pfnAccept != NULL) {
            g_channelListener.pfnAccept(szAddress, uPort);
        }
    } while (bAccepted == 1);
    return 0;
}

// NTSC-U/C: 0x002c1aa0, PAL: 0x002cba10
static int RtChannelDoPumpAll(unsigned char bProcess) {
    unsigned short i;
    int nResult;

    if (g_bChannelsStarted != 1) {
        return RT_CHANNEL_RESULT_NOT_STARTED;
    }
    for (i = 0; i < g_uChannelCount; ++i) {
        if (RtChannelIsValidId(i) == 1) {
            nResult = RtChannelUpdate(i, bProcess);
            if (nResult != 0) {
                return nResult;
            }
        }
    }
    if (g_channelService.bStarted == 1) {
        nResult = RtChannelDoReceiveDatagrams(bProcess);
        if (nResult != 0) {
            return nResult;
        }
    }
    if (g_channelListener.bOpen == 1 && bProcess == 1) {
        nResult = RtChannelDoAcceptPeers();
        if (nResult != 0) {
            return nResult;
        }
    }
    return 0;
}

// NTSC-U/C: 0x002c1b98, PAL: 0x002cbb08
static int RtChannelDoSend(
    unsigned short uId, unsigned short uPeer, int nFlags, const void *pData, unsigned short uSize) {
    RtChannel *pChannel;
    RtChannelLink *pLink;
    RtChannelLink *pDatagramLink;
    int nType;
    int nResult;

    pChannel = NULL;
    if (g_bChannelsStarted != 1) {
        return RT_CHANNEL_RESULT_NOT_STARTED;
    }
    if (pData == NULL || uSize > RT_CHANNEL_PACKET_DATA) {
        return RT_CHANNEL_RESULT_INVALID;
    }
    nResult = RtChannelGetValid(uId, &pChannel);
    if (nResult != 0) {
        return nResult;
    }
    if (pChannel->nState != kStateConnected) {
        return RT_CHANNEL_RESULT_BAD_STATE;
    }
    pLink = pChannel->pActiveLink;
    if (pChannel->nMode == RT_CHANNEL_MODE_HOST) {
        pDatagramLink = pChannel->apLinks[RT_CHANNEL_LINK_DATAGRAM];
        if (pDatagramLink != NULL && pDatagramLink->bOpen == 1 &&
            (nFlags & kStreamOnlyFlags) == 0) {
            pLink = pDatagramLink;
        }
    }
    if (uPeer == RT_CHANNEL_MIXED_PEERS) {
        nType = kRecordToServer;
    } else if (uPeer != RT_CHANNEL_ALL_PEERS) {
        nType = kRecordToPeer;
    } else {
        nType = kRecordToAll;
    }
    return RtChannelQueueRecord(pLink, nType, uPeer, pData, uSize, (nFlags >> kTrackPeerShift) & 1);
}

// NTSC-U/C: 0x002c1c98, PAL: 0x002cbc08
static int RtChannelDoSendDatagram(const char *pszAddress,
                                   unsigned short uPort,
                                   int nFlags,
                                   const void *pData,
                                   unsigned short uSize) {
    unsigned short uSent;

    uSent = 0;
    if (g_bChannelsStarted != 1) {
        return RT_CHANNEL_RESULT_NOT_STARTED;
    }
    if (g_channelService.bStarted != 1) {
        return RT_CHANNEL_API_RESULT_NO_SERVICE;
    }
    if (pszAddress == NULL || strlen(pszAddress) >= kAddressSize || pData == NULL || uSize == 0 ||
        uSize > RT_CHANNEL_PACKET_DATA) {
        return RT_CHANNEL_RESULT_INVALID;
    }
    if (RtStreamSend(g_channelService.pStream, nFlags, pszAddress, uPort, pData, uSize, &uSent) !=
        0) {
        return RT_CHANNEL_API_RESULT_STREAM;
    }
    if (uSent == 0) {
        return RT_CHANNEL_API_RESULT_NOT_SENT;
    }
    if (uSent != uSize) {
        return RT_CHANNEL_API_RESULT_PARTIAL;
    }
    return 0;
}

// NTSC-U/C: 0x002c1d90, PAL: 0x002cbd00
static int RtChannelDoPollListener(bool *pbReady, const char *pszAddress, unsigned short uPort) {
    if (pbReady == NULL) {
        return RT_CHANNEL_RESULT_INVALID;
    }
    *pbReady = false;
    if (g_bChannelsStarted != 1 || g_channelListener.bOpen != 1) {
        return RT_CHANNEL_RESULT_NOT_STARTED;
    }
    if (pszAddress == NULL || strlen(pszAddress) >= kAddressSize) {
        return RT_CHANNEL_RESULT_INVALID;
    }
    return RtChannelMapSocketResult(
        RtSocketPollListener(g_channelListener.pSocket, pbReady, pszAddress, uPort));
}

// NTSC-U/C: 0x002c1ea0, PAL: 0x002cbe10
static int RtChannelDoGetServerName(unsigned short uId, char *pszName) {
    RtChannel *pChannel;
    int nResult;

    pChannel = NULL;
    if (g_bChannelsStarted != 1) {
        return RT_CHANNEL_RESULT_NOT_STARTED;
    }
    if (pszName == NULL) {
        return RT_CHANNEL_RESULT_INVALID;
    }
    nResult = RtChannelGetValid(uId, &pChannel);
    if (nResult != 0) {
        return nResult;
    }
    if (pChannel->nState != kStateConnected) {
        return RT_CHANNEL_RESULT_BAD_STATE;
    }
    strcpy(pszName, pChannel->szServerName);
    return 0;
}

// NTSC-U/C: 0x002c1f18, PAL: 0x002cbe88
static int RtChannelDoSendFlag(unsigned short uId, unsigned char bFlag) {
    unsigned char abRecord[kFlagRecordSize];
    RtChannel *pChannel;
    int nResult;

    pChannel = NULL;
    if (g_bChannelsStarted != 1) {
        return RT_CHANNEL_RESULT_NOT_STARTED;
    }
    nResult = RtChannelGetValid(uId, &pChannel);
    if (nResult != 0) {
        return nResult;
    }
    if (RtRecordWriteByte(&abRecord[0], kFlagRecordTag) != 0 ||
        RtRecordWriteByte(&abRecord[1], bFlag == 1) != 0) {
        return RT_CHANNEL_RESULT_FIELD;
    }
    return RtChannelQueueRecord(
        pChannel->pActiveLink, kRecordFlag, RT_CHANNEL_ALL_PEERS, abRecord, kFlagRecordSize, 0);
}

// NTSC-U/C: 0x002c1fd0, PAL: 0x002cbf40
static int RtChannelDoDefaultConfig(RtChannelConfig *pConfig) {
    if (pConfig == NULL) {
        return RT_CHANNEL_RESULT_INVALID;
    }
    memset(pConfig, 0, sizeof(RtChannelConfig));
    pConfig->nMode = RT_CHANNEL_MODE_CLIENT;
    pConfig->uSendSize = kDefaultQueueSize;
    pConfig->uLocalPort = kDefaultPort;
    pConfig->uKeepaliveInterval = kDefaultKeepaliveInterval;
    pConfig->uFlushInterval = kDefaultFlushInterval;
    pConfig->pRequestData = NULL;
    pConfig->uRequestSize = 0;
    pConfig->pfnPeer = NULL;
    pConfig->pfnData = NULL;
    pConfig->pfnStatus = NULL;
    pConfig->uReceiveSize = kDefaultQueueSize;
    return 0;
}

// NTSC-U/C: 0x002c2048, PAL: 0x002cbfb8
static int RtChannelDoDefaultServiceConfig(RtChannelServiceConfig *pConfig) {
    if (pConfig == NULL) {
        return RT_CHANNEL_RESULT_INVALID;
    }
    memset(pConfig, 0, sizeof(RtChannelServiceConfig));
    pConfig->uPort = kDefaultPort;
    pConfig->uSendSize = kDefaultQueueSize;
    pConfig->uFlushInterval = kDefaultFlushInterval;
    pConfig->uPeerTimeout = kDefaultPeerTimeout;
    pConfig->pfnTimeout = NULL;
    pConfig->pfnDatagram = NULL;
    pConfig->uReceiveSize = kDefaultQueueSize;
    return 0;
}

// NTSC-U/C: 0x002c20b0, PAL: 0x002cc020
static int RtChannelDoDropPeer(const char *pszAddress, unsigned short uPort) {
    if (g_bChannelsStarted != 1) {
        return RT_CHANNEL_RESULT_NOT_STARTED;
    }
    if (g_channelService.bStarted != 1) {
        return RT_CHANNEL_API_RESULT_NO_SERVICE;
    }
    if (pszAddress == NULL || strlen(pszAddress) >= kAddressSize) {
        return RT_CHANNEL_RESULT_INVALID;
    }
    if (RtStreamDisconnect(g_channelService.pStream, pszAddress, uPort, 0) != 0) {
        return RT_CHANNEL_API_RESULT_STREAM;
    }
    return 0;
}

// NTSC-U/C: 0x002c21c0, PAL: 0x002cc130
static void
RtChannelOnMapPort(RtSocket *pStream, const char *pszAddress, unsigned short uPort, int nFailed) {
    (void)pStream;
    if (g_channelService.bStarted == 1 && g_channelService.pfnMap != NULL) {
        g_channelService.pfnMap(pszAddress, uPort, nFailed != 0 ? RT_CHANNEL_API_RESULT_STREAM : 0);
    }
}

// NTSC-U/C: 0x002c2148, PAL: 0x002cc0b8
static int
RtChannelDoMapPort(const char *pszGateway, unsigned short uPort, RtChannelMapFunc pfnDone) {
    if (g_bChannelsStarted != 1) {
        return RT_CHANNEL_RESULT_NOT_STARTED;
    }
    if (g_channelService.bStarted != 1) {
        return RT_CHANNEL_API_RESULT_NO_SERVICE;
    }
    if (pszGateway == NULL || pfnDone == NULL) {
        return RT_CHANNEL_RESULT_INVALID;
    }
    g_channelService.pfnMap = pfnDone;
    if (RtStreamMapPort(g_channelService.pStream, pszGateway, uPort, RtChannelOnMapPort) != 0) {
        return RT_CHANNEL_API_RESULT_STREAM;
    }
    return 0;
}

// NTSC-U/C: 0x002c2248, PAL: 0x002cc1b8
static int RtChannelStopService(void) {
    if (g_channelService.bStarted != 1) {
        return RT_CHANNEL_API_RESULT_NO_SERVICE;
    }
    g_channelService.bStarted = 0;
    if (RtStreamClose(g_channelService.pStream) != 0) {
        return RT_CHANNEL_API_RESULT_STREAM;
    }
    memset(&g_channelService, 0, sizeof(g_channelService));
    if (RtStreamCleanup() != 0) {
        return RT_CHANNEL_API_RESULT_STREAM;
    }
    RtDestroyConnectionKeys(RT_SECURITY_SERVER_ID);
    return 0;
}

int RtChannelApiStartup(const RtKeyPair *pKeyPair) {
    int nResult;

    if (g_nChannelApiRefCount == 0) {
        if (RtCreateMutex(&g_channelApiMutex) != 0) {
            return RT_CHANNEL_API_RESULT_LOCK;
        }
        g_bChannelApiStarted = 1;
        nResult = RtChannelMapSocketResult(RtSocketStartup());
        if (nResult != 0) {
            return nResult;
        }
        g_uChannelCount = 0;
        g_ppChannels = NULL;
        memset(&g_channelService, 0, sizeof(g_channelService));
        memset(&g_channelListener, 0, sizeof(g_channelListener));
        RtStoreKeyPair(0, pKeyPair);
        g_bChannelsStarted = 1;
    }
    ++g_nChannelApiRefCount;
    return 0;
}

// NTSC-U/C: 0x002c1098, PAL: 0x002cb008
static int RtChannelStopLayer(void) {
    unsigned short i;
    int nResult;

    if (g_nChannelApiRefCount <= 0 || g_bChannelsStarted != 1) {
        return RT_CHANNEL_RESULT_NOT_STARTED;
    }
    if (g_nChannelApiRefCount == 1) {
        for (i = 0; i < g_uChannelCount; ++i) {
            if (g_ppChannels[i] == NULL) {
                continue;
            }
            if (RtChannelIsValidId(i) == 1) {
                RtChannelDoClose(i);
            } else {
                RtChannelDestroy(i);
            }
        }
        if (g_channelService.bStarted == 1) {
            RtChannelStopService();
        }
        if (g_channelListener.bOpen == 1) {
            RtChannelDoCloseListener();
        }
        nResult = RtChannelMapSocketResult(RtSocketShutdown());
        if (nResult != 0) {
            return nResult;
        }
        if (g_ppChannels != NULL) {
            RtFreeBlock((void **)&g_ppChannels);
        }
        g_ppChannels = NULL;
        g_uChannelCount = 0;
        if (g_pbChannelScratch != NULL) {
            RtFreeBlock((void **)&g_pbChannelScratch);
            g_uChannelScratchSize = 0;
        }
        RtDestroyConnectionKeysOfClass(0);
        g_bChannelsStarted = 0;
    }
    --g_nChannelApiRefCount;
    return 0;
}

int RtChannelApiShutdown(void) {
    int nResult;

    if (g_bChannelApiStarted != 1) {
        return RT_CHANNEL_API_RESULT_LOCK;
    }
    if (RtLockMutex(&g_channelApiMutex, NULL) != 0) {
        return RT_CHANNEL_API_RESULT_LOCK;
    }
    nResult = RtChannelStopLayer();
    if (RtUnlockMutex(&g_channelApiMutex, NULL) != 0) {
        return RT_CHANNEL_API_RESULT_LOCK;
    }
    if (nResult != 0 || g_nChannelApiRefCount != 0) {
        return nResult;
    }
    if (RtDeleteMutex(&g_channelApiMutex) != 0) {
        return RT_CHANNEL_API_RESULT_LOCK;
    }
    g_bChannelApiStarted = 0;
    return nResult;
}

int RtChannelApiCloseListener(void) {
    int nResult;

    nResult = RtChannelApiEnter();
    if (nResult != 0) {
        return nResult;
    }
    return RtChannelApiLeave(RtChannelDoCloseListener());
}

int RtChannelApiStartService(const RtChannelServiceConfig *pConfig) {
    int nResult;

    nResult = RtChannelApiEnter();
    if (nResult != 0) {
        return nResult;
    }
    return RtChannelApiLeave(RtChannelDoStartService(pConfig));
}

int RtChannelApiOpen(unsigned short *puId, const RtChannelConfig *pConfig) {
    int nResult;

    nResult = RtChannelApiEnter();
    if (nResult != 0) {
        return nResult;
    }
    return RtChannelApiLeave(RtChannelDoOpen(puId, pConfig));
}

int RtChannelApiClose(unsigned short uId) {
    int nResult;

    nResult = RtChannelApiEnter();
    if (nResult != 0) {
        return nResult;
    }
    return RtChannelApiLeave(RtChannelDoClose(uId));
}

int RtChannelApiPumpOne(unsigned short uId, unsigned char bProcess) {
    int nResult;

    nResult = RtChannelApiEnter();
    if (nResult != 0) {
        return nResult;
    }
    return RtChannelApiLeave(RtChannelDoPumpOne(uId, bProcess));
}

int RtChannelApiReceiveDatagrams(unsigned char bDeliver) {
    int nResult;

    nResult = RtChannelApiEnter();
    if (nResult != 0) {
        return nResult;
    }
    return RtChannelApiLeave(RtChannelDoReceiveDatagrams(bDeliver));
}

int RtChannelApiAcceptPeers(void) {
    int nResult;

    nResult = RtChannelApiEnter();
    if (nResult != 0) {
        return nResult;
    }
    return RtChannelApiLeave(RtChannelDoAcceptPeers());
}

int RtChannelApiPumpAll(unsigned char bProcess) {
    int nResult;

    nResult = RtChannelApiEnter();
    if (nResult != 0) {
        return nResult;
    }
    return RtChannelApiLeave(RtChannelDoPumpAll(bProcess));
}

int RtChannelApiSend(
    unsigned short uId, unsigned short uPeer, int nFlags, const void *pData, unsigned short uSize) {
    int nResult;

    nResult = RtChannelApiEnter();
    if (nResult != 0) {
        return nResult;
    }
    return RtChannelApiLeave(RtChannelDoSend(uId, uPeer, nFlags, pData, uSize));
}

int RtChannelApiSendDatagram(const char *pszAddress,
                             unsigned short uPort,
                             int nFlags,
                             const void *pData,
                             unsigned short uSize) {
    int nResult;

    nResult = RtChannelApiEnter();
    if (nResult != 0) {
        return nResult;
    }
    return RtChannelApiLeave(RtChannelDoSendDatagram(pszAddress, uPort, nFlags, pData, uSize));
}

int RtChannelApiPollListener(bool *pbReady, const char *pszAddress, unsigned short uPort) {
    int nResult;

    nResult = RtChannelApiEnter();
    if (nResult != 0) {
        return nResult;
    }
    return RtChannelApiLeave(RtChannelDoPollListener(pbReady, pszAddress, uPort));
}

int RtChannelApiGetLocalAddress(char *pszAddress) {
    int nResult;

    nResult = RtChannelApiEnter();
    if (nResult != 0) {
        return nResult;
    }
    return RtChannelApiLeave(RtChannelDoGetLocalAddress(pszAddress));
}

int RtChannelApiGetServerName(unsigned short uId, char *pszName) {
    int nResult;

    nResult = RtChannelApiEnter();
    if (nResult != 0) {
        return nResult;
    }
    return RtChannelApiLeave(RtChannelDoGetServerName(uId, pszName));
}

int RtChannelApiSendFlag(unsigned short uId, unsigned char bFlag) {
    int nResult;

    nResult = RtChannelApiEnter();
    if (nResult != 0) {
        return nResult;
    }
    return RtChannelApiLeave(RtChannelDoSendFlag(uId, bFlag));
}

int RtChannelApiDefaultConfig(RtChannelConfig *pConfig) {
    int nResult;

    nResult = RtChannelApiEnter();
    if (nResult != 0) {
        return nResult;
    }
    return RtChannelApiLeave(RtChannelDoDefaultConfig(pConfig));
}

int RtChannelApiDefaultServiceConfig(RtChannelServiceConfig *pConfig) {
    int nResult;

    nResult = RtChannelApiEnter();
    if (nResult != 0) {
        return nResult;
    }
    return RtChannelApiLeave(RtChannelDoDefaultServiceConfig(pConfig));
}

int RtChannelApiDropPeer(const char *pszAddress, unsigned short uPort) {
    int nResult;

    nResult = RtChannelApiEnter();
    if (nResult != 0) {
        return nResult;
    }
    return RtChannelApiLeave(RtChannelDoDropPeer(pszAddress, uPort));
}

int RtChannelApiMapPort(const char *pszGateway, unsigned short uPort, RtChannelMapFunc pfnDone) {
    int nResult;

    nResult = RtChannelApiEnter();
    if (nResult != 0) {
        return nResult;
    }
    return RtChannelApiLeave(RtChannelDoMapPort(pszGateway, uPort, pfnDone));
}
