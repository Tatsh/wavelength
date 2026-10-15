#include "rt/rtmemory.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// NTSC-U/C: 0x003b61a4, PAL: 0x00435064
static rtAllocFunc g_pfnAllocHook;
// NTSC-U/C: 0x003b61a8, PAL: 0x00435068
static rtFreeFunc g_pfnFreeHook;

int RtAllocBlock(void **ppBlock, int nSize) {
    if (g_pfnAllocHook == NULL) {
        *ppBlock = malloc(nSize);
        if (*ppBlock == NULL) {
            return 1;
        }
    } else {
        *ppBlock = g_pfnAllocHook(nSize);
        if (*ppBlock == NULL) {
            return 1;
        }
    }
    return 0;
}

int RtAllocZeroedBlock(void **ppBlock, int nSize) {
    int nResult;

    if (ppBlock == NULL) {
        return 1;
    }
    *ppBlock = NULL;
    nResult = RtAllocBlock(ppBlock, nSize);
    if (nResult != 0) {
        return nResult;
    }
    if (*ppBlock == NULL) {
        return 1;
    }
    memset(*ppBlock, 0, nSize);
    return 0;
}

int RtFreeBlock(void **ppBlock) {
    if (*ppBlock != NULL) {
        if (g_pfnFreeHook == NULL) {
            free(*ppBlock);
            *ppBlock = NULL;
        } else {
            g_pfnFreeHook(*ppBlock);
            *ppBlock = NULL;
        }
    }
    return 0;
}

int RtAllocAlignedBlock(RtAlignedBlock *pBlock, unsigned int nSize, unsigned short nAlign) {
    int nResult;

    if (pBlock == NULL || nSize == 0) {
        return 1;
    }
    pBlock->pBlock = NULL;
    pBlock->pAligned = NULL;
    if (nAlign >= 2) {
        nSize = nSize + nAlign - 1;
        // Yes, the binary adds a further partial alignment step on top of the room it reserved.
        nSize += nAlign - (nSize % nAlign);
    }
    nResult = RtAllocBlock(&pBlock->pBlock, nSize);
    if (nResult != 0) {
        return nResult;
    }
    return RtAlignAddress(&pBlock->pAligned, pBlock->pBlock, nAlign);
}

int RtAllocAlignedZeroedBlock(RtAlignedBlock *pBlock, unsigned int nSize, unsigned short nAlign) {
    int nResult;

    if (pBlock == NULL || nSize == 0) {
        return 1;
    }
    pBlock->pBlock = NULL;
    pBlock->pAligned = NULL;
    nResult = RtAllocAlignedBlock(pBlock, nSize, nAlign);
    if (nResult != 0) {
        return nResult;
    }
    if (pBlock->pAligned == NULL) {
        return 1;
    }
    memset(pBlock->pAligned, 0, nSize);
    return 0;
}

int RtFreeAlignedBlock(RtAlignedBlock *pBlock) {
    int nResult;

    if (pBlock == NULL) {
        return 1;
    }
    if (pBlock->pBlock != NULL) {
        nResult = RtFreeBlock(&pBlock->pBlock);
        if (nResult != 0) {
            return nResult;
        }
        pBlock->pBlock = NULL;
    }
    pBlock->pAligned = NULL;
    return 0;
}

int RtSetAllocHooks(rtAllocFunc pfnAlloc, rtFreeFunc pfnFree) {
    g_pfnAllocHook = pfnAlloc;
    g_pfnFreeHook = pfnFree;
    return 0;
}

int RtAlignAddress(void **ppAligned, void *pBlock, unsigned short nAlign) {
    uintptr_t uRemainder;

    if (ppAligned == NULL || pBlock == NULL) {
        return 1;
    }
    *ppAligned = pBlock;
    if (nAlign >= 2) {
        uRemainder = (uintptr_t)pBlock % nAlign;
        if (uRemainder != 0) {
            *ppAligned = (void *)((uintptr_t)pBlock + (nAlign - uRemainder));
        }
    }
    return 0;
}
