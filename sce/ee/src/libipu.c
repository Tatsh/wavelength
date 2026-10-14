#include <eekernel.h>
#include <eeregs.h>
#include <libipu.h>

enum {
    // The hold bit of D_ENABLER and D_ENABLEW that suspends every DMA channel.
    kDmaHoldAll = 0x10000,
    // The start bit of a channel control register.
    kChcrStart = 0x100,
    // The direction bit of a channel control register, set for a transfer from memory.
    kChcrFromMemory = 1,
    kChcrToMemory = 0,
    // The output FIFO count field and the busy bit of IPU_CTRL.
    kIpuCtrlOutputFifoCount = 0xf0,
    kIpuCtrlBusyShift = 31,
    // The fields of IPU_BP: the bit position, the input FIFO count, and the FIFO pointer.
    kIpuBpBitPosition = 0x7f,
    kIpuBpInputFifoShift = 8,
    kIpuBpInputFifoMask = 0xf,
    kIpuBpFifoPointerShift = 16,
    kIpuBpFifoPointerMask = 3,
    kQuadwordShift = 4,
    // The BCLR command code of IPU_CMD.
    kIpuCommandBclr = 0,
};

// The sync modes of sceIpuSync().
enum {
    kIpuSyncWait = 0,
    kIpuSyncPoll = 1,
};

// NTSC-U/C: 0x0030ebb8, PAL: 0x0037b168
// Writes the control register of the channel from the IPU with every channel held.
static void setD3_CHCR(unsigned int nValue) {
    int nInterrupts = DIntr();

    *D_ENABLEW = *D_ENABLER | kDmaHoldAll;
    *D3_CHCR = nValue;
    *D_ENABLEW = *D_ENABLER & ~(unsigned int)kDmaHoldAll;
    if (nInterrupts != 0) {
        EIntr();
    }
}

// NTSC-U/C: 0x0030ec30, PAL: 0x0037b1e0
// Writes the control register of the channel to the IPU with every channel held.
static void setD4_CHCR(unsigned int nValue) {
    int nInterrupts = DIntr();

    *D_ENABLEW = *D_ENABLER | kDmaHoldAll;
    *D4_CHCR = nValue;
    *D_ENABLEW = *D_ENABLER & ~(unsigned int)kDmaHoldAll;
    if (nInterrupts != 0) {
        EIntr();
    }
}

void sceIpuStopDMA(sceIpuDmaEnv *env) {
    setD4_CHCR(kChcrFromMemory);
    env->d4madr = *D4_MADR;
    env->d4tadr = *D4_TADR;
    env->d4qwc = *D4_QWC;
    env->d4chcr = *D4_CHCR;
    while ((*IPU_CTRL & kIpuCtrlOutputFifoCount) != 0) {
    }
    setD3_CHCR(kChcrToMemory);
    env->d3madr = *D3_MADR;
    env->d3qwc = *D3_QWC;
    env->d3chcr = *D3_CHCR;
    env->ipubp = *IPU_BP;
    env->ipuctrl = *IPU_CTRL;
}

void sceIpuRestartDMA(sceIpuDmaEnv *env) {
    unsigned int nBitPosition = env->ipubp & kIpuBpBitPosition;
    // The quadwords the IPU had taken into its FIFO go back to the channel.
    unsigned int nFifoQuadwords = ((env->ipubp >> kIpuBpFifoPointerShift) & kIpuBpFifoPointerMask) +
                                  ((env->ipubp >> kIpuBpInputFifoShift) & kIpuBpInputFifoMask);
    unsigned int nQwc = env->d4qwc + nFifoQuadwords;
    unsigned int nMadr = env->d4madr - (nFifoQuadwords << kQuadwordShift);

    if (env->d3madr != 0 && env->d3qwc != 0) {
        *D3_MADR = env->d3madr;
        *D3_QWC = env->d3qwc;
        setD3_CHCR(env->d3chcr | kChcrStart);
    }
    while ((int)*IPU_CTRL < 0) {
    }
    *IPU_CMD = kIpuCommandBclr | nBitPosition;
    while ((int)*IPU_CTRL < 0) {
    }
    if (nMadr != 0 && nQwc != 0) {
        *D4_MADR = nMadr;
        *D4_TADR = env->d4tadr;
        *D4_QWC = nQwc;
        setD4_CHCR(env->d4chcr | kChcrStart);
    }
}

int sceIpuSync(int mode, unsigned short timeout) {
    (void)timeout;
    if (mode == kIpuSyncWait) {
        while ((int)*IPU_CTRL < 0) {
        }
        return 0;
    }
    if (mode == kIpuSyncPoll) {
        return (int)(*IPU_CTRL >> kIpuCtrlBusyShift);
    }
    return 0;
}
