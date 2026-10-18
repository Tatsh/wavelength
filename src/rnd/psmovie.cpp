#include "rnd/psmovie.h"

#include <cstdint>

#include <eekernel.h>
#include <eeregs.h>
#include <ezmpeg/ipudma.h>
#include <libipu.h>

#include "os/debug.h"
#include "os/filepath.h"
#include "os/mem.h"
#include "rnd/pstex.h"
#include "rnd/rndmanager.h"

namespace {

// A movie of this many macroblocks or more is refused.
constexpr unsigned int kMaxBlocks = 0x800;
constexpr int kBlockShift = 4;
constexpr int kMovieBpp = 16;

// Each tag of the IPU input chain moves at most 0xffff quadwords.
constexpr int kIpuTagBytes = 0xffff0;
constexpr int kQwordBytes = 16;
constexpr int kIpuTagsAlign = 0x40;

// A macroblock decodes to 16 by 16 pixels of 16 bits, 32 quadwords.
constexpr int kBlockQwordsShift = 5;

// The access PsTex::LockBitmap() records for a movie frame.
constexpr int kLockForMovie = 4;

// D_ENABLEW suspends every channel while D4_CHCR stops.
constexpr unsigned int kEnableWHold = 0x10000;
constexpr unsigned int kChcrStart = 0x100;
constexpr unsigned int kDmaAddressMask = 0x0fffffff;

// IPU_CTRL and IPU_CMD values. A reset, BCLR, an FDEC of 32 bits that skips the start code after a
// picture, and IDEC with a quantiser scale of 1 and RGB16 output. The flags of getFlags() supply
// the control bits and the DTD bit of IDEC.
constexpr unsigned int kIpuCtrlReset = 0x40000000;
constexpr unsigned int kIpuCmdBclr = 0;
constexpr unsigned int kIpuCmdSkipStartCode = 0x40000020;
constexpr unsigned int kIpuCmdIdecRgb16 = 0x18010000;
constexpr unsigned int kFlagsDtd = 4;
constexpr int kFlagsDtdShift = 2;
constexpr int kIdecDtdShift = 24;
constexpr int kIpuCtrlFlagsShift = 16;

constexpr int kHandlerNone = -1;

} // namespace

int PsMovie::sHandlerId = kHandlerNone;
PsMovie *PsMovie::sActive;

PsMovie::PsMovie(const char *pszName)
    : RndObject(pszName), RndMovie(pszName), mData(nullptr), mIpuTags(nullptr), mNext(nullptr) {
}

void PsMovie::Replace(RndObject *pFrom, RndObject *pTo) {
    if (pFrom == mTex && mNext != nullptr) {
        // The binary reads mNext once. A queued movie spins here forever.
        while (mNext != nullptr) {
        }
    }
    RndMovie::Replace(pFrom, pTo);
}

int PsMovie::DmacHandler([[maybe_unused]] int nChannel) {
    sActive->FinishFrame();
    ExitHandler();
    return 0;
}

void PsMovie::Init() {
    TheManager.SetClassFactory(RndMovie::sClassName, PsMovie::New);
    sceIpuInit();
    sHandlerId = AddDmacHandler(DMAC_FROM_IPU, DmacHandler, 0);
}

void PsMovie::Terminate() {
    DisableDmac(DMAC_FROM_IPU);
    RemoveDmacHandler(DMAC_FROM_IPU, sHandlerId);
    TheManager.SetClassFactory(RndMovie::sClassName, RndMovie::New);
}

void PsMovie::RemoveHandler() {
    RemoveDmacHandler(DMAC_FROM_IPU, sHandlerId);
    sHandlerId = kHandlerNone;
}

void PsMovie::InstallHandler() {
    if (sHandlerId == kHandlerNone) {
        sHandlerId = AddDmacHandler(DMAC_FROM_IPU, DmacHandler, 0);
    }
}

void PsMovie::OpenData() {
    TheManager.GetResource(mFile, &mData, &mDataSize);
    if (mData == nullptr) {
        return;
    }
    RndTex *pTex = mTex;
    if (pTex == nullptr) {
        return;
    }

    const auto *pHeader = static_cast<const Header *>(mData);
    mWidth = pHeader->mWidth;
    mHeight = pHeader->mHeight;
    mFrame = pHeader->mNumFrames;
    mNumFrames = pHeader->mNumFrames;
    mNumBlocks = (mWidth >> kBlockShift) * (mHeight >> kBlockShift);
    if (static_cast<unsigned int>(mNumBlocks) >= kMaxBlocks) {
        CloseData();
        return;
    }

    FilePath file;
    file.Set("");
    pTex->SetBitmap(mWidth, mHeight, kMovieBpp, file, pTex->mMipMapK, 0);
    static_cast<PsTex *>(pTex)->BuildMoviePacket(true);

    const int nTags =
        (mDataSize - static_cast<int>(sizeof(Header)) + kIpuTagBytes - 1) / kIpuTagBytes;
    mIpuTags = static_cast<unsigned long long *>(
        PoolMemAlloc(nTags * kQwordBytes, "Movie buf", kIpuTagsAlign));
    setToIPUtags(mIpuTags,
                 static_cast<unsigned int>(reinterpret_cast<std::uintptr_t>(pHeader + 1)),
                 mDataSize);
}

void PsMovie::CloseData() {
    if (mNext != nullptr) {
        // The binary reads mNext once. A queued movie spins here forever.
        while (mNext != nullptr) {
        }
    }
    if (mData != nullptr) {
        PoolMemFree(mData);
        mData = nullptr;
    }
    if (mIpuTags != nullptr) {
        PoolMemFree(mIpuTags);
        mIpuTags = nullptr;
    }
}

void PsMovie::FinishFrame() {
    sceIpuSync(0, 0);
    mTex->UnlockBitmap();
    *IPU_CMD = kIpuCmdSkipStartCode;
    sceIpuSync(0, 0);
    sceIpuStopDMA(&mIpuEnv);
    sActive = mNext;
    if (mNext != nullptr) {
        mNext = nullptr;
        sActive->StartFrame();
    }
}

void PsMovie::StartFrame() {
    if (mFrame == 0) {
        *D_ENABLEW = kEnableWHold;
        *D4_CHCR = 0;
        *D_ENABLEW = 0;
        *IPU_CTRL = kIpuCtrlReset;
        sceIpuSync(0, 0);
        *IPU_CMD = kIpuCmdBclr;
        sceIpuSync(0, 0);
        setDMAtoIPU(static_cast<unsigned int>(reinterpret_cast<std::uintptr_t>(mIpuTags)));
        *IPU_CTRL = 0;
        sceIpuStopDMA(&mIpuEnv);
    }
    sceIpuRestartDMA(&mIpuEnv);

    RndBitmap *pBitmap = mTex->LockBitmap(kLockForMovie);
    *D3_MADR = static_cast<unsigned int>(reinterpret_cast<std::uintptr_t>(pBitmap->mPixels)) &
               kDmaAddressMask;
    *D3_QWC = static_cast<unsigned int>(mNumBlocks) << kBlockQwordsShift;
    *D3_CHCR = kChcrStart;

    const unsigned int nFlags = getFlags();
    *IPU_CTRL = (nFlags & ~kFlagsDtd) << kIpuCtrlFlagsShift;
    *IPU_CMD = (((nFlags >> kFlagsDtdShift) & 1) << kIdecDtdShift) | kIpuCmdIdecRgb16;
}

int PsMovie::SetFrameSelf(float fFrame) {
    if (mData == nullptr || mTex == nullptr) {
        return 1;
    }
    const int nRemainder = static_cast<int>(fFrame) % mNumFrames;
    const int nFrame = nRemainder > -1 ? nRemainder : nRemainder + mNumFrames;
    if (nFrame == mFrame) {
        return 1;
    }

    while (mNext != nullptr) {
        DebugError("Blocked on movie decode\n");
    }
    if (nFrame < mFrame) {
        mFrame = 0;
    } else {
        mFrame = mFrame + 1;
    }

    DisableDmac(DMAC_FROM_IPU);
    if (sActive == nullptr) {
        sActive = this;
        StartFrame();
    } else {
        PsMovie *pTail = sActive;
        while (pTail->mNext != nullptr) {
            pTail = pTail->mNext;
        }
        pTail->mNext = this;
    }
    EnableDmac(DMAC_FROM_IPU);
    return 1;
}
