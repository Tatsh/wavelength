#include <stddef.h>
#include <stdint.h>

#include <eekernel.h>
#include <sifcmd.h>
#include <sifdev.h>
#include <sifrpc.h>

enum {
    kRpcPacketCount = 32,
    kRpcPacketSize = 64,

    // Packet record bits. A packet taken from the request table records its index, and the reply
    // to it goes back through the matching slot of the IOP client table.
    kRecordAllocated = 0x01,
    kRecordIndexed = 0x04,
    kRecordIndexShift = 16,

    // The option of SIF_CMDC_INIT_CMD that starts the IOP RPC layer, and the software register the
    // IOP sets once it has.
    kRpcInitOption = 1,
    kRpcInitSoftwareRegister = 0,

    kRetryDelay = 0x100000,
    // The horizontal blanks before a refused end of a data request is sent again.
    kRetryAlarmDelay = 10,

    kRpcErrorNoPacket = -1,
    kRpcErrorSend = -2,
    kRpcErrorSema = -3,
};

// The words every RPC packet begins with.
typedef struct {
    sceSifCmdHdr header;
    unsigned int nRecord;
    void *pPacket; // The packet's address on its sender, echoed back in the reply.
    unsigned int nId;
} RpcPacketHeader;

// SIF_CMDC_RPC_BIND in either direction.
typedef struct {
    RpcPacketHeader header;
    sceSifRpcData *pRequest;
    unsigned int nCommand;
} RpcBindPacket;

// SIF_CMDC_RPC_CALL in either direction.
typedef struct {
    RpcPacketHeader header;
    sceSifRpcData *pRequest;
    unsigned int nFunction;
    int nSendSize;
    void *pReceive;
    int nReceiveSize;
    int nReplyMode;
    sceSifServeData *pServe;
} RpcCallPacket;

// SIF_CMDC_RPC_RDATA in either direction.
typedef struct {
    RpcPacketHeader header;
    sceSifRpcData *pRequest;
    void *pSrc;
    void *pDest;
    int nSize;
} RpcDataPacket;

// SIF_CMDC_RPC_END in either direction. The command code is the code of the finished request.
typedef struct {
    RpcPacketHeader header;
    sceSifRpcData *pRequest;
    unsigned int nCommand;
    sceSifServeData *pServe;
    void *pBuff;
    void *pCbuff;
} RpcEndPacket;

// SIF_CMDC_RPC_END for a data request. The packet also keeps the transfer for a retry.
typedef struct {
    RpcPacketHeader header;
    sceSifRpcData *pRequest;
    unsigned int nCommand;
    void *pSrc;
    void *pDest;
    int nSize;
} RpcDataEndPacket;

typedef union {
    RpcPacketHeader header;
    RpcBindPacket bind;
    RpcCallPacket call;
    RpcDataPacket data;
    RpcEndPacket end;
    RpcDataEndPacket dataEnd;
    unsigned char abBytes[kRpcPacketSize];
} RpcPacket;

// The state the RPC command handlers share. The tables are accessed through their uncached
// addresses.
typedef struct {
    int nPacketId;
    RpcPacket *pPackets;
    int nPacketCount;
    int nUnusedFirst;  // Cleared by sceSifInitRpc() and never read.
    int nUnusedSecond; // Cleared by sceSifInitRpc() and never read.
    RpcPacket *pReplies;
    int nReplyCount;
    RpcPacket *pClientPackets;
    int nClientPacketCount;
    int nNextReply;
    sceSifQueueData *pQueues;
} RpcState;

// NTSC-U/C: 0x003c47cc
static int g_nRpcInitialized = 0;

// NTSC-U/C: 0x00533a80, the packets of requests this side makes.
// SIF DMA needs the 64-byte alignment retail gives it.
static RpcPacket g_aRpcPackets[kRpcPacketCount] __attribute__((aligned(64)));

// NTSC-U/C: 0x00534280, the ring of replies to requests the IOP makes.
// SIF DMA needs the 64-byte alignment retail gives it.
static RpcPacket g_aRpcReplies[kRpcPacketCount] __attribute__((aligned(64)));

// NTSC-U/C: 0x00534a80, the replies to calls the IOP made from its request table.
// SIF DMA needs the 64-byte alignment retail gives it.
static RpcPacket g_aRpcClientPackets[kRpcPacketCount] __attribute__((aligned(64)));

// NTSC-U/C: 0x00535280
static RpcState _data_table;

// NTSC-U/C: 0x00319490, PAL: 0x00385da8
static RpcPacket *getPacket(RpcState *pState) {
    RpcPacket *pPacket = pState->pPackets;
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

// NTSC-U/C: 0x00319538, PAL: 0x00385e50
static void freePacket(RpcPacket *pPacket) {
    pPacket->header.nId = 0;
    pPacket->header.nRecord &= ~(unsigned int)kRecordAllocated;
}

// NTSC-U/C: 0x00319558, PAL: 0x00385e70
static RpcPacket *nextReplyPacket(RpcState *pState) {
    int nIndex = pState->nNextReply % pState->nReplyCount;

    pState->nNextReply = nIndex + 1;
    return &pState->pReplies[nIndex];
}

// NTSC-U/C: 0x00319588, PAL: 0x00385ea0
static RpcPacket *_sceRpcGetFPacket2(RpcState *pState, int nIndex) {
    if (nIndex >= 0 && nIndex < pState->nClientPacketCount) {
        return &pState->pClientPackets[nIndex];
    }
    return nextReplyPacket(pState);
}

// NTSC-U/C: 0x003195c8, PAL: 0x00385ee0
static void _request_end(void *pPacket, void *pData) {
    const RpcEndPacket *pEnd = pPacket;
    sceSifRpcData *pRequest = pEnd->pRequest;
    sceSifClientData *pClient = (sceSifClientData *)pRequest;

    (void)pData;
    if (pEnd->nCommand == SIF_CMDC_RPC_CALL) {
        if (pClient->func != NULL) {
            pClient->func(pClient->para);
        }
    } else if (pEnd->nCommand == SIF_CMDC_RPC_BIND) {
        pClient->serve = pEnd->pServe;
        pClient->buff = pEnd->pBuff;
        pClient->cbuff = pEnd->pCbuff;
    }
    if (pRequest->tid >= 0) {
        iSignalSema(pRequest->tid);
    }
    freePacket(pRequest->paddr);
    pRequest->paddr = NULL;
}

// NTSC-U/C: 0x00319680, PAL: 0x00385f98
// Sends the end of a data request again, from an alarm, until the command buffer accepts it.
static void RetrySendDataEnd(int nId, unsigned short nTime, void *pArg) {
    RpcDataEndPacket *pReply = pArg;

    (void)nId;
    (void)nTime;
    if (isceSifSendCmd(
            SIF_CMDC_RPC_END, pReply, kRpcPacketSize, pReply->pSrc, pReply->pDest, pReply->nSize) ==
        0) {
        iSetAlarm(kRetryAlarmDelay, RetrySendDataEnd, pReply);
    }
}

// NTSC-U/C: 0x003196e0, PAL: 0x00385ff8
static void _request_rdata(void *pPacket, void *pData) {
    const RpcDataPacket *pRequest = pPacket;
    RpcPacket *pReply;

    if ((pRequest->header.nRecord & kRecordIndexed) != 0) {
        pReply = _sceRpcGetFPacket2(pData, (int)(pRequest->header.nRecord >> kRecordIndexShift));
    } else {
        pReply = nextReplyPacket(pData);
    }
    pReply->dataEnd.header.pPacket = pRequest->header.pPacket;
    pReply->dataEnd.pRequest = pRequest->pRequest;
    pReply->dataEnd.nCommand = SIF_CMDC_RPC_RDATA;
    pReply->dataEnd.pSrc = pRequest->pSrc;
    pReply->dataEnd.pDest = pRequest->pDest;
    pReply->dataEnd.nSize = pRequest->nSize;
    if (isceSifSendCmd(SIF_CMDC_RPC_END,
                       pReply,
                       kRpcPacketSize,
                       pRequest->pSrc,
                       pRequest->pDest,
                       pRequest->nSize) == 0) {
        iSetAlarm(kRetryAlarmDelay, RetrySendDataEnd, &pReply->dataEnd);
    }
}

// NTSC-U/C: 0x003197b0, PAL: 0x003860c8
static sceSifServeData *_search_svdata(unsigned int nCommand, RpcState *pState) {
    sceSifQueueData *pQueue;
    sceSifServeData *pServe;

    for (pQueue = pState->pQueues; pQueue != NULL; pQueue = pQueue->next) {
        for (pServe = pQueue->link; pServe != NULL; pServe = pServe->link) {
            if (pServe->command == nCommand) {
                return pServe;
            }
        }
    }
    return NULL;
}

// NTSC-U/C: 0x00319800, PAL: 0x00386118
static void _request_bind(void *pPacket, void *pData) {
    const RpcBindPacket *pRequest = pPacket;
    RpcState *pState = pData;
    RpcPacket *pReply = nextReplyPacket(pState);
    sceSifServeData *pServe;

    pReply->end.pRequest = pRequest->pRequest;
    pReply->end.header.pPacket = pRequest->header.pPacket;
    pReply->end.nCommand = SIF_CMDC_RPC_BIND;
    pServe = _search_svdata(pRequest->nCommand, pState);
    if (pServe != NULL) {
        pReply->end.pServe = pServe;
        pReply->end.pBuff = pServe->buff;
        pReply->end.pCbuff = pServe->cbuff;
    } else {
        pReply->end.pServe = NULL;
        pReply->end.pBuff = NULL;
        pReply->end.pCbuff = NULL;
    }
    isceSifSendCmd(SIF_CMDC_RPC_END, pReply, kRpcPacketSize, NULL, NULL, 0);
}

// NTSC-U/C: 0x003199f0, PAL: 0x00386308
// Queues the call on its server and wakes the queue's thread when it is idle.
static void _request_call(void *pPacket, void *pData) {
    const RpcCallPacket *pRequest = pPacket;
    sceSifServeData *pServe = pRequest->pServe;
    sceSifQueueData *pQueue = pServe->base;

    (void)pData;
    if (pQueue->start != NULL) {
        pQueue->end->next = pServe;
    } else {
        pQueue->start = pServe;
    }
    pQueue->end = pServe;
    pServe->paddr = pRequest->header.pPacket;
    pServe->client = (sceSifClientData *)pRequest->pRequest;
    pServe->fno = pRequest->nFunction;
    pServe->size = pRequest->nSendSize;
    pServe->receive = pRequest->pReceive;
    pServe->rsize = pRequest->nReceiveSize;
    pServe->rmode = pRequest->nReplyMode;
    pServe->rid = pRequest->header.nRecord;
    if (pQueue->key >= 0 && pQueue->active == 0) {
        iWakeupThread(pQueue->key);
    }
}

void sceSifInitRpc(unsigned int mode) {
    sceSifCmdHdr *pInitPacket;

    (void)mode;
    DIntr();
    if (g_nRpcInitialized != 0) {
        EIntr();
        return;
    }
    g_nRpcInitialized = 1;
    EIntr();

    sceSifInitCmd();

    DIntr();
    _data_table.nPacketId = 1;
    _data_table.pPackets = UNCACHED_SEG(g_aRpcPackets);
    _data_table.nPacketCount = kRpcPacketCount;
    _data_table.nUnusedFirst = 0;
    _data_table.nUnusedSecond = 0;
    _data_table.pReplies = UNCACHED_SEG(g_aRpcReplies);
    _data_table.nReplyCount = kRpcPacketCount;
    _data_table.pClientPackets = UNCACHED_SEG(g_aRpcClientPackets);
    _data_table.nClientPacketCount = kRpcPacketCount;
    _data_table.nNextReply = 0;
    sceSifAddCmdHandler(SIF_CMDC_RPC_END, _request_end, &_data_table);
    sceSifAddCmdHandler(SIF_CMDC_RPC_BIND, _request_bind, &_data_table);
    sceSifAddCmdHandler(SIF_CMDC_RPC_CALL, _request_call, &_data_table);
    sceSifAddCmdHandler(SIF_CMDC_RPC_RDATA, _request_rdata, &_data_table);
    EIntr();

    if (sceSifGetReg(SIF_SYSREG_RPCINIT) != 0) {
        return;
    }
    // The binary borrows the second request packet, through its cached address.
    pInitPacket = &g_aRpcPackets[1].header.header;
    pInitPacket->opt = kRpcInitOption;
    sceSifSendCmd(SIF_CMDC_INIT_CMD, pInitPacket, sizeof(*pInitPacket), NULL, NULL, 0);
    while (sceSifGetSreg(kRpcInitSoftwareRegister) == 0) {
    }
    sceSifSetReg(SIF_SYSREG_RPCINIT, 1);
}

void sceSifExitRpc(void) {
    sceSifExitCmd();
    g_nRpcInitialized = 0;
}

int sceSifBindRpc(sceSifClientData *bd, unsigned int command, unsigned int mode) {
    struct SemaParam sema = {0};
    RpcPacket *pPacket;

    bd->command = 0;
    bd->serve = NULL;
    pPacket = getPacket(&_data_table);
    if (pPacket == NULL) {
        return kRpcErrorNoPacket;
    }
    bd->rpcd.paddr = pPacket;
    bd->rpcd.pid = pPacket->header.nId;
    pPacket->bind.nCommand = command;
    pPacket->bind.pRequest = &bd->rpcd;
    if ((mode & SIF_RPCM_NOWAIT) != 0) {
        bd->rpcd.tid = -1;
        if (sceSifSendCmd(SIF_CMDC_RPC_BIND, pPacket, kRpcPacketSize, NULL, NULL, 0) == 0) {
            freePacket(pPacket);
            return kRpcErrorSend;
        }
        return 0;
    }
    sema.maxCount = 1;
    sema.initCount = 0;
    bd->rpcd.tid = CreateSema(&sema);
    if (bd->rpcd.tid < 0) {
        freePacket(pPacket);
        return kRpcErrorSema;
    }
    if (sceSifSendCmd(SIF_CMDC_RPC_BIND, pPacket, kRpcPacketSize, NULL, NULL, 0) == 0) {
        freePacket(pPacket);
        DeleteSema(bd->rpcd.tid);
        return kRpcErrorSend;
    }
    WaitSema(bd->rpcd.tid);
    DeleteSema(bd->rpcd.tid);
    return 0;
}

int sceSifCallRpc(sceSifClientData *bd,
                  unsigned int fno,
                  unsigned int mode,
                  void *send,
                  int ssize,
                  void *receive,
                  int rsize,
                  sceSifEndFunc end,
                  void *endpara) {
    struct SemaParam sema = {0};
    RpcPacket *pPacket = getPacket(&_data_table);

    if (pPacket == NULL) {
        return kRpcErrorNoPacket;
    }
    bd->para = endpara;
    bd->rpcd.paddr = pPacket;
    bd->rpcd.pid = pPacket->header.nId;
    bd->func = end;
    pPacket->call.nFunction = fno;
    pPacket->call.nSendSize = ssize;
    pPacket->call.pReceive = receive;
    pPacket->call.nReceiveSize = rsize;
    pPacket->call.pRequest = &bd->rpcd;
    pPacket->call.pServe = bd->serve;
    if ((mode & SIF_RPCM_NOWBDC) == 0) {
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
    }
    if ((mode & SIF_RPCM_NOWAIT) != 0) {
        pPacket->call.nReplyMode = end != NULL ? 1 : 0;
        bd->rpcd.tid = -1;
        if (sceSifSendCmd(SIF_CMDC_RPC_CALL, pPacket, kRpcPacketSize, send, bd->buff, ssize) != 0) {
            return 0;
        }
        freePacket(pPacket);
        return kRpcErrorSend;
    }
    sema.maxCount = 1;
    sema.initCount = 0;
    bd->rpcd.tid = CreateSema(&sema);
    if (bd->rpcd.tid < 0) {
        freePacket(pPacket);
        return kRpcErrorSema;
    }
    pPacket->call.nReplyMode = 1;
    if (sceSifSendCmd(SIF_CMDC_RPC_CALL, pPacket, kRpcPacketSize, send, bd->buff, ssize) == 0) {
        DeleteSema(bd->rpcd.tid);
        freePacket(pPacket);
        return kRpcErrorSend;
    }
    WaitSema(bd->rpcd.tid);
    DeleteSema(bd->rpcd.tid);
    return 0;
}

int sceSifCheckStatRpc(sceSifRpcData *cd) {
    RpcPacket *pPacket = cd->paddr;

    if (pPacket == NULL || cd->pid != pPacket->header.nId ||
        (pPacket->header.nRecord & kRecordAllocated) == 0) {
        return 0;
    }
    return 1;
}
