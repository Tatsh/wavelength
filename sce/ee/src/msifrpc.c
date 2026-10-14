#include <stddef.h>
#include <stdint.h>

#include <eekernel.h>
#include <msifrpc.h>
#include <sifcmd.h>
#include <sifrpc.h>

enum {
    kMRpcPacketCount = 32,
    kMRpcPacketSize = 64,

    // The command codes of the msifrpc module.
    kMRpcCommandEnd = 0x80000018,
    kMRpcCommandBind = 0x80000019,
    kMRpcCommandCall = 0x8000001a,
    kMRpcCommandUnbind = 0x8000001d,

    kRecordAllocated = 0x01,
    kRecordIndexed = 0x04,
    kRecordIndexShift = 16,

    // The software register the EE sets to report that the client is ready.
    kMRpcReadySoftwareRegister = 1,

    kMRpcBufferAlignMask = 63,

    kMRpcErrorNoPacket = -1,
    kMRpcErrorSend = -2,
    kMRpcErrorSema = -3,
};

// The words every msifrpc packet begins with.
typedef struct {
    sceSifCmdHdr header;
    unsigned int nRecord;
    void *pPacket; // The packet's address on its sender, echoed back in the reply.
    unsigned int nId;
    sceSifMClientData *pClient;
} MRpcPacketHeader;

typedef struct {
    MRpcPacketHeader header;
    unsigned int nCommand;
    int nBufferSize;
    int nStackSize;
    int nPriority;
} MRpcBindPacket;

typedef struct {
    MRpcPacketHeader header;
    unsigned int nFunction;
    int nSendSize;
    void *pReceive;
    int nReceiveSize;
    int nReplyMode;
    void *pServe;
} MRpcCallPacket;

typedef struct {
    MRpcPacketHeader header;
    unsigned int nCommand;
    void *pServe;
} MRpcUnbindPacket;

// The reply to a request. The command code is the code of the finished request.
typedef struct {
    MRpcPacketHeader header;
    unsigned int nCommand;
    union {
        void *pServe;
        int nUnbindResult;
    } result;
    void *pBuff;
    void *pCbuff;
} MRpcEndPacket;

typedef union {
    sceSifCmdSRData sreg;
    MRpcPacketHeader header;
    MRpcBindPacket bind;
    MRpcCallPacket call;
    MRpcUnbindPacket unbind;
    MRpcEndPacket end;
    unsigned char abBytes[kMRpcPacketSize];
} MRpcPacket;

typedef struct {
    int nPacketId;
    MRpcPacket *pPackets; // Accessed through the uncached address.
    int nPacketCount;
    int nUnusedFirst;  // Cleared by sceSifMInitRpc() and never read.
    int nUnusedSecond; // Cleared by sceSifMInitRpc() and never read.
} MRpcState;

static const char kModeError[] = "mode must be 0 in this version.\n";

// NTSC-U/C: 0x003c47a0
static int g_nMRpcInitialized = 0;

// NTSC-U/C: 0x00532380
// SIF DMA needs the 64-byte alignment retail gives it.
static MRpcPacket g_aMRpcPackets[kMRpcPacketCount] __attribute__((aligned(64)));

// NTSC-U/C: 0x00532b80
static MRpcState _data_table;

// NTSC-U/C: 0x003163e0, PAL: 0x00382c98
static MRpcPacket *getPacket(MRpcState *pState) {
    MRpcPacket *pPacket = pState->pPackets;
    int nId;
    int i;

    DIntr();
    for (i = 0; i < pState->nPacketCount; ++i, ++pPacket) {
        if ((pPacket->header.nRecord & kRecordAllocated) != 0) {
            continue;
        }
        pPacket->header.nRecord =
            ((unsigned int)i << kRecordIndexShift) | kRecordIndexed | kRecordAllocated;
        nId = pState->nPacketId + 1;
        pState->nPacketId = nId;
        if (nId == 1) {
            // Yes, the packet receives 1 while the counter skips past it.
            pState->nPacketId = nId + 1;
        }
        pPacket->header.pPacket = pPacket;
        pPacket->header.nId = (unsigned int)nId;
        EIntr();
        return pPacket;
    }
    EIntr();
    return NULL;
}

// NTSC-U/C: 0x00316488, PAL: 0x00382d40
static void freePacket(MRpcPacket *pPacket) {
    pPacket->header.nId = 0;
    pPacket->header.nRecord &= ~(unsigned int)kRecordAllocated;
}

// NTSC-U/C: 0x003164a8, PAL: 0x00382d60
static void _request_end(void *pPacket, void *pData) {
    const MRpcEndPacket *pEnd = pPacket;
    sceSifMClientData *pClient = pEnd->header.pClient;

    (void)pData;
    if (pEnd->nCommand == kMRpcCommandCall) {
        if (pClient->func != NULL) {
            pClient->func(pClient->para);
        }
        iSignalSema(pClient->sema);
    } else if (pEnd->nCommand == kMRpcCommandBind) {
        pClient->serve = pEnd->result.pServe;
        pClient->buff = pEnd->pBuff;
        pClient->cbuff = pEnd->pCbuff;
        iSignalSema(pClient->sema);
    } else if (pEnd->nCommand == kMRpcCommandUnbind) {
        pClient->nUnbindResult = pEnd->result.nUnbindResult;
        iSignalSema(pClient->sema);
    }
    if (pClient->tid >= 0) {
        iSignalSema(pClient->tid);
    }
    freePacket(pClient->paddr);
    pClient->paddr = NULL;
}

void sceSifMInitRpc(unsigned int mode) {
    sceSifCmdSRData *pReadyPacket;

    (void)mode;
    if (g_nMRpcInitialized != 0) {
        return;
    }
    g_nMRpcInitialized = 1;
    (void)DIntr(); // Yes, the binary enables interrupts below whatever DIntr() reports.
    _data_table.nPacketId = 1;
    _data_table.pPackets = UNCACHED_SEG(g_aMRpcPackets);
    _data_table.nPacketCount = kMRpcPacketCount;
    _data_table.nUnusedFirst = 0;
    _data_table.nUnusedSecond = 0;
    sceSifAddCmdHandler(kMRpcCommandEnd, _request_end, &_data_table);
    EIntr();

    // The binary borrows the second request packet, through its cached address.
    pReadyPacket = &g_aMRpcPackets[1].sreg;
    pReadyPacket->value = 1;
    pReadyPacket->rno = kMRpcReadySoftwareRegister;
    sceSifSendCmd(SIF_CMDC_SET_SREG, pReadyPacket, sizeof(*pReadyPacket), NULL, NULL, 0);
    while (sceSifGetSreg(kMRpcReadySoftwareRegister) == 0) {
    }
}

int sceSifMBindRpcParam(sceSifMClientData *cd,
                        unsigned int command,
                        unsigned int mode,
                        int buffersize,
                        int stacksize,
                        int priority) {
    struct SemaParam sema; // Yes, the binary sets only three of the fields.
    MRpcPacket *pPacket;
    int nWaitSema;

    if (mode != 0) {
        scePrintf(kModeError);
        return kMRpcErrorNoPacket;
    }
    sema.option = 0;
    sema.initCount = 1;
    sema.maxCount = 1;
    cd->sema = CreateSema(&sema);
    if (cd->sema < 0) {
        return kMRpcErrorSema;
    }
    WaitSema(cd->sema);
    cd->command = command;
    cd->serve = NULL;
    cd->buffersize = buffersize;
    cd->stacksize = stacksize;
    cd->priority = priority;
    pPacket = getPacket(&_data_table);
    if (pPacket == NULL) {
        DeleteSema(cd->sema);
        return kMRpcErrorNoPacket;
    }
    cd->paddr = pPacket;
    cd->pid = pPacket->header.nId;
    pPacket->bind.nCommand = command;
    pPacket->bind.nBufferSize = buffersize;
    pPacket->bind.nStackSize = stacksize;
    pPacket->bind.nPriority = priority;
    pPacket->header.pPacket = pPacket;
    pPacket->header.pClient = cd;
    sema.maxCount = 1;
    sema.initCount = 0;
    sema.option = 0;
    nWaitSema = CreateSema(&sema);
    cd->tid = nWaitSema;
    if (nWaitSema < 0) {
        freePacket(pPacket);
        DeleteSema(cd->sema);
        return kMRpcErrorSema;
    }
    if (sceSifSendCmd(kMRpcCommandBind, pPacket, kMRpcPacketSize, NULL, NULL, 0) == 0) {
        freePacket(pPacket);
        DeleteSema(nWaitSema);
        DeleteSema(cd->sema);
        return kMRpcErrorSend;
    }
    WaitSema(nWaitSema);
    DeleteSema(nWaitSema);
    if (cd->serve == NULL) {
        DeleteSema(cd->sema);
    }
    return 0;
}

int sceSifMUnBindRpc(sceSifMClientData *cd, unsigned int mode) {
    struct SemaParam sema; // Yes, the binary sets only three of the fields.
    MRpcPacket *pPacket;
    int nWaitSema;

    if (mode != 0) {
        scePrintf(kModeError);
        return kMRpcErrorNoPacket;
    }
    WaitSema(cd->sema);
    pPacket = getPacket(&_data_table);
    if (pPacket == NULL) {
        return kMRpcErrorNoPacket; // Yes, the binary does not release the client semaphore.
    }
    cd->pid = pPacket->header.nId;
    cd->paddr = pPacket;
    pPacket->unbind.nCommand = cd->command;
    pPacket->header.pPacket = pPacket;
    pPacket->header.pClient = cd;
    pPacket->unbind.pServe = cd->serve;
    sema.maxCount = 1;
    sema.initCount = 0;
    sema.option = 0;
    nWaitSema = CreateSema(&sema);
    cd->tid = nWaitSema;
    if (nWaitSema < 0) {
        freePacket(pPacket);
        return kMRpcErrorSema;
    }
    if (sceSifSendCmd(kMRpcCommandUnbind, pPacket, kMRpcPacketSize, NULL, NULL, 0) == 0) {
        freePacket(pPacket);
        DeleteSema(nWaitSema);
        return kMRpcErrorSend;
    }
    WaitSema(nWaitSema);
    DeleteSema(nWaitSema);
    DeleteSema(cd->sema);
    return cd->nUnbindResult;
}

int sceSifMCallRpc(sceSifMClientData *cd,
                   unsigned int rpc_number,
                   unsigned int mode,
                   void *send,
                   int ssize,
                   void *receive,
                   int rsize,
                   sceSifEndFunc end_function,
                   void *end_param) {
    struct SemaParam sema; // Yes, the binary sets only three of the fields.
    MRpcPacket *pPacket;
    int nWaitSema;

    if (mode != 0) {
        scePrintf(kModeError);
        return kMRpcErrorNoPacket;
    }
    WaitSema(cd->sema);
    // Yes, the binary returns from these errors without releasing the client semaphore.
    if (((uintptr_t)send & kMRpcBufferAlignMask) != 0) {
        scePrintf("%s: send buffer addr is not 64 byte align. %08x=n", "sceSifMCallRpc", send);
        return kMRpcErrorNoPacket;
    }
    if (((uintptr_t)receive & kMRpcBufferAlignMask) != 0) {
        scePrintf(
            "%s: receive buffer addr is not 64 byte align. %08x=n", "sceSifMCallRpc", receive);
        return kMRpcErrorNoPacket;
    }
    pPacket = getPacket(&_data_table);
    if (pPacket == NULL) {
        return kMRpcErrorNoPacket;
    }
    cd->para = end_param;
    cd->pid = pPacket->header.nId;
    cd->paddr = pPacket;
    cd->func = end_function;
    pPacket->call.nFunction = rpc_number;
    pPacket->header.pPacket = pPacket;
    pPacket->call.nSendSize = ssize;
    pPacket->call.pReceive = receive;
    pPacket->call.nReceiveSize = rsize;
    pPacket->header.pClient = cd;
    pPacket->call.pServe = cd->serve;
    if (send == receive) {
        sceSifWriteBackDCache(send, ssize < rsize ? rsize : ssize);
    } else {
        if (ssize > 0) {
            sceSifWriteBackDCache(send, ssize);
        }
        if (rsize > 0) {
            sceSifWriteBackDCache(receive, rsize);
        }
    }
    // Yes, the binary tests the mode again after it rejected every nonzero mode.
    if ((mode & SIF_RPCM_NOWAIT) != 0) {
        pPacket->call.nReplyMode = end_function != NULL ? 1 : 0;
        cd->tid = -1;
        if (sceSifSendCmd(kMRpcCommandCall, pPacket, kMRpcPacketSize, send, cd->buff, ssize) != 0) {
            return 0;
        }
        freePacket(pPacket);
        return kMRpcErrorSend;
    }
    sema.maxCount = 1;
    sema.initCount = 0;
    sema.option = 0;
    nWaitSema = CreateSema(&sema);
    cd->tid = nWaitSema;
    if (nWaitSema < 0) {
        freePacket(pPacket);
        return kMRpcErrorSema;
    }
    pPacket->call.nReplyMode = 1;
    if (sceSifSendCmd(kMRpcCommandCall, pPacket, kMRpcPacketSize, send, cd->buff, ssize) == 0) {
        freePacket(pPacket);
        DeleteSema(nWaitSema);
        return kMRpcErrorSend;
    }
    WaitSema(nWaitSema);
    DeleteSema(nWaitSema);
    return 0;
}
