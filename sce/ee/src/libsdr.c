#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>

#include <eekernel.h>
#include <libsdr.h>
#include <sifrpc.h>

enum {
    // The sound driver RPC server uses this identifier for the bind call.
    kSdrRpcServer = 0x80000701,
    // The bind retry loop spins this many iterations before checking readiness.
    kSdrBindDelay = 10000,
    // The selector that installs a transfer-completion handler for one DMA channel.
    kSdrCommandSetTransIntrHandler = 0x8160,
    // The selector that installs the SPU2 interrupt handler.
    kSdrCommandSetSpu2IntrHandler = 0x8170,
    // The selectors that send a buffer the caller passes, and receive into the packet.
    kSdrCommandSetEffectAttr = 0x8130,
    kSdrCommandSendBuffer1 = 0x81a0,
    kSdrCommandSendBuffer2 = 0x81b0,
    // The selector that reads a core's effect attributes into a caller buffer.
    kSdrCommandGetEffectAttr = 0x8140,
    // The selectors that send a batch of entries the caller passes, after writing its count.
    kSdrCommandBatch = 0x81c0,
    kSdrCommandBatchExtended = 0x81d0,
    // The range of selectors that send a caller buffer of a caller size.
    kSdrCommandUserFirst = 0x9000,
    kSdrCommandUserCount = 241,
    // The remote call sends this many bytes for the fixed-size paths.
    kSdrSendSize = 0x40,
    // The default path receives this many bytes into the packet.
    kSdrReceiveSize = 0x10,
    // Each batch entry is this many bytes, and the batch header is one more entry.
    kSdrBatchEntryShift = 3,
};

// The packet stores the self reference with six forwarded words. Every fixed-size call sends the
// whole 0x40-byte packet.
typedef struct {
    // The packet address before the call, and the reply word the IOP writes back over it.
    int mResult;
    // The six variadic argument words after the command, in call order.
    int mArguments[6];
    // The rest of the packet the remote call transfers.
    unsigned char mReserved1C[0x24]; // +0x1C
} SdrPacket;

// The header of a batch the caller passes to kSdrCommandBatch and kSdrCommandBatchExtended.
typedef struct {
    unsigned short nReserved;
    unsigned short nCount;
    int nExtra; // Written for kSdrCommandBatchExtended only.
} SdrBatchHeader;

// NTSC-U/C: 0x005311c0
static SdrPacket g_sdrPacket __attribute__((aligned(64)));

// NTSC-U/C: 0x00531200
static sceSifClientData g_sdrClient __attribute__((aligned(64)));

// The completion callback of a call made with a zero control word. The image never writes it.
// NTSC-U/C: 0x003c4768, PAL: 0x0045abb0
static sceSifEndFunc g_pfnSdrEndFunction;

// The transfer handler of DMA channel 0. The interrupt-handler commands record it and the five
// words below.
// NTSC-U/C: 0x003c4778, PAL: 0x0045abc0
static int _sce_sdr_transIntr0Hdr;

// The transfer handler of DMA channel 1.
// NTSC-U/C: 0x003c477c, PAL: 0x0045abc4
static int _sce_sdr_transIntr1Hdr;

// The SPU2 interrupt handler.
// NTSC-U/C: 0x003c4780, PAL: 0x0045abc8
static int _sce_sdr_spu2IntrHdr;

// The argument of the channel 0 transfer handler.
// NTSC-U/C: 0x003c4784, PAL: 0x0045abcc
static int _sce_sdr_transIntr0Arg;

// The argument of the channel 1 transfer handler.
// NTSC-U/C: 0x003c4788, PAL: 0x0045abd0
static int _sce_sdr_transIntr1Arg;

// The argument of the SPU2 interrupt handler.
// NTSC-U/C: 0x003c478c, PAL: 0x0045abd4
static int _sce_sdr_spu2IntrArg;

int sceSdRemoteInit(void) {
    sceSifClientData *pClient = &g_sdrClient;
    int nDelay;

    sceSifInitRpc(0);

    for (;;) {
        if (sceSifBindRpc(pClient, kSdrRpcServer, 0) < 0) {
            scePrintf("sceSdRemoteInit() RPC bind error!\n");
            return -1;
        }
        for (nDelay = kSdrBindDelay - 1; nDelay != -1; --nDelay) {
        }
        if (pClient->serve != NULL) {
            break;
        }
    }
    FlushCache(WRITEBACK_DCACHE);
    return 0;
}

int sceSdRemote(int arg, ...) {
    SdrPacket *pPacket = &g_sdrPacket;
    sceSifClientData *pClient = &g_sdrClient;
    va_list oArguments;
    int nCommand;
    int nMode = 0;
    sceSifEndFunc pfnEnd = NULL;
    int nPrevious = 0;
    int nResult;
    int i;

    pPacket->mResult = (int)(uintptr_t)pPacket;
    va_start(oArguments, arg);
    nCommand = va_arg(oArguments, int);
    for (i = 0; i < (int)(sizeof(pPacket->mArguments) / sizeof(pPacket->mArguments[0])); ++i) {
        pPacket->mArguments[i] = va_arg(oArguments, int);
    }
    va_end(oArguments);
    if (arg == 0) {
        nMode = SIF_RPCM_NOWAIT;
        pfnEnd = g_pfnSdrEndFunction;
    }

    if (nCommand == kSdrCommandSetTransIntrHandler) {
        if (pPacket->mArguments[0] == 0) {
            nPrevious = _sce_sdr_transIntr0Hdr;
            _sce_sdr_transIntr0Hdr = pPacket->mArguments[1];
            _sce_sdr_transIntr0Arg = pPacket->mArguments[2];
        } else {
            nPrevious = _sce_sdr_transIntr1Hdr;
            _sce_sdr_transIntr1Hdr = pPacket->mArguments[1];
            _sce_sdr_transIntr1Arg = pPacket->mArguments[2];
        }
    } else if (nCommand == kSdrCommandSetSpu2IntrHandler) {
        nPrevious = _sce_sdr_spu2IntrHdr;
        _sce_sdr_spu2IntrHdr = pPacket->mArguments[0];
        _sce_sdr_spu2IntrArg = pPacket->mArguments[1];
    }

    nResult = 0;
    if (nCommand == kSdrCommandSetEffectAttr || nCommand == kSdrCommandSendBuffer1 ||
        nCommand == kSdrCommandSendBuffer2) {
        sceSifCallRpc(pClient,
                      nCommand | pPacket->mArguments[0],
                      nMode,
                      (void *)(uintptr_t)pPacket->mArguments[1],
                      kSdrSendSize,
                      pPacket,
                      kSdrSendSize,
                      pfnEnd,
                      (void *)(uintptr_t)pPacket->mResult);
        nResult = pPacket->mResult;
    } else if (nCommand == kSdrCommandGetEffectAttr) {
        sceSifCallRpc(pClient,
                      pPacket->mArguments[0] | kSdrCommandGetEffectAttr,
                      nMode,
                      pPacket,
                      kSdrSendSize,
                      (void *)(uintptr_t)pPacket->mArguments[1],
                      kSdrSendSize,
                      pfnEnd,
                      (void *)(uintptr_t)pPacket->mArguments[1]);
    } else if (nCommand == kSdrCommandBatch || nCommand == kSdrCommandBatchExtended) {
        SdrBatchHeader *pBatch = (SdrBatchHeader *)(uintptr_t)pPacket->mArguments[0];
        void *pReceive = (void *)(uintptr_t)pPacket->mArguments[2];
        int nReceiveSize = pPacket->mArguments[3];

        pBatch->nCount = (unsigned short)pPacket->mArguments[1];
        if (nCommand == kSdrCommandBatchExtended) {
            pBatch->nExtra = pPacket->mArguments[4];
        }
        if (pReceive == NULL) {
            pReceive = &nResult;
            nReceiveSize = sizeof(nResult);
        }
        sceSifCallRpc(pClient,
                      nCommand,
                      nMode,
                      pBatch,
                      (pPacket->mArguments[1] + 1) << kSdrBatchEntryShift,
                      pReceive,
                      nReceiveSize,
                      pfnEnd,
                      pPacket);
        if (pPacket->mArguments[2] != 0) {
            nResult = *(int *)(uintptr_t)pPacket->mArguments[2];
        }
    } else if ((unsigned int)(nCommand - kSdrCommandUserFirst) < kSdrCommandUserCount) {
        sceSifCallRpc(pClient,
                      nCommand,
                      nMode,
                      (void *)(uintptr_t)pPacket->mArguments[0],
                      pPacket->mArguments[1],
                      pPacket,
                      kSdrReceiveSize,
                      pfnEnd,
                      pPacket);
        nResult = pPacket->mResult;
    } else {
        sceSifCallRpc(pClient,
                      nCommand,
                      nMode,
                      pPacket,
                      kSdrSendSize,
                      pPacket,
                      kSdrReceiveSize,
                      pfnEnd,
                      pPacket);
        nResult = pPacket->mResult;
    }
    if (nCommand == kSdrCommandSetTransIntrHandler || nCommand == kSdrCommandSetSpu2IntrHandler) {
        nResult = nPrevious;
    }
    return nResult;
}
