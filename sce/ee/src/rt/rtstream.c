#include "rt/rtstream.h"

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "rt/rtmemory.h"
#include "rt/rtsocket.h"
#include "rt/rtstats.h"

enum {
    kStreamSend = 0,
    kStreamReceive = 1,
    kStreamDirections = 2,
    kStreamScratchSize = 0x10000,
    kStreamWorkSize = 0x4000,
    kStreamMinTxQueueSize = 0x4000,
    kStreamMinRxQueueSize = 0x1000,
    kStreamListSize = 512,
    kStreamDefaultPeerTimeout = 45000,
    kStreamCloseMilliseconds = 500,
    kStreamMaxRecord = 512,
    // The size of the buffer of a queued datagram.
    kStreamDatagramBufferSize = 536,
};

// NTSC-U/C: 0x003b6ce0
static unsigned int g_uStreamQueryInterval = 500;
// NTSC-U/C: 0x003b6ce4
static unsigned int g_uStreamPeerTimeout = kStreamDefaultPeerTimeout;
// NTSC-U/C: 0x003b6ce8
static RtStreamLatest *g_apStreamLatest[kStreamDirections];
// NTSC-U/C: 0x003b6cf0
static int g_anStreamLatestCount[kStreamDirections];
// NTSC-U/C: 0x003b6cf8
static RtStreamSlot *g_apStreamSlots[kStreamDirections];
// NTSC-U/C: 0x003b6d00
static int g_anStreamSlotCount[kStreamDirections];
// NTSC-U/C: 0x003b6d08
int g_nStreamMergeSize;
// NTSC-U/C: 0x003b6d0c
RtUpnpContext *g_pStreamUpnp;
// NTSC-U/C: 0x003b6d14
RtStreamMappingFunc g_pfnStreamMapping;
// NTSC-U/C: 0x003b6d18
static unsigned char g_bStreamOpen;
// NTSC-U/C: 0x003b6d1a
unsigned short g_uStreamGatewayPort;
// The sequences an acknowledgement listed.
// NTSC-U/C: 0x00520620
static unsigned short g_auStreamAcked[kStreamListSize];
// The queried sequences held, for the acknowledgement.
// NTSC-U/C: 0x00520a20
static unsigned short g_auStreamHeld[kStreamListSize];
// The sequences a query listed.
// NTSC-U/C: 0x00520e20
static unsigned short g_auStreamQueried[kStreamListSize];
// The datagram RtStreamReceive() takes from the queue.
// NTSC-U/C: 0x00521220
static unsigned char g_abStreamDatagram[kStreamDatagramBufferSize];
// The data of the reliable records sent, kept until acknowledged.
// NTSC-U/C: 0x00521428
static unsigned char *g_pbStreamScratch;
// NTSC-U/C: 0x0052142c
static int g_nStreamScratchUsed;
// The data of the reliable records received out of order.
// NTSC-U/C: 0x00521430
static unsigned char *g_pbStreamWork;
// NTSC-U/C: 0x00521434
static int g_nStreamWorkUsed;
// NTSC-U/C: 0x00521438
int g_nStreamSocket;
// NTSC-U/C: 0x0052143c
static unsigned short g_uStreamPort;
// NTSC-U/C: 0x00521440
unsigned char *g_pbStreamTxQueue;
// NTSC-U/C: 0x00521444
unsigned char *g_pbStreamTxQueueEnd;
// NTSC-U/C: 0x00521448
int g_nStreamTxQueueSize;
// NTSC-U/C: 0x0052144c
static RtStreamTimeoutFunc g_pfnStreamTimeout;
// NTSC-U/C: 0x00521450
static RtStreamPeer g_aStreamPeers[RT_STREAM_MAX_PEERS];
// NTSC-U/C: 0x00521648
unsigned char g_abStreamMerge[RT_STREAM_DATAGRAM_SIZE];
// Where RtStreamReceive() writes the next data.
// NTSC-U/C: 0x00521850
static unsigned char *g_pbStreamOut;
// NTSC-U/C: 0x00521854
static unsigned int g_uStreamOutRoom;
// NTSC-U/C: 0x00521858
int g_nStreamMappedService;
// NTSC-U/C: 0x005219b8
RtTime g_streamLastFlush;
// NTSC-U/C: 0x005219c0
unsigned char g_uStreamFlushCount;
// NTSC-U/C: 0x005219c4
unsigned int g_uStreamFlushInterval;
// NTSC-U/C: 0x005219c8
unsigned char *g_pbStreamRxQueue;
// NTSC-U/C: 0x005219cc
unsigned char *g_pbStreamRxQueueEnd;
// NTSC-U/C: 0x005219d0
int g_nStreamRxQueueSize;
// NTSC-U/C: 0x005219d8
char g_szStreamGateway[RT_STREAM_ADDRESS_SIZE];

static inline int RtStreamIsSameAddress(const RtIpv4Address *pLeft, const RtIpv4Address *pRight) {
    return memcmp(pLeft, pRight, sizeof(RtIpv4Address)) == 0;
}

static inline int RtStreamIsAddressZero(const RtIpv4Address *pAddress) {
    static const RtIpv4Address kZero;

    return RtStreamIsSameAddress(pAddress, &kZero);
}

// NTSC-U/C: 0x002c6cc8, PAL: 0x002d0c38
static void RtStreamRemoveWork(unsigned char *pbSpan, int nSize) {
    int nAfter;

    if (pbSpan == NULL || nSize <= 0 || pbSpan < g_pbStreamWork) {
        return;
    }
    if (g_nStreamWorkUsed < nSize) {
        return;
    }
    nAfter = g_nStreamWorkUsed - nSize - (pbSpan - g_pbStreamWork);
    if (nAfter > 0) {
        memmove(pbSpan, &pbSpan[nSize], nAfter);
    }
    g_nStreamWorkUsed -= nSize;
}

// NTSC-U/C: 0x002c6d50, PAL: 0x002d0cc0
static int RtStreamGetScratchRoom(int nStream) {
    (void)nStream;
    return kStreamScratchSize - g_nStreamScratchUsed;
}

// NTSC-U/C: 0x002c6d68, PAL: 0x002d0cd8
static void RtStreamRemoveScratch(unsigned char *pbSpan, int nSize) {
    int nAfter;

    if (pbSpan == NULL || nSize <= 0 || pbSpan < g_pbStreamScratch) {
        return;
    }
    if (g_nStreamScratchUsed < nSize || g_nStreamScratchUsed > kStreamScratchSize) {
        return;
    }
    nAfter = g_nStreamScratchUsed - nSize - (pbSpan - g_pbStreamScratch);
    if (nAfter > 0) {
        memmove(pbSpan, &pbSpan[nSize], nAfter);
    }
    g_nStreamScratchUsed -= nSize;
}

// NTSC-U/C: 0x002c6e08, PAL: 0x002d0d78
static unsigned char *RtStreamAppendScratch(int nStream, const void *pData, int nSize) {
    unsigned char *pbDest;

    (void)nStream;
    pbDest = &g_pbStreamScratch[g_nStreamScratchUsed];
    if (nSize <= 0 || pData == NULL || g_nStreamScratchUsed + nSize > kStreamScratchSize) {
        return NULL;
    }
    memcpy(pbDest, pData, nSize);
    g_nStreamScratchUsed += nSize;
    return pbDest;
}

// NTSC-U/C: 0x002c6e90, PAL: 0x002d0e00
static unsigned char *RtStreamAppendWork(int nStream, const void *pData, int nSize) {
    unsigned char *pbDest;

    (void)nStream;
    pbDest = &g_pbStreamWork[g_nStreamWorkUsed];
    if (nSize <= 0 || pData == NULL || g_nStreamWorkUsed + nSize > kStreamWorkSize) {
        return NULL;
    }
    memcpy(pbDest, pData, nSize);
    g_nStreamWorkUsed += nSize;
    return pbDest;
}

// NTSC-U/C: 0x002c6f10, PAL: 0x002d0e80
static RtStreamSlot *
RtStreamFindSlot(int nStream, unsigned int uDirection, const RtIpv4Address *pAddress, int nPort) {
    RtStreamSlot *pSlot;
    int i;

    (void)nStream;
    if (uDirection >= kStreamDirections || pAddress == NULL || nPort <= 0) {
        return NULL;
    }
    pSlot = g_apStreamSlots[uDirection];
    for (i = 0; i < RT_STREAM_MAX_PEERS; ++i, ++pSlot) {
        if (RtStreamIsSameAddress(&pSlot->address, pAddress) && pSlot->uPort == nPort) {
            return pSlot;
        }
    }
    return NULL;
}

// NTSC-U/C: 0x002c6f88, PAL: 0x002d0ef8
static RtStreamSlot *RtStreamGetSlot(int nStream, unsigned int uDirection, unsigned int uIndex) {
    (void)nStream;
    if (uDirection >= kStreamDirections || uIndex >= RT_STREAM_MAX_PEERS) {
        return NULL;
    }
    return &g_apStreamSlots[uDirection][uIndex];
}

// NTSC-U/C: 0x002c6fc0, PAL: 0x002d0f30
static RtStreamSlot *
RtStreamAllocSlot(int nStream, unsigned int uDirection, const RtIpv4Address *pAddress, int nPort) {
    RtStreamSlot *pSlot;
    unsigned int uNow;
    int i;

    if (uDirection >= kStreamDirections || pAddress == NULL || nPort <= 0) {
        return NULL;
    }
    RtGetMilliseconds(&uNow);
    pSlot = RtStreamGetSlot(nStream, uDirection, g_anStreamSlotCount[uDirection]++);
    if (pSlot == NULL) {
        return NULL;
    }
    pSlot->uLastQuery = 0;
    pSlot->uSequence = 0;
    pSlot->nRecordCount = 0;
    pSlot->nNextRecord = 0;
    pSlot->uPort = nPort;
    pSlot->uLastAck = uNow;
    pSlot->address = *pAddress;
    for (i = RT_STREAM_MAX_RECORDS - 1; i >= 0; --i) {
        pSlot->apbData[i] = NULL;
    }
    return pSlot;
}

// NTSC-U/C: 0x002c70b8, PAL: 0x002d1028
static RtStreamSlot *RtStreamGetOrAllocSlot(int nStream,
                                            unsigned int uDirection,
                                            const RtIpv4Address *pAddress,
                                            int nPort) {
    RtStreamSlot *pSlot;

    if (uDirection >= kStreamDirections || pAddress == NULL || nPort <= 0) {
        return NULL;
    }
    pSlot = RtStreamFindSlot(nStream, uDirection, pAddress, nPort);
    if (pSlot == NULL) {
        pSlot = RtStreamAllocSlot(nStream, uDirection, pAddress, nPort);
    }
    return pSlot;
}

// NTSC-U/C: 0x002c7138, PAL: 0x002d10a8
static RtStreamLatest *
RtStreamFindLatest(int nStream, unsigned int uDirection, const RtIpv4Address *pAddress, int nPort) {
    RtStreamLatest *pLatest;
    int i;

    (void)nStream;
    if (uDirection >= kStreamDirections || pAddress == NULL || nPort <= 0) {
        return NULL;
    }
    for (i = 0; i < RT_STREAM_MAX_PEERS; ++i) {
        pLatest = &g_apStreamLatest[uDirection][i];
        if (RtStreamIsSameAddress(&pLatest->address, pAddress) && pLatest->uPort == nPort) {
            return pLatest;
        }
    }
    return NULL;
}

// NTSC-U/C: 0x002c71b8, PAL: 0x002d1128
static int RtStreamClearLatest(int nStream, const RtIpv4Address *pAddress, int nPort) {
    RtStreamLatest *pLatest;
    unsigned int uDirection;

    if (pAddress == NULL || nPort <= 0) {
        return 1;
    }
    for (uDirection = 0; uDirection < kStreamDirections; ++uDirection) {
        pLatest = RtStreamFindLatest(nStream, uDirection, pAddress, nPort);
        if (pLatest != NULL) {
            pLatest->uSequence = 0;
            memset(&pLatest->address, 0, sizeof(RtIpv4Address));
            pLatest->uPort = 0;
        }
    }
    return 0;
}

// NTSC-U/C: 0x002c7240, PAL: 0x002d11b0
static int RtStreamResetLatest(void) {
    unsigned int uDirection;
    int i;

    for (uDirection = 0; uDirection < kStreamDirections; ++uDirection) {
        for (i = 0; i < RT_STREAM_MAX_PEERS; ++i) {
            memset(&g_apStreamLatest[uDirection][i].address, 0, sizeof(RtIpv4Address));
            g_apStreamLatest[uDirection][i].uPort = 0;
            g_apStreamLatest[uDirection][i].uSequence = 0;
        }
        g_anStreamLatestCount[uDirection] = 0;
    }
    return 0;
}

// NTSC-U/C: 0x002c72a0, PAL: 0x002d1210
static RtStreamLatest *
RtStreamGetLatest(int nStream, unsigned int uDirection, unsigned int uIndex) {
    (void)nStream;
    if (uDirection >= kStreamDirections || uIndex >= RT_STREAM_MAX_PEERS) {
        return NULL;
    }
    return &g_apStreamLatest[uDirection][uIndex];
}

// NTSC-U/C: 0x002c72d8, PAL: 0x002d1248
static RtStreamLatest *
RtStreamAddLatest(int nStream, unsigned int uDirection, const RtIpv4Address *pAddress, int nPort) {
    RtStreamLatest *pLatest;
    int bFound;
    int i;

    pLatest = NULL;
    bFound = 0;
    if (uDirection >= kStreamDirections || pAddress == NULL || nPort <= 0) {
        return NULL;
    }
    for (i = 0; i < g_anStreamLatestCount[uDirection]; ++i) {
        pLatest = &g_apStreamLatest[uDirection][i];
        if (RtStreamIsAddressZero(&pLatest->address)) {
            bFound = 1;
            break;
        }
    }
    if (!bFound) {
        pLatest = RtStreamGetLatest(nStream, uDirection, g_anStreamLatestCount[uDirection]++);
    }
    if (pLatest == NULL) {
        return NULL;
    }
    pLatest->uPort = nPort;
    pLatest->uSequence = 0;
    pLatest->address = *pAddress;
    return pLatest;
}

// NTSC-U/C: 0x002c73d8, PAL: 0x002d1348
static RtStreamLatest *RtStreamGetOrAddLatest(int nStream,
                                              unsigned int uDirection,
                                              const RtIpv4Address *pAddress,
                                              int nPort) {
    RtStreamLatest *pLatest;

    if (uDirection >= kStreamDirections || pAddress == NULL || nPort <= 0) {
        return NULL;
    }
    pLatest = RtStreamFindLatest(nStream, uDirection, pAddress, nPort);
    if (pLatest == NULL) {
        pLatest = RtStreamAddLatest(nStream, uDirection, pAddress, nPort);
    }
    return pLatest;
}

// Forget a received record and close the gap it leaves in the work buffer.
// NTSC-U/C: 0x002c7458, PAL: 0x002d13c8
static void RtStreamRemoveReceived(RtStreamSlot *pSlot, unsigned int uIndex) {
    RtStreamSlot *pOther;
    unsigned char *pbData;
    int nSize;
    int i;
    int j;

    if (pSlot == NULL || uIndex >= RT_STREAM_MAX_RECORDS) {
        return;
    }
    nSize = pSlot->anSize[uIndex];
    pbData = pSlot->apbData[uIndex];
    RtStreamRemoveWork(pbData, nSize);
    pSlot->apbData[uIndex] = NULL;
    pSlot->anSize[uIndex] = 0;
    for (i = 0; i < g_anStreamSlotCount[kStreamReceive]; ++i) {
        pOther = RtStreamGetSlot(g_nStreamSocket, kStreamReceive, i);
        for (j = 0; j < RT_STREAM_MAX_RECORDS; ++j) {
            if (pbData < pOther->apbData[j]) {
                pOther->apbData[j] -= nSize;
            }
        }
    }
}

// Close the gap a sent record leaves in the scratch buffer.
// NTSC-U/C: 0x002c7548, PAL: 0x002d14b8
static void RtStreamReleaseSent(RtStreamSlot *pSlot, unsigned int uIndex) {
    RtStreamSlot *pOther;
    unsigned char *pbData;
    int nSize;
    int i;
    int j;

    if (pSlot == NULL || uIndex >= RT_STREAM_MAX_RECORDS) {
        return;
    }
    nSize = pSlot->anSize[uIndex];
    pbData = pSlot->apbData[uIndex];
    RtStreamRemoveScratch(pbData, nSize);
    for (i = 0; i < g_anStreamSlotCount[kStreamSend]; ++i) {
        pOther = RtStreamGetSlot(g_nStreamSocket, kStreamSend, i);
        for (j = 0; j < pOther->nRecordCount; ++j) {
            if (pbData < pOther->apbData[j]) {
                pOther->apbData[j] -= nSize;
            }
        }
    }
}

// Forget a sent record and close up the records after it.
// NTSC-U/C: 0x002c7628, PAL: 0x002d1598
static void RtStreamRemoveSent(RtStreamSlot *pSlot, unsigned int uIndex) {
    int nAfter;

    if (pSlot == NULL || uIndex >= RT_STREAM_MAX_RECORDS) {
        return;
    }
    RtStreamReleaseSent(pSlot, uIndex);
    nAfter = pSlot->nRecordCount - uIndex - 1;
    if (nAfter > 0) {
        memmove(&pSlot->apbData[uIndex],
                &pSlot->apbData[uIndex + 1],
                nAfter * sizeof(pSlot->apbData[0]));
        memmove(&pSlot->auSequence[uIndex],
                &pSlot->auSequence[uIndex + 1],
                nAfter * sizeof(pSlot->auSequence[0]));
        memmove(
            &pSlot->auTime[uIndex], &pSlot->auTime[uIndex + 1], nAfter * sizeof(pSlot->auTime[0]));
        memmove(
            &pSlot->anSize[uIndex], &pSlot->anSize[uIndex + 1], nAfter * sizeof(pSlot->anSize[0]));
        memmove(&pSlot->auFlags[uIndex],
                &pSlot->auFlags[uIndex + 1],
                nAfter * sizeof(pSlot->auFlags[0]));
    }
    --pSlot->nRecordCount;
}

// NTSC-U/C: 0x002c7708, PAL: 0x002d1678
static void RtStreamFindHighest(int *pnIndex, const RtStreamSlot *pSlot) {
    unsigned int uHighest;
    int i;

    if (pSlot == NULL || pnIndex == NULL) {
        return;
    }
    // Yes, the binary starts from the largest value, so no record is ever found.
    uHighest = 0xffffffffU;
    *pnIndex = -1;
    for (i = 0; i < RT_STREAM_MAX_RECORDS; ++i) {
        if (uHighest < pSlot->auSequence[i] && pSlot->apbData[i] != NULL) {
            *pnIndex = i;
            uHighest = pSlot->auSequence[i];
        }
    }
}

// NTSC-U/C: 0x002c7768, PAL: 0x002d16d8
static void RtStreamFindOldest(int *pnIndex, const RtStreamSlot *pSlot) {
    unsigned int uOldest;
    int i;

    if (pSlot == NULL || pnIndex == NULL) {
        return;
    }
    uOldest = pSlot->auTime[0];
    *pnIndex = 0;
    for (i = 1; i < pSlot->nRecordCount; ++i) {
        if (pSlot->auTime[i] < uOldest) {
            *pnIndex = i;
            uOldest = pSlot->auTime[i];
        }
    }
}

// Keep a sent record until it is acknowledged.
// NTSC-U/C: 0x002c77d0, PAL: 0x002d1740
static int
RtStreamKeepSent(RtStreamSlot *pSlot, unsigned char *pbData, int nSize, unsigned char uFlags) {
    unsigned short uSequence;
    int nCount;
    int nOldest;
    int nOldestSize;

    if (pSlot == NULL || pbData == NULL || nSize <= 0) {
        return -1;
    }
    uSequence = RtStreamNextSequence(pSlot->uSequence);
    nCount = pSlot->nRecordCount;
    if (nCount >= RT_STREAM_MAX_RECORDS) {
        RtStreamFindOldest(&nOldest, pSlot);
        nOldestSize = pSlot->anSize[nOldest];
        RtStreamRemoveSent(pSlot, nOldest);
        nCount = pSlot->nRecordCount;
        // Yes, the binary assumes the record dropped lay before the new one in the scratch buffer.
        pbData -= nOldestSize;
    }
    pSlot->anSize[nCount] = nSize;
    pSlot->uSequence = uSequence;
    pSlot->apbData[nCount] = pbData;
    pSlot->auSequence[nCount] = uSequence;
    pSlot->auFlags[nCount] = uFlags;
    pSlot->nRecordCount = nCount + 1;
    RtGetMilliseconds(&pSlot->auTime[nCount]);
    return uSequence;
}

// NTSC-U/C: 0x002c78f0, PAL: 0x002d1860
static void
RtStreamFindOldestOfAll(RtStreamSlot **ppSlot, int *pnIndex, int nStream, unsigned int uDirection) {
    RtStreamSlot *pSlot;
    unsigned int uOldest;
    int i;
    int j;

    uOldest = 0;
    if (ppSlot == NULL || pnIndex == NULL || uDirection >= kStreamDirections) {
        return;
    }
    *ppSlot = NULL;
    *pnIndex = 0;
    for (i = 0; i < RT_STREAM_MAX_PEERS; ++i) {
        pSlot = RtStreamGetSlot(nStream, uDirection, i);
        if (pSlot == NULL || pSlot->nRecordCount == 0) {
            continue;
        }
        for (j = 0; j < pSlot->nRecordCount; ++j) {
            if (uOldest == 0 || pSlot->auTime[j] < uOldest) {
                uOldest = pSlot->auTime[j];
                *pnIndex = j;
                *ppSlot = pSlot;
            }
        }
    }
}

// NTSC-U/C: 0x002c79e8, PAL: 0x002d1958
static void RtStreamRemoveReceiveSlot(RtStreamSlot *pSlot) {
    RtStreamSlot *pLast;
    int i;

    if (pSlot == NULL) {
        return;
    }
    for (i = 0; i < RT_STREAM_MAX_RECORDS; ++i) {
        if (pSlot->apbData[i] != NULL) {
            RtStreamRemoveReceived(pSlot, i);
        }
    }
    pLast = &g_apStreamSlots[kStreamReceive][RT_STREAM_MAX_PEERS - 1];
    if (pLast != NULL) {
        // Yes, the binary moves the slots up to the last one, but not the last one.
        memmove(pSlot, &pSlot[1], (pLast - pSlot - 1) * sizeof(RtStreamSlot));
        --g_anStreamSlotCount[kStreamReceive];
    }
}

// NTSC-U/C: 0x002c7a88, PAL: 0x002d19f8
static void RtStreamRemoveSendSlot(RtStreamSlot *pSlot) {
    RtStreamSlot *pLast;

    if (pSlot == NULL) {
        return;
    }
    while (pSlot->nRecordCount != 0) {
        RtStreamRemoveSent(pSlot, 0);
    }
    pLast = &g_apStreamSlots[kStreamSend][RT_STREAM_MAX_PEERS - 1];
    if (pLast != NULL) {
        // Yes, the binary moves the slots up to the last one, but not the last one.
        memmove(pSlot, &pSlot[1], (pLast - pSlot - 1) * sizeof(RtStreamSlot));
        --g_anStreamSlotCount[kStreamSend];
    }
}

// Query each peer whose records wait for acknowledgement, at most once an interval.
// NTSC-U/C: 0x002c7b10, PAL: 0x002d1a80
static void RtStreamQueryUnacknowledged(int nStream) {
    RtStreamSlot *pSlot;
    RtStreamListHeader header;
    char szAddress[RT_STREAM_ADDRESS_SIZE];
    unsigned short uSequence;
    unsigned short uPort;
    unsigned int uNow;
    unsigned int i;
    int j;

    RtGetMilliseconds(&uNow);
    for (i = 0; i < RT_STREAM_MAX_PEERS; ++i) {
        pSlot = RtStreamGetSlot(nStream, kStreamSend, i);
        if (pSlot == NULL || pSlot->nRecordCount == 0) {
            continue;
        }
        szAddress[0] = '\0';
        RtStreamFormatAddress(szAddress, &pSlot->address);
        uPort = pSlot->uPort;
        if (strlen(szAddress) == 0) {
            continue;
        }
        if (uNow - pSlot->uLastQuery <= g_uStreamQueryInterval) {
            continue;
        }
        // Yes, the binary sends the reserved byte uninitialised.
        header.uType = RT_STREAM_RECORD_QUERY;
        header.uValue = pSlot->nRecordCount;
        RtStreamMerge(&header, sizeof(header));
        j = 0;
        if (pSlot->nRecordCount != 0) {
            uSequence = pSlot->auSequence[0];
            while (g_nStreamMergeSize + sizeof(uSequence) <= RT_STREAM_DATAGRAM_SIZE) {
                RtStreamMerge(&uSequence, sizeof(uSequence));
                ++j;
                if (j >= pSlot->nRecordCount) {
                    break;
                }
                uSequence = pSlot->auSequence[j];
            }
        }
        if (g_nStreamMergeSize != 0) {
            RtStreamFlushMerge(g_nStreamSocket, szAddress, uPort, 0);
            pSlot->uLastQuery = uNow;
        }
    }
}

// NTSC-U/C: 0x002c7cb8, PAL: 0x002d1c28
static int RtStreamFindPeer(const char *pszAddress, int nPort) {
    int i;

    if (pszAddress == NULL || strlen(pszAddress) == 0 || nPort == 0) {
        return -1;
    }
    for (i = 0; i < RT_STREAM_MAX_PEERS; ++i) {
        if (g_aStreamPeers[i].szAddress[0] != '\0' && g_aStreamPeers[i].nPort != 0 &&
            strcmp(pszAddress, g_aStreamPeers[i].szAddress) == 0 &&
            nPort == g_aStreamPeers[i].nPort) {
            return i;
        }
    }
    return -1;
}

// NTSC-U/C: 0x002c7d88, PAL: 0x002d1cf8
static void RtStreamRemovePeer(const char *pszAddress, int nPort) {
    unsigned int uIndex;

    if (pszAddress == NULL || strlen(pszAddress) == 0 || nPort == 0) {
        return;
    }
    uIndex = RtStreamFindPeer(pszAddress, nPort);
    if (uIndex < RT_STREAM_MAX_PEERS) {
        memset(&g_aStreamPeers[uIndex], 0, sizeof(RtStreamPeer));
    }
}

// NTSC-U/C: 0x002c7e08, PAL: 0x002d1d78
static void RtStreamAddPeer(int nStream, const char *pszAddress, int nPort) {
    int i;

    (void)nStream;
    if (pszAddress == NULL || strlen(pszAddress) == 0 || nPort == 0) {
        return;
    }
    if (RtStreamFindPeer(pszAddress, nPort) != -1) {
        return;
    }
    for (i = 0; i < RT_STREAM_MAX_PEERS; ++i) {
        if (g_aStreamPeers[i].szAddress[0] == '\0' && g_aStreamPeers[i].nPort == 0) {
            strncpy(g_aStreamPeers[i].szAddress, pszAddress, RT_STREAM_ADDRESS_SIZE);
            g_aStreamPeers[i].nPort = nPort;
            RtReadClock(&g_aStreamPeers[i].lastHeard);
            return;
        }
    }
}

void RtStreamTouchPeer(int nStream, const char *pszAddress, unsigned short uPort) {
    unsigned int uIndex;

    (void)nStream;
    if (pszAddress == NULL || strlen(pszAddress) == 0 || uPort == 0) {
        return;
    }
    uIndex = RtStreamFindPeer(pszAddress, uPort);
    if (uIndex < RT_STREAM_MAX_PEERS) {
        RtReadClock(&g_aStreamPeers[uIndex].lastHeard);
    }
}

// NTSC-U/C: 0x002c7f60, PAL: 0x002d1ed0
static void RtStreamTimeOutPeers(int nStream) {
    unsigned int uElapsed;
    int i;

    (void)nStream;
    for (i = 0; i < RT_STREAM_MAX_PEERS; ++i) {
        if (g_aStreamPeers[i].szAddress[0] == '\0' || g_aStreamPeers[i].nPort == 0) {
            continue;
        }
        // Yes, the binary discards the result and may compare an unset count.
        (void)RtGetMillisecondsSince(&g_aStreamPeers[i].lastHeard, &uElapsed);
        if (g_uStreamPeerTimeout < uElapsed) {
            RtReadClock(&g_aStreamPeers[i].lastHeard);
            if (g_pfnStreamTimeout != NULL) {
                g_pfnStreamTimeout(g_aStreamPeers[i].szAddress, g_aStreamPeers[i].nPort);
            }
        }
    }
}

int RtStreamTick(int nStream) {
    unsigned int uElapsed;
    int nResult;

    if (!g_bStreamOpen) {
        return RT_STREAM_RESULT_NOT_STARTED;
    }
    nResult = RtStatsUpdate();
    if (nResult != 0) {
        return nResult;
    }
    (void)RtUpnpPollRequest(g_pStreamUpnp); // Yes, the binary discards the result.
    (void)RtGetMillisecondsSince(&g_streamLastFlush, &uElapsed);
    if (g_uStreamFlushCount != 0 || g_uStreamFlushInterval < uElapsed) {
        RtStreamFlushTx(nStream);
    }
    RtStreamQueryUnacknowledged(nStream);
    RtStreamTimeOutPeers(nStream);
    RtStreamReadDatagrams();
    return 0;
}

int RtStreamDisconnect(int nStream, const char *pszAddress, int nPort, unsigned char bKeep) {
    RtIpv4Address address;
    RtStreamSlot *pSlot;
    int nResult;

    if (pszAddress == NULL || nPort <= 0) {
        return RT_STREAM_RESULT_INVALID;
    }
    if (!g_bStreamOpen) {
        return RT_STREAM_RESULT_NOT_STARTED;
    }
    if (bKeep) {
        return 0;
    }
    RtStreamParseAddress(&address, pszAddress);
    pSlot = RtStreamFindSlot(nStream, kStreamSend, &address, nPort);
    if (pSlot != NULL) {
        RtStreamRemoveSendSlot(pSlot);
    }
    pSlot = RtStreamFindSlot(nStream, kStreamReceive, &address, nPort);
    if (pSlot != NULL) {
        RtStreamRemoveReceiveSlot(pSlot);
    }
    nResult = RtStreamClearLatest(nStream, &address, nPort);
    if (nResult != 0) {
        return nResult;
    }
    if (g_anStreamSlotCount[kStreamSend] == 0 && g_anStreamSlotCount[kStreamReceive] == 0) {
        RtStreamDropDatagrams(NULL, 0);
    } else {
        RtStreamDropDatagrams(pszAddress, nPort);
    }
    RtStreamRemovePeer(pszAddress, nPort);
    return 0;
}

// NTSC-U/C: 0x002c8208, PAL: 0x002d2178
static int RtStreamSendReliable(int nStream,
                                unsigned char uFlags,
                                const char *pszAddress,
                                int nPort,
                                const void *pData,
                                unsigned short uSize,
                                unsigned short *puSent) {
    RtStreamRecordHeader header;
    RtIpv4Address address;
    RtStreamSlot *pSlot;
    RtStreamSlot *pOldestSlot;
    unsigned char *pbKept;
    int nOldest;

    if (pszAddress == NULL || nPort <= 0 || pData == NULL || uSize == 0 || puSent == NULL) {
        return RT_STREAM_RESULT_INVALID;
    }
    if (uSize + sizeof(header) > RT_STREAM_DATAGRAM_SIZE) {
        return RT_STREAM_RESULT_TOO_LARGE;
    }
    RtStreamParseAddress(&address, pszAddress);
    pSlot = RtStreamGetOrAllocSlot(nStream, kStreamSend, &address, nPort);
    if (pSlot == NULL) {
        return RT_STREAM_RESULT_NO_SLOT;
    }
    header.uFlags = uFlags;
    header.uType = RT_STREAM_RECORD_DATA;
    header.uSequence = RtStreamNextSequence(pSlot->uSequence);
    header.uSize = uSize;
    RtStreamMerge(&header, sizeof(header));
    RtStreamMerge(pData, uSize);
    if (RtStreamFlushMerge(g_nStreamSocket, pszAddress, nPort, uFlags) == 0) {
        *puSent = 0;
        return RT_STREAM_RESULT_NOT_SENT;
    }
    while (RtStreamGetScratchRoom(nStream) < uSize) {
        RtStreamFindOldestOfAll(&pOldestSlot, &nOldest, nStream, kStreamSend);
        if (pOldestSlot == NULL) {
            break;
        }
        RtStreamRemoveSent(pOldestSlot, nOldest);
    }
    pbKept = RtStreamAppendScratch(nStream, pData, uSize);
    if (pbKept == NULL) {
        *puSent = 0;
        return RT_STREAM_RESULT_NOT_SENT;
    }
    RtStreamKeepSent(pSlot, pbKept, uSize, uFlags);
    *puSent = uSize;
    return 0;
}

// NTSC-U/C: 0x002c83b8, PAL: 0x002d2328
static int RtStreamSendSequenced(int nStream,
                                 unsigned char uFlags,
                                 const char *pszAddress,
                                 int nPort,
                                 const void *pData,
                                 unsigned short uSize,
                                 unsigned short *puSent) {
    RtStreamRecordHeader header;
    RtIpv4Address address;
    RtStreamLatest *pLatest;
    unsigned short uSequence;

    if (pszAddress == NULL || nPort <= 0 || pData == NULL || uSize == 0 || puSent == NULL) {
        return RT_STREAM_RESULT_INVALID;
    }
    if (uSize + sizeof(header) > RT_STREAM_DATAGRAM_SIZE) {
        return RT_STREAM_RESULT_TOO_LARGE;
    }
    RtStreamParseAddress(&address, pszAddress);
    pLatest = RtStreamGetOrAddLatest(nStream, kStreamSend, &address, nPort);
    if (pLatest == NULL) {
        return RT_STREAM_RESULT_NO_SLOT;
    }
    header.uFlags = uFlags;
    header.uType = RT_STREAM_RECORD_DATA;
    uSequence = RtStreamNextSequence(pLatest->uSequence);
    header.uSequence = uSequence;
    pLatest->uSequence = uSequence;
    header.uSize = uSize;
    RtStreamMerge(&header, sizeof(header));
    RtStreamMerge(pData, uSize);
    if (RtStreamFlushMerge(g_nStreamSocket, pszAddress, nPort, uFlags) != 0) {
        *puSent = uSize;
        return 0;
    }
    *puSent = 0;
    return RT_STREAM_RESULT_NOT_SENT;
}

// NTSC-U/C: 0x002c84f0, PAL: 0x002d2460
static int RtStreamSendPlain(int nStream,
                             unsigned char uFlags,
                             const char *pszAddress,
                             int nPort,
                             const void *pData,
                             unsigned short uSize,
                             unsigned short *puSent) {
    RtStreamRecordHeader header;

    (void)nStream;
    if (pszAddress == NULL || nPort <= 0 || pData == NULL || uSize == 0 || puSent == NULL) {
        return RT_STREAM_RESULT_INVALID;
    }
    if (uSize + sizeof(header) > RT_STREAM_DATAGRAM_SIZE) {
        return RT_STREAM_RESULT_TOO_LARGE;
    }
    // Yes, the binary sends the flags and the sequence of the header uninitialised.
    header.uType = RT_STREAM_RECORD_PLAIN;
    header.uSize = uSize;
    RtStreamMerge(&header, sizeof(header));
    RtStreamMerge(pData, uSize);
    if (RtStreamFlushMerge(g_nStreamSocket, pszAddress, nPort, uFlags) != 0) {
        *puSent = uSize;
        return 0;
    }
    *puSent = 0;
    return RT_STREAM_RESULT_NOT_SENT;
}

int RtStreamSend(int nStream,
                 int nFlags,
                 const char *pszAddress,
                 int nPort,
                 const void *pData,
                 unsigned short uSize,
                 unsigned short *puSent) {
    int nResult;

    if (pszAddress == NULL || nPort <= 0 || pData == NULL || uSize == 0 || puSent == NULL) {
        return RT_STREAM_RESULT_INVALID;
    }
    if (!g_bStreamOpen) {
        return RT_STREAM_RESULT_NOT_STARTED;
    }
    nResult = RtStatsRecordTraffic(nFlags, uSize, 0);
    if (nResult != 0) {
        return nResult;
    }
    if ((nFlags & RT_STREAM_FLAG_FLUSH) != 0) {
        ++g_uStreamFlushCount;
    }
    if ((nFlags & RT_STREAM_FLAG_GUARANTEED) != 0) {
        RtStreamAddPeer(nStream, pszAddress, nPort);
        nResult = RtStreamSendReliable(nStream, nFlags, pszAddress, nPort, pData, uSize, puSent);
    } else if ((nFlags & RT_STREAM_FLAG_ORDERED) != 0) {
        RtStreamAddPeer(nStream, pszAddress, nPort);
        nResult = RtStreamSendSequenced(nStream, nFlags, pszAddress, nPort, pData, uSize, puSent);
    } else {
        nResult = RtStreamSendPlain(nStream, nFlags, pszAddress, nPort, pData, uSize, puSent);
    }
    if (nResult != 0) {
        return nResult;
    }
    return 0;
}

// Copy the records of a slot that follow its last delivered one, while they fit.
// NTSC-U/C: 0x002c8760, PAL: 0x002d26d0
static void RtStreamDeliverInOrder(RtStreamSlot *pSlot) {
    unsigned short uNext;
    int bDelivered;
    int i;

    if (pSlot == NULL) {
        return;
    }
    do {
        uNext = RtStreamNextSequence(pSlot->uSequence);
        bDelivered = 0;
        for (i = 0; i < RT_STREAM_MAX_RECORDS; ++i) {
            if (pSlot->auSequence[i] != uNext) {
                continue;
            }
            if (pSlot->apbData[i] == NULL) {
                pSlot->uSequence = uNext;
                bDelivered = 1;
            } else if ((unsigned int)pSlot->anSize[i] < g_uStreamOutRoom) {
                memcpy(g_pbStreamOut, pSlot->apbData[i], pSlot->anSize[i]);
                g_pbStreamOut += pSlot->anSize[i];
                g_uStreamOutRoom -= pSlot->anSize[i];
                RtStreamRemoveReceived(pSlot, i);
                pSlot->uSequence = uNext;
                bDelivered = 1;
            }
            break;
        }
    } while (bDelivered);
}

// NTSC-U/C: 0x002c8880, PAL: 0x002d27f0
static int RtStreamDeliverAny(int nStream, char *pszAddress, int *pnPort) {
    RtStreamSlot *pSlot;
    unsigned int uRoom;
    int i;

    (void)nStream;
    uRoom = g_uStreamOutRoom;
    if (pszAddress == NULL || pnPort == NULL) {
        return 0;
    }
    for (i = 0; i < g_anStreamSlotCount[kStreamReceive]; ++i) {
        pSlot = &g_apStreamSlots[kStreamReceive][i];
        if (pSlot == NULL) {
            continue;
        }
        RtStreamDeliverInOrder(pSlot);
        if (uRoom != g_uStreamOutRoom) {
            RtStreamFormatAddress(pszAddress, &pSlot->address);
            *pnPort = pSlot->uPort;
            return 1;
        }
    }
    return 0;
}

// Check that a record header and its data fit in a datagram, and in the room left for the data.
static inline int RtStreamReadHeader(const unsigned char *pbDatagram,
                                     unsigned int *puOffset,
                                     unsigned short uSize,
                                     RtStreamRecordHeader *pHeader) {
    if (*puOffset >= uSize || uSize - *puOffset < sizeof(RtStreamRecordHeader) + 1) {
        *puOffset = uSize;
        return 0;
    }
    memcpy(pHeader, &pbDatagram[*puOffset], sizeof(RtStreamRecordHeader));
    *puOffset += sizeof(RtStreamRecordHeader);
    if (*puOffset >= uSize || uSize - *puOffset < pHeader->uSize ||
        g_uStreamOutRoom < pHeader->uSize || pHeader->uSize > kStreamMaxRecord) {
        *puOffset = uSize;
        return 0;
    }
    return 1;
}

// NTSC-U/C: 0x002c8988, PAL: 0x002d28f8
static int
RtStreamHandlePlain(const unsigned char *pbDatagram, unsigned int *puOffset, unsigned short uSize) {
    RtStreamRecordHeader header;

    if (pbDatagram == NULL || puOffset == NULL || uSize == 0) {
        return RT_STREAM_RESULT_INVALID;
    }
    if (!RtStreamReadHeader(pbDatagram, puOffset, uSize, &header)) {
        return 0;
    }
    memcpy(g_pbStreamOut, &pbDatagram[*puOffset], header.uSize);
    g_pbStreamOut += header.uSize;
    g_uStreamOutRoom -= header.uSize;
    *puOffset += header.uSize;
    return 0;
}

// NTSC-U/C: 0x002c8ee0, PAL: 0x002d2e50
static int RtStreamKeepOutOfOrder(const unsigned char *pbDatagram,
                                  const unsigned int *puOffset,
                                  unsigned short uSize,
                                  int nIndex,
                                  int nStream,
                                  RtStreamSlot *pSlot,
                                  const RtStreamRecordHeader *pHeader) {
    unsigned char *pbKept;
    int nHighest;

    (void)uSize;
    while (g_nStreamWorkUsed + pHeader->uSize > kStreamWorkSize) {
        RtStreamFindHighest(&nHighest, pSlot);
        if (nHighest == -1) {
            break;
        }
        pSlot->uSequence = pSlot->auSequence[nHighest];
        RtStreamRemoveReceived(pSlot, nHighest);
    }
    pbKept = RtStreamAppendWork(nStream, &pbDatagram[*puOffset], pHeader->uSize);
    if (pbKept == NULL) {
        return RT_STREAM_RESULT_NO_SLOT;
    }
    pSlot->anSize[nIndex] = pHeader->uSize;
    pSlot->apbData[nIndex] = pbKept;
    return 0;
}

// NTSC-U/C: 0x002c8c58, PAL: 0x002d2bc8
static int RtStreamHandleReliable(const unsigned char *pbDatagram,
                                  unsigned int *puOffset,
                                  unsigned short uSize,
                                  int nStream,
                                  RtIpv4Address address,
                                  unsigned short uPort,
                                  RtStreamSlot **ppSlot,
                                  const RtStreamRecordHeader *pHeader,
                                  unsigned char *pbDeliver) {
    RtStreamSlot *pSlot;
    unsigned short uSequence;
    int bHeld;
    int bInOrder;
    int nIndex;
    int nHighest;
    int i;

    bHeld = 0;
    bInOrder = 0;
    if (pHeader == NULL) {
        return RT_STREAM_RESULT_INVALID;
    }
    pSlot = RtStreamGetOrAllocSlot(nStream, kStreamReceive, &address, uPort);
    *ppSlot = pSlot;
    if (pSlot == NULL) {
        return RT_STREAM_RESULT_NO_SLOT;
    }
    uSequence = pHeader->uSequence;
    for (i = 0; i < RT_STREAM_MAX_RECORDS; ++i) {
        if (pSlot->auSequence[i] == uSequence) {
            bHeld = 1;
            break;
        }
    }
    if (uSequence == RtStreamNextSequence(pSlot->uSequence)) {
        bInOrder = 1;
        pSlot->uSequence = uSequence;
        *pbDeliver = 1;
    }
    if (bHeld) {
        return 0;
    }
    pSlot = *ppSlot;
    nIndex = pSlot->nNextRecord++;
    if (pSlot->apbData[nIndex] != NULL) {
        RtStreamFindHighest(&nHighest, pSlot);
        while (nHighest != nIndex && nHighest != -1) {
            RtStreamRemoveReceived(*ppSlot, nHighest);
            RtStreamFindHighest(&nHighest, *ppSlot);
        }
        pSlot = *ppSlot;
        pSlot->uSequence = pSlot->auSequence[nIndex];
        RtStreamRemoveReceived(pSlot, nIndex);
        pSlot = *ppSlot;
    }
    if (pSlot->nNextRecord >= RT_STREAM_MAX_RECORDS) {
        pSlot->nNextRecord = 0;
    }
    pSlot->auSequence[nIndex] = pHeader->uSequence;
    if (pSlot->nRecordCount < RT_STREAM_MAX_RECORDS) {
        ++pSlot->nRecordCount;
    }
    RtGetMilliseconds(&pSlot->auTime[nIndex]);
    if ((pHeader->uFlags & RT_STREAM_FLAG_ORDERED) != 0 && !bInOrder) {
        return RtStreamKeepOutOfOrder(
            pbDatagram, puOffset, uSize, nIndex, nStream, *ppSlot, pHeader);
    }
    memcpy(g_pbStreamOut, &pbDatagram[*puOffset], pHeader->uSize);
    g_pbStreamOut += pHeader->uSize;
    g_uStreamOutRoom -= pHeader->uSize;
    return 0;
}

// NTSC-U/C: 0x002c8ff8, PAL: 0x002d2f68
static int RtStreamHandleLatest(const unsigned char *pbDatagram,
                                const unsigned int *puOffset,
                                unsigned short uSize,
                                int nStream,
                                RtIpv4Address address,
                                unsigned short uPort,
                                const RtStreamRecordHeader *pHeader) {
    RtStreamLatest *pLatest;

    (void)uSize;
    if (pHeader == NULL) {
        return RT_STREAM_RESULT_INVALID;
    }
    pLatest = RtStreamGetOrAddLatest(nStream, kStreamReceive, &address, uPort);
    if (pLatest == NULL) {
        return RT_STREAM_RESULT_NO_SLOT;
    }
    if (RtStreamIsNewer(pHeader->uSequence, pLatest->uSequence)) {
        memcpy(g_pbStreamOut, &pbDatagram[*puOffset], pHeader->uSize);
        g_pbStreamOut += pHeader->uSize;
        g_uStreamOutRoom -= pHeader->uSize;
        pLatest->uSequence = pHeader->uSequence;
    }
    return 0;
}

// NTSC-U/C: 0x002c8aa0, PAL: 0x002d2a10
static int RtStreamHandleData(const unsigned char *pbDatagram,
                              unsigned int *puOffset,
                              unsigned short uSize,
                              int nStream,
                              const char *pszAddress,
                              unsigned short uPort,
                              RtStreamSlot **ppSlot,
                              unsigned char *pbDeliver) {
    RtStreamRecordHeader header;
    RtIpv4Address address;
    int nResult;

    memset(&address, 0, sizeof(address));
    if (pbDatagram == NULL || puOffset == NULL || uSize == 0 || pszAddress == NULL || uPort == 0 ||
        ppSlot == NULL || pbDeliver == NULL) {
        return RT_STREAM_RESULT_INVALID;
    }
    if (*puOffset >= uSize || uSize - *puOffset < sizeof(header) + 1) {
        *puOffset = uSize;
        return 0;
    }
    RtStreamParseAddress(&address, pszAddress);
    if (!RtStreamReadHeader(pbDatagram, puOffset, uSize, &header)) {
        return 0;
    }
    if ((header.uFlags & RT_STREAM_FLAG_GUARANTEED) != 0) {
        nResult = RtStreamHandleReliable(
            pbDatagram, puOffset, uSize, nStream, address, uPort, ppSlot, &header, pbDeliver);
    } else {
        nResult =
            RtStreamHandleLatest(pbDatagram, puOffset, uSize, nStream, address, uPort, &header);
    }
    if (nResult != 0) {
        return nResult;
    }
    *puOffset += header.uSize;
    return 0;
}

// Drop the records a peer acknowledged.
// NTSC-U/C: 0x002c90c8, PAL: 0x002d3038
static int RtStreamHandleAck(const unsigned char *pbDatagram,
                             unsigned int *puOffset,
                             unsigned short uSize,
                             int nStream,
                             const char *pszAddress,
                             unsigned short uPort) {
    RtStreamListHeader header;
    RtIpv4Address address;
    RtStreamSlot *pSlot;
    int nCount;
    int i;
    int j;

    if (pbDatagram == NULL || puOffset == NULL || uSize == 0 || pszAddress == NULL || uPort == 0) {
        return RT_STREAM_RESULT_INVALID;
    }
    if (*puOffset >= uSize || uSize - *puOffset < sizeof(header) + 1) {
        *puOffset = uSize;
        return 0;
    }
    RtStreamParseAddress(&address, pszAddress);
    memcpy(&header, &pbDatagram[*puOffset], sizeof(header));
    *puOffset += sizeof(header);
    if (*puOffset >= uSize || uSize - *puOffset < header.uValue * sizeof(g_auStreamAcked[0])) {
        *puOffset = uSize;
        return 0;
    }
    // Yes, the binary does not check that the list fits the buffer.
    memcpy(g_auStreamAcked, &pbDatagram[*puOffset], header.uValue * sizeof(g_auStreamAcked[0]));
    *puOffset += header.uValue * sizeof(g_auStreamAcked[0]);
    pSlot = RtStreamFindSlot(nStream, kStreamSend, &address, uPort);
    if (pSlot == NULL) {
        return 0;
    }
    RtGetMilliseconds(&pSlot->uLastAck);
    for (i = 0; i < header.uValue; ++i) {
        nCount = pSlot->nRecordCount;
        for (j = 0; j < nCount; ++j) {
            if (pSlot->auSequence[j] == g_auStreamAcked[i]) {
                RtStreamRemoveSent(pSlot, j);
                break;
            }
        }
    }
    return 0;
}

// Acknowledge the queried records held, and ask for each one missing.
// NTSC-U/C: 0x002c9270, PAL: 0x002d31e0
static int RtStreamHandleQuery(const unsigned char *pbDatagram,
                               unsigned int *puOffset,
                               unsigned short uSize,
                               int nStream,
                               const char *pszAddress,
                               unsigned short uPort) {
    RtStreamListHeader header;
    RtStreamListHeader reply;
    RtStreamListHeader nak;
    RtIpv4Address address;
    RtStreamSlot *pSlot;
    int nHeld;
    int nCount;
    int bHeld;
    int i;
    int j;

    if (pbDatagram == NULL || puOffset == NULL || uSize == 0 || pszAddress == NULL || uPort == 0) {
        return RT_STREAM_RESULT_INVALID;
    }
    if (*puOffset >= uSize || uSize - *puOffset < sizeof(header) + 1) {
        *puOffset = uSize;
        return 0;
    }
    RtStreamParseAddress(&address, pszAddress);
    memcpy(&header, &pbDatagram[*puOffset], sizeof(header));
    *puOffset += sizeof(header);
    if (*puOffset >= uSize || uSize - *puOffset < header.uValue * sizeof(g_auStreamQueried[0])) {
        *puOffset = uSize;
        return 0;
    }
    pSlot = RtStreamGetOrAllocSlot(nStream, kStreamReceive, &address, uPort);
    if (pSlot == NULL) {
        return 0;
    }
    // Yes, the binary does not check that the list fits the buffer.
    memcpy(g_auStreamQueried, &pbDatagram[*puOffset], header.uValue * sizeof(g_auStreamQueried[0]));
    *puOffset += header.uValue * sizeof(g_auStreamQueried[0]);
    // Yes, the binary sends the reserved byte uninitialised.
    reply.uType = RT_STREAM_RECORD_ACK;
    nHeld = 0;
    nCount = pSlot->nRecordCount;
    for (i = 0; i < header.uValue; ++i) {
        for (j = 0; j < nCount; ++j) {
            if (pSlot->auSequence[j] == g_auStreamQueried[i]) {
                g_auStreamHeld[nHeld++] = g_auStreamQueried[i];
                break;
            }
        }
    }
    if (nHeld != 0) {
        reply.uValue = nHeld;
        for (i = nHeld; i > 0; --i) {
            // Yes, the binary runs an empty loop over the sequences held.
        }
        if (sizeof(reply) + nHeld * sizeof(g_auStreamHeld[0]) > RT_STREAM_DATAGRAM_SIZE) {
            return RT_STREAM_RESULT_TOO_LARGE;
        }
        RtStreamMerge(&reply, sizeof(reply));
        RtStreamMerge(g_auStreamHeld, nHeld * sizeof(g_auStreamHeld[0]));
        RtStreamFlushMerge(nStream, pszAddress, uPort, 0);
    }
    for (i = 0; i < header.uValue; ++i) {
        bHeld = 0;
        for (j = 0; j < pSlot->nRecordCount; ++j) {
            if (pSlot->auSequence[j] == g_auStreamQueried[i]) {
                bHeld = 1;
                break;
            }
        }
        if (bHeld) {
            continue;
        }
        // Yes, the binary sends the reserved byte uninitialised.
        nak.uType = RT_STREAM_RECORD_NAK;
        nak.uValue = g_auStreamQueried[i];
        RtStreamMerge(&nak, sizeof(nak));
        RtStreamFlushMerge(nStream, pszAddress, uPort, 0);
    }
    return 0;
}

// Resend the record a peer is missing.
// NTSC-U/C: 0x002c9580, PAL: 0x002d34f0
static int RtStreamHandleNak(const unsigned char *pbDatagram,
                             unsigned int *puOffset,
                             unsigned short uSize,
                             int nStream,
                             const char *pszAddress,
                             unsigned short uPort) {
    RtStreamListHeader nak;
    RtStreamRecordHeader header;
    RtIpv4Address address;
    RtStreamSlot *pSlot;
    unsigned char *pbData;
    int i;

    if (pbDatagram == NULL || puOffset == NULL || uSize == 0 || pszAddress == NULL || uPort == 0) {
        return RT_STREAM_RESULT_INVALID;
    }
    if (*puOffset >= uSize || uSize - *puOffset < sizeof(nak)) {
        *puOffset = uSize;
        return 0;
    }
    RtStreamParseAddress(&address, pszAddress);
    memcpy(&nak, &pbDatagram[*puOffset], sizeof(nak));
    *puOffset += sizeof(nak);
    if (uSize < *puOffset) {
        *puOffset = uSize;
        return 0;
    }
    pSlot = RtStreamFindSlot(nStream, kStreamSend, &address, uPort);
    if (pSlot == NULL) {
        return 0;
    }
    for (i = 0; i < pSlot->nRecordCount; ++i) {
        if (pSlot->auSequence[i] != nak.uValue) {
            continue;
        }
        pbData = pSlot->apbData[i];
        header.uType = RT_STREAM_RECORD_DATA;
        header.uFlags = pSlot->auFlags[i];
        header.uSequence = nak.uValue;
        header.uSize = pSlot->anSize[i];
        if (header.uSize - 1 >= RT_STREAM_DATAGRAM_SIZE) {
            return 0;
        }
        if (header.uSize >= kStreamMaxRecord) {
            *puOffset = uSize;
            return 0;
        }
        RtStreamMerge(&header, sizeof(header));
        RtStreamMerge(pbData, pSlot->anSize[i]);
        RtStreamFlushMerge(nStream, pszAddress, uPort, 0);
        return 0;
    }
    return 0;
}

int RtStreamReceive(int nStream,
                    char *pszAddress,
                    int *pnPort,
                    void *pBuffer,
                    unsigned short uSize,
                    unsigned short *puReceived) {
    // Yes, the binary writes the port of a delivered stream over its copy of pnPort, so the caller
    // then receives the port of the socket.
    union {
        int *pnPort;
        int nPort;
    } port;
    RtStreamSlot *pSlot;
    unsigned int uOffset;
    unsigned short uSender;
    unsigned short uDatagramSize;
    unsigned char bDeliver;
    int nResult;

    port.pnPort = pnPort;
    uOffset = 0;
    uDatagramSize = 0;
    bDeliver = 0;
    pSlot = NULL;
    if (pszAddress == NULL || pnPort == NULL || pBuffer == NULL || uSize == 0 ||
        puReceived == NULL) {
        return RT_STREAM_RESULT_INVALID;
    }
    if (!g_bStreamOpen) {
        return RT_STREAM_RESULT_NOT_STARTED;
    }
    g_pbStreamOut = pBuffer;
    g_uStreamOutRoom = uSize;
    *puReceived = 0;
    *pnPort = g_uStreamPort;
    if (RtStreamDeliverAny(nStream, pszAddress, &port.nPort)) {
        *puReceived = uSize - g_uStreamOutRoom;
        return 0;
    }
    nResult = RtStreamTakeDatagram(pszAddress, &uSender, &uDatagramSize, g_abStreamDatagram);
    if (nResult != 0) {
        return nResult;
    }
    if (uDatagramSize == 0) {
        return 0;
    }
    while (uOffset < uDatagramSize) {
        switch (g_abStreamDatagram[uOffset]) {
        case RT_STREAM_RECORD_PLAIN:
            nResult = RtStreamHandlePlain(g_abStreamDatagram, &uOffset, uDatagramSize);
            break;
        case RT_STREAM_RECORD_DATA:
            RtStreamAddPeer(nStream, pszAddress, uSender);
            nResult = RtStreamHandleData(g_abStreamDatagram,
                                         &uOffset,
                                         uDatagramSize,
                                         nStream,
                                         pszAddress,
                                         uSender,
                                         &pSlot,
                                         &bDeliver);
            break;
        case RT_STREAM_RECORD_QUERY:
            nResult = RtStreamHandleQuery(
                g_abStreamDatagram, &uOffset, uDatagramSize, nStream, pszAddress, uSender);
            break;
        case RT_STREAM_RECORD_ACK:
            nResult = RtStreamHandleAck(
                g_abStreamDatagram, &uOffset, uDatagramSize, nStream, pszAddress, uSender);
            break;
        case RT_STREAM_RECORD_NAK:
            nResult = RtStreamHandleNak(
                g_abStreamDatagram, &uOffset, uDatagramSize, nStream, pszAddress, uSender);
            break;
        default:
            nResult = 0;
            uOffset = uDatagramSize;
            break;
        }
        if (nResult != 0) {
            return nResult;
        }
    }
    if (bDeliver && pSlot != NULL) {
        RtStreamDeliverInOrder(pSlot);
    }
    *puReceived = uSize - g_uStreamOutRoom;
    *port.pnPort = uSender;
    return 0;
}

int RtStreamMapPort(int nStream, const char *pszGateway, int nPort, RtStreamMappingFunc pfnDone) {
    (void)nStream;
    if (pszGateway == NULL || nPort <= 0 || pfnDone == NULL) {
        return RT_STREAM_RESULT_INVALID;
    }
    if (!g_bStreamOpen) {
        return RT_STREAM_RESULT_NOT_STARTED;
    }
    // Yes, the binary may leave the address unterminated.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstringop-truncation"
    strncpy(g_szStreamGateway, pszGateway, RT_STREAM_ADDRESS_SIZE);
#pragma GCC diagnostic pop
    g_uStreamGatewayPort = nPort;
    if (RtUpnpBeginMapping(g_pStreamUpnp, pszGateway, nPort, RtStreamOnMapping) != 0) {
        return RT_STREAM_RESULT_FAILED;
    }
    g_pfnStreamMapping = pfnDone;
    return 0;
}

int RtStreamOpen(int *pnStream,
                 int nPort,
                 int nPeerTimeout,
                 int nTxQueueSize,
                 int nRxQueueSize,
                 unsigned int uFlushInterval,
                 RtStreamTimeoutFunc pfnTimeout) {
    RtSocketOptions options;
    int nResult;

    if (pnStream == NULL || nPort <= 0) {
        return RT_STREAM_RESULT_INVALID;
    }
    if (g_bStreamOpen == 1) {
        return RT_STREAM_RESULT_ALREADY_STARTED;
    }
    g_uStreamFlushInterval = uFlushInterval;
    g_uStreamFlushCount = 0;
    (void)RtReadClock(&g_streamLastFlush); // Yes, the binary discards the result.
    g_uStreamPeerTimeout = nPeerTimeout != 0 ? nPeerTimeout : kStreamDefaultPeerTimeout;
    g_pfnStreamTimeout = pfnTimeout;
    memset(g_aStreamPeers, 0, sizeof(g_aStreamPeers));
    memset(&options, 0, sizeof(options));
    options.nType = RT_SOCKET_DATAGRAM;
    options.nBufferSize = RT_STREAM_DATAGRAM_SIZE;
    if (RtSocketCreate(&g_nStreamSocket, NULL, nPort, &options) != 0) {
        return RT_STREAM_RESULT_FAILED;
    }
    g_uStreamPort = nPort;
    if (RtUpnpCreateContext(&g_pStreamUpnp, g_nStreamSocket, nPort, NULL) != 0) {
        return RT_STREAM_RESULT_FAILED;
    }
    if (RtAllocZeroedBlock((void **)&g_apStreamLatest[kStreamSend],
                           RT_STREAM_MAX_PEERS * sizeof(RtStreamLatest)) != 0) {
        return RT_STREAM_RESULT_FAILED;
    }
    if (RtAllocZeroedBlock((void **)&g_apStreamLatest[kStreamReceive],
                           RT_STREAM_MAX_PEERS * sizeof(RtStreamLatest)) != 0) {
        return RT_STREAM_RESULT_FAILED;
    }
    nResult = RtStreamResetLatest();
    if (nResult != 0) {
        return nResult;
    }
    if (RtAllocZeroedBlock((void **)&g_apStreamSlots[kStreamSend],
                           RT_STREAM_MAX_PEERS * sizeof(RtStreamSlot)) != 0) {
        return RT_STREAM_RESULT_FAILED;
    }
    g_anStreamSlotCount[kStreamSend] = 0;
    if (RtAllocZeroedBlock((void **)&g_apStreamSlots[kStreamReceive],
                           RT_STREAM_MAX_PEERS * sizeof(RtStreamSlot)) != 0) {
        return RT_STREAM_RESULT_FAILED;
    }
    g_anStreamSlotCount[kStreamReceive] = 0;
    g_nStreamScratchUsed = 0;
    if (RtAllocZeroedBlock((void **)&g_pbStreamScratch, kStreamScratchSize) != 0) {
        return RT_STREAM_RESULT_FAILED;
    }
    g_nStreamWorkUsed = 0;
    if (RtAllocZeroedBlock((void **)&g_pbStreamWork, kStreamWorkSize) != 0) {
        return RT_STREAM_RESULT_FAILED;
    }
    g_nStreamRxQueueSize =
        (unsigned int)nRxQueueSize >= kStreamMinRxQueueSize ? nRxQueueSize : kStreamMinRxQueueSize;
    if (RtAllocZeroedBlock((void **)&g_pbStreamRxQueue, g_nStreamRxQueueSize) != 0) {
        return RT_STREAM_RESULT_FAILED;
    }
    g_nStreamTxQueueSize =
        (unsigned int)nTxQueueSize >= kStreamMinTxQueueSize ? nTxQueueSize : kStreamMinTxQueueSize;
    g_pbStreamRxQueueEnd = g_pbStreamRxQueue;
    if (RtAllocZeroedBlock((void **)&g_pbStreamTxQueue, g_nStreamTxQueueSize) != 0) {
        return RT_STREAM_RESULT_FAILED;
    }
    g_pbStreamTxQueueEnd = g_pbStreamTxQueue;
    *pnStream = 0;
    g_bStreamOpen = 1;
    return 0;
}

// Release a block that is set, reporting whether the release failed.
static inline int RtStreamFree(void **ppBlock) {
    if (*ppBlock != NULL && RtFreeBlock(ppBlock) != 0) {
        return 1;
    }
    return 0;
}

int RtStreamClose(int nStream) {
    RtTime start;
    unsigned int uElapsed;
    unsigned int uDirection;

    if (!g_bStreamOpen) {
        return RT_STREAM_RESULT_NOT_STARTED;
    }
    RtReadClock(&start);
    do {
        RtStreamTick(nStream);
        RtGetMillisecondsSince(&start, &uElapsed);
    } while (uElapsed < kStreamCloseMilliseconds);
    if (RtStreamFree((void **)&g_pbStreamScratch) || RtStreamFree((void **)&g_pbStreamWork) ||
        RtStreamFree((void **)&g_pbStreamTxQueue) || RtStreamFree((void **)&g_pbStreamRxQueue)) {
        return RT_STREAM_RESULT_FAILED;
    }
    for (uDirection = 0; uDirection < kStreamDirections; ++uDirection) {
        if (g_apStreamSlots[uDirection] != NULL) {
            if (RtFreeBlock((void **)&g_apStreamSlots[uDirection]) != 0) {
                return RT_STREAM_RESULT_FAILED;
            }
            g_apStreamSlots[uDirection] = NULL;
        }
    }
    for (uDirection = 0; uDirection < kStreamDirections; ++uDirection) {
        if (g_apStreamLatest[uDirection] != NULL) {
            if (RtFreeBlock((void **)&g_apStreamLatest[uDirection]) != 0) {
                return RT_STREAM_RESULT_FAILED;
            }
            g_apStreamLatest[uDirection] = NULL;
        }
    }
    RtUpnpDestroyContext(g_pStreamUpnp);
    RtSocketClose(g_nStreamSocket);
    g_bStreamOpen = 0;
    return 0;
}

// The header of a datagram in the queue of datagrams received.
typedef struct {
    char szAddress[RT_STREAM_ADDRESS_SIZE];
    unsigned short uPort;
    short nSize; // Read unsigned.
} RtStreamRxEntry;

// The header of a datagram in the queue of datagrams to send.
typedef struct {
    char szAddress[RT_STREAM_ADDRESS_SIZE];
    unsigned char uFlags;
    unsigned short uPort;
    int nSize;
} RtStreamTxEntry;

enum {
    // The room the queue of datagrams received must have to read one more.
    kStreamRxRoom = 541,
    // The largest records a datagram to send is merged up to.
    kStreamMergeLimit = 512,
    kStreamLastSequence = 0xfffd,
    kStreamSequenceWindow = 32767,
    kStreamAddressOctets = 4,
    // The results of RtStreamTakeDatagram() for a damaged queue.
    kStreamResultTruncated = 5,
    kStreamResultUnderflow = 7,
};

void RtStreamOnMapping(struct RtUpnpContext *pContext,
                       int nResult,
                       int nService,
                       const char *pszAddress,
                       unsigned short uPort,
                       void *pUser) {
    (void)pUser;
    if (pContext == g_pStreamUpnp && nResult == 0) {
        g_pfnStreamMapping(0, pszAddress, uPort, 0);
        g_nStreamMappedService = nService;
        return;
    }
    g_pfnStreamMapping(0, "", 0, 1);
}

void RtStreamReadDatagrams(void) {
    RtStreamRxEntry entry;
    unsigned char *pbEnd;

    for (;;) {
        pbEnd = g_pbStreamRxQueueEnd;
        memset(&entry, 0, sizeof(entry));
        if (&g_pbStreamRxQueue[g_nStreamRxQueueSize] < g_pbStreamRxQueueEnd) {
            return;
        }
        if ((unsigned int)(g_nStreamRxQueueSize - (g_pbStreamRxQueueEnd - g_pbStreamRxQueue)) <
            kStreamRxRoom) {
            return;
        }
        if (RtSocketReceiveFrom(g_nStreamSocket,
                                entry.szAddress,
                                &entry.uPort,
                                &pbEnd[sizeof(entry)],
                                RT_STREAM_DATAGRAM_SIZE,
                                &entry.nSize) != 0) {
            return;
        }
        if ((unsigned short)entry.nSize == 0 ||
            (unsigned short)entry.nSize > RT_STREAM_DATAGRAM_SIZE) {
            return;
        }
        if (strlen(entry.szAddress) == 0 || entry.uPort == 0) {
            return;
        }
        if (g_uStreamGatewayPort == entry.uPort &&
            strcmp(entry.szAddress, g_szStreamGateway) == 0) {
            // Yes, the binary drops a datagram of the gateway and stops reading until the next
            // tick.
            return;
        }
        memcpy(g_pbStreamRxQueueEnd, &entry, sizeof(entry));
        g_pbStreamRxQueueEnd = &g_pbStreamRxQueueEnd[(unsigned short)entry.nSize + sizeof(entry)];
        RtStreamTouchPeer(0, entry.szAddress, entry.uPort);
    }
}

void RtStreamDropDatagrams(const char *pszAddress, unsigned short uPort) {
    RtStreamRxEntry entry;
    unsigned char *pbEntry;
    unsigned int uEntrySize;

    if (pszAddress == NULL && uPort == 0) {
        g_pbStreamRxQueueEnd = g_pbStreamRxQueue;
        return;
    }
    pbEntry = g_pbStreamRxQueue;
    while (pbEntry < g_pbStreamRxQueueEnd) {
        memcpy(&entry, pbEntry, sizeof(entry));
        uEntrySize = (unsigned short)entry.nSize + sizeof(entry);
        if (strcmp(entry.szAddress, pszAddress) == 0 && entry.uPort == uPort) {
            memmove(
                pbEntry, &pbEntry[uEntrySize], (g_pbStreamRxQueueEnd - pbEntry) - (int)uEntrySize);
            g_pbStreamRxQueueEnd = &g_pbStreamRxQueueEnd[-(int)uEntrySize];
            if (g_pbStreamRxQueueEnd < g_pbStreamRxQueue) {
                return;
            }
        } else {
            pbEntry = &pbEntry[uEntrySize];
        }
    }
}

int RtStreamTakeDatagram(char *pszAddress,
                         unsigned short *puPort,
                         unsigned short *puSize,
                         unsigned char *pbData) {
    RtStreamRxEntry entry;
    unsigned char *pbEntry;

    pbEntry = g_pbStreamRxQueue;
    if (pszAddress == NULL || puPort == NULL || puSize == NULL || pbData == NULL) {
        return RT_STREAM_RESULT_INVALID;
    }
    *puSize = 0;
    *puPort = 0;
    pszAddress[0] = '\0';
    pbData[0] = 0;
    if (g_pbStreamRxQueueEnd == pbEntry) {
        return 0;
    }
    if ((unsigned int)(g_pbStreamRxQueueEnd - pbEntry) < sizeof(entry)) {
        g_pbStreamRxQueueEnd = g_pbStreamRxQueue;
        return kStreamResultTruncated;
    }
    memcpy(&entry, pbEntry, sizeof(entry));
    pbEntry = &pbEntry[sizeof(entry)];
    if (g_pbStreamRxQueueEnd - pbEntry < (unsigned short)entry.nSize) {
        g_pbStreamRxQueueEnd = g_pbStreamRxQueue;
        return kStreamResultTruncated;
    }
    memcpy(pbData, pbEntry, (unsigned short)entry.nSize);
    memcpy(pszAddress, entry.szAddress, RT_STREAM_ADDRESS_SIZE);
    *puPort = entry.uPort;
    *puSize = entry.nSize;
    pbEntry = &pbEntry[(unsigned short)entry.nSize];
    memmove(g_pbStreamRxQueue, pbEntry, g_pbStreamRxQueueEnd - pbEntry);
    g_pbStreamRxQueueEnd =
        &g_pbStreamRxQueueEnd[-(int)((unsigned short)entry.nSize + sizeof(entry))];
    return g_pbStreamRxQueueEnd < g_pbStreamRxQueue ? kStreamResultUnderflow : 0;
}

int RtStreamGetTxRoom(int nStream) {
    (void)nStream;
    return &g_pbStreamTxQueue[g_nStreamTxQueueSize] - g_pbStreamTxQueueEnd;
}

int RtStreamQueueTx(int nStream,
                    const char *pszAddress,
                    unsigned short uPort,
                    unsigned char uFlags,
                    int nSize,
                    const void *pData) {
    RtStreamTxEntry entry;
    RtStreamTxEntry queued;
    unsigned char *pbEntry;
    unsigned char *pbNext;
    int nAfter;

    pbEntry = g_pbStreamTxQueue;
    if (pszAddress == NULL || uPort == 0 || nSize <= 0 || pData == NULL || pbEntry == NULL) {
        return 0;
    }
    if (g_pbStreamTxQueueEnd < pbEntry) {
        return 0;
    }
    while (pbEntry < g_pbStreamTxQueueEnd && RtStreamGetTxRoom(nStream) >= nSize) {
        memcpy(&queued, pbEntry, sizeof(queued));
        pbNext = &pbEntry[queued.nSize + sizeof(queued)];
        if (queued.nSize + nSize <= kStreamMergeLimit &&
            strcmp(pszAddress, queued.szAddress) == 0 && queued.uPort == uPort &&
            ((signed char)queued.uFlags & RT_STREAM_FLAG_FLUSH) == 0) {
            nAfter = g_pbStreamTxQueueEnd - pbNext;
            if (nAfter != 0) {
                memmove(&pbNext[nSize], pbNext, nAfter);
            }
            memcpy(pbNext, pData, nSize);
            queued.nSize += nSize;
            queued.uFlags |= uFlags;
            memcpy(pbEntry, &queued, sizeof(queued));
            g_pbStreamTxQueueEnd = &g_pbStreamTxQueueEnd[nSize];
            return 1;
        }
        pbEntry = pbNext;
    }
    if (RtStreamGetTxRoom(nStream) < nSize + (int)sizeof(entry)) {
        RtStreamFlushTx(nStream);
        if (RtStreamGetTxRoom(nStream) < nSize + (int)sizeof(entry)) {
            return 0;
        }
    }
    // Yes, the binary copies 16 bytes of the address past its terminator, and sends the byte after
    // the flags uninitialised.
    memcpy(entry.szAddress, pszAddress, RT_STREAM_ADDRESS_SIZE);
    entry.uPort = uPort;
    entry.nSize = nSize;
    entry.uFlags = uFlags;
    memcpy(g_pbStreamTxQueueEnd, &entry, sizeof(entry));
    g_pbStreamTxQueueEnd = &g_pbStreamTxQueueEnd[sizeof(entry)];
    memcpy(g_pbStreamTxQueueEnd, pData, nSize);
    g_pbStreamTxQueueEnd = &g_pbStreamTxQueueEnd[nSize];
    return 1;
}

void RtStreamRemoveTx(unsigned char *pbSpan, int nSize) {
    int nAfter;

    nAfter = g_pbStreamTxQueueEnd - &pbSpan[nSize];
    if (nAfter < 0 || pbSpan < g_pbStreamTxQueue || g_pbStreamTxQueueEnd < pbSpan ||
        g_pbStreamTxQueueEnd < &pbSpan[nSize]) {
        return;
    }
    if (nAfter > 0) {
        memmove(pbSpan, &pbSpan[nSize], nAfter);
    }
    g_pbStreamTxQueueEnd = &g_pbStreamTxQueueEnd[-nSize];
}

void RtStreamFlushTx(int nStream) {
    RtStreamTxEntry entry;
    short nSent;
    int bStopped;

    (void)nStream;
    bStopped = 0;
    if (g_pbStreamTxQueue >= g_pbStreamTxQueueEnd) {
        return;
    }
    do {
        if (g_pbStreamTxQueue == NULL) {
            return;
        }
        memcpy(&entry, g_pbStreamTxQueue, sizeof(entry));
        // Yes, the binary discards the result and looks only at the bytes sent.
        (void)RtSocketSendTo(g_nStreamSocket,
                             entry.szAddress,
                             entry.uPort,
                             &g_pbStreamTxQueue[sizeof(entry)],
                             entry.nSize,
                             &nSent);
        if ((unsigned short)nSent == entry.nSize) {
            if (((signed char)entry.uFlags & RT_STREAM_FLAG_FLUSH) != 0) {
                --g_uStreamFlushCount;
            }
            RtStreamRemoveTx(g_pbStreamTxQueue, (unsigned short)nSent + sizeof(entry));
            RtReadClock(&g_streamLastFlush);
        } else {
            bStopped = 1;
        }
    } while (g_pbStreamTxQueue < g_pbStreamTxQueueEnd && !bStopped);
}

void RtStreamSendTx(int nStream,
                    const char *pszAddress,
                    unsigned short uPort,
                    unsigned char uFlags,
                    const void *pData,
                    int nSize,
                    unsigned short *puQueued) {
    unsigned int uElapsed;

    if (RtStreamQueueTx(nStream, pszAddress, uPort, uFlags, nSize, pData) != 0) {
        *puQueued = nSize;
    } else {
        *puQueued = 0;
    }
    (void)RtGetMillisecondsSince(&g_streamLastFlush, &uElapsed); // Yes, the binary discards it.
    if (g_uStreamFlushCount != 0 || g_uStreamFlushInterval < uElapsed) {
        RtStreamFlushTx(nStream);
    }
}

void RtStreamMerge(const void *pData, int nSize) {
    if (pData == NULL || nSize <= 0) {
        return;
    }
    if ((unsigned int)(g_nStreamMergeSize + nSize) > RT_STREAM_DATAGRAM_SIZE) {
        return;
    }
    memcpy(&g_abStreamMerge[g_nStreamMergeSize], pData, nSize);
    g_nStreamMergeSize += nSize;
}

unsigned short RtStreamFlushMerge(int nStream,
                                  const char *pszAddress,
                                  unsigned short uPort,
                                  unsigned char uFlags) {
    unsigned short uQueued;

    uQueued = 0;
    if (g_nStreamMergeSize != 0) {
        RtStreamSendTx(
            nStream, pszAddress, uPort, uFlags, g_abStreamMerge, g_nStreamMergeSize, &uQueued);
        g_nStreamMergeSize = 0;
    }
    return uQueued;
}

unsigned short RtStreamNextSequence(unsigned short uSequence) {
    if (uSequence > kStreamLastSequence) {
        return 0;
    }
    return uSequence + 1;
}

int RtStreamIsNewer(int nSequence, int nLast) {
    int nDifference;

    nDifference = nSequence - nLast;
    if (nDifference > kStreamSequenceWindow) {
        return 0;
    }
    if (nDifference < -kStreamSequenceWindow) {
        return 1;
    }
    return nDifference > 0;
}

void RtStreamParseAddress(RtIpv4Address *pAddress, const char *pszAddress) {
    char szText[RT_STREAM_ADDRESS_SIZE];
    char *pszPart;
    char *pszDot;
    int i;

    pszPart = szText;
    if (pAddress == NULL || pszAddress == NULL || strlen(pszAddress) == 0) {
        return;
    }
    memset(pAddress, 0, sizeof(RtIpv4Address));
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstringop-truncation"
    strncpy(szText, pszAddress, sizeof(szText));
#pragma GCC diagnostic pop
    for (i = 0; i < kStreamAddressOctets - 1; ++i) {
        pszDot = strstr(pszPart, ".");
        if (pszDot != NULL) {
            *pszDot = '\0';
            pAddress->abOctets[i] = atoi(pszPart);
            pszPart = &pszDot[1];
        }
    }
    pAddress->abOctets[kStreamAddressOctets - 1] = atoi(pszPart);
}

int RtStreamFormatAddress(char *pszAddress, const RtIpv4Address *pAddress) {
    if (pszAddress == NULL || pAddress == NULL) {
        // Yes, the binary returns the address argument here.
        return (int)(uintptr_t)pAddress;
    }
    return sprintf(pszAddress,
                   "%u.%u.%u.%u",
                   pAddress->abOctets[0],
                   pAddress->abOctets[1],
                   pAddress->abOctets[2],
                   pAddress->abOctets[3]);
}

int RtStreamStartup(void) {
    int nResult;

    (void)RtSocketStartup(); // Yes, the binary discards the result.
    nResult = RtStatsInit();
    if (nResult != 0) {
        return nResult;
    }
    g_uStreamGatewayPort = 0;
    g_szStreamGateway[0] = '\0';
    return nResult;
}

int RtStreamCleanup(void) {
    RtSocketShutdown();
    return 0;
}
