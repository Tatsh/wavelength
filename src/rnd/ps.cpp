#include "rnd/ps.h"

#include <cstdint>
#include <cstring>

#include <eekernel.h>
#include <eeregs.h>
#include <libdma.h>
#include <libgraph.h>

#include "gfx/renderstats.h"
#include "os/debug.h"
#include "os/joypad.h"
#include "os/mem.h"
#include "os/string.h"
#include "os/timer.h"
#include "rnd/dmapacket.h"
#include "rnd/pscam.h"
#include "rnd/psenviron.h"
#include "rnd/psmat.h"
#include "rnd/psmesh.h"
#include "rnd/psmovie.h"
#include "rnd/psmultimesh.h"
#include "rnd/psparticlesys.h"
#include "rnd/pstex.h"
#include "rnd/rndbitmap.h"
#include "rnd/rndcam.h"
#include "rnd/rndmanager.h"
#include "rnd/rndmat.h"
#include "rnd/rndmesh.h"
#include "rnd/rndmultimesh.h"
#include "rnd/rndparticlesys.h"
#include "rnd/rndtex.h"
#include "script/scriptfunction.h"

namespace {

// The scratchpad, the two halves the frame packet alternates between, and the bit that makes a
// main memory address uncached.
constexpr std::uintptr_t kScratchpadAddress = 0x70000000;
constexpr int kScratchpadSize = 0x4000;
constexpr std::uintptr_t kPacketHalf0 = 0x70000020;
constexpr std::uintptr_t kPacketHalf1 = 0x70002010;
constexpr std::uintptr_t kUncachedBit = 0x20000000;
constexpr int kPacketQwords = 0x1ff;
constexpr int kQwordShift = 4;

// The MFIFO ring in main memory, its D_RBSR mask, and the MFD field of D_CTRL that sends it to
// VIF1.
constexpr int kMfifoSize = 0x80000;
constexpr unsigned int kMfifoBytes = kMfifoSize;
constexpr unsigned int kMfifoRingMask = 0x7fff0;
constexpr unsigned char kCtrlMfdMask = 0xf3;
constexpr unsigned char kCtrlMfdVif1 = 8;
constexpr unsigned int kChcrChainStart = 0x105;

// The DMA tags the frame packet is closed with, and the VIF code that starts Vu1Init.
constexpr int kDmaTagIdCnt = 1;
constexpr int kDmaTagIdEnd = 7;
constexpr unsigned int kVifMscalInit = 0x14000508;
constexpr int kVifCodeWord = 0;

// The display sizes the GS shows, in pixels.
constexpr int kWidth640 = 640;
constexpr int kWidth512 = 512;
constexpr int kWidth384 = 384;
constexpr int kWidth256 = 256;
constexpr int kHeight512 = 512;
constexpr int kHeight448 = 448;
constexpr int kHeight256 = 256;
constexpr int kHeight224 = 224;

// sceGsResetGraph() arguments.
constexpr short kResetModeFull = 0;
constexpr short kInterlace = 1;
constexpr short kVideoNtsc = 2;
constexpr short kVideoPal = 3;
constexpr short kFieldModeField = 0;

// Display depths, and the GS storage modes of the frame and depth buffers for each.
constexpr int kBpp16 = 16;
constexpr int kBpp24 = 24;
constexpr int kBpp32 = 32;
constexpr short kPsmCt32 = 0x00;
constexpr short kPsmCt24 = 0x01;
constexpr short kPsmCt16 = 0x02;
constexpr short kPsmZ24 = 0x31;
constexpr short kPsmZ16 = 0x32;
constexpr short kPsmZ16S = 0x3a;
constexpr int kDepthBytes16 = 2;
constexpr int kDepthBytes24 = 3;
constexpr short kZTestGreaterEqual = 3;
constexpr short kClearOn = 1;
constexpr int kPageWordsShift = 11;
constexpr int kBytesPerWordShift = 2;

// The GS registers the renderer writes, and the fields it sets.
constexpr int kGsRegTex01 = 0x06;
constexpr int kGsRegClamp1 = 0x08;
constexpr int kGsRegTex11 = 0x14;
constexpr int kGsRegXyOffset1 = 0x18;
constexpr int kGsRegPrModeCont = 0x1a;
constexpr int kGsRegTexA = 0x3b;
constexpr int kGsRegScissor1 = 0x40;
constexpr int kGsRegAlpha1 = 0x42;
constexpr int kGsRegDimX = 0x44;
constexpr int kGsRegDthe = 0x45;
constexpr int kGsRegColClamp = 0x46;
constexpr int kGsRegTest1 = 0x47;
constexpr int kGsRegPabe = 0x49;
constexpr int kGsRegFrame1 = 0x4c;
constexpr int kGsRegZbuf1 = 0x4e;
constexpr unsigned long long kTestZTestAlways = 0x20000;
constexpr unsigned long long kTestZTestMask = 0x60000;
constexpr unsigned long long kFrameSelectMask = 0x3f3f01ff;
constexpr unsigned long long kXyOffsetSelectMask = 0x0000ffff0000ffffULL;
constexpr unsigned long long kDthe = 1;
constexpr unsigned long long kTexATa1 = 0x80ULL << 32;
constexpr unsigned long long kZbufZmsk = 1ULL << 32;
constexpr unsigned long long kAlphaBlendSrc = 0x44;
constexpr unsigned long long kAlphaBlendFix = 0x64;
constexpr unsigned long long kAlphaMask = 0xff;
constexpr unsigned long long kAlphaFixMask = 0xff000000ffULL;
constexpr int kAlphaFixShift = 32;
constexpr unsigned long long kDimXAdditive = 0x1212303012120303ULL;
constexpr unsigned long long kDimXMask = 0x7777777777777777ULL;
constexpr unsigned long long kTex1MminLinear = 0x40;
constexpr unsigned long long kTex1MminMask = 0x1c0;
constexpr unsigned long long kClampBothAxes = 5;
constexpr unsigned long long kClampMask = 0xf;

// TEX0 of the blur. The blur reads the buffer on display as a 1024 by 1024 decal.
constexpr unsigned long long kFrameFbpMask = 0x1ff;
constexpr int kFrameFbwShift = 16;
constexpr int kFramePsmShift = 24;
constexpr unsigned long long kFrameFieldMask = 0x3f;
constexpr int kTex0TbpShift = 5;
constexpr unsigned long long kTex0LogSize1024 = 10;
constexpr unsigned long long kTex0TfxDecal = 1;

// The GS TEX0 register by field.
struct Tex0 {
    unsigned long long mTbp0 : 14;
    unsigned long long mTbw : 6;
    unsigned long long mPsm : 6;
    unsigned long long mTw : 4;
    unsigned long long mTh : 4;
    unsigned long long mTcc : 1;
    unsigned long long mTfx : 2;
    unsigned long long mCbp : 14;
    unsigned long long mCpsm : 4;
    unsigned long long mCsm : 1;
    unsigned long long mCsa : 5;
    unsigned long long mCld : 3;
};

// GIF tags. A+D sends one register for each quadword, and a register list packs two.
constexpr int kGifFlgPacked = 0;
constexpr int kGifFlgRegList = 1;
constexpr int kGifAdRegs = 1;
constexpr unsigned long long kGifRegAd = 0xe;
constexpr int kRectRegs = 4;
constexpr unsigned long long kRectRegList = 0x5510;
constexpr int kBlurRegs = 6;
constexpr unsigned long long kBlurRegList = 0x535310;
constexpr int kGlyphRegs = 8;
constexpr unsigned long long kGlyphRegList = 0x55555510;
constexpr unsigned long long kPrimLineStrip = 2;
constexpr unsigned long long kPrimSpriteBlended = 0x46;
constexpr unsigned long long kPrimSpriteTexturedBlended = 0x156;

// Primitive coordinates are offset by the centre of the GS space and have four fractional bits.
constexpr float kGsCoordinateCentre = 2048.0f;
constexpr int kGsCoordinateOrigin = 0x8000;
constexpr int kSubpixelShift = 4;
constexpr int kHalfPixel = 8;
constexpr int kHalfScreenShift = 3;
constexpr float kSubpixels = 16.0f;
constexpr int kGsYShift = 16;
constexpr unsigned long long kZFar = 0xffffULL << 32;
constexpr unsigned long long kZBlur = 10000ULL << 32;

// RGBAQ fields. A colour component is scaled to 255, and an alpha to 128, with Q fixed at 1.0.
constexpr float kColorScale = 255.0f;
constexpr float kAlphaScale = 128.0f;
constexpr int kRgbaGreenShift = 8;
constexpr int kRgbaBlueShift = 16;
constexpr int kRgbaAlphaShift = 24;
constexpr unsigned long long kRgbaqQOne = 0x3f800000ULL << 32;
constexpr unsigned long long kRgbaqBlur = 0x3f80000064808080ULL;

// The line font. A glyph is a line strip of six points in a unit cell, and every character but a
// newline advances by one and a half cells.
constexpr int kGlyphPoints = 6;
constexpr int kGlyphFloats = kGlyphPoints * 2;
constexpr int kGlyphPairs = 3;
constexpr int kPairFloats = 4;
constexpr int kLetterGlyphs = 31;
constexpr int kLowercaseGlyphs = 26;
constexpr int kPunctuationGlyphs = 20;
constexpr int kPunctuationGlyphOffset = 8;
constexpr double kGlyphAdvance = 1.5;
constexpr int kPointX = 0;
constexpr int kPointY = 1;
constexpr int kNextPointX = 2;
constexpr int kNextPointY = 3;

// The cell is the unit square, and the six points of each glyph follow one another.
// NTSC-U/C: 0x003afd10, PAL: 0x0042eba8
constexpr float kLineFont[][kGlyphFloats] = {
    {0.0f, 1.0f, 0.5f, 0.0f, 1.0f, 1.0f, 0.0f, 0.5f, 0.0f, 0.5f, 0.0f, 0.5f},
    {0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.5f, 1.0f, 1.0f, 0.0f, 1.0f},
    {1.0f, 0.0f, 0.0f, 0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f},
    {0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f},
    {1.0f, 0.0f, 0.0f, 0.0f, 0.5f, 0.5f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f},
    {0.0f, 1.0f, 0.0f, 0.5f, 1.0f, 0.5f, 0.0f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f},
    {0.5f, 0.0f, 0.0f, 0.5f, 0.5f, 1.0f, 1.0f, 0.5f, 1.0f, 0.5f, 1.0f, 0.5f},
    {0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.5f, 1.0f, 0.5f, 1.0f, 0.0f, 1.0f, 1.0f},
    {0.0f, 0.0f, 1.0f, 0.0f, 0.5f, 0.0f, 0.5f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f},
    {1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.5f, 0.0f, 0.5f, 0.0f, 0.5f, 0.0f, 0.5f},
    {0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.5f, 1.0f, 0.0f, 0.0f, 0.5f, 1.0f, 1.0f},
    {0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f},
    {0.0f, 1.0f, 0.0f, 0.0f, 0.5f, 0.5f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f},
    {0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f},
    {0.5f, 0.0f, 0.0f, 0.5f, 0.5f, 1.0f, 1.0f, 0.5f, 0.5f, 0.0f, 0.5f, 0.0f},
    {0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.5f, 0.0f, 0.5f, 0.0f, 0.5f},
    {0.5f, 1.0f, 1.0f, 0.5f, 0.5f, 0.0f, 0.0f, 0.5f, 0.5f, 1.0f, 1.0f, 1.0f},
    {0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.5f, 1.0f, 1.0f, 1.0f, 1.0f},
    {1.0f, 0.0f, 0.0f, 0.5f, 1.0f, 0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f},
    {0.0f, 0.0f, 0.5f, 0.0f, 0.5f, 1.0f, 0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f},
    {0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f},
    {0.0f, 0.0f, 0.5f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f},
    {0.0f, 0.0f, 0.0f, 1.0f, 0.5f, 0.5f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f},
    {0.0f, 0.0f, 1.0f, 1.0f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f},
    {0.0f, 0.0f, 0.5f, 0.5f, 0.5f, 1.0f, 0.5f, 0.5f, 1.0f, 0.0f, 1.0f, 0.0f},
    {0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f},
    {0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.5f, 1.0f, 0.5f, 1.0f, 0.5f, 1.0f},
    {0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f},
    {0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.5f, 1.0f, 0.5f, 1.0f, 0.5f, 1.0f},
    {0.0f, 0.5f, 0.5f, 0.0f, 1.0f, 0.5f, 1.0f, 0.5f, 1.0f, 0.5f, 1.0f, 0.5f},
    {0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f},
    {0.5f, 0.0f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f},
    {0.5f, 0.0f, 0.0f, 0.5f, 0.5f, 1.0f, 0.5f, 1.0f, 0.5f, 1.0f, 0.5f, 1.0f},
    {0.5f, 0.0f, 1.0f, 0.5f, 0.5f, 1.0f, 0.5f, 1.0f, 0.5f, 1.0f, 0.5f, 1.0f},
    {0.4f, 0.4f, 0.4f, 0.6f, 0.6f, 0.6f, 0.6f, 0.4f, 0.4f, 0.4f, 0.4f, 0.4f},
    {0.5f, 0.0f, 0.5f, 1.0f, 0.5f, 0.5f, 0.0f, 0.5f, 1.0f, 0.5f, 1.0f, 0.5f},
    {0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f},
    {0.0f, 0.5f, 1.0f, 0.5f, 1.0f, 0.5f, 1.0f, 0.5f, 1.0f, 0.5f, 1.0f, 0.5f},
    {0.5f, 0.5f, 0.5f, 1.0f, 0.5f, 1.0f, 0.5f, 1.0f, 0.5f, 1.0f, 0.5f, 1.0f},
    {0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f},
    {0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f},
    {0.5f, 0.0f, 0.5f, 1.0f, 0.5f, 1.0f, 0.5f, 1.0f, 0.5f, 1.0f, 0.5f, 1.0f},
    {0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.5f, 0.0f, 0.5f, 0.0f, 1.0f, 1.0f, 1.0f},
    {0.0f, 0.0f, 1.0f, 0.0f, 0.5f, 0.5f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f},
    {0.0f, 0.0f, 0.0f, 0.5f, 1.0f, 0.5f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f},
    {1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.5f, 1.0f, 0.5f, 1.0f, 1.0f, 0.0f, 1.0f},
    {0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.5f, 0.0f, 0.5f, 0.0f, 0.5f},
    {0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f},
    {0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f},
    {1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.5f, 1.0f, 0.5f, 1.0f, 0.5f},
    {0.0f, 0.5f, 1.0f, 0.5f, 1.0f, 0.5f, 1.0f, 0.5f, 1.0f, 0.5f, 1.0f, 0.5f},
};

// ShowModal() draws the message in white on dark blue, from (30, 30) in cells of 7 pixels. A
// state of 2 is cleared by the button at bit 11.
constexpr Color kModalBackground{0.0f, 0.0f, 0.5f, 1.0f};
constexpr Color kModalText{1.0f, 1.0f, 1.0f, 1.0f};
constexpr float kModalMargin = 30.0f;
constexpr float kModalCellSize = 7.0f;
constexpr int kModalStateWaiting = 2;
constexpr int kModalCancelButton = 0x800;

// The heap index the `toggle_heapnum` command cycles through.
// NTSC-U/C: 0x003afd0c, PAL: 0x0042eba4
int sHeapIndex;

// The image ScreenDump() reads the frame buffer through.
// NTSC-U/C: 0x0043b990
sceGsStoreImage sStoreImage;

// The display buffers, read and written through an uncached address.
// NTSC-U/C: 0x0043bf00
alignas(64) GsDoubleBuffer sDoubleBuffer;

inline void PushQuadword(unsigned long long qwLow, unsigned long long qwHigh) {
    DmaPacket &packet = DmaPacket::shared();
    packet.mCursor->mDword[0] = qwLow;
    packet.mCursor->mDword[1] = qwHigh;
    ++packet.mCursor;
}

inline unsigned long long PackColor(const Color &color) {
    const int nRed = static_cast<int>(color.r * kColorScale);
    const int nGreen = static_cast<int>(color.g * kColorScale);
    const int nBlue = static_cast<int>(color.b * kColorScale);
    const int nAlpha = static_cast<int>(color.a * kAlphaScale);
    return static_cast<unsigned long long>(nRed) |
           (static_cast<unsigned long long>(nGreen) << kRgbaGreenShift) |
           (static_cast<unsigned long long>(nBlue) << kRgbaBlueShift) |
           (static_cast<unsigned long long>(nAlpha) << kRgbaAlphaShift) | kRgbaqQOne;
}

inline unsigned long long PackXy(int nX, int nY) {
    return static_cast<unsigned long long>(nX) | (static_cast<unsigned long long>(nY) << kGsYShift);
}

inline DmaQuadword *PacketHalf(std::uintptr_t address) {
    return reinterpret_cast<DmaQuadword *>(address);
}

inline unsigned char *Scratchpad() {
    return reinterpret_cast<unsigned char *>(kScratchpadAddress);
}

// The draw environment of one of the two buffers.
inline const sceGsDrawEnv1 &DrawEnv(const GsDoubleBuffer *pBuffers, int nHalf) {
    return pBuffers->mHalves[nHalf].mDraw;
}

// NTSC-U/C: 0x0020fba8, PAL: 0x002189c0
void ToggleHeapNum([[maybe_unused]] DataArray *pCommand, [[maybe_unused]] void *pUserData) {
    if (++sHeapIndex >= MemNumHeaps()) {
        sHeapIndex = 0;
    }
}

// NTSC-U/C: 0x002102b8, PAL: 0x002190d0
int DrawVSyncCB([[maybe_unused]] int nCause) {
    gVCount = gVCount + 1;
    ExitHandler();
    return 0;
}

// NTSC-U/C: 0x00210308, PAL: 0x00219120
void ScreenDumpCommand([[maybe_unused]] DataArray *pCommand, [[maybe_unused]] void *pUserData) {
    ThePs.ScreenDump("screendump");
}

} // namespace

Ps ThePs;
RndRenderer *TheRnd = &ThePs;
RenderStats g_renderStats;
volatile int gVCount;

// The unit's static initialiser at NTSC-U/C: 0x00211f00, PAL: 0x0021ad18, constructs ThePs and
// sets TheRnd. Its global constructor is at NTSC-U/C: 0x00211f70, PAL: 0x0021ad88, and its global
// destructor at NTSC-U/C: 0x00211f90, PAL: 0x0021ada8.

Ps::Ps()
    : mScratchpadCopy(nullptr), mSwapVblank(0), mDrawBuffer(0), mReserved458(0),
      mDisplayBuffers(nullptr), mReserved460(5), mReserved464(0), mReserved468(0), mBlurActive(0),
      mBlurRect{0.0f, 0.0f, 1.0f, 1.0f}, mBlurAmount(0.78f), mBlurCount(0), mInFrame(0),
      mScreenDumpCount(1) {
    ScriptFunction::Register(ToggleHeapNum, "toggle_heapnum", nullptr);
}

Ps::~Ps() {
}

void Ps::ModalCallback(int *pState, const char *pszText) {
    ThePs.ShowModal(pState, pszText);
}

void Ps::ShowModal(int *pState, const char *pszText) {
    DebugPrint("%s\n", pszText);
    const Color savedColor = mClearColor;
    const int bInFrame = mInFrame;
    const int bShowStats = mShowStats;
    const int bShowRate = mShowRate;
    const int bShowTimers = mShowTimers;
    if (bInFrame != 0) {
        EndFrame();
    }

    mShowTimers = 0;
    mShowRate = 0;
    mShowStats = 0;
    SetClearColor(kModalBackground);
    BeginFrame();
    DrawString(pszText,
               Vector2{kModalMargin, kModalMargin},
               Vector2{kModalCellSize, kModalCellSize},
               kModalText);
    EndFrame();
    SwapBuffers();
    mDrawBuffer = mDrawBuffer == 0;

    const int nButtons = JoypadWait();
    if (*pState == kModalStateWaiting && (nButtons & kModalCancelButton) != 0) {
        *pState = 0;
    }
    mShowStats = bShowStats;
    mShowRate = bShowRate;
    mShowTimers = bShowTimers;
    SetClearColor(savedColor);
    if (bInFrame != 0) {
        BeginFrame();
    }
}

float Ps::DrawString(const char *pszText,
                     const Vector2 &pos,
                     const Vector2 &charSize,
                     const Color &color) {
    SetGsReg(kGsRegTest1, kTestZTestAlways, kTestZTestMask);
    DmaPacket &packet = DmaPacket::shared();
    packet.OpenGifTag(kGifFlgRegList, kGlyphRegs, kGlyphRegList, 0);

    int nX = static_cast<int>(
        (pos.x + kGsCoordinateCentre - static_cast<float>(mScreenWidth >> 1)) * kSubpixels);
    float fY = pos.y;
    const int nCellWidth = static_cast<int>(charSize.x * kSubpixels);
    const int nCellHeight = static_cast<int>(charSize.y * kSubpixels);
    int nY = static_cast<int>((fY + kGsCoordinateCentre - static_cast<float>(mScreenHeight >> 1)) *
                              kSubpixels);

    while (*pszText != '\0') {
        const char c = *pszText;
        if (c == '\n') {
            Flush();
            ++pszText;
            packet.OpenGifTag(kGifFlgRegList, kGlyphRegs, kGlyphRegList, 0);
            nX = static_cast<int>(
                (pos.x + kGsCoordinateCentre - static_cast<float>(mScreenWidth >> 1)) * kSubpixels);
            fY += charSize.y + charSize.y;
            nY += nCellHeight * 2;
            continue;
        }

        int nGlyph = -1;
        if (static_cast<unsigned int>(c - 'A') < kLetterGlyphs) {
            nGlyph = c - 'A';
        } else if (static_cast<unsigned int>(c - 'a') < kLowercaseGlyphs) {
            nGlyph = c - 'a';
        } else if (static_cast<unsigned int>(c - '\'') < kPunctuationGlyphs) {
            nGlyph = c - kPunctuationGlyphOffset;
        }

        if (nGlyph >= 0) {
            PushQuadword(kPrimLineStrip, PackColor(color));
            const float fCellWidth = static_cast<float>(nCellWidth);
            const float fCellHeight = static_cast<float>(nCellHeight);
            const float *pPoints = kLineFont[nGlyph];
            for (int i = 0; i < kGlyphPairs; ++i, pPoints += kPairFloats) {
                const int nX0 = nX + static_cast<int>(pPoints[kPointX] * fCellWidth);
                const int nY0 = nY + static_cast<int>(pPoints[kPointY] * fCellHeight);
                const int nX1 = nX + static_cast<int>(pPoints[kNextPointX] * fCellWidth);
                const int nY1 = nY + static_cast<int>(pPoints[kNextPointY] * fCellHeight);
                PushQuadword(PackXy(nX0, nY0) | kZFar, PackXy(nX1, nY1) | kZFar);
            }
        }
        ++pszText;
        nX += static_cast<int>(nCellWidth * kGlyphAdvance);
    }
    Flush();
    return fY + (charSize.y + charSize.y);
}

int Ps::GetReservedVramWords() const {
    return (mDisplayBuffers->mZbp << kPageWordsShift) +
           ((mScreenWidth * mScreenHeight * mDepthBytes) >> kBytesPerWordShift);
}

void Ps::Init() {
    RndRenderer::Init();
    ScriptFunction::Register(ScreenDumpCommand, "screen_dump", nullptr);

    if (mScreenWidth >= kWidth640) {
        mScreenWidth = kWidth640;
    } else if (mScreenWidth >= kWidth512) {
        mScreenWidth = kWidth512;
    } else if (mScreenWidth >= kWidth384) {
        mScreenWidth = kWidth384;
    } else {
        mScreenWidth = kWidth256;
    }
    if (mScreenHeight >= kHeight512) {
        mNtsc = 0;
        mScreenHeight = kHeight512;
    } else if (mScreenHeight >= kHeight448) {
        mScreenHeight = kHeight448;
        mNtsc = 1;
    } else if (mScreenHeight >= kHeight256) {
        mNtsc = 0;
        mScreenHeight = kHeight256;
    } else {
        mScreenHeight = kHeight224;
        mNtsc = 1;
    }

    mMfifo = PoolMemAlloc(kMfifoSize, "MFIFO", kMfifoSize);
    std::memset(reinterpret_cast<void *>(reinterpret_cast<std::uintptr_t>(mMfifo) | kUncachedBit),
                0,
                kMfifoSize);
    DmaPacket::shared().Init(PacketHalf(kPacketHalf0), DMAC_FROM_SPR, 1);
    DmaPacket::shared().OpenDmaTag();

    PsMesh::Init();
    PsCam::Init();
    PsMat::Init();
    PsTex::Init();
    PsMovie::Init();
    PsEnviron::Init();
    TheManager.SetClassFactory(RndParticleSys::sClassName, PsParticleSys::New);
    TheManager.SetClassFactory(RndMultiMesh::sClassName, PsMultiMesh::New);
    TheDebug.mModalCallback = ModalCallback;
    RestoreAfterMovie();
}

void Ps::Terminate() {
    TheManager.SetClassFactory(RndMesh::sClassName, RndMesh::New);
    RndCam::sCurrent = nullptr;
    TheManager.SetClassFactory(RndCam::sClassName, RndCam::New);
    TheManager.SetClassFactory(RndMat::sClassName, RndMat::New);
    TheManager.SetClassFactory(RndTex::sClassName, RndTex::New);
    PsMovie::Terminate();
    PsEnviron::Terminate();
    TheManager.SetClassFactory(RndParticleSys::sClassName, RndParticleSys::New);
    TheManager.SetClassFactory(RndMultiMesh::sClassName, RndMultiMesh::New);
    sceGsSyncVCallback(nullptr);
    TheDebug.mModalCallback = nullptr;
    sceGsSyncPath(0, 0);
    PoolMemFree(mMfifo); // Yes, the binary does not clear mMfifo.
}

void Ps::ReleaseForMovie() {
    SaveScratchpad();
    sceGsSyncPath(0, 0);
    PsMovie::RemoveHandler();
}

void Ps::RestoreAfterMovie() {
    std::memset(mGsRegs, 0, sizeof(mGsRegs));
    sceGsResetPath();
    sceDmaReset(1);
    sceGsSyncPath(0, 0);
    sceGsSyncV(0);
    sceGsResetGraph(
        kResetModeFull, kInterlace, mNtsc != 0 ? kVideoNtsc : kVideoPal, kFieldModeField);

    short nPsm;
    short nZPsm;
    switch (mScreenBpp) {
    case kBpp24:
        mDepthBytes = kDepthBytes16;
        nPsm = kPsmCt24;
        nZPsm = kPsmZ16S;
        break;
    case kBpp16:
        nPsm = kPsmCt16;
        nZPsm = kPsmZ16;
        mDepthBytes = kDepthBytes16;
        break;
    case kBpp32:
        mDepthBytes = kDepthBytes24;
        nPsm = kPsmCt32;
        nZPsm = kPsmZ24;
        break;
    default:
        DebugPrint("Unsupported video mode\n");
        mDepthBytes = kDepthBytes24;
        mScreenBpp = kBpp32;
        nPsm = kPsmCt32;
        nZPsm = kPsmZ24;
        break;
    }

    mDisplayBuffers = reinterpret_cast<GsDoubleBuffer *>(
        reinterpret_cast<std::uintptr_t>(&sDoubleBuffer) | kUncachedBit);
    mDisplayBuffers->SetDefaults(static_cast<short>(mScreenWidth),
                                 static_cast<short>(mScreenHeight),
                                 nPsm,
                                 kZTestGreaterEqual,
                                 nZPsm,
                                 kClearOn);
    SetClearColor(mClearColor);
    mSwapVblank = gVCount + 1;
    sceGsSyncVCallback(DrawVSyncCB);
    ClearAndFlip();
    RestoreScratchpad();
    PsTex::ReserveDisplayMemory();
    PsMovie::InstallHandler();

    DmaPacket packet;
    packet.mBase = nullptr; // Yes, the binary clears the base that Init() then replaces.
    packet.Init(g_vu0MicrocodeChain, DMAC_VIF0, 0);
    packet.Send(DmaPacket::kModeChainTagged, 0);
    packet.Init(g_vu1MicrocodeChain, DMAC_VIF1, 0);
    packet.Send(DmaPacket::kModeChainTagged, 0);

    DmaPacket &frame = DmaPacket::shared();
    frame.mCursor->mWord[kVifCodeWord] = kVifMscalInit;
    frame.mCursor->mWord[1] = 0;
    frame.mCursor->mWord[2] = 0;
    frame.mCursor->mWord[3] = 0;
    ++frame.mCursor;
    SetGsReg(kGsRegPabe, 0, 0);
    SetGsReg(kGsRegTexA, kTexATa1, 0);
    packet.Wait();

    const auto nMfifo = static_cast<unsigned int>(reinterpret_cast<std::uintptr_t>(mMfifo));
    *D_RBOR = nMfifo;
    *D_RBSR = kMfifoRingMask;
    *D_CTRL_LOW = static_cast<unsigned char>((*D_CTRL_LOW & kCtrlMfdMask) | kCtrlMfdVif1);
    *D1_TADR = static_cast<unsigned int>(reinterpret_cast<std::uintptr_t>(mMfifo));
    *D8_MADR = static_cast<unsigned int>(reinterpret_cast<std::uintptr_t>(mMfifo));
}

void Ps::SaveScratchpad() {
    DmaPacket::shared().Wait();
    if (mScratchpadCopy != nullptr) {
        PoolMemFree(mScratchpadCopy);
        mScratchpadCopy = nullptr;
    }
    mScratchpadCopy = PoolMemAlloc(kScratchpadSize, "scratch", 0);
    std::memcpy(mScratchpadCopy, Scratchpad(), kScratchpadSize);
}

void Ps::RestoreScratchpad() {
    if (mScratchpadCopy == nullptr) {
        return;
    }
    std::memcpy(Scratchpad(), mScratchpadCopy, kScratchpadSize);
    if (mScratchpadCopy != nullptr) { // Yes, the binary tests the copy again.
        PoolMemFree(mScratchpadCopy);
        mScratchpadCopy = nullptr;
    }
}

void Ps::SetGsReg(int nReg, unsigned long long qwValue, unsigned long long qwMask) {
    if (qwMask != 0) {
        const unsigned long long qwCurrent = mGsRegs[nReg];
        const unsigned long long qwBits = qwValue & qwMask;
        if ((qwCurrent & qwMask) == qwBits) {
            return;
        }
        mGsRegs[nReg] = (qwCurrent & ~qwMask) | qwBits;
    } else {
        mGsRegs[nReg] = qwValue;
    }

    DmaPacket &packet = DmaPacket::shared();
    if (packet.mGifTag == nullptr || packet.mGifTag->mDword[1] != kGifRegAd) {
        packet.OpenGifTag(kGifFlgPacked, kGifAdRegs, kGifRegAd, 0);
    }
    PushQuadword(mGsRegs[nReg], static_cast<unsigned long long>(nReg));
}

void Ps::SetClearColor(const Color &color) {
    mClearColor = color;
    mDisplayBuffers->mHalves[0].mClear.rgbaqWord = PackColor(color);
    mDisplayBuffers->mHalves[1].mClear.rgbaqWord = PackColor(color);
}

int Ps::PacketQwordsFree() const {
    const DmaPacket &packet = DmaPacket::shared();
    return kPacketQwords - static_cast<int>(packet.mCursor - packet.mBase);
}

void Ps::Flush() {
    DmaPacket &packet = DmaPacket::shared();
    if (packet.mCursor == packet.mBase) {
        return;
    }
    packet.CloseDmaTag(kDmaTagIdCnt, 0, 0);
    packet.Wait();

    const auto nBytes = static_cast<unsigned int>(packet.mCursor - packet.mBase) << kQwordShift;
    unsigned int nFree;
    do {
        const unsigned int nRead = *D1_TADR;
        const unsigned int nWrite = *D8_MADR;
        nFree = nRead == nWrite ? kMfifoBytes : ((nRead + kMfifoBytes - nWrite) & kMfifoRingMask);
    } while (!(nBytes < nFree));
    packet.Send(DmaPacket::kModeNormal, 0);

    if (packet.mBase == PacketHalf(kPacketHalf0)) {
        packet.Init(PacketHalf(kPacketHalf1), DMAC_FROM_SPR, 1);
    } else {
        packet.Init(PacketHalf(kPacketHalf0), DMAC_FROM_SPR, 1);
    }
    packet.OpenDmaTag();
}

void Ps::RestoreFrameBufferTarget() {
    const sceGsDrawEnv1 &env = DrawEnv(mDisplayBuffers, mDrawBuffer != 0 ? 1 : 0);
    SetGsReg(kGsRegFrame1, env.frame1, kFrameSelectMask);
    SetGsReg(kGsRegXyOffset1, env.xyoffset1, kXyOffsetSelectMask);
    SetGsReg(kGsRegDthe, mScreenBpp == kBpp16, kDthe);
}

void Ps::SwapBuffers() {
    sceGsSyncPath(0, 0);
    for (MemHandle *pHandle : mFrameHandles) {
        MemHandle::Free(pHandle);
    }
    mFrameHandles.clear();

    while (gVCount < mSwapVblank) {
    }
    mSwapVblank = gVCount + 1;
    mDisplayBuffers->PutDispBuffer(mDrawBuffer, 1);
    mDrawBuffer = mDrawBuffer == 0;
    mDisplayBuffers->PutDrawBufferSmall(mDrawBuffer, 1);

    const sceGsDrawEnv1 &env = DrawEnv(mDisplayBuffers, mDrawBuffer != 0 ? 1 : 0);
    mGsRegs[kGsRegFrame1] = env.frame1;
    mGsRegs[kGsRegZbuf1] = env.zbuf1;
    mGsRegs[kGsRegXyOffset1] = env.xyoffset1;
    mGsRegs[kGsRegScissor1] = env.scissor1;
    mGsRegs[kGsRegPrModeCont] = env.prmodecont;
    mGsRegs[kGsRegColClamp] = env.colclamp;
    mGsRegs[kGsRegDthe] = env.dthe;
    mGsRegs[kGsRegTest1] = env.test1;
}

void Ps::BeginFrame() {
    ++mFrameCount;
    SwapBuffers();
    *D1_TADR = *D8_MADR;
    *D1_CHCR = kChcrChainStart;
    std::memset(&g_renderStats, 0, sizeof(g_renderStats));
    RndCam::sDefault->Draw();
    PsMat::SelectDefault();
    PsTex::BeginFrame();
    for (auto &timer : Timer::sTimers) {
        timer.Reset();
    }
    mInFrame = 1;
}

void Ps::EndFrame() {
    if (mBlurActive != 0) {
        DrawBlur();
    }
    DmaPacket::shared().CloseDmaTag(kDmaTagIdEnd, 0, 0);
    Flush();
    mInFrame = 0;
}

void Ps::ClearAndFlip() {
    mDisplayBuffers->PutDrawBufferSmall(mDrawBuffer, 1);
    SwapBuffers();
    mDrawBuffer = mDrawBuffer == 0;
}

void Ps::DumpStats() {
    DebugPrint("\nSTATS");
    DebugPrint(" points:%d\n", g_renderStats.mPoints);
    DebugPrint(" draws:%d\n", g_renderStats.mDraws);
    DebugPrint(" culled_tris:%d\n", g_renderStats.mCulledTris);
    DebugPrint(" tris:%d\n", g_renderStats.mTris);
    DebugPrint(" clipped_tris:%d\n", g_renderStats.mClippedTris);
    DebugPrint(" culled_lines:%d\n", g_renderStats.mCulledLines);
    DebugPrint(" lines:%d\n", g_renderStats.mLines);
    DebugPrint(" sprites:%d\n", g_renderStats.mSprites);
    DebugPrint(" culled_sprites:%d\n", g_renderStats.mCulledSprites);
    DebugPrint(" verts:%d\n", g_renderStats.mVerts);
    DebugPrint(" tags:%d\n", g_renderStats.mTags);
    DebugPrint(" packets:%d\n", g_renderStats.mPackets);
    DebugPrint(" mats:%d\n", g_renderStats.mMats);
    DebugPrint(" lit_verts:%d\n", g_renderStats.mLitVerts);
    DebugPrint(" tex_swaps:%d\n", g_renderStats.mTexSwaps);
    DebugPrint(" tex_loads:%d\n", g_renderStats.mTexLoads);
    DebugPrint(" free_ranges:%d\n", static_cast<int>(PsTex::sFreeRanges.size()));
    DebugPrint(" free_blocks:%d\n", PsTex::FreeBlockCount());
}

void Ps::DrawRect(const Rect &rect, const Color &color) {
    SetGsReg(kGsRegTest1, kTestZTestAlways, kTestZTestMask);
    SetGsReg(kGsRegAlpha1, kAlphaBlendSrc, kAlphaMask);
    DmaPacket::shared().OpenGifTag(kGifFlgRegList, kRectRegs, kRectRegList, 0);
    PushQuadword(kPrimSpriteBlended, PackColor(color));

    const int nY = static_cast<int>(
        (rect.y + kGsCoordinateCentre - static_cast<float>(mScreenHeight >> 1)) * kSubpixels);
    const int nHeight = static_cast<int>(rect.h * kSubpixels);
    const int nX = static_cast<int>(
        (rect.x + kGsCoordinateCentre - static_cast<float>(mScreenWidth >> 1)) * kSubpixels);
    const int nWidth = static_cast<int>(rect.w * kSubpixels);
    PushQuadword(PackXy(nX, nY) | kZFar, PackXy(nX + nWidth, nY + nHeight) | kZFar);
    Flush();
}

void Ps::DrawBlur() {
    SetGsReg(kGsRegTest1, kTestZTestAlways, kTestZTestMask);
    SetGsReg(kGsRegZbuf1, kZbufZmsk, kZbufZmsk);
    const auto qwFix = static_cast<unsigned long long>(mBlurAmount * kAlphaScale);
    SetGsReg(kGsRegAlpha1, (qwFix << kAlphaFixShift) | kAlphaBlendFix, kAlphaFixMask);
    SetGsReg(kGsRegDimX, kDimXAdditive, kDimXMask);

    // The texture is the buffer on display.
    const unsigned long long qwFrame = DrawEnv(mDisplayBuffers, mDrawBuffer != 0 ? 0 : 1).frame1;
    Tex0 tex0; // Yes, the binary leaves the CLUT bits of this TEX0 uninitialised.
    tex0.mTbp0 = (qwFrame & kFrameFbpMask) << kTex0TbpShift;
    tex0.mTbw = (qwFrame >> kFrameFbwShift) & kFrameFieldMask;
    tex0.mPsm = (qwFrame >> kFramePsmShift) & kFrameFieldMask;
    tex0.mTw = kTex0LogSize1024;
    tex0.mTh = kTex0LogSize1024;
    tex0.mTcc = 1;
    tex0.mTfx = kTex0TfxDecal;
    unsigned long long qwTex0;
    std::memcpy(&qwTex0, &tex0, sizeof(qwTex0));
    SetGsReg(kGsRegTex01, qwTex0, 0);
    SetGsReg(kGsRegTex11, kTex1MminLinear, kTex1MminMask);
    SetGsReg(kGsRegClamp1, kClampBothAxes, kClampMask);

    DmaPacket::shared().OpenGifTag(kGifFlgRegList, kBlurRegs, kBlurRegList, 0);
    PushQuadword(kPrimSpriteTexturedBlended, kRgbaqBlur);

    const float fHeight = static_cast<float>(mScreenHeight);
    const float fWidth = static_cast<float>(mScreenWidth);
    const int nOriginY = kGsCoordinateOrigin - (mScreenHeight << kHalfScreenShift);
    const int nOriginX = kGsCoordinateOrigin - (mScreenWidth << kHalfScreenShift);
    const int nTop = static_cast<int>(mBlurRect[1] * fHeight) << kSubpixelShift;
    const int nLeft = static_cast<int>(mBlurRect[0] * fWidth) << kSubpixelShift;
    PushQuadword(PackXy(nLeft + mBlurCount + kHalfPixel, nTop + mBlurCount + kHalfPixel),
                 PackXy(nOriginX + nLeft, nOriginY + nTop) | kZBlur);

    const int nBottom = static_cast<int>((mBlurRect[1] + mBlurRect[3]) * fHeight) << kSubpixelShift;
    const int nRight = static_cast<int>((mBlurRect[0] + mBlurRect[2]) * fWidth) << kSubpixelShift;
    PushQuadword(PackXy(nRight - mBlurCount + kHalfPixel, nBottom - mBlurCount + kHalfPixel),
                 PackXy(nOriginX + nRight, nOriginY + nBottom) | kZBlur);
}

void Ps::ScreenDump(const char *pszName) {
    const int bInFrame = mInFrame;
    if (bInFrame != 0) {
        EndFrame();
    }

    RndBitmap screen;
    screen.Create(
        mScreenWidth, mScreenHeight, 0, mScreenBpp, RndBitmap::kOrderRGBA | RndBitmap::kOrderGs);
    sceGsSyncPath(0, 0);
    *D_CTRL_LOW = static_cast<unsigned char>(*D_CTRL_LOW & kCtrlMfdMask);

    const unsigned long long qwFrame = DrawEnv(mDisplayBuffers, mDrawBuffer != 0 ? 0 : 1).frame1;
    sceGsSetDefStoreImage(&sStoreImage,
                          static_cast<short>((qwFrame & kFrameFbpMask) << kTex0TbpShift),
                          static_cast<short>((qwFrame >> kFrameFbwShift) & kFrameFieldMask),
                          static_cast<short>((qwFrame >> kFramePsmShift) & kFrameFieldMask),
                          0,
                          0,
                          static_cast<short>(mScreenWidth),
                          static_cast<short>(mScreenHeight));
    FlushCache(WRITEBACK_DCACHE);
    sceGsExecStoreImage(&sStoreImage, screen.mPixels);

    RndBitmap rgb;
    rgb.Create(mScreenWidth, mScreenHeight, 0, kBpp24, 0);
    rgb.Blit(screen, 0, 0, 0, 0, mScreenWidth, mScreenHeight);
    rgb.SaveBmp(FormatString("%s_%d.bmp", pszName, mScreenDumpCount++));
    *D_CTRL_LOW = static_cast<unsigned char>((*D_CTRL_LOW & kCtrlMfdMask) | kCtrlMfdVif1);

    if (bInFrame != 0) {
        mDrawBuffer = mDrawBuffer == 0;
        BeginFrame();
    }
}

void Ps::ReleaseAfterFrame(MemHandle *pHandle) {
    if (pHandle != nullptr) {
        mFrameHandles.push_back(pHandle);
    }
}
