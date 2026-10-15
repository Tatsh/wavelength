#include <stdint.h>
#include <string.h>

#include <eekernel.h>
#include <libmc.h>
#include <sifcmd.h>
#include <sifrpc.h>

enum {
    kMcServerId = 0x80000400,
};

enum {
    // The server function numbers.
    kMcFunctionCardInfo = 1,
    kMcFunctionOpen = 2,
    kMcFunctionClose = 3,
    kMcFunctionSeek = 4,
    kMcFunctionRead = 5,
    kMcFunctionWrite = 6,
    kMcFunctionGetDir = 13,
    kMcFunctionDelete = 15,
    kMcFunctionFormat = 16,
    kMcFunctionUnformat = 17,
    kMcFunctionInit = 254,
    kMcBindDelay = 0x100000,
    kMcSyncDelayTicks = 60,
    kMcMinimumServerVersion = 0x20a,
    kMcMinimumManagerVersion = 0x20e,
    kMcErrorUnbound = -100,
    kMcErrorRpcBias = -100,
    kMcErrorOldServer = -120,
    kMcErrorOldManager = -121,
    kMcErrorBusy = -200,
    kMcErrorBadName = -210,
    kMcNameSize = 1024,
    kMcEndDataSize = 64,
    kMcWriteInlineSize = 16,
    kMcWriteAlignMask = 15,
    // The reply sizes of the initialisation call and of every other call.
    kMcInitResultSize = 12,
    kMcResultSize = 4,
};

// The server writes the buffer back through the uncached segment before the end callback runs.
typedef struct {
    union {
        struct {
            int nHeadSize;
            int nTailSize;
            unsigned char *pHead;
            unsigned char *pTail;
            unsigned char abHead[kMcEndDataSize];
            unsigned char abTail[kMcEndDataSize];
        } read;
        struct {
            int nType;
            int nFree;
        } info;
    };
    int nFormat; // Written by the card information call only.
} __attribute__((aligned(64))) McEndData;

// The packet of the calls that address a descriptor or a card.
typedef struct {
    int nFd;
    int nPort;
    int nSlot;
    int nSize;   // sceMcGetInfo() sets it when the format state is wanted.
    int nOffset; // sceMcGetInfo() sets it when the free space is wanted.
    int nOrigin; // The inline byte count of a write, or the card type request.
    const unsigned char *pBuffer;
    McEndData *pEndData;
    unsigned char abData[kMcWriteInlineSize];
} McDescPacket;

// The packet of the calls that address a path.
typedef struct {
    int nPort;
    int nSlot;
    int nFlags;
    int nMaxEntries;
    sceMcTblGetDir *pTable;
    char szName[kMcNameSize];
} McNamePacket;

// NTSC-U/C: 0x003c2b40, PAL: 0x00458f80
static int g_nMcCommand = 0;
// NTSC-U/C: 0x003c2b44, PAL: 0x00458f84
static int g_nMcSemaId = -1;
#ifdef VIDEO_STANDARD_PAL
// PAL: 0x00458f88. McDelayThread() waits on the semaphore until its alarm signals it.
static int g_nMcDelaySemaId = -1;
#endif

// NTSC-U/C: 0x0052f7c0
static sceSifClientData g_mcClient __attribute__((aligned(64)));
// NTSC-U/C: 0x0052f7e8
static int *g_pnMcInfoType;
// NTSC-U/C: 0x0052f7ec
static int *g_pnMcInfoFree;
// NTSC-U/C: 0x0052f7f0
static int *g_pnMcInfoFormat;
// SIF DMA moves whole quadwords. The buffers below retain retail's placement (64-byte boundaries,
// and a 16-byte boundary for the path packet).
// NTSC-U/C: 0x0052f840
static McDescPacket g_mcDescPacket __attribute__((aligned(64)));
// NTSC-U/C: 0x0052f870
static McNamePacket g_mcNamePacket __attribute__((aligned(16)));
// NTSC-U/C: 0x0052fcc0
static McEndData g_mcEndData __attribute__((aligned(64)));
// NTSC-U/C: 0x00530d80
static sceMcRpcResult g_mcResult __attribute__((aligned(64)));

int sceMcInitLibrary(void) {
    struct SemaParam param;
    int nResult;
    int i;

    if (g_nMcSemaId < 0) {
        param.maxCount = 1;
        param.initCount = 1;
        param.option = 0;
        g_nMcSemaId = CreateSema(&param);
#ifdef VIDEO_STANDARD_PAL
        param.maxCount = 1;
        param.initCount = 0;
        param.option = 0;
        g_nMcDelaySemaId = CreateSema(&param);
#endif
    }
    sceMcSync(sceMcWait, NULL, NULL);
    WaitSema(g_nMcSemaId);
    sceSifInitRpc(0);
    for (;;) {
        if (sceSifBindRpc(&g_mcClient, kMcServerId, 0) < 0) {
            scePrintf("bind error libmc \n");
            for (;;) {
            }
        }
        if (g_mcClient.serve != NULL) {
            break;
        }
        for (i = kMcBindDelay; i != 0; --i) {
            __asm__ volatile("nop");
        }
    }

    nResult = sceSifCallRpc(&g_mcClient,
                            kMcFunctionInit,
                            0,
                            &g_mcDescPacket,
                            sizeof(g_mcDescPacket),
                            &g_mcResult,
                            kMcInitResultSize,
                            NULL,
                            NULL);
    SignalSema(g_nMcSemaId);
    if (nResult < 0) {
        g_mcClient.serve = NULL;
        return nResult + kMcErrorRpcBias;
    }
    if (g_mcResult.nServerVersion < kMcMinimumServerVersion) {
        scePrintf("libmc: too old release of mcserv.irx\n");
        g_mcClient.serve = NULL;
        return kMcErrorOldServer;
    }
    if (g_mcResult.nManagerVersion < kMcMinimumManagerVersion) {
        scePrintf("libmc: too old release of mcman.irx\n");
        g_mcClient.serve = NULL;
        return kMcErrorOldManager;
    }
    return g_mcResult.nResult;
}

// Reserve the library for a command. Returns zero, or the error the entry point returns.
static inline int McBeginCommand(void) {
    if (g_mcClient.serve == NULL) {
        return kMcErrorUnbound;
    }
    if (PollSema(g_nMcSemaId) < 0) {
        return kMcErrorBusy;
    }
    return 0;
}

// Record the command a sent call started, or release the library when the call failed.
static inline int McEndCommand(int nResult, int nCommand) {
    if (nResult != 0) {
        SignalSema(g_nMcSemaId);
    } else {
        g_nMcCommand = nCommand;
    }
    return nResult;
}

// Send the descriptor packet without waiting for the reply.
static inline int McCallDesc(int nFunction, sceSifEndFunc pfnEnd, void *pEndParameter) {
    return sceSifCallRpc(&g_mcClient,
                         nFunction,
                         SIF_RPCM_NOWAIT,
                         &g_mcDescPacket,
                         sizeof(g_mcDescPacket),
                         &g_mcResult,
                         kMcResultSize,
                         pfnEnd,
                         pEndParameter);
}

// Send the path packet without waiting for the reply.
static inline int McCallName(int nFunction, sceSifEndFunc pfnEnd, void *pEndParameter) {
    return sceSifCallRpc(&g_mcClient,
                         nFunction,
                         SIF_RPCM_NOWAIT,
                         &g_mcNamePacket,
                         sizeof(g_mcNamePacket),
                         &g_mcResult,
                         kMcResultSize,
                         pfnEnd,
                         pEndParameter);
}

static inline int McIsBadName(const char *pszName) {
    return pszName == NULL || pszName[0] == '\0';
}

static inline void McSetName(const char *pszName) {
    strncpy(g_mcNamePacket.szName, pszName, kMcNameSize - 1);
    g_mcNamePacket.szName[kMcNameSize - 1] = '\0';
}

int sceMcEnd(void) {
    if (g_nMcSemaId >= 0) {
        DeleteSema(g_nMcSemaId);
        g_nMcSemaId = -1;
#ifdef VIDEO_STANDARD_PAL
        DeleteSema(g_nMcDelaySemaId); // Yes, the binary does not reset the identifier.
#endif
    }
    return 1;
}

int sceMcOpen(int nPort, int nSlot, const char *pszName, int nMode) {
    int nResult;

    if ((nResult = McBeginCommand()) != 0) {
        return nResult;
    }
    if (McIsBadName(pszName)) {
        SignalSema(g_nMcSemaId);
        return kMcErrorBadName;
    }
    McSetName(pszName);
    g_mcNamePacket.nPort = nPort;
    g_mcNamePacket.nFlags = nMode;
    g_mcNamePacket.nSlot = nSlot;
    return McEndCommand(McCallName(kMcFunctionOpen, NULL, NULL), sceMcFuncNoOpen);
}

int sceMcMkdir(int nPort, int nSlot, const char *pszName) {
    const int nResult = sceMcOpen(nPort, nSlot, pszName, sceMcFileCreateDir);

    if (nResult == 0) {
        g_nMcCommand = sceMcFuncNoMkdir;
    }
    return nResult;
}

int sceMcClose(int nFd) {
    int nResult;

    if ((nResult = McBeginCommand()) != 0) {
        return nResult;
    }
    g_mcDescPacket.nFd = nFd;
    return McEndCommand(McCallDesc(kMcFunctionClose, NULL, NULL), sceMcFuncNoClose);
}

int sceMcSeek(int nFd, int nOffset, int nMode) {
    int nResult;

    if ((nResult = McBeginCommand()) != 0) {
        return nResult;
    }
    g_mcDescPacket.nFd = nFd;
    g_mcDescPacket.nOffset = nOffset;
    g_mcDescPacket.nOrigin = nMode;
    return McEndCommand(McCallDesc(kMcFunctionSeek, NULL, NULL), sceMcFuncNoSeek);
}

// NTSC-U/C: 0x00313450, PAL: 0x0037fb10
static void mceIntrReadFixAlign(void *pParameter) {
    const McEndData *pEnd = UNCACHED_SEG(pParameter);
    int i;

    for (i = 0; i < pEnd->read.nHeadSize; ++i) {
        pEnd->read.pHead[i] = pEnd->read.abHead[i];
    }
    for (i = 0; i < pEnd->read.nTailSize; ++i) {
        pEnd->read.pTail[i] = pEnd->read.abTail[i];
    }
}

int sceMcRead(int nFd, void *pBuffer, int nSize) {
    int nResult;

    if ((nResult = McBeginCommand()) != 0) {
        return nResult;
    }
    g_mcDescPacket.nFd = nFd;
    g_mcDescPacket.pEndData = &g_mcEndData;
    g_mcDescPacket.pBuffer = pBuffer;
    g_mcDescPacket.nSize = nSize;
    sceSifWriteBackDCache(pBuffer, nSize);
    sceSifWriteBackDCache(&g_mcEndData, sizeof(g_mcEndData));
    return McEndCommand(McCallDesc(kMcFunctionRead, mceIntrReadFixAlign, &g_mcEndData),
                        sceMcFuncNoRead);
}

int sceMcWrite(int nFd, const void *pBuffer, int nSize) {
    const unsigned char *pbSource = pBuffer;
    unsigned int nHead;
    unsigned int i;
    int nResult;

    if ((nResult = McBeginCommand()) != 0) {
        return nResult;
    }
    g_mcDescPacket.nFd = nFd;
    if (nSize <= kMcWriteInlineSize) {
        g_mcDescPacket.nOrigin = nSize;
        g_mcDescPacket.pBuffer = NULL;
        g_mcDescPacket.nSize = 0;
    } else {
        // The bytes before the next 16-byte boundary go inline, and the server reads the rest by
        // DMA.
        nHead = (((uintptr_t)pbSource - 1) & ~kMcWriteAlignMask) -
                ((uintptr_t)pbSource - kMcWriteInlineSize);
        g_mcDescPacket.pBuffer = &pbSource[nHead];
        g_mcDescPacket.nSize = nSize - nHead;
        g_mcDescPacket.nOrigin = nHead;
    }
    for (i = 0; i < (unsigned int)g_mcDescPacket.nOrigin; ++i) {
        g_mcDescPacket.abData[i] = pbSource[i];
    }
    FlushCache(WRITEBACK_DCACHE);
    return McEndCommand(McCallDesc(kMcFunctionWrite, NULL, NULL), sceMcFuncNoWrite);
}

#ifdef VIDEO_STANDARD_PAL
// PAL: 0x0037fe30
static void McAlarmHandler(int nId, unsigned short nTime, void *pSema) {
    (void)nId;
    (void)nTime;
    iSignalSema((int)(intptr_t)pSema);
}

// PAL: 0x0037fe38
static void McDelayThread(unsigned short nTicks) {
    SetAlarm(nTicks, McAlarmHandler, (void *)(intptr_t)g_nMcDelaySemaId);
    WaitSema(g_nMcDelaySemaId);
}
#else
// NTSC-U/C: 0x00313770
static void McAlarmHandler(int nId, unsigned short nTime, void *pThread) {
    (void)nId;
    (void)nTime;
    iWakeupThread((int)(intptr_t)pThread);
    ExitHandler();
}

// NTSC-U/C: 0x00313798, PAL: 0x0037fe38
static void McDelayThread(unsigned short nTicks) {
    SetAlarm(nTicks, McAlarmHandler, (void *)(intptr_t)GetThreadId());
    SleepThread();
}
#endif

int sceMcSync(int nMode, int *pnCommand, int *pnResult) {
    int nBusy;

    if (g_nMcCommand == 0) {
        return sceMcExecIdle;
    }
    nBusy = sceSifCheckStatRpc(&g_mcClient.rpcd);
    if (nMode == sceMcWait && nBusy != 0) {
        while (sceSifCheckStatRpc(&g_mcClient.rpcd) != 0) {
            McDelayThread(kMcSyncDelayTicks);
        }
        nBusy = 0;
    }
    if (pnCommand != NULL) {
        *pnCommand = g_nMcCommand;
    }
    if (nBusy != 0) {
        return sceMcExecRun;
    }
    g_nMcCommand = 0;
    if (pnResult != NULL) {
        *pnResult = g_mcResult.nResult;
    }
    SignalSema(g_nMcSemaId);
    return sceMcExecFinish;
}

// NTSC-U/C: 0x003138c0, PAL: 0x0037ff50
static void mceGetInfoApdx(void *pParameter) {
    const McEndData *pEnd = UNCACHED_SEG(pParameter);

    if (g_pnMcInfoType != NULL) {
        *g_pnMcInfoType = pEnd->info.nType;
    }
    if (g_pnMcInfoFree != NULL) {
        *g_pnMcInfoFree = pEnd->info.nFree;
    }
    if (g_pnMcInfoFormat != NULL) {
        *g_pnMcInfoFormat = pEnd->nFormat;
    }
}

int sceMcGetInfo(int nPort, int nSlot, int *pnType, int *pnFree, int *pnFormat) {
    int nResult;

    if ((nResult = McBeginCommand()) != 0) {
        return nResult;
    }
    g_mcDescPacket.nPort = nPort;
    g_mcDescPacket.nSlot = nSlot;
    g_mcDescPacket.pEndData = &g_mcEndData;
    g_mcDescPacket.nOrigin = pnType != NULL;
    g_mcDescPacket.nOffset = pnFree != NULL;
    g_mcDescPacket.nSize = pnFormat != NULL;
    g_pnMcInfoType = pnType;
    g_pnMcInfoFree = pnFree;
    g_pnMcInfoFormat = pnFormat;
    sceSifWriteBackDCache(&g_mcEndData, sizeof(g_mcEndData));
    return McEndCommand(McCallDesc(kMcFunctionCardInfo, mceGetInfoApdx, &g_mcEndData),
                        sceMcFuncNoCardInfo);
}

int sceMcGetDir(int nPort,
                int nSlot,
                const char *pszName,
                unsigned int nMode,
                int nMaxEntries,
                sceMcTblGetDir *pTable) {
    int nResult;

    if ((nResult = McBeginCommand()) != 0) {
        return nResult;
    }
    if (McIsBadName(pszName)) {
        SignalSema(g_nMcSemaId);
        return kMcErrorBadName;
    }
    g_mcNamePacket.nPort = nPort;
    g_mcNamePacket.nSlot = nSlot;
    g_mcNamePacket.nFlags = nMode;
    g_mcNamePacket.nMaxEntries = nMaxEntries;
    g_mcNamePacket.pTable = pTable;
    McSetName(pszName);
    if (nMaxEntries >= 0) {
        sceSifWriteBackDCache(pTable, nMaxEntries * sizeof(sceMcTblGetDir));
    }
    return McEndCommand(McCallName(kMcFunctionGetDir, NULL, NULL), sceMcFuncNoGetDir);
}

int sceMcFormat(int nPort, int nSlot) {
    int nResult;

    if ((nResult = McBeginCommand()) != 0) {
        return nResult;
    }
    g_mcDescPacket.nPort = nPort;
    g_mcDescPacket.nSlot = nSlot;
    return McEndCommand(McCallDesc(kMcFunctionFormat, NULL, NULL), sceMcFuncNoFormat);
}

int sceMcDelete(int nPort, int nSlot, const char *pszName) {
    int nResult;

    if ((nResult = McBeginCommand()) != 0) {
        return nResult;
    }
    if (McIsBadName(pszName)) {
        SignalSema(g_nMcSemaId);
        return kMcErrorBadName;
    }
    McSetName(pszName);
    g_mcNamePacket.nPort = nPort;
    g_mcNamePacket.nSlot = nSlot;
    g_mcNamePacket.nFlags = 0;
    return McEndCommand(McCallName(kMcFunctionDelete, NULL, NULL), sceMcFuncNoDelete);
}

int sceMcUnformat(int nPort, int nSlot) {
    int nResult;

    if ((nResult = McBeginCommand()) != 0) {
        return nResult;
    }
    g_mcDescPacket.nPort = nPort;
    g_mcDescPacket.nSlot = nSlot;
    return McEndCommand(McCallDesc(kMcFunctionUnformat, NULL, NULL), sceMcFuncNoUnformat);
}
