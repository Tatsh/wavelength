#include <string.h>

#include <eekernel.h>
#include <libezctl.h>
#include <sifdev.h>
#include <sifrpc.h>

enum {
    kEzNetCtlServerId = 0x75488909,
    // The server functions.
    kEzNetCtlFunctionGetDataAddress = 0,
    kEzNetCtlFunctionApplyData = 1,
    kEzNetCtlFunctionGetStatus = 3,
    kEzNetCtlFunctionUpInterface = 5,
    kEzNetCtlFunctionDownInterface = 6,
    kEzNetCtlFunctionLookUpName = 7,
    // The iterations of the wait between two attempts to bind.
    kEzNetCtlBindDelay = 0x10000,
    kEzNetCtlNotBound = -1,
    kEzNetCtlNoDataAddress = -2,
    // The attempts of ezNetCtlLookUpName().
    kEzNetCtlLookUpAttempts = 3,
    // SIF DMA moves whole quadwords.
    kSifDmaAlignMask = 15,
};

static const char kEzNetCtlModuleName[] = "eznetctl";
// The source file name the diagnostics print.
static const char kEzNetCtlFileName[] = "libezctl.c";

// NTSC-U/C: 0x003b14c4, PAL: 0x00430394
static int g_nEznetctlBound = 0;
// NTSC-U/C: 0x00440900
static sceSifClientData g_eznetctlClient __attribute__((aligned(64)));
// The word of the calls that pass one value, the address of the settings buffer of the server,
// a result, or an interface.
// NTSC-U/C: 0x00440940
static int g_dwEznetctlPropAddr __attribute__((aligned(64)));
// NTSC-U/C: 0x00440a80
static char g_abEznetctlNameBuf[EZ_NETCTL_NAME_SIZE] __attribute__((aligned(64)));

// NTSC-U/C: 0x0026cd60, PAL: 0x00276900
// Copy a buffer to IOP memory by SIF DMA, padding the size to whole quadwords, and wait for the
// transfer unless bNoWait is set.
static unsigned int TransferSifDma(const void *src, unsigned int dest, int size, int bNoWait) {
    sceSifDmaData dma;
    dma.data = (unsigned int)src;
    dma.addr = dest;
    dma.size = (size + kSifDmaAlignMask) & ~kSifDmaAlignMask;
    dma.mode = 0;
    if ((size & kSifDmaAlignMask) != 0) {
        scePrintf("%s> ", kEzNetCtlFileName);
        scePrintf("warning: SIF DMA transfer size %d padded to %d\n", size, dma.size);
    }
    unsigned int id;
    while ((id = sceSifSetDma(&dma, 1)) == 0) {
    }
    if (bNoWait == 0) {
        while (sceSifDmaStat(id) >= 0) {
        }
    }
    return id;
}

int ezNetCtlInit(void) {
    const int result = sceSifSearchModuleByName(kEzNetCtlModuleName);
    if (result < 0) {
        scePrintf("\"%s\" module not found (%d)\n", kEzNetCtlModuleName, result);
    }
    for (;;) {
        if (sceSifBindRpc(&g_eznetctlClient, kEzNetCtlServerId, 0) < 0) {
            scePrintf("could not bind to eznetctl service\n");
        }
        if (g_eznetctlClient.serve != 0) {
            break;
        }
        for (int i = kEzNetCtlBindDelay; i != -1; --i) {
            __asm__ volatile("nop\n\tnop\n\tnop\n\tnop\n\tnop");
        }
    }
    g_nEznetctlBound = 1;
    return 1;
}

#ifdef VIDEO_STANDARD_PAL
sceSifClientData *ezNetCtlGetClient(void) {
    return &g_eznetctlClient;
}
#endif

int ezNetCtlSetProperties(const ezNetCnfData_t *data) {
    if (g_nEznetctlBound == 0) {
        scePrintf("**** eznetctl RPC server not currently running\n");
        return kEzNetCtlNotBound;
    }
    sceSifCallRpc(&g_eznetctlClient,
                  kEzNetCtlFunctionGetDataAddress,
                  0,
                  &g_dwEznetctlPropAddr,
                  sizeof(g_dwEznetctlPropAddr),
                  &g_dwEznetctlPropAddr,
                  sizeof(g_dwEznetctlPropAddr),
                  0,
                  0);
    if (g_dwEznetctlPropAddr == 0) {
        scePrintf("%s> ", kEzNetCtlFileName);
        scePrintf("eCtlGetPropertiesAddress returned 0\n");
        g_dwEznetctlPropAddr = kEzNetCtlNoDataAddress;
        return kEzNetCtlNoDataAddress;
    }
    TransferSifDma(data, (unsigned int)g_dwEznetctlPropAddr, sizeof(*data), 0);
    sceSifCallRpc(&g_eznetctlClient,
                  kEzNetCtlFunctionApplyData,
                  0,
                  &g_dwEznetctlPropAddr,
                  sizeof(g_dwEznetctlPropAddr),
                  &g_dwEznetctlPropAddr,
                  sizeof(g_dwEznetctlPropAddr),
                  0,
                  0);
    scePrintf("%s> ", kEzNetCtlFileName);
    scePrintf("ezNetCtlSetProperties returned %d\n", g_dwEznetctlPropAddr);
    return g_dwEznetctlPropAddr;
}

int ezNetCtlGetStatus(ezNetCtlStatus_t *status) {
    if (g_nEznetctlBound == 0) {
        scePrintf("**** eznetctl RPC server not currently running\n");
        return kEzNetCtlNotBound;
    }
    sceSifCallRpc(&g_eznetctlClient,
                  kEzNetCtlFunctionGetStatus,
                  0,
                  status,
                  sizeof(*status),
                  status,
                  sizeof(*status),
                  0,
                  0);
    if (status->result != 0) {
        scePrintf("%s> ", kEzNetCtlFileName);
        scePrintf("ezNetCtlGetStatus returned %d\n", status->result);
    }
    return status->result;
}

int ezNetCtlUpInterface(int id) {
    if (g_nEznetctlBound == 0) {
        scePrintf("**** eznetctl RPC server not currently running\n");
        return kEzNetCtlNotBound;
    }
    g_dwEznetctlPropAddr = id;
    sceSifCallRpc(&g_eznetctlClient,
                  kEzNetCtlFunctionUpInterface,
                  0,
                  &g_dwEznetctlPropAddr,
                  sizeof(g_dwEznetctlPropAddr),
                  &g_dwEznetctlPropAddr,
                  sizeof(g_dwEznetctlPropAddr),
                  0,
                  0);
    return 0;
}

int ezNetCtlDownInterface(int id) {
    if (g_nEznetctlBound == 0) {
        scePrintf("**** eznetctl RPC server not currently running\n");
        return kEzNetCtlNotBound;
    }
    g_dwEznetctlPropAddr = id;
    sceSifCallRpc(&g_eznetctlClient,
                  kEzNetCtlFunctionDownInterface,
                  0,
                  &g_dwEznetctlPropAddr,
                  sizeof(g_dwEznetctlPropAddr),
                  &g_dwEznetctlPropAddr,
                  sizeof(g_dwEznetctlPropAddr),
                  0,
                  0);
    return 0;
}

char *ezNetCtlGetNameBuffer(void) {
    return g_abEznetctlNameBuf;
}

int ezNetCtlLookUpName(const char *name) {
    // The routine does not test whether ezNetCtlInit() bound.
    FlushCache(WRITEBACK_DCACHE);
    for (int i = 0; i < kEzNetCtlLookUpAttempts; ++i) {
        strcpy(g_abEznetctlNameBuf, name);
        sceSifCallRpc(&g_eznetctlClient,
                      kEzNetCtlFunctionLookUpName,
                      0,
                      g_abEznetctlNameBuf,
                      sizeof(g_abEznetctlNameBuf),
                      g_abEznetctlNameBuf,
                      sizeof(g_abEznetctlNameBuf),
                      0,
                      0);
        if (g_abEznetctlNameBuf[0] != '\0') {
            break;
        }
        scePrintf("DNS failed:%d\n", i);
    }
    return 0;
}
