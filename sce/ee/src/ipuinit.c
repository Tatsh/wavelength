#include <eekernel.h>
#include <eeregs.h>
#include <eetypes.h>
#include <libipu.h>

enum {
    kDmaHoldAll = 0x10000,
    kChcrFromMemory = 1,
    // The reset bit of IPU_CTRL.
    kIpuCtrlReset = 0x40000000,
    kQuadwordSize = 16,
    kQuantizerMatrixQuadwords = 4,
    kInitFifoQuadwords = kQuantizerMatrixQuadwords + 1,
    kVqClutSize = 16,
    kVqClutQuadwords = 2,
};

// IPU_CMD codes. SETIQ loads the intra matrix, or the non-intra matrix with its bit 27 set.
enum {
    kIpuCommandBclr = 0x00000000,
    kIpuCommandSetIqIntra = 0x50000000,
    kIpuCommandSetIqNonIntra = 0x58000000,
    kIpuCommandSetVq = 0x60000000,
    kIpuCommandSetTh = 0x90000000,
};

// NTSC-U/C: 0x003c29d0, PAL: 0x00458e10, the identification tag of the library.
#ifdef VIDEO_STANDARD_PAL
static char g_acLibraryId[16] __attribute__((used)) = "PsIIlibipu  2700";
#else
static char g_acLibraryId[16] __attribute__((used)) = "PsIIlibipu  2600";
#endif

// The default matrices of SETIQ: the MPEG intra matrix in zigzag order and one quadword of the flat
// non-intra matrix, sent four times.
// NTSC-U/C: 0x003c29e0, PAL: 0x00458e20
static const union {
    unsigned char abMatrices[kInitFifoQuadwords][kQuadwordSize];
    u_long128 aQuads[kInitFifoQuadwords];
} g_abIpuInitFifoData = {{
    {8, 16, 16, 19, 16, 19, 22, 22, 22, 22, 22, 22, 26, 24, 26, 27},
    {27, 27, 26, 26, 26, 26, 27, 27, 27, 29, 29, 29, 34, 34, 34, 29},
    {29, 29, 27, 27, 29, 29, 32, 32, 34, 34, 37, 38, 37, 35, 35, 34},
    {35, 38, 38, 40, 40, 40, 48, 48, 46, 46, 56, 56, 58, 69, 69, 83},
    {16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16},
}};

// The colour table of SETVQ, sixteen RGB555 entries.
// NTSC-U/C: 0x003c2a30, PAL: 0x00458e70
static const union {
    unsigned short anColors[kVqClutSize];
    u_long128 aQuads[kVqClutQuadwords];
} g_IpuInitVqClut = {{
    0x0000,
    0x0421,
    0x0842,
    0x03e0,
    0x1084,
    0x14a5,
    0x18c6,
    0x1ce7,
    0x001f,
    0x2529,
    0x294a,
    0x7c00,
    0x318c,
    0x35ad,
    0x7fff,
    0x39ce,
}};

// NTSC-U/C: 0x0030ef48, PAL: 0x0037b4f8
// Writes the control register of the channel to the IPU with every channel held.
static void setD4_CHCR(unsigned int nValue) {
    (void)DIntr(); // Yes, the binary enables interrupts below whatever DIntr() reports.
    *D_ENABLEW = *D_ENABLER | kDmaHoldAll;
    *D4_CHCR = nValue;
    *D_ENABLEW = *D_ENABLER & ~(unsigned int)kDmaHoldAll;
    EIntr();
}

static inline void WaitIpu(void) {
    while ((int)*IPU_CTRL < 0) {
    }
}

void sceIpuInit(void) {
    int i;

    setD4_CHCR(kChcrFromMemory);
    *IPU_CTRL = kIpuCtrlReset;
    WaitIpu();
    *IPU_CMD = kIpuCommandBclr;
    WaitIpu();
    for (i = 0; i < kQuantizerMatrixQuadwords; ++i) {
        ee_store_quadword(IPU_IN_FIFO, g_abIpuInitFifoData.aQuads[i]);
    }
    for (i = 0; i < kQuantizerMatrixQuadwords; ++i) {
        ee_store_quadword(IPU_IN_FIFO, g_abIpuInitFifoData.aQuads[kQuantizerMatrixQuadwords]);
    }
    *IPU_CMD = kIpuCommandSetIqIntra;
    WaitIpu();
    *IPU_CMD = kIpuCommandSetIqNonIntra;
    WaitIpu();
    for (i = 0; i < kVqClutQuadwords; ++i) {
        ee_store_quadword(IPU_IN_FIFO, g_IpuInitVqClut.aQuads[i]);
    }
    *IPU_CMD = kIpuCommandSetVq;
    WaitIpu();
    *IPU_CMD = kIpuCommandSetTh;
    WaitIpu();
    *IPU_CTRL = kIpuCtrlReset;
    WaitIpu();
    *IPU_CMD = kIpuCommandBclr;
    WaitIpu();
}
