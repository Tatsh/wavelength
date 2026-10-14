#include "rt/rtchannel.h"

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "rt/rtchannelapi.h"
#include "rt/rtmemory.h"
#include "rt/rtrecord.h"
#include "rt/rtsecurity.h"
#include "rt/rtsocket.h"
#include "rt/rttitle.h"

enum {
    // The record types.
    kRecordRequest = 0,
    kRecordDisconnect = 1,
    kRecordPeerData = 2,
    kRecordPeerDataGuaranteed = 3,
    kRecordAcknowledge = 5,
    kRecordConnectRejected = 6,
    kRecordConnectAccepted = 7,
    kRecordPeerWithKey = 8,
    kRecordPeer = 9,
    kRecordBroadcast = 10,
    kRecordControlNoValue = 15,
    kRecordControl = 16,
    kRecordStatus = 17,
    kRecordPublicKey = 18,
    kRecordSessionKey = 19,
    kRecordConnectionKey = 20,
    kRecordHostRequest = 21,
    kRecordConnect = 22,
    kRecordConfigured = 23,
    kRecordRedirect = 24,
    kRecordConfig = 25,
    kRecordStateUpdate = 26,
    kRecordPeerSessionKey = 27,
    kRecordTypeMask = 0x7f,
    kRecordEncryptedShift = 7,
    // The first type the dispatch table has, and the number of entries.
    kRecordFirstDispatched = 2,
    kRecordDispatchCount = 26,
    // The states of a channel.
    kStateCreated = 0,
    kStateReady = 1,
    kStateKeyWait = 3,
    kStateRequested = 4,
    kStateHostAccepted = 5,
    kStateRedirected = 6,
    kStateConfigWait = 7,
    kStateConnected = 8,
    kStateClosed = 11,
    kStateCount = 12,
    // The reason RtChannelConnectFunc gets for an incompatible peer.
    kReasonBadVersion = 3,
    // The growth of the channel table.
    kChannelTableGrowth = 10,
    kDisconnectTimeout = 10000,
    // The largest read from a stream.
    kStreamReadSize = 4096,
    // The bytes of a request before its data.
    kRequestHeaderSize = 73,
    // The size of a connection request.
    kConnectSize = 29,
    kEncryptionHeaderSize = 4,
    kPeerFieldSize = 2,
    kRecordReserve = 72,
    kRsaBlockMask = 63,
    kVersionMajor = 1,
    kVersionMinorClient = 6,
    kVersionBuildClient = 66,
    kVersionMinorHost = 7,
    kVersionBuildHost = 12,
    // The minor version of a client peer that sends no state update record.
    kVersionMinorNoStateUpdate = 6,
    kRejectReasonCount = 7,
    kStatusCount = 5,
};

// The methods of the encryption header that pad the data to whole RSA blocks.
#define RECORD_METHOD_PUBLIC 0xe0000000U

// NTSC-U/C: 0x003b6cc4
unsigned char g_bChannelsStarted;
// NTSC-U/C: 0x003b6cc8
unsigned short g_uChannelCount;
// NTSC-U/C: 0x003b6ccc
RtChannel **g_ppChannels;
// NTSC-U/C: 0x003b6cd0
unsigned char *g_pbChannelScratch;
// NTSC-U/C: 0x003b6cd4
unsigned int g_uChannelScratchSize;
// The connection request RtChannelSendConnect() builds.
// NTSC-U/C: 0x005205b0
static unsigned char g_abChannelConnect[kConnectSize];

// The codes RtChannelConnectFunc gets for the reasons of a rejection.
static const int g_anRejectReasons[kRejectReasonCount] = {2, 4, 5, 6, 7, 8, 9};

// NTSC-U/C: 0x002c3430, PAL: 0x002cd3a0
static int RtChannelGetRequestSize(unsigned short uDataSize, unsigned short *puSize) {
    if (puSize == NULL) {
        return RT_CHANNEL_RESULT_INVALID;
    }
    *puSize = uDataSize + kRequestHeaderSize;
    return 0;
}

// NTSC-U/C: 0x002c3450, PAL: 0x002cd3c0
static int RtChannelGetConnectSize(unsigned short *puSize) {
    if (puSize == NULL) {
        return RT_CHANNEL_RESULT_INVALID;
    }
    *puSize = kConnectSize;
    return 0;
}

// NTSC-U/C: 0x002c59d0, PAL: 0x002cf940
static int RtChannelGetVersion(int nMode,
                               unsigned char *puMajor,
                               unsigned char *puMinor,
                               unsigned char *puBuild) {
    if ((unsigned int)nMode >= 2 || puMajor == NULL || puMinor == NULL || puBuild == NULL) {
        return RT_CHANNEL_RESULT_INVALID;
    }
    *puMajor = kVersionMajor;
    *puMinor = kVersionMinorHost;
    *puBuild = kVersionBuildHost;
    if (nMode == RT_CHANNEL_MODE_CLIENT) {
        *puMinor = kVersionMinorClient;
        *puBuild = kVersionBuildClient;
    }
    return 0;
}

// NTSC-U/C: 0x002c5970, PAL: 0x002cf8e0
static int RtChannelCheckVersion(int nMode,
                                 unsigned char uMajor,
                                 unsigned char uMinor,
                                 unsigned char uBuild,
                                 unsigned char *pbCompatible) {
    (void)uBuild;
    if (pbCompatible == NULL) {
        return RT_CHANNEL_RESULT_INVALID;
    }
    if ((unsigned int)nMode >= 2) {
        return RT_CHANNEL_RESULT_INVALID;
    }
    *pbCompatible = 0;
    if (nMode == RT_CHANNEL_MODE_CLIENT) {
        if (uMajor == 0 || uMinor < kVersionMinorClient) {
            return 0;
        }
    } else if (uMajor == 0 || uMinor < kVersionMinorHost) {
        return 0;
    }
    *pbCompatible = 1;
    return 0;
}

// NTSC-U/C: 0x002c5a30, PAL: 0x002cf9a0
static int RtChannelPeerAwaitsConfig(unsigned char uMajor,
                                     unsigned char uMinor,
                                     unsigned char uBuild,
                                     unsigned char *pbAwaits) {
    (void)uMajor;
    (void)uBuild;
    if (pbAwaits == NULL) {
        return RT_CHANNEL_RESULT_INVALID;
    }
    *pbAwaits = 1;
    if (uMinor == kVersionMinorNoStateUpdate) {
        *pbAwaits = 0;
    }
    return 0;
}

// NTSC-U/C: 0x002c5298, PAL: 0x002cf208
static int RtChannelMapRejectReason(int *pnReason, unsigned char uReason) {
    if (pnReason == NULL) {
        return RT_CHANNEL_RESULT_INVALID;
    }
    if (uReason >= kRejectReasonCount) {
        return RT_CHANNEL_RESULT_SHORT;
    }
    *pnReason = g_anRejectReasons[uReason];
    return 0;
}

// NTSC-U/C: 0x002c5350, PAL: 0x002cf2c0
static int RtChannelMapStatus(int *pnStatus, unsigned char uStatus) {
    if (pnStatus == NULL) {
        return RT_CHANNEL_RESULT_INVALID;
    }
    if (uStatus >= kStatusCount) {
        return RT_CHANNEL_RESULT_SHORT;
    }
    *pnStatus = uStatus;
    return 0;
}

// NTSC-U/C: 0x002c5148, PAL: 0x002cf0b8
static int RtChannelIsQueueableRecord(int nType) {
    switch (nType) {
    case kRecordPeerData:
    case 11:
    case kRecordAcknowledge:
    case 14:
    case kRecordControlNoValue:
    case kRecordControl:
    case kRecordRequest:
    case kRecordHostRequest:
    case kRecordConnect:
    case kRecordConfigured:
    case 13:
    case kRecordPublicKey:
    case kRecordPeerSessionKey:
    case kRecordPeerDataGuaranteed:
        return 1;
    default:
        return 0;
    }
}

// NTSC-U/C: 0x002c58d0, PAL: 0x002cf840
static int RtChannelGetEncryption(unsigned short uChannel, unsigned int *puHeader) {
    if (puHeader == NULL) {
        return RT_CHANNEL_RESULT_INVALID;
    }
    *puHeader = RtEncryptPacket(uChannel, RT_SECURITY_ALL_PEERS, NULL, 0);
    return 0;
}

// NTSC-U/C: 0x002c4b90, PAL: 0x002ceb00
static int RtChannelWriteRecordHeader(unsigned char *pbRecord,
                                      const RtRecordHeader *pHeader,
                                      unsigned short *puSize) {
    if (pbRecord == NULL || pHeader == NULL || puSize == NULL) {
        return RT_CHANNEL_RESULT_INVALID;
    }
    *puSize = 0;
    if (RtRecordWriteByte(pbRecord, pHeader->uType) != 0) {
        return RT_CHANNEL_RESULT_FIELD;
    }
    ++*puSize;
    if (RtRecordWriteWord(&pbRecord[1], pHeader->uSize) != 0) {
        return RT_CHANNEL_RESULT_FIELD;
    }
    *puSize += 2;
    return 0;
}

// NTSC-U/C: 0x002c4c20, PAL: 0x002ceb90
static int RtChannelReadRecordHeader(const unsigned char *pbRecord,
                                     RtRecordHeader *pHeader,
                                     unsigned short *puSize) {
    if (pbRecord == NULL || pHeader == NULL || puSize == NULL) {
        return RT_CHANNEL_RESULT_INVALID;
    }
    *puSize = 0;
    if (RtRecordReadByte(pbRecord, &pHeader->uType) != 0) {
        return RT_CHANNEL_RESULT_FIELD;
    }
    ++*puSize;
    if (RtRecordReadWord(&pbRecord[1], &pHeader->uSize) != 0) {
        return RT_CHANNEL_RESULT_FIELD;
    }
    *puSize += 2;
    return 0;
}

// Count the whole records at the start of a buffer that fit in a size.
// NTSC-U/C: 0x002c56d8, PAL: 0x002cf648
static int RtChannelMeasureRecords(const unsigned char *pbData,
                                   unsigned int uSize,
                                   unsigned int uMaxSize,
                                   unsigned int *puSize) {
    const unsigned char *pbEnd;
    RtRecordHeader header;
    unsigned short uHeaderSize;
    unsigned short uRecordSize;
    int nResult;

    uHeaderSize = 0;
    if (puSize == NULL) {
        return RT_CHANNEL_RESULT_INVALID;
    }
    *puSize = 0;
    if (pbData == NULL) {
        return RT_CHANNEL_RESULT_INVALID;
    }
    if (uMaxSize >= uSize) {
        *puSize = uSize;
        return 0;
    }
    pbEnd = &pbData[uSize];
    while (pbData < pbEnd) {
        nResult = RtChannelReadRecordHeader(pbData, &header, &uHeaderSize);
        if (nResult != 0) {
            return nResult;
        }
        uRecordSize = uHeaderSize + header.uSize;
        if (header.uSize != 0 && (header.uType & RT_CHANNEL_RECORD_ENCRYPTED) != 0) {
            uRecordSize += kEncryptionHeaderSize;
        }
        if (*puSize + uRecordSize > uMaxSize) {
            return 0;
        }
        pbData += uRecordSize;
        *puSize += uRecordSize;
    }
    return 0;
}

// Send what the socket takes of some bytes of a link.
// NTSC-U/C: 0x002c57c8, PAL: 0x002cf738
static int RtChannelSendLink(RtChannelLink *pLink,
                             const unsigned char *pbData,
                             unsigned int uSize,
                             unsigned short *puSent) {
    unsigned int uChunk;
    int nResult;

    uChunk = 0;
    if (puSent == NULL) {
        return RT_CHANNEL_RESULT_INVALID;
    }
    *puSent = 0;
    if (RtChannelLinkIsValid(pLink) != 1 || pbData == NULL) {
        return RT_CHANNEL_RESULT_INVALID;
    }
    if (pLink->nTransport == RT_CHANNEL_TRANSPORT_STREAM) {
        uChunk = uSize;
        if (uSize > kStreamReadSize) {
            uChunk = kStreamReadSize;
        }
        nResult = RtSocketSend(pLink->pSocket, pbData, uChunk, puSent);
    } else if (pLink->nTransport == RT_CHANNEL_TRANSPORT_DATAGRAM) {
        nResult = RtChannelMeasureRecords(pbData, uSize, RT_CHANNEL_MAX_PACKET, &uChunk);
        if (nResult != 0) {
            return nResult;
        }
        if (uChunk == 0) {
            return 0;
        }
        nResult =
            RtSocketSendTo(pLink->pSocket, pLink->szAddress, pLink->uPort, pbData, uChunk, puSent);
    } else {
        return RT_CHANNEL_RESULT_UNKNOWN_RECORD;
    }
    nResult = RtChannelMapSocketResult(nResult);
    if (nResult != 0) {
        return nResult;
    }
    return 0;
}

// Pack a request record into a buffer of exactly its size.
// NTSC-U/C: 0x002c3250, PAL: 0x002cd1c0
static int RtChannelPackRequest(unsigned char *pbRequest,
                                unsigned short uSize,
                                int nMode,
                                unsigned short uChannel,
                                unsigned short uClientValue,
                                const void *pData,
                                unsigned short uDataSize) {
    RtKey modulus;
    RtKey exponent;
    unsigned short uExpected;
    unsigned char uMajor;
    unsigned char uMinor;
    unsigned char uBuild;
    unsigned char bEncrypted;
    unsigned char *pbField;
    int nResult;

    bEncrypted = 0;
    uExpected = 0;
    if (g_bChannelsStarted != 1) {
        return RT_CHANNEL_RESULT_NOT_STARTED;
    }
    if ((pData == NULL && uDataSize != 0) || pbRequest == NULL) {
        return RT_CHANNEL_RESULT_INVALID;
    }
    nResult = RtChannelGetRequestSize(uDataSize, &uExpected);
    if (nResult != 0) {
        return nResult;
    }
    if (uSize != uExpected) {
        return RT_CHANNEL_RESULT_INVALID;
    }
    pbField = pbRequest;
    nResult = RtChannelGetVersion(nMode, &uMajor, &uMinor, &uBuild);
    if (nResult != 0) {
        return nResult;
    }
    if (RtRecordWriteByte(pbField, uMajor) != 0) {
        return RT_CHANNEL_RESULT_FIELD;
    }
    ++pbField;
    if (RtRecordWriteByte(pbField, uMinor) != 0) {
        return RT_CHANNEL_RESULT_FIELD;
    }
    ++pbField;
    if (RtRecordWriteByte(pbField, uBuild) != 0) {
        return RT_CHANNEL_RESULT_FIELD;
    }
    ++pbField;
    if (RtRecordWriteWord(pbField, uClientValue) != 0) {
        return RT_CHANNEL_RESULT_FIELD;
    }
    pbField += 2;
    (void)RtRecordWriteDword(pbField, RtTitleGetApplicationId());
    pbField += 4;
    nResult = RtChannelIsEncrypted(uChannel, &bEncrypted);
    if (nResult != 0) {
        return nResult;
    }
    if (bEncrypted == 1) {
        RtTitleGetSigningKey(&modulus, &exponent);
        if (RtSignPeerKey(uChannel, RT_SECURITY_ALL_PEERS, &modulus, &exponent, (RtKey *)pbField) !=
            0) {
            return RT_CHANNEL_RESULT_NOT_SIGNED;
        }
    } else {
        memset(pbField, 0, sizeof(RtKey));
    }
    pbField += sizeof(RtKey);
    if (pData != NULL && uDataSize != 0) {
        memcpy(pbField, pData, uDataSize);
    }
    return 0;
}

// Make room in the send ring of a link for a contiguous write.
// NTSC-U/C: 0x002c3968, PAL: 0x002cd8d8
static int RtChannelMakeSendRoom(RtChannelLink *pLink, unsigned int uSize) {
    unsigned int uRoom;
    int nResult;

    uRoom = 0;
    if (g_bChannelsStarted != 1) {
        return RT_CHANNEL_RESULT_NOT_STARTED;
    }
    if (RtChannelLinkIsValid(pLink) != 1) {
        return RT_CHANNEL_RESULT_INVALID;
    }
    if (RtGetRingBufferWritable(&pLink->send, &uRoom) != 0) {
        return RT_CHANNEL_RESULT_RING;
    }
    if (uRoom >= uSize) {
        return 0;
    }
    nResult = RtChannelFlushSend(pLink);
    if (nResult != 0) {
        return nResult;
    }
    if (RtCompactRingBufferWith(&pLink->send, 1, g_pbChannelScratch) != 0) {
        return RT_CHANNEL_RESULT_RING;
    }
    if (RtGetRingBufferWritable(&pLink->send, &uRoom) != 0) {
        return RT_CHANNEL_RESULT_RING;
    }
    if (uRoom < uSize) {
        return RT_CHANNEL_RESULT_NO_ROOM;
    }
    return 0;
}

// Report a result to the receiver of the connection, and whether the channel survived it.
static inline int RtChannelNotifyConnect(
    RtChannel *pChannel, int nConnected, unsigned short uPeer, unsigned short uValue, int nReason) {
    unsigned short uId;

    if (pChannel->pfnConnect == NULL) {
        return 0;
    }
    uId = pChannel->uId;
    pChannel->pfnConnect(uId, nConnected, uPeer, uValue, nReason);
    if (RtChannelIsValidId(uId) != 0) {
        return 0;
    }
    return RT_CHANNEL_RESULT_CLOSED;
}

// NTSC-U/C: 0x002c44b0, PAL: 0x002ce420
static int RtChannelHandleStateUpdate(RtChannel *pChannel,
                                      int nType,
                                      const unsigned char *pbData,
                                      unsigned short uSize) {
    unsigned short uValue;
    int nResult;

    if (g_bChannelsStarted != 1) {
        return RT_CHANNEL_RESULT_NOT_STARTED;
    }
    if (RtChannelIsValid(pChannel) != 1 || pbData == NULL || nType != kRecordStateUpdate) {
        return RT_CHANNEL_RESULT_INVALID;
    }
    if (uSize <= 1) {
        return RT_CHANNEL_RESULT_SHORT;
    }
    if (RtRecordReadWord(pbData, &uValue) != 0) {
        return RT_CHANNEL_RESULT_FIELD;
    }
    pChannel->uServerValue = uValue;
    nResult = RtChannelSetState(pChannel, kStateConnected);
    if (nResult != 0) {
        return nResult;
    }
    return RtChannelNotifyConnect(pChannel, 1, pChannel->uPeer, pChannel->uServerValue, 0);
}

// NTSC-U/C: 0x002c3f50, PAL: 0x002cdec0
static int RtChannelHandleConnectReply(RtChannel *pChannel,
                                       int nType,
                                       const unsigned char *pbData,
                                       unsigned short uSize) {
    unsigned char uMajor;
    unsigned char uMinor;
    unsigned char uBuild;
    unsigned char bCompatible;
    unsigned short uPeer;
    unsigned char uReason;
    unsigned short uValue;
    unsigned char bAwaits;
    int nReason;
    unsigned int uOffset;
    int nResult;

    nReason = 0;
    bCompatible = 0;
    if (g_bChannelsStarted != 1) {
        return RT_CHANNEL_RESULT_NOT_STARTED;
    }
    if (RtChannelIsValid(pChannel) != 1) {
        return RT_CHANNEL_RESULT_INVALID;
    }
    if (pbData == NULL) {
        return RT_CHANNEL_RESULT_INVALID;
    }
    if (uSize == 0) {
        return RT_CHANNEL_RESULT_SHORT;
    }
    if (RtRecordReadByte(pbData, &uMajor) != 0) {
        return RT_CHANNEL_RESULT_FIELD;
    }
    ++pbData;
    if (uSize <= 1) {
        return RT_CHANNEL_RESULT_SHORT;
    }
    if (RtRecordReadByte(pbData, &uMinor) != 0) {
        return RT_CHANNEL_RESULT_FIELD;
    }
    ++pbData;
    if (uSize <= 2) {
        return RT_CHANNEL_RESULT_SHORT;
    }
    if (RtRecordReadByte(pbData, &uBuild) != 0) {
        return RT_CHANNEL_RESULT_FIELD;
    }
    nResult = RtChannelCheckVersion(pChannel->nMode, uMajor, uMinor, uBuild, &bCompatible);
    ++pbData;
    if (nResult != 0) {
        return nResult;
    }
    if (bCompatible != 1) {
        return RT_CHANNEL_RESULT_BAD_VERSION;
    }
    if (nType == kRecordConnectAccepted) {
        if (uSize <= 4) {
            return RT_CHANNEL_RESULT_SHORT;
        }
        if (RtRecordReadWord(pbData, &uPeer) != 0) {
            return RT_CHANNEL_RESULT_FIELD;
        }
        pbData += 2;
        uOffset = 5;
    } else if (nType == kRecordConnectRejected) {
        if (uSize <= 3) {
            return RT_CHANNEL_RESULT_SHORT;
        }
        if (RtRecordReadByte(pbData, &uReason) != 0) {
            return RT_CHANNEL_RESULT_FIELD;
        }
        nResult = RtChannelMapRejectReason(&nReason, uReason);
        if (nResult != 0) {
            return nResult;
        }
        ++pbData;
        uOffset = 4;
    } else {
        return RT_CHANNEL_RESULT_SHORT;
    }
    if (uOffset + 1 >= uSize) {
        return RT_CHANNEL_RESULT_SHORT;
    }
    if (RtRecordReadWord(pbData, &uValue) != 0) {
        return RT_CHANNEL_RESULT_FIELD;
    }
    pbData += 2;
    uOffset = (unsigned short)(uOffset + 2);
    if (uSize < uOffset + RT_CHANNEL_TOKEN_SIZE) {
        return RT_CHANNEL_RESULT_SHORT;
    }
    memcpy(pChannel->szServerName, pbData, RT_CHANNEL_TOKEN_SIZE);
    if (nType != kRecordConnectAccepted) {
        nResult = RtChannelSetState(pChannel, kStateClosed);
        if (nResult != 0) {
            return nResult;
        }
        return RtChannelNotifyConnect(pChannel, 0, 0, uValue, nReason);
    }
    pChannel->uServerValue = uValue;
    pChannel->uPeer = uPeer;
    RtSetConnectionPeer(pChannel->uId, uPeer);
    if (pChannel->nMode == RT_CHANNEL_MODE_CLIENT) {
        bAwaits = 1;
        nResult = RtChannelPeerAwaitsConfig(uMajor, uMinor, uBuild, &bAwaits);
        if (nResult != 0) {
            return nResult;
        }
        if (bAwaits == 1) {
            nResult = RtChannelSetState(pChannel, kStateConfigWait);
        } else {
            nResult = RtChannelHandleStateUpdate(
                pChannel, kRecordStateUpdate, (const unsigned char *)&uValue, 2);
        }
    } else if (pChannel->nMode == RT_CHANNEL_MODE_HOST) {
        nResult = RtChannelSetState(pChannel, kStateHostAccepted);
    } else {
        return RT_CHANNEL_RESULT_UNKNOWN_RECORD;
    }
    if (nResult != 0) {
        return nResult;
    }
    return 0;
}

// NTSC-U/C: 0x002c4288, PAL: 0x002ce1f8
static int RtChannelHandleConfig(RtChannel *pChannel,
                                 int nType,
                                 const unsigned char *pbData,
                                 unsigned short uSize) {
    unsigned char uMajor;
    unsigned char uMinor;
    unsigned char uBuild;
    unsigned char bCompatible;
    unsigned short uPeer;
    int nState;
    int nResult;

    bCompatible = 0;
    if (g_bChannelsStarted != 1) {
        return RT_CHANNEL_RESULT_NOT_STARTED;
    }
    if (RtChannelIsValid(pChannel) != 1) {
        return RT_CHANNEL_RESULT_INVALID;
    }
    if (pbData == NULL || nType != kRecordConfig) {
        return RT_CHANNEL_RESULT_INVALID;
    }
    nState = pChannel->nState;
    if (nState != kStateRedirected || pChannel->nMode != RT_CHANNEL_MODE_HOST || uSize == 0) {
        return RT_CHANNEL_RESULT_SHORT;
    }
    if (RtRecordReadByte(pbData, &uMajor) != 0) {
        return RT_CHANNEL_RESULT_FIELD;
    }
    ++pbData;
    if (uSize <= 1) {
        return RT_CHANNEL_RESULT_SHORT;
    }
    if (RtRecordReadByte(pbData, &uMinor) != 0) {
        return RT_CHANNEL_RESULT_FIELD;
    }
    ++pbData;
    if (uSize <= 2) {
        return RT_CHANNEL_RESULT_SHORT;
    }
    if (RtRecordReadByte(pbData, &uBuild) != 0) {
        return RT_CHANNEL_RESULT_FIELD;
    }
    ++pbData;
    nResult = RtChannelCheckVersion(pChannel->nMode, uMajor, uMinor, uBuild, &bCompatible);
    if (nResult != 0) {
        return nResult;
    }
    if (bCompatible != 1) {
        return RT_CHANNEL_RESULT_BAD_VERSION;
    }
    if (uSize <= 4) {
        return RT_CHANNEL_RESULT_SHORT;
    }
    if (RtRecordReadWord(pbData, &uPeer) != 0) {
        return RT_CHANNEL_RESULT_FIELD;
    }
    pbData += 2;
    if (uPeer != pChannel->uPeer) {
        return RT_CHANNEL_RESULT_SHORT;
    }
    // Yes, the binary compares the size with the state, which is 6 here.
    if (uSize <= (unsigned int)nState) {
        return RT_CHANNEL_RESULT_SHORT;
    }
    if (RtRecordReadWord(pbData, &pChannel->uServerValue) != 0) {
        return RT_CHANNEL_RESULT_FIELD;
    }
    if (uSize < 23) {
        return RT_CHANNEL_RESULT_SHORT;
    }
    pbData += 2;
    memcpy(pChannel->szServerName, pbData, RT_CHANNEL_TOKEN_SIZE);
    if (uSize <= 24) {
        return RT_CHANNEL_RESULT_SHORT;
    }
    pbData += RT_CHANNEL_TOKEN_SIZE;
    if (RtRecordReadWord(pbData, &pChannel->uConfigValue) != 0) {
        return RT_CHANNEL_RESULT_FIELD;
    }
    nResult = RtChannelQueueRecord(
        pChannel->pActiveLink, kRecordConfigured, RT_CHANNEL_ALL_PEERS, NULL, 0, 1);
    if (nResult != 0) {
        return nResult;
    }
    return RtChannelSetState(pChannel, kStateConfigWait);
}

// NTSC-U/C: 0x002c45c0, PAL: 0x002ce530
static int RtChannelHandlePeer(RtChannel *pChannel,
                               int nType,
                               const unsigned char *pbData,
                               unsigned short uSize) {
    RtChannelPeerInfo info;
    unsigned short uPeer;
    int bHasKey;
    unsigned short uId;

    bHasKey = 0;
    uPeer = 0;
    if (RtChannelIsValid(pChannel) != 1 || pbData == NULL) {
        return RT_CHANNEL_RESULT_INVALID;
    }
    if (nType == kRecordPeerWithKey) {
        bHasKey = 1;
    } else if (nType != kRecordPeer) {
        return RT_CHANNEL_RESULT_INVALID;
    }
    if (uSize <= 1) {
        return RT_CHANNEL_RESULT_SHORT;
    }
    if (RtRecordReadWord(pbData, &uPeer) != 0) {
        return RT_CHANNEL_RESULT_FIELD;
    }
    pbData += kPeerFieldSize;
    if (bHasKey) {
        if (uSize - kPeerFieldSize != sizeof(RtChannelPeerInfo)) {
            return RT_CHANNEL_RESULT_SHORT;
        }
    } else if (uSize - kPeerFieldSize != RT_CHANNEL_ADDRESS_SIZE) {
        return RT_CHANNEL_RESULT_SHORT;
    }
    memcpy(info.szName, pbData, RT_CHANNEL_ADDRESS_SIZE);
    if (bHasKey) {
        memcpy(&info.publicKey, &pbData[RT_CHANNEL_ADDRESS_SIZE], sizeof(RtKey));
        RtStorePeerPublicKey(pChannel->uId, uPeer, &info.publicKey);
    }
    if (pChannel->pfnPeer == NULL) {
        return 0;
    }
    uId = pChannel->uId;
    pChannel->pfnPeer(uId, uPeer, &info, bHasKey);
    if (RtChannelIsValidId(uId) != 0) {
        return 0;
    }
    return RT_CHANNEL_RESULT_CLOSED;
}

// NTSC-U/C: 0x002c47a8, PAL: 0x002ce718
static int RtChannelHandleData(RtChannel *pChannel,
                               int nType,
                               const unsigned char *pbData,
                               unsigned short uSize) {
    unsigned short uPeer;
    unsigned short uHeaderSize;
    unsigned short uId;

    uPeer = 0;
    uHeaderSize = 0;
    if (RtChannelIsValid(pChannel) != 1 || pbData == NULL) {
        return RT_CHANNEL_RESULT_INVALID;
    }
    if (nType == kRecordPeerData || nType == kRecordPeerDataGuaranteed) {
        if (uSize <= 1) {
            return RT_CHANNEL_RESULT_SHORT;
        }
        if (RtRecordReadWord(pbData, &uPeer) != 0) {
            return RT_CHANNEL_RESULT_FIELD;
        }
        pbData += kPeerFieldSize;
        uHeaderSize = kPeerFieldSize;
    } else if (nType == kRecordBroadcast) {
        uPeer = RT_CHANNEL_ALL_PEERS;
    } else {
        return RT_CHANNEL_RESULT_INVALID;
    }
    if (pChannel->pfnData == NULL) {
        return 0;
    }
    uId = pChannel->uId;
    pChannel->pfnData(uId, uPeer, pbData, uSize - uHeaderSize);
    if (RtChannelIsValidId(uId) != 0) {
        return 0;
    }
    return RT_CHANNEL_RESULT_CLOSED;
}

// NTSC-U/C: 0x002c48a8, PAL: 0x002ce818
static int RtChannelHandleStatus(RtChannel *pChannel,
                                 int nType,
                                 const unsigned char *pbData,
                                 unsigned short uSize) {
    unsigned char uStatus;
    int nStatus;
    unsigned short uId;
    int nResult;

    (void)nType;
    nStatus = 0;
    if (g_bChannelsStarted != 1) {
        return RT_CHANNEL_RESULT_NOT_STARTED;
    }
    if (RtChannelIsValid(pChannel) != 1 || pbData == NULL) {
        return RT_CHANNEL_RESULT_INVALID;
    }
    if (uSize == 0) {
        return RT_CHANNEL_RESULT_SHORT;
    }
    if (RtRecordReadByte(pbData, &uStatus) != 0) {
        return RT_CHANNEL_RESULT_FIELD;
    }
    nResult = RtChannelMapStatus(&nStatus, uStatus);
    if (nResult != 0) {
        return nResult;
    }
    if (pChannel->pfnStatus == NULL) {
        return 0;
    }
    uId = pChannel->uId;
    pChannel->pfnStatus(uId, nStatus);
    if (RtChannelIsValidId(uId) != 0) {
        return 0;
    }
    return RT_CHANNEL_RESULT_CLOSED;
}

// NTSC-U/C: 0x002c4988, PAL: 0x002ce8f8
static int RtChannelHandleSessionKey(RtChannel *pChannel,
                                     int nType,
                                     const unsigned char *pbData,
                                     unsigned short uSize) {
    unsigned short uPeer;
    int nResult;

    uPeer = RT_CHANNEL_ALL_PEERS;
    if (g_bChannelsStarted != 1) {
        return RT_CHANNEL_RESULT_NOT_STARTED;
    }
    if (pChannel == NULL || pbData == NULL) {
        return RT_CHANNEL_RESULT_INVALID;
    }
    if (nType == kRecordPeerSessionKey) {
        (void)RtRecordReadWord(pbData, &uPeer); // Yes, the binary discards the result.
        pbData += kPeerFieldSize;
        uSize -= kPeerFieldSize;
    }
    if (uSize != sizeof(RtKey)) {
        return RT_CHANNEL_RESULT_INVALID;
    }
    (void)RtSetPeerKey(pChannel->uId, uPeer, (const RtKey *)pbData); // Yes, the binary discards it.
    if (pChannel->nState != kStateKeyWait) {
        return 0;
    }
    nResult = RtChannelSendRequest(pChannel->nMode,
                                   pChannel->pActiveLink,
                                   pChannel->config.uClientValue,
                                   pChannel->config.pRequestData,
                                   pChannel->config.uRequestSize);
    if (nResult != 0) {
        return nResult;
    }
    nResult = RtChannelSetState(pChannel, kStateRequested);
    if (nResult != 0) {
        return nResult;
    }
    return 0;
}

// NTSC-U/C: 0x002c4a70, PAL: 0x002ce9e0
static int RtChannelHandleRedirect(RtChannel *pChannel,
                                   int nType,
                                   const unsigned char *pbData,
                                   unsigned short uSize) {
    RtChannelLink *pLink;
    int nResult;

    (void)nType;
    if (g_bChannelsStarted != 1) {
        return RT_CHANNEL_RESULT_NOT_STARTED;
    }
    if (RtChannelIsValid(pChannel) != 1 || pbData == NULL) {
        return RT_CHANNEL_RESULT_INVALID;
    }
    if (pChannel->nMode != RT_CHANNEL_MODE_HOST) {
        return RT_CHANNEL_RESULT_SHORT;
    }
    pLink = pChannel->apLinks[RT_CHANNEL_LINK_DATAGRAM];
    if (pLink == NULL) {
        return RT_CHANNEL_RESULT_NO_LINK;
    }
    if (uSize < RT_CHANNEL_ADDRESS_SIZE) {
        return RT_CHANNEL_RESULT_SHORT;
    }
    memcpy(pLink->szAddress, pbData, RT_CHANNEL_ADDRESS_SIZE);
    if (uSize <= 17) {
        return RT_CHANNEL_RESULT_SHORT;
    }
    if (RtRecordReadWord(&pbData[RT_CHANNEL_ADDRESS_SIZE], &pLink->uPort) != 0) {
        return RT_CHANNEL_RESULT_FIELD;
    }
    if (pLink->uPort == 0) {
        return RT_CHANNEL_RESULT_SHORT;
    }
    pLink->bOpen = 1;
    nResult = RtChannelSendConnect(pChannel, pLink);
    if (nResult != 0) {
        return nResult;
    }
    return RtChannelSetState(pChannel, kStateRedirected);
}

// NTSC-U/C: 0x002c3c20, PAL: 0x002cdb90
static int RtChannelDispatchRecord(RtChannel *pChannel,
                                   RtChannelLink *pLink,
                                   unsigned char uType,
                                   unsigned char *pbData,
                                   unsigned short uSize) {
    unsigned int uHeader;
    unsigned short uPeer;
    int bEncrypted;
    int nType;
    int nResult;

    bEncrypted = (uType >> kRecordEncryptedShift) & 1;
    nType = uType & kRecordTypeMask;
    uHeader = 0;
    if (RtChannelIsValid(pChannel) != 1 || RtChannelLinkIsValid(pLink) != 1 || pLink->bOpen != 1 ||
        pChannel->uId != pLink->uChannel || pbData == NULL) {
        return RT_CHANNEL_RESULT_INVALID;
    }
    if (bEncrypted && uSize != 0) {
        RtRecordReadDword(pbData, &uHeader);
        pbData += kEncryptionHeaderSize;
        if (RtChannelGetSecurityState() != RT_CHANNEL_SECURITY_KEYS) {
            return RT_CHANNEL_RESULT_NOT_SIGNED;
        }
        if (uHeader == 0) {
            return RT_CHANNEL_RESULT_NO_ENCRYPTION;
        }
        // The decryption results are discarded, as in the binary.
        if (nType == kRecordPeerDataGuaranteed || nType == kRecordPeerSessionKey) {
            uPeer = 0;
            RtRecordReadWord(pbData, &uPeer);
            RtDecryptPacket(pChannel->uId,
                            RT_SECURITY_ALL_PEERS,
                            uHeader,
                            &pbData[kPeerFieldSize],
                            uSize - kPeerFieldSize);
            // Yes, the binary overwrites the peer it just read.
            uPeer = RT_CHANNEL_ALL_PEERS;
            (void)uPeer;
        } else {
            RtDecryptPacket(pChannel->uId, RT_SECURITY_ALL_PEERS, uHeader, pbData, uSize);
        }
    }
    if ((unsigned int)(nType - kRecordFirstDispatched) >= kRecordDispatchCount) {
        return RT_CHANNEL_RESULT_UNKNOWN_RECORD;
    }
    switch (nType) {
    case kRecordPeerData:
    case kRecordPeerDataGuaranteed:
    case kRecordBroadcast:
        if (pChannel->nState != kStateConnected) {
            return 0;
        }
        return RtChannelHandleData(pChannel, nType, pbData, uSize);
    case kRecordAcknowledge:
        --pLink->uPending;
        return 0;
    case kRecordConnectRejected:
    case kRecordConnectAccepted:
        nResult = RtChannelHandleConnectReply(pChannel, nType, pbData, uSize);
        if (nResult != RT_CHANNEL_RESULT_BAD_VERSION) {
            return nResult;
        }
        nResult = RtChannelSetState(pChannel, kStateClosed);
        if (nResult != 0) {
            return nResult;
        }
        return RtChannelNotifyConnect(pChannel, 0, 0, 0, kReasonBadVersion);
    case kRecordPeerWithKey:
    case kRecordPeer:
        return RtChannelHandlePeer(pChannel, nType, pbData, uSize);
    case kRecordStatus:
        return RtChannelHandleStatus(pChannel, nType, pbData, uSize);
    case kRecordSessionKey:
    case kRecordPeerSessionKey:
        return RtChannelHandleSessionKey(pChannel, nType, pbData, uSize);
    case kRecordConnectionKey:
        RtSetConnectionKey(pChannel->uId, pChannel->uId, (const RtKey *)pbData);
        return 0;
    case kRecordRedirect:
        return RtChannelHandleRedirect(pChannel, nType, pbData, uSize);
    case kRecordConfig:
        if (pChannel->nState != kStateRedirected) {
            // Yes, the binary reports 1 for a configuration record out of turn.
            return RT_CHANNEL_RESULT_SHORT;
        }
        return RtChannelHandleConfig(pChannel, nType, pbData, uSize);
    case kRecordStateUpdate:
        return RtChannelHandleStateUpdate(pChannel, nType, pbData, uSize);
    default:
        return RT_CHANNEL_RESULT_UNKNOWN_RECORD;
    }
}

int RtChannelCreate(unsigned short *puId,
                    int nMode,
                    unsigned int uSendSize,
                    unsigned int uReceiveSize) {
    RtChannel **ppTable;
    RtChannel *pChannel;
    RtChannelLink *pLink;
    unsigned short uId;
    unsigned short uNewCount;
    int nResult;

    ppTable = NULL;
    pLink = NULL;
    uId = 0;
    if (g_bChannelsStarted != 1) {
        return RT_CHANNEL_RESULT_NOT_STARTED;
    }
    if (puId == NULL || (unsigned int)nMode >= 2 || uSendSize == 0 || uReceiveSize == 0) {
        return RT_CHANNEL_RESULT_INVALID;
    }
    if (g_uChannelCount != 0 && g_ppChannels[0] != NULL) {
        do {
            ++uId;
        } while (uId < g_uChannelCount && g_ppChannels[uId] != NULL);
    }
    if (uId == g_uChannelCount) {
        uNewCount = g_uChannelCount + kChannelTableGrowth;
        ppTable = NULL;
        if (RtAllocZeroedBlock((void **)&ppTable, uNewCount * sizeof(RtChannel *)) != 0) {
            return RT_CHANNEL_RESULT_NO_MEMORY;
        }
        if (g_ppChannels != NULL) {
            memcpy(ppTable, g_ppChannels, g_uChannelCount * sizeof(RtChannel *));
            RtFreeBlock((void **)&g_ppChannels);
        }
        g_uChannelCount = uNewCount;
        g_ppChannels = ppTable;
    }
    pChannel = NULL;
    if (RtAllocZeroedBlock((void **)&pChannel, sizeof(RtChannel)) != 0) {
        return RT_CHANNEL_RESULT_NO_MEMORY;
    }
    g_ppChannels[uId] = pChannel;
    pChannel = g_ppChannels[uId];
    pChannel->uId = uId;
    pChannel->nMode = nMode;
    nResult = RtChannelSetState(pChannel, kStateCreated);
    if (nResult != 0) {
        return nResult;
    }
    if (nMode == RT_CHANNEL_MODE_HOST) {
        uSendSize >>= 1;
        uReceiveSize >>= 1;
    }
    if (uSendSize < RT_CHANNEL_MAX_PACKET) {
        uSendSize = RT_CHANNEL_MAX_PACKET;
    }
    if (uReceiveSize < RT_CHANNEL_MAX_PACKET) {
        uReceiveSize = RT_CHANNEL_MAX_PACKET;
    }
    pLink = NULL;
    if (RtAllocZeroedBlock((void **)&pLink, sizeof(RtChannelLink)) != 0) {
        return RT_CHANNEL_RESULT_NO_MEMORY;
    }
    if (pLink == NULL) {
        return RT_CHANNEL_RESULT_NO_LINK;
    }
    pLink->bAllocated = 1;
    pLink->uChannel = uId;
    pLink->nTransport = RT_CHANNEL_TRANSPORT_STREAM;
    if (RtInitRingBuffer(&pLink->send, uSendSize, NULL) != 0) {
        return RT_CHANNEL_RESULT_RING;
    }
    pLink->uSendSize = uSendSize;
    if (RtInitRingBuffer(&pLink->receive, uReceiveSize, NULL) != 0) {
        return RT_CHANNEL_RESULT_RING;
    }
    pLink->uReceiveSize = uReceiveSize;
    g_ppChannels[uId]->apLinks[RT_CHANNEL_LINK_STREAM] = pLink;
    if (nMode == RT_CHANNEL_MODE_HOST) {
        pLink = NULL;
        if (RtAllocZeroedBlock((void **)&pLink, sizeof(RtChannelLink)) != 0) {
            return RT_CHANNEL_RESULT_NO_MEMORY;
        }
        if (pLink == NULL) {
            return RT_CHANNEL_RESULT_NO_LINK;
        }
        pLink->bAllocated = nMode;
        pLink->nTransport = RT_CHANNEL_TRANSPORT_DATAGRAM;
        pLink->uChannel = uId;
        if (RtInitRingBuffer(&pLink->send, uSendSize, NULL) != 0) {
            return RT_CHANNEL_RESULT_RING;
        }
        pLink->uSendSize = uSendSize;
        if (RtInitRingBuffer(&pLink->receive, uReceiveSize, NULL) != 0) {
            return RT_CHANNEL_RESULT_RING;
        }
        pLink->uReceiveSize = uReceiveSize;
        g_ppChannels[uId]->apLinks[RT_CHANNEL_LINK_DATAGRAM] = pLink;
    }
    nResult = RtChannelReserveScratch(uSendSize);
    if (nResult != 0) {
        return nResult;
    }
    nResult = RtChannelReserveScratch(uReceiveSize);
    if (nResult != 0) {
        return nResult;
    }
    nResult = RtChannelSetState(g_ppChannels[uId], kStateReady);
    if (nResult != 0) {
        return nResult;
    }
    *puId = uId;
    return 0;
}

int RtChannelDestroy(unsigned short uId) {
    RtChannel *pChannel;
    unsigned int i;

    if (g_bChannelsStarted != 1) {
        return RT_CHANNEL_RESULT_NOT_STARTED;
    }
    if (uId >= g_uChannelCount) {
        return RT_CHANNEL_RESULT_INVALID;
    }
    if (g_ppChannels[uId] != NULL) {
        for (i = 0; i < RT_CHANNEL_LINK_COUNT; ++i) {
            if (g_ppChannels[uId]->apLinks[i] != NULL) {
                RtDestroyRingBuffer(&g_ppChannels[uId]->apLinks[i]->receive);
                RtDestroyRingBuffer(&g_ppChannels[uId]->apLinks[i]->send);
                RtFreeBlock((void **)&g_ppChannels[uId]->apLinks[i]);
            }
        }
        pChannel = g_ppChannels[uId];
        if (pChannel->config.pRequestData != NULL) {
            RtFreeBlock(&pChannel->config.pRequestData);
        }
        memset(g_ppChannels[uId], 0, sizeof(RtChannel));
        RtFreeBlock((void **)&g_ppChannels[uId]);
        g_ppChannels[uId] = NULL;
    }
    RtDestroyConnectionKeys(uId);
    return 0;
}

int RtChannelIsValid(const RtChannel *pChannel) {
    const RtChannelLink *pLink;
    unsigned int i;

    if (pChannel == NULL || (unsigned int)pChannel->nState < 2 ||
        !RtChannelLinkIsValid(pChannel->pActiveLink)) {
        return 0;
    }
    for (i = 0; i < RT_CHANNEL_LINK_COUNT; ++i) {
        pLink = pChannel->apLinks[i];
        if (pLink != NULL) {
            if (pChannel->uId != pLink->uChannel) {
                return 0;
            }
            if (!RtChannelLinkIsValid(pLink)) {
                return 0;
            }
        }
    }
    return 1;
}

int RtChannelIsValidId(unsigned short uId) {
    if (g_ppChannels == NULL || uId >= g_uChannelCount) {
        return 0;
    }
    return RtChannelIsValid(g_ppChannels[uId]);
}

int RtChannelGet(unsigned short uId, RtChannel **ppChannel) {
    if (ppChannel == NULL) {
        return RT_CHANNEL_RESULT_INVALID;
    }
    if (g_ppChannels == NULL || g_uChannelCount == 0) {
        return RT_CHANNEL_RESULT_SHORT;
    }
    if (uId >= g_uChannelCount) {
        return RT_CHANNEL_RESULT_INVALID;
    }
    *ppChannel = g_ppChannels[uId];
    return 0;
}

int RtChannelGetValid(unsigned short uId, RtChannel **ppChannel) {
    if (ppChannel == NULL) {
        return RT_CHANNEL_RESULT_INVALID;
    }
    if (RtChannelGet(uId, ppChannel) != 0) {
        return RT_CHANNEL_RESULT_CLOSED;
    }
    if (RtChannelIsValid(*ppChannel) == 0) {
        return RT_CHANNEL_RESULT_CLOSED;
    }
    return 0;
}

int RtChannelLinkIsValid(const RtChannelLink *pLink) {
    // Yes, the binary also rejects the link addresses whose rings would sit at address 0.
    return pLink != NULL && (uintptr_t)pLink != (uintptr_t)-offsetof(RtChannelLink, receive) &&
           (uintptr_t)pLink != (uintptr_t)-offsetof(RtChannelLink, send);
}

int RtChannelSendPublicKey(RtChannelLink *pLink) {
    RtKey modulus;

    if (g_bChannelsStarted != 1) {
        return RT_CHANNEL_RESULT_NOT_STARTED;
    }
    if (RtChannelLinkIsValid(pLink) != 1) {
        return RT_CHANNEL_RESULT_INVALID;
    }
    RtReadKeyPairModulus(0, &modulus);
    return RtChannelQueueRecord(
        pLink, kRecordPublicKey, RT_CHANNEL_ALL_PEERS, &modulus, sizeof(modulus), 1);
}

int RtChannelSendRequest(int nMode,
                         RtChannelLink *pLink,
                         unsigned short uClientValue,
                         const void *pData,
                         unsigned short uSize) {
    unsigned char *pbRequest;
    unsigned short uRequestSize;
    int nResult;

    pbRequest = NULL;
    uRequestSize = 0;
    if (g_bChannelsStarted != 1) {
        return RT_CHANNEL_RESULT_NOT_STARTED;
    }
    if (RtChannelLinkIsValid(pLink) != 1) {
        return RT_CHANNEL_RESULT_INVALID;
    }
    if ((pData == NULL && uSize != 0) || (unsigned int)nMode >= 2) {
        return RT_CHANNEL_RESULT_INVALID;
    }
    nResult = RtChannelGetRequestSize(uSize, &uRequestSize);
    if (nResult != 0) {
        return nResult;
    }
    if (RtAllocZeroedBlock((void **)&pbRequest, uRequestSize) != 0) {
        return RT_CHANNEL_RESULT_NO_MEMORY;
    }
    nResult = RtChannelPackRequest(
        pbRequest, uRequestSize, nMode, pLink->uChannel, uClientValue, pData, uSize);
    if (nResult == 0) {
        nResult = RtChannelQueueRecord(pLink,
                                       nMode == RT_CHANNEL_MODE_CLIENT ? kRecordRequest :
                                                                         kRecordHostRequest,
                                       RT_CHANNEL_ALL_PEERS,
                                       pbRequest,
                                       uRequestSize,
                                       1);
    }
    if (pbRequest != NULL) {
        RtFreeBlock((void **)&pbRequest);
    }
    return nResult;
}

int RtChannelSendDisconnect(RtChannelLink *pLink) {
    unsigned char abRecord[RT_CHANNEL_RECORD_HEADER_SIZE];
    RtRecordHeader header;
    unsigned short uSize;
    int nResult;

    uSize = 0;
    if (g_bChannelsStarted != 1) {
        return RT_CHANNEL_RESULT_NOT_STARTED;
    }
    if (RtChannelLinkIsValid(pLink) != 1) {
        return RT_CHANNEL_RESULT_INVALID;
    }
    header.uType = kRecordDisconnect;
    header.uSize = 0;
    nResult = RtChannelWriteRecordHeader(abRecord, &header, &uSize);
    if (nResult != 0) {
        return nResult;
    }
    return RtChannelSendBlocking(pLink, abRecord, uSize, kDisconnectTimeout);
}

int RtChannelSendControl(RtChannelLink *pLink, unsigned short uValue) {
    unsigned char abValue[2];
    const unsigned char *pbData;
    unsigned short uSize;
    int nType;
    int nResult;

    uSize = 0;
    if (g_bChannelsStarted != 1) {
        return RT_CHANNEL_RESULT_NOT_STARTED;
    }
    if (RtChannelLinkIsValid(pLink) != 1) {
        return RT_CHANNEL_RESULT_INVALID;
    }
    if (uValue == RT_CHANNEL_ALL_PEERS) {
        return 0;
    }
    if (uValue == RT_CHANNEL_MIXED_PEERS) {
        nType = kRecordControlNoValue;
        pbData = NULL;
    } else {
        nType = kRecordControl;
        if (RtRecordWriteWord(abValue, uValue) != 0) {
            return RT_CHANNEL_RESULT_FIELD;
        }
        pbData = abValue;
        uSize = sizeof(abValue);
    }
    nResult = RtChannelQueueRecord(pLink, nType, RT_CHANNEL_ALL_PEERS, pbData, uSize, 0);
    if (nResult != 0) {
        return nResult;
    }
    return 0;
}

int RtChannelSendConnect(RtChannel *pChannel, RtChannelLink *pLink) {
    unsigned short uSize;
    unsigned char uMajor;
    unsigned char uMinor;
    unsigned char uBuild;
    unsigned char *pbField;
    int nResult;

    pbField = g_abChannelConnect;
    uSize = 0;
    if (g_bChannelsStarted != 1) {
        return RT_CHANNEL_RESULT_NOT_STARTED;
    }
    if (RtChannelIsValid(pChannel) != 1 || RtChannelLinkIsValid(pLink) != 1 || pLink->bOpen != 1 ||
        pChannel->uId != pLink->uChannel) {
        return RT_CHANNEL_RESULT_INVALID;
    }
    nResult = RtChannelGetConnectSize(&uSize);
    if (nResult != 0) {
        return nResult;
    }
    if (uSize != kConnectSize) {
        return RT_CHANNEL_RESULT_SHORT;
    }
    nResult = RtChannelGetVersion(pChannel->nMode, &uMajor, &uMinor, &uBuild);
    if (nResult != 0) {
        return nResult;
    }
    if (RtRecordWriteByte(pbField, uMajor) != 0) {
        return RT_CHANNEL_RESULT_FIELD;
    }
    ++pbField;
    if (RtRecordWriteByte(pbField, uMinor) != 0) {
        return RT_CHANNEL_RESULT_FIELD;
    }
    ++pbField;
    if (RtRecordWriteByte(pbField, uBuild) != 0) {
        return RT_CHANNEL_RESULT_FIELD;
    }
    ++pbField;
    if (RtRecordWriteWord(pbField, pChannel->config.uClientValue) != 0) {
        return RT_CHANNEL_RESULT_FIELD;
    }
    pbField += 2;
    (void)RtRecordWriteDword(pbField, RtTitleGetApplicationId());
    pbField += 4;
    memcpy(pbField, pChannel->config.szAddress, RT_CHANNEL_ADDRESS_SIZE);
    pbField += RT_CHANNEL_ADDRESS_SIZE;
    if (RtRecordWriteWord(pbField, pChannel->config.uPort) != 0) {
        return RT_CHANNEL_RESULT_FIELD;
    }
    pbField += 2;
    if (RtRecordWriteWord(pbField, pChannel->uPeer) != 0) {
        return RT_CHANNEL_RESULT_FIELD;
    }
    nResult = RtChannelQueueRecord(
        pLink, kRecordConnect, RT_CHANNEL_ALL_PEERS, g_abChannelConnect, kConnectSize, 1);
    if (nResult != 0) {
        return nResult;
    }
    if (RtReadTime(&pChannel->connectTime) != 0) {
        return RT_CHANNEL_RESULT_CLOCK;
    }
    return 0;
}

int RtChannelQueueRecord(RtChannelLink *pLink,
                         int nType,
                         unsigned short uPeer,
                         const void *pData,
                         unsigned short uSize,
                         unsigned char bTrackPeer) {
    RtChannelPacket *pPacket;
    int bJoins;
    int nResult;

    if (RtChannelLinkIsValid(pLink) != 1) {
        return RT_CHANNEL_RESULT_INVALID;
    }
    if (g_bChannelsStarted != 1) {
        return RT_CHANNEL_RESULT_NOT_STARTED;
    }
    if (pData == NULL && uSize != 0) {
        return RT_CHANNEL_RESULT_INVALID;
    }
    if (RtChannelIsQueueableRecord(nType) != 1 || uSize > RT_CHANNEL_PACKET_DATA) {
        return RT_CHANNEL_RESULT_INVALID;
    }
    pPacket = &pLink->packet;
    if (pPacket->uCount == 0) {
        bJoins = 1;
    } else if ((nType == kRecordPeerData || nType == kRecordPeerDataGuaranteed) &&
               pPacket->nType == nType && pPacket->uPeer == uPeer &&
               pPacket->uSize + uSize <= RT_CHANNEL_PACKET_DATA) {
        bJoins = 1;
    } else {
        bJoins = 0;
    }
    if (!bJoins) {
        nResult = RtChannelFlushPacket(pLink);
        if (nResult != 0) {
            return nResult;
        }
    }
    pPacket->nType = nType;
    pPacket->uPeer = uPeer;
    memcpy(&pPacket->abData[pPacket->uSize], pData, uSize);
    pPacket->uSize += uSize;
    ++pPacket->uCount;
    if (nType != kRecordPeerData && nType != kRecordPeerDataGuaranteed) {
        (void)RtChannelFlushPacket(pLink); // Yes, the binary discards the result.
    }
    if (bTrackPeer != 1) {
        return 0;
    }
    if (pLink->bPeerNoted != 1 || pLink->uPeer == RT_CHANNEL_ALL_PEERS) {
        pLink->uPeer = uPeer;
    } else if (pLink->uPeer != uPeer && uPeer != RT_CHANNEL_ALL_PEERS) {
        pLink->uPeer = RT_CHANNEL_MIXED_PEERS;
    }
    pLink->bPeerNoted = 1;
    return 0;
}

int RtChannelProcessRecords(RtChannel *pChannel, RtChannelLink *pLink) {
    RtRecordHeader header;
    unsigned char *pbRecord;
    unsigned int uReadable;
    unsigned short uHeaderSize;
    unsigned char bWrapped;
    unsigned int uRecordSize;
    int nState;
    int nResult;

    pbRecord = NULL;
    uReadable = 0;
    uHeaderSize = 0;
    if (RtChannelIsValid(pChannel) != 1 || RtChannelLinkIsValid(pLink) != 1 || pLink->bOpen != 1 ||
        pChannel->uId != pLink->uChannel) {
        return RT_CHANNEL_RESULT_INVALID;
    }
    if (RtIsRingBufferWrapped(&pLink->receive, &bWrapped) != 0) {
        return RT_CHANNEL_RESULT_RING;
    }
    if (bWrapped == 1 && RtCompactRingBufferWith(&pLink->receive, 1, g_pbChannelScratch) != 0) {
        return RT_CHANNEL_RESULT_RING;
    }
    if (RtGetRingBufferReadable(&pLink->receive, &uReadable) != 0) {
        return RT_CHANNEL_RESULT_RING;
    }
    if (uReadable < RT_CHANNEL_RECORD_HEADER_SIZE) {
        return 0;
    }
    nState = pChannel->nState;
    for (;;) {
        if (RtGetRingBufferReadPointer(&pLink->receive, &pbRecord) != 0) {
            return RT_CHANNEL_RESULT_RING;
        }
        nResult = RtChannelReadRecordHeader(pbRecord, &header, &uHeaderSize);
        if (nResult != 0) {
            return nResult;
        }
        uRecordSize = uHeaderSize + header.uSize;
        if (header.uSize != 0 && (header.uType & RT_CHANNEL_RECORD_ENCRYPTED) != 0) {
            uRecordSize += kEncryptionHeaderSize;
        }
        if (uReadable < uRecordSize) {
            return 0;
        }
        pbRecord += uHeaderSize;
        nResult = RtChannelDispatchRecord(pChannel, pLink, header.uType, pbRecord, header.uSize);
        if (nResult != 0) {
            return nResult;
        }
        if (RtConsumeRingBuffer(&pLink->receive, uRecordSize) != 0) {
            return RT_CHANNEL_RESULT_RING;
        }
        if (RtGetRingBufferReadable(&pLink->receive, &uReadable) != 0) {
            return RT_CHANNEL_RESULT_RING;
        }
        if (uReadable < RT_CHANNEL_RECORD_HEADER_SIZE) {
            return 0;
        }
        if (nState != pChannel->nState) {
            return 0;
        }
    }
}

int RtChannelSendBlocking(RtChannelLink *pLink,
                          const void *pData,
                          unsigned int uSize,
                          unsigned int uTimeout) {
    const unsigned char *pbData;
    RtTime start;
    unsigned int uElapsed;
    unsigned short uSent;
    int nResult;

    pbData = pData;
    uElapsed = 0;
    uSent = 0;
    if (g_bChannelsStarted != 1) {
        return RT_CHANNEL_RESULT_NOT_STARTED;
    }
    if (RtChannelLinkIsValid(pLink) != 1 || pbData == NULL) {
        return RT_CHANNEL_RESULT_INVALID;
    }
    if (RtReadTime(&start) != 0) {
        return RT_CHANNEL_RESULT_CLOCK;
    }
    do {
        nResult = RtChannelSendLink(pLink, pbData, uSize, &uSent);
        if (nResult != 0) {
            return nResult;
        }
        pbData += uSent;
        uSize -= uSent;
        if (RtGetMillisecondsSince(&start, &uElapsed) != 0) {
            return RT_CHANNEL_RESULT_CLOCK;
        }
        if (uElapsed > uTimeout) {
            return RT_CHANNEL_RESULT_TIMEOUT;
        }
    } while (uSize != 0);
    return 0;
}

int RtChannelFlushSend(RtChannelLink *pLink) {
    unsigned char *pbData;
    unsigned int uReadable;
    unsigned char bWrapped;
    unsigned short uSent;
    int nResult;

    pbData = NULL;
    uReadable = 0;
    uSent = 0;
    if (g_bChannelsStarted != 1) {
        return RT_CHANNEL_RESULT_NOT_STARTED;
    }
    if (RtChannelLinkIsValid(pLink) != 1) {
        return RT_CHANNEL_RESULT_INVALID;
    }
    if (RtGetRingBufferReadable(&pLink->send, &uReadable) != 0) {
        return RT_CHANNEL_RESULT_RING;
    }
    if (uReadable == 0) {
        return 0;
    }
    if (pLink->nTransport == RT_CHANNEL_TRANSPORT_DATAGRAM) {
        bWrapped = RT_CHANNEL_TRANSPORT_DATAGRAM;
        if (RtIsRingBufferWrapped(&pLink->send, &bWrapped) != 0) {
            return RT_CHANNEL_RESULT_RING;
        }
        // A datagram must leave in one piece, so a wrapped ring is compacted first.
        if (bWrapped == 1) {
            if (RtCompactRingBufferWith(&pLink->send, 1, g_pbChannelScratch) != 0) {
                return RT_CHANNEL_RESULT_RING;
            }
            if (RtGetRingBufferReadable(&pLink->send, &uReadable) != 0) {
                return RT_CHANNEL_RESULT_RING;
            }
        }
    }
    for (;;) {
        if (RtGetRingBufferReadPointer(&pLink->send, &pbData) != 0) {
            return RT_CHANNEL_RESULT_RING;
        }
        uSent = 0;
        nResult = RtChannelSendLink(pLink, pbData, uReadable, &uSent);
        if (nResult != 0) {
            return nResult;
        }
        if (uSent != 0 && RtReadTime(&pLink->lastSend) != 0) {
            return RT_CHANNEL_RESULT_CLOCK;
        }
        if (RtConsumeRingBuffer(&pLink->send, uSent) != 0) {
            return RT_CHANNEL_RESULT_RING;
        }
        if (RtGetRingBufferReadable(&pLink->send, &uReadable) != 0) {
            return RT_CHANNEL_RESULT_RING;
        }
        if (uReadable == 0 || uSent == 0) {
            return 0;
        }
    }
}

int RtChannelReceive(RtChannelLink *pLink) {
    unsigned short uReceived;
    unsigned char *pbWrite;
    unsigned int uRoom;
    char szSender[RT_CHANNEL_ADDRESS_SIZE];
    unsigned short uSenderPort;
    unsigned short uChunk;
    int bFull;
    int nResult;

    pbWrite = NULL;
    uRoom = 0;
    uReceived = 0;
    if (g_bChannelsStarted != 1) {
        return RT_CHANNEL_RESULT_NOT_STARTED;
    }
    if (RtChannelLinkIsValid(pLink) != 1 || pLink->bOpen != 1) {
        return RT_CHANNEL_RESULT_INVALID;
    }
    if (RtCompactRingBufferWith(&pLink->receive, 0, NULL) != 0) {
        return RT_CHANNEL_RESULT_RING;
    }
    if (RtGetRingBufferWritable(&pLink->receive, &uRoom) != 0) {
        return RT_CHANNEL_RESULT_RING;
    }
    if (uRoom == 0) {
        return 0;
    }
    do {
        uReceived = 0;
        bFull = 0;
        if (RtGetRingBufferWritePointer(&pLink->receive, &pbWrite) != 0) {
            return RT_CHANNEL_RESULT_RING;
        }
        if (pLink->nTransport == RT_CHANNEL_TRANSPORT_STREAM) {
            uChunk = kStreamReadSize;
            if (uRoom <= kStreamReadSize) {
                uChunk = uRoom;
            }
            nResult = RtChannelMapSocketResult(
                RtSocketReceive(pLink->pSocket, pbWrite, uChunk, &uReceived));
            if (nResult != 0) {
                return nResult;
            }
            if (uReceived == uChunk) {
                bFull = 1;
            }
        } else if (pLink->nTransport == RT_CHANNEL_TRANSPORT_DATAGRAM) {
            if (uRoom >= RT_CHANNEL_MAX_PACKET) {
                nResult = RtChannelMapSocketResult(RtSocketReceiveFrom(pLink->pSocket,
                                                                       szSender,
                                                                       &uSenderPort,
                                                                       pbWrite,
                                                                       RT_CHANNEL_MAX_PACKET,
                                                                       &uReceived));
                if (nResult != 0) {
                    return nResult;
                }
                if (uReceived != 0) {
                    // Datagrams from another sender are dropped.
                    if (uSenderPort != pLink->uPort || strcmp(szSender, pLink->szAddress) != 0) {
                        uReceived = 0;
                    }
                    bFull = 1;
                } else {
                    goto next;
                }
            }
        } else {
            return RT_CHANNEL_RESULT_UNKNOWN_RECORD;
        }
        if (uReceived != 0) {
            if (RtReadTime(&pLink->lastReceive) != 0) {
                return RT_CHANNEL_RESULT_CLOCK;
            }
            if (RtCommitRingBuffer(&pLink->receive, uReceived) != 0) {
                return RT_CHANNEL_RESULT_RING;
            }
        }
    next:
        if (RtGetRingBufferWritable(&pLink->receive, &uRoom) != 0) {
            return RT_CHANNEL_RESULT_RING;
        }
    } while (uRoom != 0 && bFull == 1);
    return 0;
}

int RtChannelSetState(RtChannel *pChannel, int nState) {
    if (pChannel == NULL) {
        return RT_CHANNEL_RESULT_INVALID;
    }
    if ((unsigned int)nState >= kStateCount) {
        return RT_CHANNEL_RESULT_BAD_STATE;
    }
    pChannel->nState = nState;
    return 0;
}

int RtChannelIsDataRecord(int nType) {
    switch (nType) {
    case kRecordPeerData:
    case 4:
    case kRecordPeerDataGuaranteed:
    case kRecordAcknowledge:
    case kRecordConnectRejected:
    case kRecordConnectAccepted:
    case kRecordPeer:
    case kRecordBroadcast:
        return 1;
    default:
        return 0;
    }
}

int RtChannelMapSocketResult(int nResult) {
    return nResult != 0 ? RT_CHANNEL_RESULT_SOCKET : 0;
}

int RtChannelReserveScratch(unsigned int uSize) {
    if (uSize == 0) {
        return RT_CHANNEL_RESULT_INVALID;
    }
    if (g_uChannelScratchSize >= uSize) {
        return 0;
    }
    if (g_pbChannelScratch != NULL) {
        RtFreeBlock((void **)&g_pbChannelScratch);
    }
    if (RtAllocZeroedBlock((void **)&g_pbChannelScratch, uSize) != 0) {
        return RT_CHANNEL_RESULT_NO_MEMORY;
    }
    g_uChannelScratchSize = uSize;
    return 0;
}

int RtChannelFlushPacket(RtChannelLink *pLink) {
    RtChannelPacket *pPacket;
    RtRecordHeader header;
    unsigned short uHeaderSize;
    unsigned int uEncryption;
    unsigned char *pbWrite;
    unsigned char *pbField;
    unsigned int uSize;
    unsigned int uPadded;
    unsigned short uPayloadSize;
    int bEncrypted;
    int nType;
    int nResult;

    uEncryption = 0;
    pbWrite = NULL;
    if (RtChannelLinkIsValid(pLink) != 1) {
        return RT_CHANNEL_RESULT_INVALID;
    }
    pPacket = &pLink->packet;
    nType = pPacket->nType & kRecordTypeMask;
    uSize = pPacket->uSize;
    if (pPacket->uCount == 0) {
        return 0;
    }
    if (uSize + kRecordReserve > RT_CHANNEL_MAX_PACKET) {
        return RT_CHANNEL_RESULT_INVALID;
    }
    if (RtChannelMakeSendRoom(pLink, uSize + kRecordReserve) != 0) {
        return RT_CHANNEL_RESULT_NO_MEMORY;
    }
    nResult = RtChannelGetEncryption(pLink->uChannel, &uEncryption);
    if (nResult != 0) {
        return nResult;
    }
    bEncrypted = uEncryption != 0;
    if (bEncrypted == 1 && RtChannelGetSecurityState() != RT_CHANNEL_SECURITY_KEYS) {
        return RT_CHANNEL_RESULT_NO_ENCRYPTION;
    }
    if (RtGetRingBufferWritePointer(&pLink->send, &pbWrite) != 0) {
        return RT_CHANNEL_RESULT_RING;
    }
    pbField = pbWrite;
    uPayloadSize = 0;
    if (bEncrypted && uSize != 0) {
        pbField += RT_CHANNEL_RECORD_HEADER_SIZE + kEncryptionHeaderSize;
    } else {
        pbField += RT_CHANNEL_RECORD_HEADER_SIZE;
    }
    if (nType == kRecordPeerDataGuaranteed || nType == kRecordPeerSessionKey) {
        if (RtRecordWriteWord(pbField, pPacket->uPeer) != 0) {
            return RT_CHANNEL_RESULT_FIELD;
        }
        pbField += kPeerFieldSize;
        uPayloadSize = kPeerFieldSize;
    }
    if (uSize != 0) {
        memcpy(pbField, pPacket->abData, uSize);
        if (bEncrypted == 1) {
            if ((uEncryption & RECORD_METHOD_PUBLIC) == RECORD_METHOD_PUBLIC) {
                uPadded = (uSize + kRsaBlockMask) & 0xffc0;
                if (uSize < uPadded) {
                    memset(&pbField[uSize], 0, uPadded - uSize);
                    uSize = uPadded;
                }
            }
            uEncryption = RtEncryptPacket(pLink->uChannel, RT_SECURITY_ALL_PEERS, pbField, uSize);
        }
        pbField += uSize;
        uPayloadSize += uSize;
    }
    header.uType = nType;
    if (bEncrypted) {
        header.uType = nType | RT_CHANNEL_RECORD_ENCRYPTED;
    }
    header.uSize = uPayloadSize;
    nResult = RtChannelWriteRecordHeader(pbWrite, &header, &uHeaderSize);
    if (nResult != 0) {
        return nResult;
    }
    if (bEncrypted == 1 && uSize != 0) {
        RtRecordWriteDword(&pbWrite[RT_CHANNEL_RECORD_HEADER_SIZE], uEncryption);
    }
    if (RtCommitRingBuffer(&pLink->send, pbField - pbWrite) != 0) {
        return RT_CHANNEL_RESULT_RING;
    }
    if (pbField - pbWrite > RT_CHANNEL_MAX_PACKET) {
        return RT_CHANNEL_RESULT_INVALID;
    }
    pPacket->uCount = 0;
    pPacket->uSize = 0;
    return 0;
}

int RtChannelIsEncrypted(unsigned short uChannel, unsigned char *pbEncrypted) {
    unsigned int uEncryption;
    int nResult;

    uEncryption = 0;
    if (pbEncrypted == NULL) {
        return RT_CHANNEL_RESULT_INVALID;
    }
    *pbEncrypted = 0;
    nResult = RtChannelGetEncryption(uChannel, &uEncryption);
    if (nResult != 0) {
        return nResult;
    }
    if (uEncryption != 0) {
        *pbEncrypted = 1;
    }
    return 0;
}
