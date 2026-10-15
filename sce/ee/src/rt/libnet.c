#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include <eekernel.h>
#include <libnet.h>
#include <sifdev.h>

#include "rt/rtmemory.h"

enum {
    kLibnetServerId = 0x80001201,
    // The functions of the module.
    kFunctionCreate = 1,
    kFunctionOpen = 2,
    kFunctionClose = 3,
    kFunctionRecv = 4,
    kFunctionSend = 5,
    kFunctionName2Address = 6,
    kFunctionGetInterfaceList = 8,
    kFunctionInterfaceControl = 9,
    kFunctionAbort = 15,
    kFunctionControl = 19,
    kFunctionRegisterHandler = 30,
    kFunctionUnregisterHandler = 31,
    kFunctionWaitInterfaceUp = 34,
    kFunctionStartAsyncRead = 100,
    kFunctionStartAsyncSend = 101,
    kFunctionCloseAsync = 102,
    // The module moves whole 64-byte lines.
    kRpcAlign = 64,
    kMinBufferSize = 128,
    // The unbind and rebind attempts wait this many iterations, four at a time.
    kRetryDelay = 0xffff,
    kRetryDelayStep = 4,
    // The interface control codes.
    kInterfaceControlGetFlags = 8,
    kInterfaceControlGetAddress = 9,
    kInterfaceFlagUp = 2,
    kMaxInterfaces = 256,
    // sceLibnetWaitGetAddress() uses an interface already up only with this flag.
    kWaitGetAddressUseUp = 2,
    // The results of the interface search.
    kNoInterfaceError = -1,
    kNoInterfaceUp = -2,
    // sceInetName2Address() reads one more argument with this flag.
    kName2AddressOption = 0x80,
    // The asynchronous transfers.
    kAsyncBufferSize = 0x2000,
    kAsyncStartSize = 0x50,
    kAsyncCloseSize = 16,
    kAsyncMarkerSize = 2,
    kRawMaxPacketSize = 4,
    // The results of the ring routines.
    kRingOk = 0,
    kRingFull = 1,
    kRingArgument = 2,
    kRingAllocation = 3,
    kRingDma = 4,
};

// One datagram of the ring buffer of an asynchronous transfer, as the module defines it.
typedef struct LibnetAsyncBlock {
    int nUsed;                      // 1 while the block stores a datagram.
    int nPadding;                   // 1 when the block only fills the end of the ring buffer.
    unsigned int uCapacity;         // Bytes after the header that belong to the block.
    unsigned int uLength;           // Length of the datagram.
    sceInetAddress address;         // Remote address of the datagram.
    int nPort;                      // Remote port of the datagram.
    int nFlags;                     // Flags of the transfer.
    int nResult;                    // Result of the transfer.
    struct LibnetAsyncBlock *pSelf; // The block in the memory of the side that filled it.
    unsigned int auReserved30[3];   // +0x30
    int nValid;                     // 1 when the datagram is complete.
    unsigned char abData[];         // The datagram.
} LibnetAsyncBlock;

// The EE copy of an asynchronous transfer, as the module defines it, 0x40 bytes.
typedef struct {
    int nDataWaitingSize;        // Bytes waiting, or a negative error.
    int nLastReturnedFlag;       // Flags of the last transfer.
    int nMaxPacketSize;          // Largest datagram.
    unsigned int uReserved0C;    // +0x0c
    int nCid;                    // Connection identifier.
    int nInetFlag;               // Flags passed to each transfer.
    unsigned int uBufferSize;    // Size of the buffer.
    unsigned char *pbBuffer;     // The buffer.
    void *pPeer;                 // The module's copy of this record.
    unsigned int uReadOffset;    // The bytes of a TCP stream already read.
    int nThid;                   // The transfer thread of the module.
    unsigned char *pbPeerBuffer; // The module's copy of the buffer.
    int nClose;                  // 1 when the thread must stop.
    int nType;                   // A sceInetType.
    int nRemotePort;             // Remote port.
    unsigned int uWriteOffset;   // Offset of the current block of the ring buffer.
} LibnetAsyncInfo;

// A transfer record and its buffer, as one 64-byte aligned allocation.
typedef struct {
    LibnetAsyncInfo info;
    unsigned char abBuffer[kAsyncBufferSize];
} LibnetAsyncChannel;

struct LibnetAsyncHandle {
    int nType;                    // A sceInetType.
    int nRemotePort;              // Remote port.
    int nCid;                     // Connection identifier.
    int nMaxPacketSize;           // Largest datagram.
    RtAlignedBlock receiveBlock;  // The allocation of pReceive.
    RtAlignedBlock sendBlock;     // The allocation of pSend.
    LibnetAsyncChannel *pReceive; // The receiving transfer.
    LibnetAsyncChannel *pSend;    // The sending transfer.
};

static const char kTextAddressFormat[] = "%u.%u.%u.%u";

// NTSC-U/C: 0x003c27d8
// The version of the IOP module this client was built against. The library never reads it.
#ifdef VIDEO_STANDARD_PAL
const char *g_pszLibnetbVersion = "libnetb version: 1.09.0005";
#else
const char *g_pszLibnetbVersion = "libnetb version: 1.09.0003";
#endif

// The octets of an IPv4 address in sceInetAddress::data, first octet last.
enum {
    kOctet0 = 3,
    kOctet1 = 2,
    kOctet2 = 1,
    kOctet3 = 0,
    kOctetCount = 4,
};

static inline LibnetAsyncBlock *BlockAt(const LibnetAsyncInfo *pInfo, unsigned int uOffset) {
    return (LibnetAsyncBlock *)&pInfo->pbBuffer[uOffset];
}

static inline void RetryDelay(void) {
    for (int i = kRetryDelay; i != -1; i -= kRetryDelayStep) {
        __asm__ volatile("nop\n\tnop\n\tnop\n\tnop\n\tnop");
    }
}

int sceLibnetInitialize(sceSifMClientData *cd, int buffersize, int stacksize, int priority) {
    if ((unsigned int)buffersize < kMinBufferSize) {
        return LIBNET_ERROR_BUFFER_SIZE;
    }
    if (buffersize != ((buffersize + kRpcAlign - 1) & -kRpcAlign)) {
        scePrintf("sceLibnetInitialize() buffersize\n");
        return LIBNET_ERROR_BUFFER_SIZE;
    }
    for (;;) {
        if (sceSifMBindRpcParam(cd, kLibnetServerId, 0, buffersize, stacksize, priority) < 0) {
            return LIBNET_ERROR_BIND;
        }
        if (cd->serve != NULL) {
            return 0;
        }
        RetryDelay();
    }
}

int sceLibnetTerminate(sceSifMClientData *cd) {
    for (;;) {
        const int nResult = sceSifMUnBindRpc(cd, 0);
        if (nResult < 0) {
            return LIBNET_ERROR_BIND;
        }
        if (nResult == 1) {
            return 0;
        }
        RetryDelay();
    }
}

// NTSC-U/C: 0x00305428, PAL: 0x0030f4c0
// Calls a function of the module with the arguments and the reply in the same buffer.
static int
CallRpc(sceSifMClientData *cd, int nFunction, int *pBuffer, int nSendSize, int nReceiveSize) {
    const int nSend = (nSendSize + kRpcAlign - 1) & -kRpcAlign;
    const int nReceive = (nReceiveSize + kRpcAlign - 1) & -kRpcAlign;

    if (cd->buffersize < nSend || cd->buffersize < nReceive) {
        scePrintf(
            "CallRpc() in Libnet send:%d, recv:%d, max:%d\n", nSend, nReceive, cd->buffersize);
        return LIBNET_ERROR_BUFFER_SIZE;
    }
    return sceSifMCallRpc(cd, nFunction, 0, pBuffer, nSend, pBuffer, nReceive, NULL, NULL);
}

// NTSC-U/C: 0x003054a8, PAL: 0x0030f540
// Copies data into the argument buffer at a word index. Null data copies nothing.
static int PutData(sceSifMClientData *cd, int *pBuffer, int nIndex, const void *pData, int nSize) {
    const int nRoom = cd->buffersize - nIndex * (int)sizeof(int);

    if (pData == NULL) {
        return 0;
    }
    if (nSize < 0 || nRoom < nSize) {
        return LIBNET_ERROR_BUFFER_SIZE;
    }
    memcpy(&pBuffer[nIndex], pData, nSize);
    return 0;
}

// NTSC-U/C: 0x00305500, PAL: 0x0030f598
// Copies data out of the reply buffer from a word index. A null destination copies nothing.
static int GetData(sceSifMClientData *cd, void *pData, int *pBuffer, int nIndex, int nSize) {
    const int nRoom = cd->buffersize - nIndex * (int)sizeof(int);

    if (pData == NULL) {
        return 0;
    }
    if (nSize < 0 || nRoom < nSize) {
        return LIBNET_ERROR_BUFFER_SIZE;
    }
    memcpy(pData, &pBuffer[nIndex], nSize);
    return 0;
}

// NTSC-U/C: 0x00305558, PAL: 0x0030f5f0
// Copies a string into the argument buffer at a word index and reports its size, terminator
// included. A null string copies nothing and reports 0.
static int PutString(sceSifMClientData *cd, int *pBuffer, int nIndex, const char *pszText) {
    const int nRoom = cd->buffersize - nIndex * (int)sizeof(int);
    int nSize;

    if (pszText == NULL) {
        return 0;
    }
    nSize = strlen(pszText) + 1;
    if (nSize < 0 || nRoom < nSize) {
        return LIBNET_ERROR_BUFFER_SIZE;
    }
    strcpy((char *)&pBuffer[nIndex], pszText);
    return nSize;
}

int sceInetName2Address(sceSifMClientData *cd,
                        int *buffer,
                        int flags,
                        sceInetAddress *address,
                        const char *name,
                        int timeout,
                        int retries,
                        ...) {
    enum {
        kFlags = 0,
        kTimeout = 1,
        kRetries = 2,
        kOption = 3,
        kNameLength = 4,
        kName = 5,
        kHeaderSize = 5 * sizeof(int),
        kAddress = 1,
    };
    int nLength;

    buffer[kTimeout] = timeout;
    buffer[kRetries] = retries;
    buffer[kOption] = 0;
    buffer[kFlags] = flags;
    if ((flags & kName2AddressOption) != 0) {
        va_list args;
        va_start(args, retries);
        buffer[kOption] = va_arg(args, int);
        va_end(args);
    }
    nLength = PutString(cd, buffer, kName, name);
    if (nLength < 0) {
        return nLength;
    }
    buffer[kNameLength] = nLength;
    if (CallRpc(cd, kFunctionName2Address, buffer, nLength + kHeaderSize, kHeaderSize) < 0) {
        return LIBNET_ERROR_CALL;
    }
    nLength = GetData(cd, address, buffer, kAddress, sizeof(*address));
    if (nLength < 0) {
        return nLength;
    }
    return buffer[0];
}

int sceInetCreate(sceSifMClientData *cd, int *buffer, sceInetParam *param) {
    enum { kParam = 1 };
    int nResult = PutData(cd, buffer, kParam, param, sizeof(*param));

    if (nResult < 0) {
        return nResult;
    }
    if (CallRpc(cd, kFunctionCreate, buffer, sizeof(int) + sizeof(*param), sizeof(int)) < 0) {
        return LIBNET_ERROR_CALL;
    }
    nResult = buffer[0];
    return (nResult != 0) ? nResult : LIBNET_ERROR_BIND;
}

int sceInetOpen(sceSifMClientData *cd, int *buffer, int cid, int timeout) {
    buffer[0] = cid;
    buffer[1] = timeout;
    if (CallRpc(cd, kFunctionOpen, buffer, 2 * sizeof(int), sizeof(int)) < 0) {
        return LIBNET_ERROR_CALL;
    }
    return buffer[0];
}

#ifdef VIDEO_STANDARD_PAL
int sceInetClose(sceSifMClientData *cd, int *buffer, int cid, int timeout) {
    buffer[0] = cid;
    buffer[1] = timeout;
    if (CallRpc(cd, kFunctionClose, buffer, 2 * sizeof(int), sizeof(int)) < 0) {
        return LIBNET_ERROR_CALL;
    }
    return buffer[0];
}

int sceInetRecv(
    sceSifMClientData *cd, int *buffer, int cid, void *data, int size, int *flags, int timeout) {
    enum { kId = 0, kFlags = 1, kSize = 2, kTimeout = 3, kData = 2, kHeaderSize = 4 * sizeof(int) };
    int nResult;

    buffer[kId] = cid;
    buffer[kFlags] = flags != NULL ? *flags : 0;
    buffer[kTimeout] = timeout;
    buffer[kSize] = size;
    if (CallRpc(cd, kFunctionRecv, buffer, kHeaderSize, size + 2 * (int)sizeof(int)) < 0) {
        return LIBNET_ERROR_CALL;
    }
    if (buffer[kId] < 0) {
        return buffer[kId];
    }
    if (flags != NULL) {
        *flags = buffer[kFlags];
    }
    nResult = GetData(cd, data, buffer, kData, buffer[kId]);
    if (nResult < 0) {
        return nResult;
    }
    return buffer[kId];
}

int sceInetSend(sceSifMClientData *cd,
                int *buffer,
                int cid,
                const void *data,
                int size,
                int *flags,
                int timeout) {
    enum { kId = 0, kFlags = 1, kSize = 2, kTimeout = 3, kData = 4, kHeaderSize = 4 * sizeof(int) };
    int nResult;

    buffer[kId] = cid;
    buffer[kFlags] = flags != NULL ? *flags : 0;
    buffer[kTimeout] = timeout;
    buffer[kSize] = size;
    nResult = PutData(cd, buffer, kData, data, size);
    if (nResult < 0) {
        return nResult;
    }
    if (CallRpc(cd, kFunctionSend, buffer, size + kHeaderSize, 2 * sizeof(int)) < 0) {
        return LIBNET_ERROR_CALL;
    }
    if (flags != NULL) {
        *flags = buffer[kFlags];
    }
    return buffer[kId];
}

int sceInetAbort(sceSifMClientData *cd, int *buffer, int cid, int flags) {
    buffer[0] = cid;
    buffer[1] = flags;
    if (CallRpc(cd, kFunctionAbort, buffer, 2 * sizeof(int), sizeof(int)) < 0) {
        return LIBNET_ERROR_CALL;
    }
    return buffer[0];
}
#endif

// The body of sceInetControl() and sceInetInterfaceControl().
static inline int ControlCall(sceSifMClientData *cd,
                              int nFunction,
                              int *pBuffer,
                              int nId,
                              int nCode,
                              void *pData,
                              int nSize) {
    enum { kId = 0, kCode = 1, kSize = 2, kData = 3, kHeaderSize = 3 * sizeof(int) };
    int nResult;

    pBuffer[kId] = nId;
    pBuffer[kCode] = nCode;
    pBuffer[kSize] = nSize;
    nResult = PutData(cd, pBuffer, kData, pData, nSize);
    if (nResult < 0) {
        return nResult;
    }
    if (CallRpc(cd, nFunction, pBuffer, nSize + kHeaderSize, nSize + kHeaderSize) < 0) {
        return LIBNET_ERROR_CALL;
    }
    nResult = GetData(cd, pData, pBuffer, kData, nSize);
    if (nResult < 0) {
        return nResult;
    }
    return pBuffer[0];
}

int sceInetControl(sceSifMClientData *cd, int *buffer, int cid, int code, void *data, int size) {
    return ControlCall(cd, kFunctionControl, buffer, cid, code, data, size);
}

int sceInetGetInterfaceList(sceSifMClientData *cd, int *buffer, int *interfaceIds, int count) {
    int nResult;

    buffer[0] = count;
    if (CallRpc(cd, kFunctionGetInterfaceList, buffer, sizeof(int), (count + 1) * sizeof(int)) <
        0) {
        return LIBNET_ERROR_CALL;
    }
    nResult = buffer[0];
    if (nResult < 0) {
        return nResult;
    }
    nResult = GetData(cd, interfaceIds, buffer, 1, nResult * sizeof(int));
    if (nResult < 0) {
        return nResult;
    }
    return buffer[0];
}

int sceInetInterfaceControl(
    sceSifMClientData *cd, int *buffer, int interfaceId, int code, void *data, int size) {
    return ControlCall(cd, kFunctionInterfaceControl, buffer, interfaceId, code, data, size);
}

// The body of the calls that take no arguments and reply with a result.
static inline int SimpleCall(sceSifMClientData *cd, int nFunction, int *pBuffer) {
    if (CallRpc(cd, nFunction, pBuffer, 0, sizeof(int)) < 0) {
        return LIBNET_ERROR_CALL;
    }
    return pBuffer[0];
}

int sceLibnetRegisterHandler(sceSifMClientData *cd, int *buffer) {
    return SimpleCall(cd, kFunctionRegisterHandler, buffer);
}

int sceLibnetUnregisterHandler(sceSifMClientData *cd, int *buffer) {
    return SimpleCall(cd, kFunctionUnregisterHandler, buffer);
}

// NTSC-U/C: 0x00305ba8, PAL: 0x0030fe40
// Calls a function that replies with a result and a list of interface identifiers.
static int InterfaceListCall(
    sceSifMClientData *cd, int *pBuffer, int *pInterfaceIds, int nCount, int nFunction) {
    if (CallRpc(cd, nFunction, pBuffer, 0, (nCount + 1) * sizeof(int)) < 0) {
        return LIBNET_ERROR_CALL;
    }
    for (int i = 0; i < nCount; ++i) {
        pInterfaceIds[i] = pBuffer[i + 1];
    }
    return pBuffer[0];
}

// NTSC-U/C: 0x00305c30, PAL: 0x0030fec8
// Waits until interfaces are up and lists them.
static int WaitInterfaceUp(sceSifMClientData *cd, int *pBuffer, int *pInterfaceIds, int nCount) {
    return InterfaceListCall(cd, pBuffer, pInterfaceIds, nCount, kFunctionWaitInterfaceUp);
}

// NTSC-U/C: 0x00305c48, PAL: 0x0030fee0
// Finds an interface that is up.
static int FindInterfaceUp(sceSifMClientData *cd, int *pBuffer, int *pInterfaceId) {
    int anInterfaceIds[kMaxInterfaces];
    int nFlags;
    int nCount;
    int i;

    nCount = sceInetGetInterfaceList(cd, pBuffer, anInterfaceIds, kMaxInterfaces);
    if (nCount < 0) {
        return kNoInterfaceError;
    }
    if (nCount == 0) {
        return kNoInterfaceUp;
    }
    for (i = 0; i < nCount; ++i) {
        if (sceInetInterfaceControl(cd,
                                    pBuffer,
                                    anInterfaceIds[i],
                                    kInterfaceControlGetFlags,
                                    &nFlags,
                                    sizeof(nFlags)) < 0) {
            return kNoInterfaceError;
        }
        if ((nFlags & kInterfaceFlagUp) != 0) {
            break;
        }
    }
    if (i == nCount) {
        return kNoInterfaceUp;
    }
    *pInterfaceId = anInterfaceIds[i];
    return 0;
}

// NTSC-U/C: 0x00305d28, PAL: 0x0030ffc0
static int GetInterfaceAddress(sceSifMClientData *cd,
                               int *pBuffer,
                               int nInterfaceId,
                               sceInetAddress *pAddress) {
    return sceInetInterfaceControl(
        cd, pBuffer, nInterfaceId, kInterfaceControlGetAddress, pAddress, sizeof(*pAddress));
}

// NTSC-U/C: 0x00305d48, PAL: 0x0030ffe0
// Reports the address of an interface already up, waiting for one when none is.
static int GetAddressOfInterfaceUp(
    sceSifMClientData *cd, int *pBuffer, int *pInterfaceIds, int nCount, sceInetAddress *pAddress) {
    int nResult;

    if (FindInterfaceUp(cd, pBuffer, pInterfaceIds) < 0) {
        nResult = WaitInterfaceUp(cd, pBuffer, pInterfaceIds, nCount);
        if (nResult < 0) {
            return nResult;
        }
    }
    nResult = GetInterfaceAddress(cd, pBuffer, pInterfaceIds[0], pAddress);
    return (nResult < 0) ? nResult : 0;
}

// NTSC-U/C: 0x00305de0, PAL: 0x00310078
// Waits for interfaces to come up and reports the address of the first.
static int WaitGetAddress(
    sceSifMClientData *cd, int *pBuffer, int *pInterfaceIds, int nCount, sceInetAddress *pAddress) {
    int nResult = WaitInterfaceUp(cd, pBuffer, pInterfaceIds, nCount);

    if (nResult < 0) {
        return nResult;
    }
    nResult = GetInterfaceAddress(cd, pBuffer, pInterfaceIds[0], pAddress);
    return (nResult < 0) ? nResult : 0;
}

int sceLibnetWaitGetAddress(sceSifMClientData *cd,
                            int *buffer,
                            int *interfaceIds,
                            int count,
                            sceInetAddress *address,
                            int flags) {
    if ((flags & kWaitGetAddressUseUp) != 0) {
        return GetAddressOfInterfaceUp(cd, buffer, interfaceIds, count, address);
    }
    return WaitGetAddress(cd, buffer, interfaceIds, count, address);
}

#pragma mark - Ring buffers

// NTSC-U/C: 0x00306f88, PAL: 0x00311210
// Turns a buffer into a ring buffer of one empty block.
static int InitRing(LibnetAsyncBlock *pBlock, unsigned int uSize) {
    if (pBlock == NULL || uSize < 2 * sizeof(LibnetAsyncBlock) ||
        uSize < sizeof(LibnetAsyncBlock)) {
        return kRingArgument;
    }
    pBlock->uLength = uSize - sizeof(LibnetAsyncBlock);
    pBlock->nValid = 0;
    pBlock->uCapacity = uSize - sizeof(LibnetAsyncBlock);
    pBlock->nPadding = 0;
    pBlock->nUsed = 0;
    return kRingOk;
}

// NTSC-U/C: 0x00306fc8, PAL: 0x00311250
// Measures the empty blocks from pBlock, uSize bytes before the end of the ring buffer.
static int MeasureFree(LibnetAsyncBlock *pBlock, unsigned int uSize, unsigned int *puFree) {
    unsigned int uFree = 0;

    if (puFree == NULL) {
        return kRingArgument;
    }
    *puFree = 0;
    if (pBlock == NULL) {
        return kRingArgument;
    }
    if (uSize >= sizeof(LibnetAsyncBlock)) {
        while (pBlock->nUsed == 0 && pBlock->nValid == 0) {
            const unsigned int uStep = pBlock->uCapacity + sizeof(LibnetAsyncBlock);
            uSize -= uStep;
            pBlock = (LibnetAsyncBlock *)&pBlock->abData[pBlock->uCapacity];
            uFree += uStep;
            if (uSize < sizeof(LibnetAsyncBlock)) {
                break;
            }
        }
    }
    *puFree = uFree;
    return kRingOk;
}

// NTSC-U/C: 0x003066c8, PAL: 0x00310950
// Copies bytes to IOP memory and waits for the copy.
static int CopyToIop(void *pSource, void *pDestination, unsigned int uSize) {
    sceSifDmaData dma;
    unsigned int uId;

    if (pSource == NULL || pDestination == NULL || uSize == 0) {
        return kRingArgument;
    }
    FlushCache(0);
    dma.data = (unsigned int)pSource;
    dma.addr = (unsigned int)pDestination;
    dma.size = uSize;
    dma.mode = 0;
    uId = sceSifSetDma(&dma, 1);
    if (uId == 0) {
        return kRingDma;
    }
    while (sceSifDmaStat(uId) >= 0) {
    }
    return kRingOk;
}

// NTSC-U/C: 0x00307030, PAL: 0x003112b8
// Makes the block at the write offset large enough for a datagram, splitting off the rest of the
// free space and skipping the end of the ring buffer when it is too small.
static int ReserveBlock(LibnetAsyncInfo *pInfo,
                        unsigned int uSize,
                        unsigned int *puReserved,
                        unsigned char *pbReady) {
    unsigned int uFree = 0;
    unsigned int uNeed;
    unsigned int uRest;
    LibnetAsyncBlock *pBlock;
    int nResult;

    if (pInfo == NULL || puReserved == NULL || pbReady == NULL) {
        return kRingArgument;
    }
    uNeed = uSize + sizeof(LibnetAsyncBlock);
    if ((uNeed % sizeof(LibnetAsyncBlock)) != 0) {
        uNeed = uNeed - (uNeed % sizeof(LibnetAsyncBlock)) + sizeof(LibnetAsyncBlock);
    }
    *puReserved = uNeed;
    if (pInfo->uWriteOffset >= pInfo->uBufferSize) {
        return kRingFull;
    }
    pBlock = BlockAt(pInfo, pInfo->uWriteOffset);
    if (pBlock->nUsed != 0 || pBlock->nValid != 0) {
        *pbReady = 0;
        return kRingOk;
    }
    uRest = pInfo->uBufferSize - pInfo->uWriteOffset;
    nResult = MeasureFree(pBlock, uRest, &uFree);
    if (nResult != kRingOk) {
        return nResult;
    }
    if (uFree < uNeed) {
        if (uRest - uFree < sizeof(LibnetAsyncBlock) && pBlock != BlockAt(pInfo, 0)) {
            // The free space reaches the end of the ring buffer. Pad it out and start over.
            pBlock->uLength = uRest - sizeof(LibnetAsyncBlock);
            pBlock->nValid = 1;
            pBlock->nUsed = 1;
            pBlock->nPadding = 1;
            pBlock->uCapacity = uRest - sizeof(LibnetAsyncBlock);
            pBlock->pSelf = pBlock;
            nResult = CopyToIop(
                pBlock, &pInfo->pbPeerBuffer[pInfo->uWriteOffset], sizeof(LibnetAsyncBlock));
            if (nResult != kRingOk) {
                return nResult;
            }
            pInfo->uWriteOffset = 0;
            return ReserveBlock(pInfo, uSize, puReserved, pbReady);
        }
        *pbReady = 0;
        return kRingOk;
    }
    if (uNeed < uFree) {
        LibnetAsyncBlock *pNext = BlockAt(pInfo, pInfo->uWriteOffset + uNeed);
        pNext->nUsed = 0;
        pNext->nPadding = 0;
        pNext->nValid = 0;
        pNext->uLength = uFree - uNeed - sizeof(LibnetAsyncBlock);
        pNext->uCapacity = uFree - uNeed - sizeof(LibnetAsyncBlock);
        nResult = CopyToIop(
            pNext, &pInfo->pbPeerBuffer[pInfo->uWriteOffset + uNeed], sizeof(LibnetAsyncBlock));
        if (nResult != kRingOk) {
            return nResult;
        }
    }
    pBlock = BlockAt(pInfo, pInfo->uWriteOffset);
    pBlock->uLength = uNeed - sizeof(LibnetAsyncBlock);
    pBlock->nValid = 0;
    pBlock->nUsed = 0;
    pBlock->nPadding = 0;
    pBlock->uCapacity = uNeed - sizeof(LibnetAsyncBlock);
    nResult =
        CopyToIop(pBlock, &pInfo->pbPeerBuffer[pInfo->uWriteOffset], sizeof(LibnetAsyncBlock));
    if (nResult != kRingOk) {
        return nResult;
    }
    *pbReady = 1;
    return kRingOk;
}

// NTSC-U/C: 0x00307258, PAL: 0x003114e0
// Finds the next complete block from the write offset, skipping padding blocks.
static int
FindFilledBlock(LibnetAsyncInfo *pInfo, LibnetAsyncBlock **ppBlock, unsigned char *pbFound) {
    unsigned int uStart;
    unsigned int uOffset;
    LibnetAsyncBlock *pBlock;

    if (pInfo == NULL || ppBlock == NULL) {
        return kRingArgument;
    }
    *ppBlock = NULL;
    if (pbFound == NULL) {
        return kRingArgument;
    }
    *pbFound = 0;
    uStart = pInfo->uWriteOffset;
    uOffset = uStart;
    pBlock = BlockAt(pInfo, uOffset);
    while (pBlock->nPadding == 1) {
        uOffset += pBlock->uCapacity + sizeof(LibnetAsyncBlock);
        if (uOffset >= pInfo->uBufferSize) {
            uOffset = 0;
        }
        if (uOffset == uStart) {
            *ppBlock = NULL;
            *pbFound = 0;
            return kRingOk;
        }
        pBlock = BlockAt(pInfo, uOffset);
    }
    if (pBlock->nUsed == 1 && pBlock->nValid == 1) {
        *ppBlock = pBlock;
        *pbFound = 1;
    } else {
        *ppBlock = NULL;
        *pbFound = 0;
    }
    return kRingOk;
}

// NTSC-U/C: 0x00306ed0, PAL: 0x00311158
// Reports whether data is waiting. For a datagram connection a sending transfer tests the block
// at the write offset, and a receiving one searches for a complete block.
static int Poll(LibnetAsyncInfo *pInfo,
                unsigned char bBlocks,
                unsigned char bSending,
                unsigned char *pbAvailable) {
    if (pbAvailable == NULL) {
        return kRingArgument;
    }
    *pbAvailable = 0;
    if (pInfo == NULL) {
        return kRingArgument;
    }
    if (bBlocks == 1) {
        LibnetAsyncBlock *pBlock = NULL;
        if (bSending == 0) {
            unsigned char bFound = 0;
            const int nResult = FindFilledBlock(pInfo, &pBlock, &bFound);
            if (nResult != kRingOk) {
                return nResult;
            }
            *pbAvailable = bFound;
            return kRingOk;
        }
        pBlock = BlockAt(pInfo, pInfo->uWriteOffset);
        if (pBlock->nUsed == 1 && pBlock->nValid == 1) {
            *pbAvailable = 1;
        } else {
            *pbAvailable = 0;
        }
    } else if (pInfo->nDataWaitingSize > 0) {
        *pbAvailable = 1;
    } else {
        *pbAvailable = 0;
    }
    return kRingOk;
}

// NTSC-U/C: 0x00306760, PAL: 0x003109e8
// Takes the next datagram of the receiving transfer and returns its block to the module.
static int Dequeue(LibnetAsyncHandle *pHandle,
                   void *pData,
                   int nSize,
                   int *pnFlags,
                   sceInetAddress *pAddress,
                   int *pnPort,
                   int *pnReceived,
                   int *pnStatus) {
    LibnetAsyncInfo *pInfo;
    LibnetAsyncBlock *pBlock;
    unsigned char bAvailable = 0;
    int bPadding;
    int nResult;

    if (pnReceived == NULL) {
        return kRingArgument;
    }
    *pnReceived = 0;
    if (pData == NULL || nSize <= 0 || pnFlags == NULL) {
        return kRingArgument;
    }
    *pnFlags = 0;
    if (pAddress == NULL || pnPort == NULL || pnStatus == NULL) {
        return kRingArgument;
    }
    *pnStatus = 0;
    if (pHandle == NULL) {
        return kRingArgument;
    }
    if (pHandle->pReceive == NULL) {
        return kRingArgument;
    }
    pInfo = &pHandle->pReceive->info;
    FlushCache(0);
    nResult = Poll(pInfo, 1, 0, &bAvailable);
    if (nResult != kRingOk) {
        return nResult;
    }
    if (bAvailable == 0) {
        *pnReceived = 0;
        *pnFlags = 0;
        *pnStatus = 0;
        return kRingOk;
    }
    pBlock = BlockAt(pInfo, pInfo->uWriteOffset);
    bPadding = (pBlock->nPadding != 0);
    if (!bPadding) {
        const unsigned int uTaken =
            (pBlock->uLength < (unsigned int)nSize) ? pBlock->uLength : (unsigned int)nSize;
        if (uTaken != 0) {
            memcpy(pData, pBlock->abData, uTaken);
        }
        *pAddress = pBlock->address;
        *pnFlags = pBlock->nFlags;
        *pnPort = pBlock->nPort;
        *pnReceived = uTaken;
        *pnStatus = pBlock->nResult;
    }
    pBlock->nUsed = 0;
    pBlock->uLength = pBlock->uCapacity;
    pBlock->nValid = 0;
    pBlock->nPadding = 0;
    nResult = CopyToIop(pBlock, pBlock->pSelf, sizeof(LibnetAsyncBlock));
    if (nResult != kRingOk) {
        return nResult;
    }
    pInfo->uWriteOffset += pBlock->uCapacity + sizeof(LibnetAsyncBlock);
    if (pInfo->uWriteOffset >= pInfo->uBufferSize) {
        pInfo->uWriteOffset = 0;
    }
    if (bPadding == 1) {
        return Dequeue(pHandle, pData, nSize, pnFlags, pAddress, pnPort, pnReceived, pnStatus);
    }
    return kRingOk;
}

// NTSC-U/C: 0x00306988, PAL: 0x00310c10
// Takes a two-byte marker datagram, setting *pbTaken when one was taken.
static int DequeueMarker(LibnetAsyncHandle *pHandle,
                         void *pMarker,
                         sceInetAddress *pAddress,
                         int *pbTaken,
                         int *pnStatus) {
    int nFlags = 0;
    int nPort;
    int nReceived = 0;
    int nResult;

    if (pbTaken == NULL) {
        return kRingArgument;
    }
    *pbTaken = 0;
    if (pMarker == NULL || pAddress == NULL || pnStatus == NULL) {
        return kRingArgument;
    }
    *pnStatus = 0;
    if (pHandle == NULL) {
        return kRingArgument;
    }
    nResult = Dequeue(
        pHandle, pMarker, kAsyncMarkerSize, &nFlags, pAddress, &nPort, &nReceived, pnStatus);
    if (nReceived == kAsyncMarkerSize) {
        *pbTaken = 1;
    }
    return nResult;
}

// NTSC-U/C: 0x00306a18, PAL: 0x00310ca0
// Copies a datagram into a reserved block of the sending transfer and hands it to the module.
static int Enqueue(LibnetAsyncHandle *pHandle,
                   const void *pData,
                   int nSize,
                   int nFlags,
                   const sceInetAddress *pAddress,
                   int nPort,
                   int *pnSent,
                   int *pnStatus) {
    LibnetAsyncInfo *pInfo;
    LibnetAsyncBlock *pBlock;
    unsigned int uReserved;
    unsigned char bReady = 0;
    int nResult;

    if (pnSent == NULL) {
        return kRingArgument;
    }
    *pnSent = 0;
    if (pData == NULL || nSize <= 0 || pAddress == NULL || pnStatus == NULL || pHandle == NULL) {
        return kRingArgument;
    }
    if (pHandle->pSend == NULL) {
        return kRingArgument;
    }
    pInfo = &pHandle->pSend->info;
    if (pInfo->nMaxPacketSize < nSize ||
        (unsigned int)nSize >= pInfo->uBufferSize - sizeof(LibnetAsyncBlock)) {
        return kRingArgument;
    }
    FlushCache(0);
    if (pInfo->nDataWaitingSize < 0) {
        *pnStatus = pInfo->nDataWaitingSize;
        return kRingFull;
    }
    nResult = ReserveBlock(pInfo, nSize, &uReserved, &bReady);
    if (nResult != kRingOk) {
        return nResult;
    }
    if (bReady == 1) {
        const unsigned int uOffset = pInfo->uWriteOffset;
        pBlock = BlockAt(pInfo, uOffset);
        pBlock->nUsed = bReady;
        pBlock->nPadding = 0;
        pBlock->uLength = nSize;
        pBlock->uCapacity = uReserved - sizeof(LibnetAsyncBlock);
        pBlock->address = *pAddress;
        pBlock->nPort = nPort;
        pBlock->nValid = bReady;
        pBlock->nFlags = nFlags;
        pBlock->pSelf = pBlock;
        memcpy(pBlock->abData, pData, nSize);
        nResult = CopyToIop(pBlock->abData,
                            &pInfo->pbPeerBuffer[uOffset + sizeof(LibnetAsyncBlock)],
                            uReserved - sizeof(LibnetAsyncBlock));
        if (nResult != kRingOk) {
            return nResult;
        }
        nResult = CopyToIop(pBlock, &pInfo->pbPeerBuffer[uOffset], sizeof(LibnetAsyncBlock));
        if (nResult != kRingOk) {
            return nResult;
        }
        pInfo->uWriteOffset += uReserved;
        if (pInfo->uWriteOffset >= pInfo->uBufferSize) {
            pInfo->uWriteOffset = 0;
        }
        *pnSent = nSize;
    } else {
        *pnSent = 0;
    }
    *pnStatus = *pnSent;
    return kRingOk;
}

// NTSC-U/C: 0x00306c28, PAL: 0x00310eb0
// Queues a two-byte marker datagram, setting *pbQueued when it was queued.
static int EnqueueMarker(LibnetAsyncHandle *pHandle,
                         unsigned short uMarker,
                         const sceInetAddress *pAddress,
                         int *pbQueued,
                         int *pnStatus) {
    unsigned short uLocalMarker = uMarker;
    int nSent = 0;
    int nResult;

    if (pbQueued == NULL) {
        return kRingArgument;
    }
    *pbQueued = 0;
    if (pAddress == NULL || pnStatus == NULL || pHandle == NULL) {
        return kRingArgument;
    }
    nResult = Enqueue(pHandle, &uLocalMarker, kAsyncMarkerSize, 0, pAddress, 0, &nSent, pnStatus);
    if (nSent == kAsyncMarkerSize) {
        *pbQueued = 1;
    }
    return nResult;
}

// NTSC-U/C: 0x00306ca8, PAL: 0x00310f30
static int AllocBuffers(LibnetAsyncHandle *pHandle) {
    if (pHandle == NULL) {
        return kRingArgument;
    }
    if (pHandle->nMaxPacketSize + (int)sizeof(LibnetAsyncBlock) > kAsyncBufferSize) {
        return kRingFull;
    }
    if (RtAllocAlignedZeroedBlock(&pHandle->receiveBlock, sizeof(LibnetAsyncChannel), kRpcAlign) !=
        0) {
        return kRingAllocation;
    }
    pHandle->pReceive = pHandle->receiveBlock.pAligned;
    if (RtAllocAlignedZeroedBlock(&pHandle->sendBlock, sizeof(LibnetAsyncChannel), kRpcAlign) !=
        0) {
        return kRingAllocation;
    }
    pHandle->pSend = pHandle->sendBlock.pAligned;
    return kRingOk;
}

// NTSC-U/C: 0x00306d28, PAL: 0x00310fb0
static int FreeBuffers(LibnetAsyncHandle *pHandle) {
    if (pHandle == NULL) {
        return kRingArgument;
    }
    if (RtFreeAlignedBlock(&pHandle->receiveBlock) != 0) {
        return kRingFull;
    }
    return RtFreeAlignedBlock(&pHandle->sendBlock) != 0;
}

// The body of StartRead() and StartSend().
static inline int StartTransfer(sceSifMClientData *cd, int nFunction, LibnetAsyncInfo *pInfo) {
    int nResult;

    pInfo->nDataWaitingSize = 0;
    pInfo->pPeer = pInfo;
    pInfo->uReadOffset = 0;
    pInfo->nLastReturnedFlag = 0;
    if (sceSifMCallRpc(
            cd, nFunction, 0, pInfo, kAsyncStartSize, pInfo, kAsyncStartSize, NULL, NULL) < 0) {
        return kRingFull;
    }
    if (pInfo->nType != sceINETT_DGRAM && pInfo->nType != sceINETT_RAW) {
        return kRingOk;
    }
    if (pInfo->uBufferSize < pInfo->nMaxPacketSize + sizeof(LibnetAsyncBlock)) {
        return kRingFull;
    }
    nResult = InitRing((LibnetAsyncBlock *)pInfo->pbBuffer, pInfo->uBufferSize);
    if (nResult != kRingOk) {
        return nResult;
    }
    return kRingOk;
}

// NTSC-U/C: 0x00306d80, PAL: 0x00311008
static int StartRead(sceSifMClientData *cd, LibnetAsyncInfo *pInfo) {
    return StartTransfer(cd, kFunctionStartAsyncRead, pInfo);
}

// NTSC-U/C: 0x00306e28, PAL: 0x003110b0
static int StartSend(sceSifMClientData *cd, LibnetAsyncInfo *pInfo) {
    return StartTransfer(cd, kFunctionStartAsyncSend, pInfo);
}

#pragma mark - Connections

int LibnetAsyncCreate(sceSifMClientData *cd,
                      int *buffer,
                      LibnetAsyncHandle **handle,
                      LibnetAsyncParam *param) {
    LibnetAsyncHandle *pHandle;
    int nCid;

    if (handle == NULL || param == NULL) {
        return LIBNET_ERROR_ARGUMENT;
    }
    *handle = NULL;
    pHandle = NULL;
    if (RtAllocZeroedBlock((void **)&pHandle, sizeof(*pHandle)) != 0) {
        return LIBNET_ERROR_ASYNC;
    }
    if (pHandle == NULL) {
        return 0; // Yes, the binary reports success here.
    }
    pHandle->nType = param->inet.type;
    pHandle->nRemotePort = param->inet.remotePort;
    if (pHandle->nType == sceINETT_DGRAM) {
        pHandle->nMaxPacketSize = param->maxPacketSize;
    } else if (pHandle->nType != sceINETT_RAW) {
        pHandle->nMaxPacketSize = 0;
    } else {
        pHandle->nMaxPacketSize = kRawMaxPacketSize;
    }
    if (AllocBuffers(pHandle) != kRingOk) {
        return LIBNET_ERROR_ASYNC;
    }
    nCid = sceInetCreate(cd, buffer, &param->inet);
    if (nCid < 0) {
        FreeBuffers(pHandle);
        RtFreeBlock((void **)&pHandle);
        return nCid;
    }
    pHandle->nCid = nCid;
    *handle = pHandle;
    return 0;
}

int LibnetAsyncOpen(sceSifMClientData *cd, int *buffer, LibnetAsyncHandle *handle, int timeout) {
    LibnetAsyncChannel *pChannel;

    if (handle == NULL) {
        return LIBNET_ERROR_ARGUMENT;
    }
    pChannel = handle->pReceive;
    pChannel->info.nType = handle->nType;
    pChannel->info.pbBuffer = pChannel->abBuffer;
    pChannel->info.nRemotePort = handle->nRemotePort;
    pChannel->info.nCid = handle->nCid;
    pChannel->info.nInetFlag = 0;
    pChannel->info.nMaxPacketSize = handle->nMaxPacketSize;
    pChannel->info.uBufferSize = kAsyncBufferSize;
    if (StartRead(cd, &pChannel->info) != kRingOk) {
        return LIBNET_ERROR_ASYNC;
    }
    pChannel = handle->pSend;
    pChannel->info.nType = handle->nType;
    pChannel->info.pbBuffer = pChannel->abBuffer;
    pChannel->info.nRemotePort = handle->nRemotePort;
    pChannel->info.nCid = handle->nCid;
    pChannel->info.uBufferSize = kAsyncBufferSize;
    pChannel->info.nMaxPacketSize = handle->nMaxPacketSize;
    pChannel->info.nInetFlag = 0;
    if (StartSend(cd, &pChannel->info) != kRingOk) {
        return LIBNET_ERROR_ASYNC;
    }
    return sceInetOpen(cd, buffer, handle->nCid, timeout);
}

int LibnetAsyncClose(sceSifMClientData *cd, int *buffer, LibnetAsyncHandle *handle, int timeout) {
    LibnetAsyncHandle *pHandle = handle;
    int nResult;

    if (handle == NULL) {
        return LIBNET_ERROR_ARGUMENT;
    }
    buffer[0] = handle->nCid;
    buffer[1] = timeout;
    nResult = sceSifMCallRpc(
        cd, kFunctionCloseAsync, 0, buffer, kAsyncCloseSize, buffer, kAsyncCloseSize, NULL, NULL);
    if (FreeBuffers(pHandle) != kRingOk || RtFreeBlock((void **)&pHandle) != 0) {
        return LIBNET_ERROR_ASYNC;
    }
    return (nResult < 0) ? LIBNET_ERROR_ASYNC : 0;
}

int LibnetAsyncReadStream(sceSifMClientData *cd,
                          int *buffer,
                          LibnetAsyncHandle *handle,
                          void *data,
                          int size,
                          int *flags) {
    LibnetAsyncInfo *pInfo;
    sceSifDmaData dma;
    unsigned int uId;
    int nTaken;

    (void)cd;
    (void)buffer;
    if (handle == NULL) {
        return LIBNET_ERROR_ARGUMENT;
    }
    pInfo = &handle->pReceive->info;
    FlushCache(0);
#ifdef VIDEO_STANDARD_PAL
    // The library of the European release reports the flags of every read.
    *flags = pInfo->nLastReturnedFlag;
    if (pInfo->nDataWaitingSize <= 0) {
        return pInfo->nDataWaitingSize;
    }
#else
    if (pInfo->nDataWaitingSize == 0) {
        *flags = pInfo->nLastReturnedFlag;
        return 0;
    }
    if (pInfo->nDataWaitingSize < 0) {
        return pInfo->nDataWaitingSize;
    }
#endif
    nTaken = (pInfo->nDataWaitingSize < size) ? pInfo->nDataWaitingSize : size;
    memcpy(data, &pInfo->pbBuffer[pInfo->uReadOffset], nTaken);
    pInfo->nDataWaitingSize -= nTaken;
    if (pInfo->nDataWaitingSize != 0) {
        pInfo->uReadOffset += nTaken;
        return nTaken;
    }
    // The buffer is drained. Tell the module by copying the count to its record.
    pInfo->uReadOffset = 0;
    FlushCache(0);
    dma.data = (unsigned int)pInfo;
    dma.addr = (unsigned int)pInfo->pPeer;
    dma.size = sizeof(pInfo->nDataWaitingSize);
    dma.mode = 0;
    uId = sceSifSetDma(&dma, 1);
    if (uId == 0) {
        printf("Error DMAing data to IOP memory!!!\n");
    }
    while (sceSifDmaStat(uId) >= 0) {
    }
    return nTaken;
}

int LibnetAsyncWriteStream(sceSifMClientData *cd,
                           int *buffer,
                           LibnetAsyncHandle *handle,
                           const void *data,
                           unsigned int size) {
    LibnetAsyncInfo *pInfo;
    sceSifDmaData dma;
    unsigned int uId;
    unsigned int uTaken;

    (void)cd;
    (void)buffer;
    if (handle == NULL) {
        return LIBNET_ERROR_ARGUMENT;
    }
    pInfo = &handle->pSend->info;
    FlushCache(0);
    if (pInfo->nDataWaitingSize > 0) {
        return 0;
    }
    if (pInfo->nDataWaitingSize < 0) {
        return pInfo->nDataWaitingSize;
    }
    uTaken = (pInfo->uBufferSize < size) ? pInfo->uBufferSize : size;
    memcpy(pInfo->pbBuffer, data, uTaken);
    FlushCache(0);
    dma.data = (unsigned int)pInfo->pbBuffer;
    dma.addr = (unsigned int)pInfo->pbPeerBuffer;
    dma.size = uTaken;
    dma.mode = 0;
    uId = sceSifSetDma(&dma, 1);
    if (uId == 0) {
        printf("Error DMAing data to IOP memory!!!\n");
    }
    while (sceSifDmaStat(uId) >= 0) {
    }
    pInfo->nDataWaitingSize = uTaken;
    FlushCache(0);
    dma.addr = (unsigned int)pInfo->pPeer;
    dma.size = sizeof(pInfo->nDataWaitingSize);
    dma.data = (unsigned int)pInfo;
    dma.mode = 0;
    uId = sceSifSetDma(&dma, 1);
    if (uId == 0) {
        printf("Error DMAing data to IOP memory!!!\n");
    }
    while (sceSifDmaStat(uId) >= 0) {
    }
    return uTaken;
}

int LibnetAsyncRecvFrom(sceSifMClientData *cd,
                        int *buffer,
                        LibnetAsyncHandle *handle,
                        void *data,
                        int size,
                        int *flags,
                        sceInetAddress *address,
                        int *port,
                        int timeout) {
    int nReceived;
    int nStatus = LIBNET_ERROR_ASYNC;

    if (cd == NULL || timeout < -1 || buffer == NULL) {
        return LIBNET_ERROR_ASYNC;
    }
    if (size == 0) {
        return 0;
    }
    if (Dequeue(handle, data, size, flags, address, port, &nReceived, &nStatus) != kRingOk) {
        return (nStatus != 0) ? nStatus : LIBNET_ERROR_ASYNC;
    }
    return nReceived;
}

int LibnetAsyncRecvMarker(sceSifMClientData *cd,
                          int *buffer,
                          LibnetAsyncHandle *handle,
                          void *marker,
                          sceInetAddress *address) {
    int bTaken = 0;
    int nStatus = LIBNET_ERROR_ASYNC;

    (void)buffer;
    if (cd == NULL) {
        return LIBNET_ERROR_ASYNC;
    }
    if (DequeueMarker(handle, marker, address, &bTaken, &nStatus) != kRingOk) {
        return (nStatus != 0) ? nStatus : LIBNET_ERROR_ASYNC;
    }
    return bTaken;
}

int LibnetAsyncSendTo(sceSifMClientData *cd,
                      int *buffer,
                      LibnetAsyncHandle *handle,
                      const void *data,
                      int size,
                      int *flags,
                      const sceInetAddress *address,
                      int port,
                      int timeout) {
    int nSent = 0;
    int nStatus = LIBNET_ERROR_ASYNC;

    if (cd == NULL || timeout < -1 || buffer == NULL) {
        return LIBNET_ERROR_ASYNC;
    }
    if (size == 0) {
        return 0;
    }
    if (Enqueue(handle, data, size, *flags, address, port, &nSent, &nStatus) != kRingOk) {
        return (nStatus != 0) ? nStatus : LIBNET_ERROR_ASYNC;
    }
    return nSent;
}

int LibnetAsyncSendMarker(sceSifMClientData *cd,
                          int *buffer,
                          LibnetAsyncHandle *handle,
                          unsigned short marker,
                          const sceInetAddress *address) {
    int bQueued = 0;
    int nStatus = LIBNET_ERROR_ASYNC;

    (void)buffer;
    if (cd == NULL) {
        return LIBNET_ERROR_ASYNC;
    }
    if (EnqueueMarker(handle, marker, address, &bQueued, &nStatus) != kRingOk) {
        return (nStatus != 0) ? nStatus : LIBNET_ERROR_ASYNC;
    }
    return bQueued;
}

int LibnetAsyncControl(
    sceSifMClientData *cd, int *buffer, LibnetAsyncHandle *handle, int code, void *data, int size) {
    if (handle == NULL) {
        return LIBNET_ERROR_ARGUMENT;
    }
    return sceInetControl(cd, buffer, handle->nCid, code, data, size);
}

#pragma mark - Addresses

int LibnetStringToAddress(sceInetAddress *address, const char *text) {
    unsigned int auOctets[kOctetCount];

    if (address == NULL || text == NULL) {
        return LIBNET_ERROR_ARGUMENT;
    }
    memset(address, 0, sizeof(*address));
    if (sscanf(text, kTextAddressFormat, &auOctets[0], &auOctets[1], &auOctets[2], &auOctets[3]) !=
        kOctetCount) {
        return LIBNET_ERROR_ARGUMENT;
    }
    address->reserved = 0;
    address->data[kOctet0] = auOctets[0];
    address->data[kOctet3] = auOctets[3];
    address->data[kOctet2] = auOctets[2];
    address->data[kOctet1] = auOctets[1];
    return 0;
}

int LibnetAddressToString(char *text, const sceInetAddress *address) {
    if (address == NULL || text == NULL) {
        return LIBNET_ERROR_ARGUMENT;
    }
    sprintf(text,
            kTextAddressFormat,
            address->data[kOctet0],
            address->data[kOctet1],
            address->data[kOctet2],
            address->data[kOctet3]);
    return 0;
}
