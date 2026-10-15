#include <stddef.h>
#include <stdint.h>

#include <sifdev.h>
#include <sifrpc.h>

// The client of the IOP heap server.

enum {
    kHeapServerId = 0x80000003,
};

enum {
    kHeapFunctionAlloc = 1,
    kHeapFunctionFree = 2,

    kBindRetryDelay = 0x100000,
};

// NTSC-U/C: 0x003c486c, PAL: 0x0045acb4, negative until the client is bound.
static int _bind = -1;

// NTSC-U/C: 0x00536ac0
static sceSifClientData g_heapClient __attribute__((aligned(64)));

// NTSC-U/C: 0x00536b00. SIF DMA needs the 64-byte alignment retail gives it.
static int g_nHeapReceive __attribute__((aligned(64)));

// NTSC-U/C: 0x00536b40. SIF DMA needs the 64-byte alignment retail gives it.
static union {
    unsigned int nSize;
    void *pAddr;
} g_heapSend __attribute__((aligned(64)));

int sceSifInitIopHeap(void) {
    int nDelay;

    for (;;) {
        if (sceSifBindRpc(&g_heapClient, kHeapServerId, 0) < 0) {
            return -1;
        }
        if (g_heapClient.serve != NULL) {
            break;
        }
        for (nDelay = kBindRetryDelay; nDelay != -1; --nDelay) {
            __asm__ volatile("nop");
        }
    }
    _bind = 0;
    return 0;
}

void *sceSifAllocIopHeap(unsigned int size) {
    if (_bind < 0) {
        return NULL;
    }
    g_heapSend.nSize = size;
    if (sceSifCallRpc(&g_heapClient,
                      kHeapFunctionAlloc,
                      0,
                      &g_heapSend,
                      sizeof(g_heapSend.nSize),
                      &g_nHeapReceive,
                      sizeof(g_nHeapReceive),
                      NULL,
                      NULL) < 0) {
        return NULL;
    }
    return (void *)(uintptr_t)g_nHeapReceive;
}

int sceSifFreeIopHeap(void *addr) {
    if (_bind < 0) {
        return 0;
    }
    g_heapSend.pAddr = addr;
    if (sceSifCallRpc(&g_heapClient,
                      kHeapFunctionFree,
                      0,
                      &g_heapSend,
                      sizeof(g_heapSend.pAddr),
                      &g_nHeapReceive,
                      sizeof(g_nHeapReceive),
                      NULL,
                      NULL) < 0) {
        return -1;
    }
    return g_nHeapReceive;
}

int sceSifFreeSysMemory(void *addr) {
    return sceSifFreeIopHeap(addr);
}
