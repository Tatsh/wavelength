#include <stddef.h>
#include <string.h>

#include <sifdev.h>
#include <sifrpc.h>

// The client of the IOP module loader server. Every call binds the server on first use and checks
// the version it reported.

enum {
    kLoadFileServerId = 0x80000006,
};

enum {
    kLoadFileFunctionModule = 0,
    kLoadFileFunctionStop = 7,
    kLoadFileFunctionUnload = 8,
    kLoadFileFunctionSearchName = 9,
    kLoadFileFunctionVersion = 255,

    kLoadPathSize = 252,
    kLoadArgSize = 252,
    kVersionSize = 4,
    kWordSize = 4,
    kResultSize = 8,
    kBindRetryDelay = 0x100000,

    kLoadFileErrorBind = -0x10000,
    kLoadFileErrorRpc = -0x10001,
    kLoadFileErrorVersion = -0x10004,
};

// The argument and reply block every call shares. The server writes its reply over the start of
// the block.
typedef struct {
    union {
        int nArgLength;
        int nModuleId;
        int nResult;
    };
    union {
        int nBufferArgLength;
        int nModuleResult;
    };
    char szPath[kLoadPathSize];
    char abArgs[kLoadArgSize];
} LoadFileArgs;

// NTSC-U/C: 0x003c47b0, the server version the library was built for.
static const char g_abLoadFileLibraryVersion[kVersionSize] = {'2', '6', '0', '0'};

// NTSC-U/C: 0x003c4870, negative until the client is bound.
static int _bind_check = -1;

// NTSC-U/C: 0x003c4874, a second server version the library accepts.
static const char *g_pszLoadFileAltVersion = "....";

// NTSC-U/C: 0x00536c80. SIF DMA needs the 64-byte alignment retail gives it.
static LoadFileArgs g_loadFileArgs __attribute__((aligned(64)));

// NTSC-U/C: 0x00536e80
static sceSifClientData g_loadFileClient __attribute__((aligned(64)));

// NTSC-U/C: 0x00536ea8, the version the server reported when the client bound it.
static char _lfversion[kVersionSize];

// Copy the module arguments into the block, at most kLoadArgSize bytes of them, and return the
// count copied.
static inline int copyModuleArgs(char *pDest, int nArgs, const char *pArgs) {
    if (nArgs <= kLoadArgSize) {
        memcpy(pDest, pArgs, (size_t)nArgs);
        return nArgs;
    }
    memcpy(pDest, pArgs, kLoadArgSize);
    return kLoadArgSize;
}

// NTSC-U/C: 0x0031bba8, PAL: 0x003884c0
static int _lf_bind(void) {
    int nDelay;

    if (_bind_check >= 0) {
        return 0;
    }
    for (;;) {
        if (sceSifBindRpc(&g_loadFileClient, kLoadFileServerId, 0) < 0) {
            return -1;
        }
        if (g_loadFileClient.serve != NULL) {
            break;
        }
        for (nDelay = kBindRetryDelay; nDelay != -1; --nDelay) {
            __asm__ volatile("nop");
        }
    }
    _bind_check = 0;
    if (sceSifCallRpc(&g_loadFileClient,
                      kLoadFileFunctionVersion,
                      0,
                      NULL,
                      0,
                      &g_loadFileArgs,
                      kWordSize,
                      NULL,
                      NULL) < 0) {
        return kLoadFileErrorRpc;
    }
    memcpy(_lfversion, &g_loadFileArgs, kVersionSize);
    return 0;
}

// NTSC-U/C: 0x0031bca8, PAL: 0x003885c0
// Reports a mismatch only when the server matches neither accepted version and the two accepted
// versions also differ from each other.
static int loadFileVersionMismatch(void) {
    if (memcmp(_lfversion, g_abLoadFileLibraryVersion, kVersionSize) == 0) {
        return 0;
    }
    if (memcmp(_lfversion, g_pszLoadFileAltVersion, kVersionSize) == 0) {
        return 0;
    }
    return memcmp(g_abLoadFileLibraryVersion, g_pszLoadFileAltVersion, kVersionSize) != 0;
}

int sceSifStopModule(int modid, int args, const char *argp, int *result) {
    if (_lf_bind() < 0) {
        return kLoadFileErrorBind;
    }
    if (loadFileVersionMismatch() != 0) {
        return kLoadFileErrorVersion;
    }
    g_loadFileArgs.nModuleId = modid;
    if (argp != NULL) {
        g_loadFileArgs.nBufferArgLength = copyModuleArgs(g_loadFileArgs.abArgs, args, argp);
    } else {
        g_loadFileArgs.nBufferArgLength = 0;
    }
    if (sceSifCallRpc(&g_loadFileClient,
                      kLoadFileFunctionStop,
                      0,
                      &g_loadFileArgs,
                      sizeof(g_loadFileArgs),
                      &g_loadFileArgs,
                      kResultSize,
                      NULL,
                      NULL) < 0) {
        return kLoadFileErrorRpc;
    }
    *result = g_loadFileArgs.nModuleResult;
    return g_loadFileArgs.nResult;
}

int sceSifUnloadModule(int modid) {
    if (_lf_bind() < 0) {
        return kLoadFileErrorBind;
    }
    if (loadFileVersionMismatch() != 0) {
        return kLoadFileErrorVersion;
    }
    g_loadFileArgs.nModuleId = modid;
    if (sceSifCallRpc(&g_loadFileClient,
                      kLoadFileFunctionUnload,
                      0,
                      &g_loadFileArgs,
                      kWordSize,
                      &g_loadFileArgs,
                      kWordSize,
                      NULL,
                      NULL) < 0) {
        return kLoadFileErrorRpc;
    }
    return g_loadFileArgs.nResult;
}

int sceSifSearchModuleByName(const char *modulename) {
    if (_lf_bind() < 0) {
        return kLoadFileErrorBind;
    }
    if (loadFileVersionMismatch() != 0) {
        return kLoadFileErrorVersion;
    }
    strncpy(g_loadFileArgs.szPath, modulename, kLoadPathSize);
    g_loadFileArgs.szPath[kLoadPathSize - 1] = '\0';
    if (sceSifCallRpc(&g_loadFileClient,
                      kLoadFileFunctionSearchName,
                      0,
                      &g_loadFileArgs,
                      sizeof(g_loadFileArgs),
                      &g_loadFileArgs,
                      kWordSize,
                      NULL,
                      NULL) < 0) {
        return kLoadFileErrorRpc;
    }
    return g_loadFileArgs.nResult;
}

// NTSC-U/C: 0x0031c080, PAL: 0x00388998
static int
loadModule(const char *pPath, int nArgs, const char *pArgs, int *pResult, int nFunction) {
    if (_lf_bind() < 0) {
        return kLoadFileErrorBind;
    }
    if (loadFileVersionMismatch() != 0) {
        return kLoadFileErrorVersion;
    }
    strncpy(g_loadFileArgs.szPath, pPath, kLoadPathSize);
    g_loadFileArgs.szPath[kLoadPathSize - 1] = '\0';
    if (pArgs != NULL) {
        g_loadFileArgs.nArgLength = copyModuleArgs(g_loadFileArgs.abArgs, nArgs, pArgs);
    } else {
        g_loadFileArgs.abArgs[0] = '\0';
        g_loadFileArgs.nArgLength = 0;
    }
    if (sceSifCallRpc(&g_loadFileClient,
                      (unsigned int)nFunction,
                      0,
                      &g_loadFileArgs,
                      sizeof(g_loadFileArgs),
                      &g_loadFileArgs,
                      kResultSize,
                      NULL,
                      NULL) < 0) {
        return kLoadFileErrorRpc;
    }
    *pResult = g_loadFileArgs.nModuleResult;
    return g_loadFileArgs.nResult;
}

int sceSifLoadModule(const char *filename, int args, const char *argp) {
    int nResult;

    return loadModule(filename, args, argp, &nResult, kLoadFileFunctionModule);
}
