#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <eekernel.h>
#include <eeregs.h>
#include <eetypes.h>
#include <libipu.h>
#include <libmpeg.h>
#include <libmpeg_mc.h>

enum {
    kAlignMask = 3,
    kPictureAlignMask = 63,
    kPhysicalAddressMask = 0x0fffffff,
    kUncachedSegment = 0x20000000,
    kContextSize = 4288,
    kErrorMessageSize = 256,
    kMaxStreamCallbacks = 64,
    kStreamTableSize = kMaxStreamCallbacks * 24,
    kStreamTableAlign = 8,
    kFrameBufferAlign = 64,
    kSlotCount = 7,
    kPictureCount = 9,
    kFrameCentreOffsetCount = 3,
    kStreamTypeCount = 10,
    kMacroblockSize = 384,
    kMacroblockQuadwords = kMacroblockSize / 16,
    kMcPredictionSlots = 4,
    kMcKernelVariants = 8,
    kMcStagingSize = 4 * kMacroblockSize,
    kMcColumnSize = 2 * kMacroblockSize,
    kMcChromaOffset = 256,
    kMcChromaOutputOffset = 512,
    kIpuSpinLimit = 5000,
    kQuantMatrixQuadwords = 4,
    kMaxVerticalSize = 2800,
    kCscChunkMacroblocks = 1023,
    kCscChunkQuadwords = kCscChunkMacroblocks * 64,
    kCscChunkBytes = kCscChunkMacroblocks * 1024,
    kCscDirectLimit = 1024,
    kToIpuChunkQuadwords = 0xffff,
    kToIpuChunkBytes = kToIpuChunkQuadwords * 16,
    kMacroblockToIpuQuadwords = 24,
};

// Picture types, structures, and the macroblock and motion types of ISO/IEC 13818-2.
enum {
    kPictureI = 1,
    kPictureP = 2,
    kPictureB = 3,
    kPictureD = 4,
    kTopField = 1,
    kBottomField = 2,
    kFramePicture = 3,
    kMbIntra = 1,
    kMbPattern = 2,
    kMbMotionBackward = 4,
    kMbMotionForward = 8,
    kMbQuant = 16,
    kMcField = 1,
    kMcFrame = 2,
    kMc16x8 = 2,
    kMcDmv = 3,
    kMvField = 0,
    kMvFrame = 1,
};

// Start codes.
enum {
    kPictureStartCode = 0x100,
    kSliceStartCodeMin = 0x101,
    kSliceStartCodeCount = 175,
    kUserDataStartCode = 0x1b2,
    kSequenceHeaderCode = 0x1b3,
    kExtensionStartCode = 0x1b5,
    kSequenceEndCode = 0x1b7,
    kGroupStartCode = 0x1b8,
    kProgramEndCode = 0x1b9,
    kPackStartCode = 0x1ba,
    kSystemHeaderStartCode = 0x1bb,
    kPacketStartPrefix = 1,
    kExtensionIdCount = 11,
};

// IPU_CMD codes and the fields of IPU_CTRL.
enum {
    kIpuCmdBclr = 0x00000000,
    kIpuCmdBdec = 0x20000000,
    kIpuCmdVdec = 0x30000000,
    kIpuCmdFdec = 0x40000000,
    kIpuCmdSetIqIntra = 0x50000000,
    kIpuCmdSetIqNonIntra = 0x58000000,
    kIpuCmdCsc = 0x70000000,
    kIpuCmdShift = 28,
    kIpuCmdFdecIndex = 4,
    kIpuVdecTableShift = 26,
    kIpuBdecIntraShift = 27,
    kIpuBdecDcResetShift = 26,
    kIpuBdecDctTypeShift = 25,
    kIpuBdecQscShift = 16,
    kIpuCtrlErrorCode = 0x4000,
    kIpuCtrlIdpShift = 16,
    kIpuCtrlIdpMask = 0x30000,
    kIpuCtrlAlternateScanShift = 20,
    kIpuCtrlAlternateScan = 0x100000,
    kIpuCtrlIntraVlcShift = 21,
    kIpuCtrlIntraVlc = 0x200000,
    kIpuCtrlQstShift = 22,
    kIpuCtrlQst = 0x400000,
    kIpuCtrlMpeg1Shift = 23,
    kIpuCtrlMpeg1 = 0x800000,
    kIpuCtrlPctShift = 24,
    kIpuCtrlPctMask = 0x7000000,
    kIpuCtrlReset = 0x40000000,
    kIpuBpPositionMask = 31,
    kIpuBpByteMask = 7,
    kIpuTopBits = 32,
};

// The address bit that selects the scratchpad for a DMA channel.
static const unsigned int kScratchpadFlag = 0x80000000;

// IPU_CTRL with its busy bit set and its error bit clear.
static const unsigned int kIpuCtrlBusyOrError = 0x80004000;
static const unsigned int kIpuCtrlBusy = 0x80000000;

// VDEC tables.
enum {
    kVlcMacroblockAddressIncrement = 0,
    kVlcMacroblockType = 1,
    kVlcMotionCode = 2,
    kVlcDmVector = 3,
    kMbaiStuffing = 34,
    kMbaiEscape = 35,
    kMbaiEscapeIncrement = 33,
    kMbaiStuffingCode = 15,
};

// DMA channel settings.
enum {
    kDmaHoldAll = 0x10000,
    kChcrStart = 0x100,
    kChcrFromMemory = 1,
    kChcrChain = 4,
    kDmaTagRef = 3,
    kDmaTagRefe = 0,
    kDmaTagIdShift = 28,
    kDStatFromIpu = 8,
    kDStatToIpu = 16,
};

// Values of slice() and pictureData() results.
enum {
    kSliceDone = 0,
    kSliceNext = 1,
    kSliceError = 2,
    kSliceResync = 3,
    kSliceAbort = 4,
};

// Output states and the states of the pending time stamp.
enum {
    kOutputIdle = 0,
    kOutputDecoding = 1,
    kOutputShown = 2,
    kPendingPtsArmed = 1,
    kPendingPtsReady = 2,
};

// One stream entry, 24 bytes. A packet matches when its key masked with mMask equals mKey.
typedef struct {
    unsigned long long mKey;
    unsigned long long mMask;
    sceMpegCallback mCallback;
    void *mData;
} StreamEntry;

typedef struct {
    sceMpegCallback mCallback;
    void *mData;
} MpegSlot;

// The bump allocator over the rest of the work area. mDynamic marks the end of the allocations
// that survive a new sequence.
typedef struct {
    unsigned char *mBase;
    int mSize;
    unsigned char *mCurrent;
    unsigned char *mDynamic;
} MpegAlalc;

// One picture, 104 bytes. The sequence setup writes the first five words, and the reference
// update copies the picture header state into the rest.
typedef struct {
    unsigned char *mData; // Macroblocks in column order, uncached.
    int mWidth;
    int mHeight;
    int mMbWidth;
    int mMbHeight;   // Also the macroblock stride of a column.
    int mReserved14; // +0x14
    long long mPts;
    long long mDts;
    int mDecoded;
    int mPictureCodingType;
    int mPictureStructure;
    int mProgressiveSequence;
    int mProgressiveFrame;
    int mTopFieldFirst;
    int mRepeatFirstField;
    int mFrameCentreHorizontalOffset[kFrameCentreOffsetCount];
    int mFrameCentreVerticalOffset[kFrameCentreOffsetCount];
    int mDisplayHorizontalSize;
    int mDisplayVerticalSize;
    int mReserved64; // +0x64
} MpegPicture;

typedef struct {
    MpegPicture *mForward;
    MpegPicture *mBackward;
    MpegPicture *mCurrent;
    MpegPicture *mAux;
} MpegPictureSet;

// One macroblock in flight, 320 bytes. Two alternate so the kernels of one macroblock run while
// the IPU decodes the next.
typedef struct {
    unsigned char *mStaging;      // The reference macroblocks land here.
    unsigned char *mCoefficients; // The IPU writes the block here.
    unsigned char *mRefLeft[kMcPredictionSlots];
    unsigned char *mRefRight[kMcPredictionSlots];
    MpegMcKernel mLumaKernels[kMcPredictionSlots];
    MpegMcKernel mChromaKernels[kMcPredictionSlots];
    MpegMcDescriptor mLuma[kMcPredictionSlots];
    MpegMcDescriptor mChroma[kMcPredictionSlots];
    unsigned char *mOutput; // The macroblock in the frame.
    int mPredictionCount;
    int mIntra;
    int mCoded; // Never read.
    int mDmaPending;
    int mNotCoded;
} MpegMcBuffer;

// The decoder state at the start of the work area, 4288 bytes.
struct MpegContext {
    int mIsEnd;
    int mPendingOutput;
    int mOutputState;
    MpegSlot mSlots[kSlotCount];
    StreamEntry *mStreamTable;
    int mStreamCount;
    sceIpuDmaEnv mDmaEnv;
    int mUseDefaultPtsGap;
    int mReserved74; // +0x74
    unsigned long long mDefaultPtsGap;
    int mLastPts;
    int mReserved84; // +0x84
    unsigned long long mDisplayFieldCount;
    int mOddGapPictures;
    int mDecodeLimits[3];
    int mDecodeCounts[3];
    int mFrameCountBase;
    int mConvertColours;
    int mFrameCentreHorizontalOffset[kFrameCentreOffsetCount];
    int mFrameCentreVerticalOffset[kFrameCentreOffsetCount];
    int mDisplayHorizontalSize;
    int mDisplayVerticalSize;
    int mFirstFieldStructure;
    unsigned char *mPictureAddress;
    int mOutputWidth;
    int mOutputHeight;
    int mOutputMacroblocks;
    int mForceBrokenLink;
    int mReservedEC; // +0xec
    long long mPendingPts;
    int mPendingPtsState;
    unsigned char *mFrameBuffers[3];
    MpegAlalc mAlalc;
    int mPictureCount;
    int mFault;
    int mSecondField;
    int mHorizontalSize;
    int mVerticalSize;
    int mMbWidth;
    int mMbHeight;
    int mBitRate;
    int mVbvBufferSize;
    int mProgressiveSequence;
    int mChromaFormat;
    int mMatrixCoefficients;
    int mSequenceDisplayHorizontalSize;
    int mSequenceDisplayVerticalSize;
    int mPictureCodingType;
    int mFullPelForwardVector;
    int mForwardFCode;
    int mFullPelBackwardVector;
    int mBackwardFCode;
    int mFCode[2][2];
    int mPictureStructure;
    int mTopFieldFirst;
    int mFramePredFrameDct;
    int mConcealmentMotionVectors;
    int mRepeatFirstField;
    int mProgressiveFrame;
    int mPictureFrameCentreHorizontalOffset[kFrameCentreOffsetCount];
    int mPictureFrameCentreVerticalOffset[kFrameCentreOffsetCount];
    int mClosedGop;
    int mBrokenLink;
    int mPictureNumber;
    int mResetDcPredictor;
    int mQuantizerScaleCode;
    MpegPictureSet mFrames;
    MpegPictureSet mTopFields;
    MpegPictureSet mBottomFields;
    MpegPicture mPictures[kPictureCount];
    MpegMcBuffer mMcBuffers[2];
    int mMcCurrent;
    int mPictureResetWord; // Cleared per picture and never read.
    int mTopStale;         // Nonzero once a command invalidates mTopBits.
    unsigned char *mMcOutput;
    int mPictureReady;
    int mReserved824; // +0x824
    long long mPts;
    long long mDts;
    unsigned int mTopBits;
    int mTopBitCount;
    int mLoadIntraQuantiserMatrix;
    int mLoadNonIntraQuantiserMatrix;
    int mMpeg2;
    int mGopPictureBase;
    int mMaxPictureNumber;
    int mNewGop;
    sceMpeg *mMpeg;
    unsigned char abReserved85C[0x10a0 - 0x85c];         // +0x85c
    int mAbort;                                          // Never written. +0x10a0
    unsigned char abReserved10A4[kContextSize - 0x10a4]; // +0x10a4
};

typedef struct MpegContext MpegContext;

// Two DMA tags per prediction, each in its own quadword.
typedef struct {
    unsigned long long mTag;
    unsigned long long mUnused;
} MpegDmaTag;

// NTSC-U/C: 0x003c27e0, PAL: 0x00458b70
static const unsigned long long g_aMpegStreamIdTable[kStreamTypeCount][2] = {
    {0xe000000000ULL, 0xff00000000ULL},
    {0xbdffc00000ULL, 0xffffffffffULL},
    {0xbdffa00000ULL, 0xffffffffffULL},
    {0xbdffa10000ULL, 0xffffffffffULL},
    {0xbdff900000ULL, 0xffffffffffULL},
    {0xc000000000ULL, 0xff00000000ULL},
    {0xbd80000000ULL, 0xffff000000ULL},
    {0xbda0000000ULL, 0xffff000000ULL},
    {0xbd88000000ULL, 0xffff000000ULL},
    {0xbd90000000ULL, 0xffff000000ULL},
};

// NTSC-U/C: 0x003c2880, sent to the IPU by DMA.
static const unsigned char g_abDefaultIntraQuantMatrix[64] __attribute__((aligned(16))) = {
    8,  16, 16, 19, 16, 19, 22, 22, 22, 22, 22, 22, 26, 24, 26, 27, 27, 27, 26, 26, 26, 26,
    27, 27, 27, 29, 29, 29, 34, 34, 34, 29, 29, 29, 27, 27, 29, 29, 32, 32, 34, 34, 37, 38,
    37, 35, 35, 34, 35, 38, 38, 40, 40, 40, 48, 48, 46, 46, 56, 56, 58, 69, 69, 83,
};

// NTSC-U/C: 0x003c28c0, sent to the IPU by DMA.
static const unsigned char g_abDefaultNonIntraQuantMatrix[64] __attribute__((aligned(16))) = {
    16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
    16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
    16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
};

// NTSC-U/C: 0x003c2900, PAL: 0x00458cc0, indexed by averaging, horizontal half pel, and vertical
// half pel.
static const MpegMcKernel g_apfnMpegLumaKernels[kMcKernelVariants] = {
    sceMpegMcLumaCopy,
    sceMpegMcLumaHalfV,
    sceMpegMcLumaHalfH,
    sceMpegMcLumaHalfHV,
    sceMpegMcLumaAverageCopy,
    sceMpegMcLumaAverageHalfV,
    sceMpegMcLumaAverageHalfH,
    sceMpegMcLumaAverageHalfHV,
};

// NTSC-U/C: 0x003c2920, PAL: 0x00458ce0
static const MpegMcKernel g_apfnMpegChromaKernels[kMcKernelVariants] = {
    sceMpegMcChromaCopy,
    sceMpegMcChromaHalfV,
    sceMpegMcChromaHalfH,
    sceMpegMcChromaHalfHV,
    sceMpegMcChromaAverageCopy,
    sceMpegMcChromaAverageHalfV,
    sceMpegMcChromaAverageHalfH,
    sceMpegMcChromaAverageHalfHV,
};

// NTSC-U/C: 0x003c2940, PAL: 0x00458d00, nonzero for the IPU commands that leave IPU_TOP stale.
static const int g_anIpuCmdTypeTable[] = {1, 1, 0, 0, 0, 1, 1, 1, 1, 1};

// NTSC-U/C: 0x003c2968, the fields a picture shows by its repeat_first_field, top_field_first,
// progressive_frame, and progressive_sequence bits.
static const unsigned int g_anMpegDisplayFieldCount[] = {
    2, 0, 2, 0, 2, 3, 2, 3, 0, 0, 0, 0, 2, 4, 0, 6};

// NTSC-U/C: 0x0052d3c0, the DMA chain that stages the reference macroblocks.
static MpegDmaTag g_aMpegRefDmaChain[2 * kMcPredictionSlots] __attribute__((aligned(16)));

static inline unsigned char *Uncached(const void *pAddress) {
    return (unsigned char *)(((uintptr_t)pAddress & kPhysicalAddressMask) | kUncachedSegment);
}

static inline unsigned int PhysicalAddress(const void *pAddress) {
    return (unsigned int)((uintptr_t)pAddress & kPhysicalAddressMask);
}

#pragma mark - Demultiplexer bit reader

typedef struct {
    unsigned long long mCache;
    unsigned char *mStart;
    unsigned char *mFetch;
    int mCached;
    unsigned long long mPosition; // In bits from mStart.
    unsigned char *mRingBase;
    unsigned char *mRingEnd;
    int mRingSize;
} BitReader;

enum {
    kCacheRefillBits = 57,
    kCacheTopShift = 56,
};

// NTSC-U/C: 0x0030e8d8, PAL: 0x0037ad68
static void AdvanceMpegBitReader(BitReader *pReader, int nBits) {
    pReader->mCache <<= nBits;
    pReader->mCached -= nBits;
    // An overdrawn count wraps to a large unsigned value and skips the refill, as in the binary.
    while ((unsigned int)pReader->mCached < kCacheRefillBits) {
        pReader->mCache |= (unsigned long long)*pReader->mFetch
                           << (kCacheTopShift - pReader->mCached);
        ++pReader->mFetch;
        if (pReader->mFetch >= pReader->mRingEnd) {
            pReader->mFetch = pReader->mRingBase;
        }
        pReader->mCached += 8;
    }
    pReader->mPosition += (unsigned long long)(long long)nBits;
}

// NTSC-U/C: 0x0030e880, PAL: 0x0037ad10
static void InitMpegBitReader(BitReader *pReader,
                              unsigned char *pStart,
                              unsigned char *pRingBase,
                              int nRingSize) {
    pReader->mFetch = pStart;
    pReader->mRingEnd = pRingBase + nRingSize;
    pReader->mRingSize = nRingSize;
    pReader->mStart = pStart;
    pReader->mCache = 0;
    pReader->mCached = 0;
    pReader->mPosition = 0;
    pReader->mRingBase = pRingBase;
    AdvanceMpegBitReader(pReader, 0);
}

// NTSC-U/C: 0x0030e8b8, PAL: 0x0037ad48
static int PeekMpegBits(const BitReader *pReader, int nBits) {
    return (int)(pReader->mCache >> (64 - nBits));
}

// NTSC-U/C: 0x0030e970, PAL: 0x0037ae00
static int getBits(BitReader *pReader, int nBits) {
    int nValue = PeekMpegBits(pReader, nBits);

    AdvanceMpegBitReader(pReader, nBits);
    return nValue;
}

// NTSC-U/C: 0x0030e9c0, PAL: 0x0037ae50
static int getBit(BitReader *pReader) {
    int nValue = PeekMpegBits(pReader, 1);

    AdvanceMpegBitReader(pReader, 1);
    return nValue;
}

// NTSC-U/C: 0x0030ea08, PAL: 0x0037ae98
// Moves the reader nBytes ahead of its bit position, rounded down to a byte, and refills.
static void SkipMpegBitReaderBytes(BitReader *pReader, int nBytes) {
    unsigned long long nPosition =
        (unsigned long long)(long long)(nBytes << 3) + pReader->mPosition;
    unsigned char *pFetch;

    pReader->mCache = 0;
    pReader->mCached = 0;
    pFetch = pReader->mStart + (int)(nPosition >> 3);
    pReader->mPosition = nPosition;
    pReader->mFetch = pFetch;
    if (pFetch >= pReader->mRingEnd) {
        pReader->mFetch = pFetch - pReader->mRingSize;
    }
    AdvanceMpegBitReader(pReader, 0);
}

// NTSC-U/C: 0x0030ea60, PAL: 0x0037aef0
static unsigned char *GetMpegBitReaderBytePointer(const BitReader *pReader, int nBitPosition) {
    unsigned char *pByte = pReader->mStart + (nBitPosition >> 3);

    if (pByte >= pReader->mRingEnd) {
        pByte -= pReader->mRingSize;
    }
    return pByte;
}

#pragma mark - Callbacks and errors

// NTSC-U/C: 0x00308338, PAL: 0x003747c8
// Calls the slot callback of the callback type, and returns its result or zero.
static int _sceMpegCallback(sceMpeg *mp, sceMpegCbData *pCbData) {
    MpegContext *w;
    MpegSlot *pSlot;

    if (mp == NULL || (w = mp->sys) == NULL) {
        return 0;
    }
    pSlot = &w->mSlots[pCbData->type];
    if (pSlot->mCallback == NULL) {
        return 0;
    }
    return pSlot->mCallback(mp, pCbData, pSlot->mData);
}

// NTSC-U/C: 0x00308388, PAL: 0x00374818
static int _sceMpegCbNodata(sceMpeg *mp) {
    sceMpegCbData cbData;

    cbData.type = sceMpegCbNodata;
    _sceMpegCallback(mp, &cbData);
    return 1;
}

// NTSC-U/C: 0x00308ad0, PAL: 0x00374f60
static void _sceMpegPrintError(const char *pMessage) {
    scePrintf("[MPEG ERROR]%s\n", pMessage);
}

// NTSC-U/C: 0x00308b18, PAL: 0x00374fa8
static void _sceMpegError(MpegContext *w, const char *pMessage) {
    sceMpeg *mp = w->mMpeg;
    sceMpegCbData cbData;

    if (mp != NULL && w != NULL && w->mSlots[sceMpegCbError].mCallback != NULL) {
        cbData.error.type = sceMpegCbError;
        cbData.error.errMessage = (char *)pMessage;
        _sceMpegCallback(mp, &cbData);
    } else {
        _sceMpegPrintError(pMessage);
    }
}

// NTSC-U/C: 0x00308ae0, PAL: 0x00374f70
static void _sceMpegErrorFormat(MpegContext *w, const char *pFormat, unsigned int nValue) {
    char szMessage[kErrorMessageSize];

    sprintf(szMessage, pFormat, nValue);
    _sceMpegError(w, szMessage);
}

#pragma mark - Demultiplexer

typedef struct {
    int mScrExtension;
    unsigned int mScrLow;
    int mScrHigh;
    int mHasSystemHeader;
} PackHeader;

// One parsed packet. Positions are bit positions from the start of the input.
typedef struct {
    unsigned long long mKey; // The stream identifier above bit 32 and a substream word below.
    int mPacketLength;
    int mScrambling;
    long long mPts;
    long long mDts;
    int mDataPosition;
    int mDataLength;
    int mHeaderPosition;
} PesPacket;

enum {
    kStreamIdProgramStreamMap = 0xbc,
    kStreamIdPrivate1 = 0xbd,
    kStreamIdPadding = 0xbe,
    kStreamIdPrivate2 = 0xbf,
    kStreamIdEcm = 0xf0,
    kStreamIdEmm = 0xf1,
    kStreamIdDsmcc = 0xf2,
    kStreamIdH2221TypeE = 0xf8,
    kStreamIdDirectory = 0xff,
    kPtsFlag = 2,
    kPtsDtsFlags = 3,
    kSubstreamHeaderBytes = 4,
    // The packet length counts the three bytes of flags and header length ahead of the header.
    kPesFlagBytes = 3,
};

// The stream key the demultiplexer falls back to when no registered stream matches a packet.
static const unsigned long long kDefaultStreamKey = 0xbdffULL << 24;
static const unsigned long long kSubstreamByteMask = 0xffffULL << 24;
static const unsigned long long kStreamIdMask = 0xff00ULL << 24;

// NTSC-U/C: 0x00307318, PAL: 0x003737a8
static unsigned long long buildStreamKey(int nType, int nChannel) {
    unsigned long long nMask;
    int nShift = 0;

    if ((unsigned int)nType >= kStreamTypeCount) {
        return 0;
    }
    nMask = g_aMpegStreamIdTable[nType][1];
    if (nMask == kSubstreamByteMask) {
        nShift = 24;
    } else if (nMask > kSubstreamByteMask) {
        nShift = 0;
    } else if (nMask == kStreamIdMask) {
        nShift = 32;
    }
    return g_aMpegStreamIdTable[nType][0] | ((unsigned long long)(long long)nChannel << nShift);
}

static inline unsigned long long StreamIdKey(int nStreamId) {
    return (unsigned long long)(unsigned int)nStreamId << 32;
}

// A 33-bit time stamp split 3, 15, and 15 around marker bits, after its four-bit prefix.
static inline long long ReadTimeStamp(BitReader *pReader) {
    unsigned int nHigh;
    unsigned int nMiddle;
    unsigned int nLow;

    (void)getBits(pReader, 4);
    nHigh = (unsigned int)getBits(pReader, 3);
    (void)getBit(pReader);
    nMiddle = (unsigned int)getBits(pReader, 15);
    (void)getBit(pReader);
    nLow = (unsigned int)getBits(pReader, 15);
    (void)getBit(pReader);
    return (long long)(((unsigned long long)((nHigh >> 2) & 1) << 32) |
                       ((nHigh << 30) | (nMiddle << 15) | nLow));
}

// NTSC-U/C: 0x00307908, PAL: 0x00373d98
static int parseSystemHeader(BitReader *pReader, PackHeader *pPack) {
    (void)pPack; // Yes, the binary passes the pack header and never reads it.
    (void)getBits(pReader, 56);
    (void)getBits(pReader, 40);
    while (PeekMpegBits(pReader, 1) == 1) {
        (void)getBits(pReader, 24);
    }
    return 1;
}

// NTSC-U/C: 0x003077b8, PAL: 0x00373c48
static int parsePackHeader(BitReader *pReader, PackHeader *pPack) {
    unsigned int nHigh;
    unsigned int nMiddle;
    unsigned int nLow;
    unsigned int nStuffing;
    unsigned int i;

    (void)getBits(pReader, 34);
    nHigh = (unsigned int)getBits(pReader, 3);
    (void)getBit(pReader);
    nMiddle = (unsigned int)getBits(pReader, 15);
    (void)getBit(pReader);
    nLow = (unsigned int)getBits(pReader, 15);
    (void)getBit(pReader);
    pPack->mScrExtension = getBits(pReader, 9);
    (void)getBits(pReader, 30);
    nStuffing = (unsigned int)getBits(pReader, 3);
    pPack->mScrHigh = (int)((nHigh >> 2) & 1);
    pPack->mScrLow = (nHigh << 30) | (nMiddle << 15) | nLow;
    for (i = 0; i < nStuffing; ++i) {
        (void)getBits(pReader, 8);
    }
    if (PeekMpegBits(pReader, 32) == kSystemHeaderStartCode) {
        pPack->mHasSystemHeader = 1;
        parseSystemHeader(pReader, pPack);
    } else {
        pPack->mHasSystemHeader = 0;
    }
    return 1;
}

// NTSC-U/C: 0x00307978, PAL: 0x00373e08
// Returns zero when the packet embeds a pack header, which a program stream may not.
static int parsePacket(MpegContext *w, BitReader *pReader, PesPacket *pPacket) {
    // Bits the optional fields take for each combination of the ES rate, trick mode, copy
    // information, and CRC flags.
    const unsigned char abOptionalFieldBits[] = {
        0, 16, 8, 24, 8, 24, 16, 32, 24, 40, 32, 48, 32, 48, 40, 56};
    int nPtsDtsFlags;
    int nHasEscr;
    int nOptionalFlags;
    int nHasExtension;
    int nHeaderDataLength;
    int nHeaderStart;
    int nPayload;
    int nSkip;

    pPacket->mHeaderPosition = (int)pReader->mPosition;
    (void)getBits(pReader, 24);
    pPacket->mKey = StreamIdKey(getBits(pReader, 8));
    pPacket->mPacketLength = getBits(pReader, 16);
    pPacket->mPts = -1;
    pPacket->mDts = -1;
    if (pPacket->mKey == StreamIdKey(kStreamIdProgramStreamMap) ||
        pPacket->mKey == StreamIdKey(kStreamIdPadding) ||
        pPacket->mKey == StreamIdKey(kStreamIdPrivate2) ||
        pPacket->mKey == StreamIdKey(kStreamIdEcm) || pPacket->mKey == StreamIdKey(kStreamIdEmm) ||
        pPacket->mKey == StreamIdKey(kStreamIdDirectory) ||
        pPacket->mKey == StreamIdKey(kStreamIdDsmcc) ||
        pPacket->mKey == StreamIdKey(kStreamIdH2221TypeE)) {
        if (pPacket->mKey == StreamIdKey(kStreamIdPadding)) {
            if (pPacket->mPacketLength != 0) {
                SkipMpegBitReaderBytes(pReader, pPacket->mPacketLength);
            }
            return 1;
        }
        nSkip = pPacket->mPacketLength;
        if (pPacket->mKey == StreamIdKey(kStreamIdPrivate2)) {
            pPacket->mKey |= (unsigned int)getBits(pReader, 32);
            nSkip -= kSubstreamHeaderBytes;
        }
        if (nSkip != 0) {
            SkipMpegBitReaderBytes(pReader, nSkip);
        }
        return 1;
    }

    (void)getBits(pReader, 2);
    pPacket->mScrambling = getBits(pReader, 2);
    (void)getBits(pReader, 4);
    nPtsDtsFlags = getBits(pReader, 2);
    nHasEscr = getBits(pReader, 1);
    nOptionalFlags = getBits(pReader, 4);
    nHasExtension = getBits(pReader, 1);
    nHeaderDataLength = getBits(pReader, 8);
    nHeaderStart = (int)pReader->mPosition;
    if ((nPtsDtsFlags & kPtsFlag) != 0) {
        pPacket->mPts = ReadTimeStamp(pReader);
    }
    if (nPtsDtsFlags == kPtsDtsFlags) {
        pPacket->mDts = ReadTimeStamp(pReader);
    }
    if (nHasEscr == 1) {
        (void)getBits(pReader, 48);
    }
    if (nOptionalFlags != 0) {
        (void)getBits(pReader, abOptionalFieldBits[nOptionalFlags]);
    }
    if (nHasExtension == 1) {
        int nHasPrivateData = getBits(pReader, 1);
        int nHasPackHeader = getBits(pReader, 1);
        int nHasSequenceCounter = getBits(pReader, 1);
        int nHasPStdBuffer = getBits(pReader, 1);
        int nHasExtension2;

        (void)getBits(pReader, 3);
        nHasExtension2 = getBits(pReader, 1);
        if (nHasPrivateData == 1) {
            (void)getBits(pReader, 48);
            (void)getBits(pReader, 48);
            (void)getBits(pReader, 32);
        }
        if (nHasPackHeader == 1) {
            _sceMpegError(w, "pack_header_field_flag needs to be '0' in PS\n");
            return 0;
        }
        if (nHasSequenceCounter == 1) {
            (void)getBits(pReader, 16);
        }
        if (nHasPStdBuffer == 1) {
            (void)getBits(pReader, 16);
        }
        if (nHasExtension2 == 1) {
            unsigned int nLength;
            unsigned int j;

            (void)getBit(pReader);
            nLength = (unsigned int)getBits(pReader, 7);
            for (j = 0; j < nLength; ++j) {
                (void)getBits(pReader, 8);
            }
        }
    }
    nSkip = nHeaderDataLength -
            (int)((pReader->mPosition - (unsigned long long)(long long)nHeaderStart) >> 3);
    if (nSkip != 0) {
        SkipMpegBitReaderBytes(pReader, nSkip);
    }
    nPayload = pPacket->mPacketLength - nHeaderDataLength;
    pPacket->mDataLength = nPayload - kPesFlagBytes;
    pPacket->mDataPosition = (int)pReader->mPosition;
    nSkip = nPayload - kPesFlagBytes;
    if (pPacket->mKey == StreamIdKey(kStreamIdPrivate1)) {
        // The substream word stays in the reported data and length.
        pPacket->mKey |= (unsigned int)getBits(pReader, 32);
        nSkip = nPayload - kPesFlagBytes - kSubstreamHeaderBytes;
    }
    if (nSkip != 0) {
        SkipMpegBitReaderBytes(pReader, nSkip);
    }
    return 1;
}

static inline int DeliverPacket(sceMpeg *mp,
                                const BitReader *pReader,
                                const PesPacket *pPacket,
                                sceMpegCallback pfnCallback,
                                void *pData) {
    sceMpegCbDataStr cbData;

    cbData.type = sceMpegCbStr;
    cbData.header = GetMpegBitReaderBytePointer(pReader, pPacket->mHeaderPosition);
    cbData.data = GetMpegBitReaderBytePointer(pReader, pPacket->mDataPosition);
    cbData.len = (unsigned int)pPacket->mDataLength;
    cbData.pts = pPacket->mPts;
    cbData.dts = pPacket->mDts;
    return pfnCallback(mp, &cbData, pData);
}

static inline int IsPacketStart(const BitReader *pReader) {
    return PeekMpegBits(pReader, 24) == kPacketStartPrefix &&
           PeekMpegBits(pReader, 32) != kPackStartCode &&
           PeekMpegBits(pReader, 32) != kProgramEndCode;
}

int sceMpegDemuxPssRing(
    sceMpeg *mp, unsigned char *data, int size, unsigned char *buffer, int buffer_size) {
    MpegContext *w = mp->sys;
    StreamEntry *pTable = w->mStreamTable;
    BitReader reader;
    PackHeader pack;
    // A packet without header data does not set the payload fields. A callback then reads the
    // previous packet's values, and the first packet reads zeros instead of stale stack memory.
    PesPacket packet = {0};
    sceMpegCallback pfnDefault = NULL;
    void *pDefaultData = NULL;
    unsigned long long nLimit = (unsigned long long)(long long)(size << 3);
    int nConsumed = 0;
    int nProceed = 1;
    int i;

    InitMpegBitReader(&reader, data, buffer, buffer_size);
    for (i = 0; i < w->mStreamCount; ++i) {
        if (pTable[i].mKey == kDefaultStreamKey) {
            pDefaultData = pTable[i].mData;
            pfnDefault = pTable[i].mCallback;
        }
        if (pfnDefault != NULL) {
            break;
        }
    }
    if (PeekMpegBits(&reader, 32) == kPackStartCode) {
        parsePackHeader(&reader, &pack);
    }
    for (;;) {
        if (IsPacketStart(&reader) && reader.mPosition < nLimit) {
            if (nProceed == 0) {
                return nConsumed;
            }
            parsePacket(w, &reader, &packet);
            if (nLimit < reader.mPosition) {
                continue;
            }
            for (i = 0; i < w->mStreamCount; ++i) {
                if (pTable[i].mKey == (packet.mKey & pTable[i].mMask)) {
                    nProceed =
                        DeliverPacket(mp, &reader, &packet, pTable[i].mCallback, pTable[i].mData);
                    break;
                }
            }
            // The count is read again after a callback, which may register a stream.
            if (i == w->mStreamCount && pfnDefault != NULL) {
                nProceed = DeliverPacket(mp, &reader, &packet, pfnDefault, pDefaultData);
            }
            if (nProceed != 0) {
                nConsumed = (int)(reader.mPosition >> 3);
            }
            continue;
        }
        if (nLimit < reader.mPosition || PeekMpegBits(&reader, 32) != kPackStartCode) {
            return nConsumed;
        }
        parsePackHeader(&reader, &pack);
    }
}

int sceMpegDemuxPss(sceMpeg *mp, unsigned char *data, int size) {
    return sceMpegDemuxPssRing(mp, data, size, NULL, -1);
}

sceMpegCallback
sceMpegAddStrCallback(sceMpeg *mp, int type, int ch, sceMpegCallback func, void *data) {
    MpegContext *w = mp->sys;
    StreamEntry *pTable = w->mStreamTable;
    unsigned long long nKey = buildStreamKey(type, ch);
    sceMpegCallback pfnPrevious = NULL;
    int nCount = w->mStreamCount;
    int i;

    for (i = 0; i < nCount; ++i) {
        if (nKey == pTable[i].mKey) {
            pfnPrevious = pTable[i].mCallback;
            break;
        }
    }
    if (i < kMaxStreamCallbacks) {
        w->mStreamCount = nCount + 1; // Yes, the binary counts a duplicate key too.
        pTable[i].mKey = nKey;
        pTable[i].mData = data;
        pTable[i].mCallback = func;
        pTable[i].mMask = g_aMpegStreamIdTable[type][1];
    }
    return pfnPrevious;
}

#pragma mark - Work area allocator

// NTSC-U/C: 0x003083d8, PAL: 0x00374868
static void _sceMpegAlalcInit(MpegAlalc *pAlalc, unsigned char *pBase, int nSize) {
    pAlalc->mDynamic = pBase;
    pAlalc->mSize = nSize;
    pAlalc->mBase = pBase;
    pAlalc->mCurrent = pBase;
}

// NTSC-U/C: 0x003083f0, PAL: 0x00374880
static void _sceMpegAlalcSetDynamic(MpegAlalc *pAlalc) {
    pAlalc->mDynamic = pAlalc->mCurrent;
}

// NTSC-U/C: 0x00308400, PAL: 0x00374890
static void _sceMpegAlalcFree(MpegAlalc *pAlalc) {
    pAlalc->mCurrent = pAlalc->mDynamic;
}

// NTSC-U/C: 0x00308410, PAL: 0x003748a0
static unsigned char *
_sceMpegAlalcAlloc(MpegContext *w, MpegAlalc *pAlalc, unsigned int nSize, unsigned int nAlign) {
    uintptr_t nStart = (((uintptr_t)pAlalc->mCurrent + nAlign - 1) / nAlign) * nAlign;

    if ((uintptr_t)(pAlalc->mBase + pAlalc->mSize) < nStart + nSize) {
        _sceMpegError(w, "work area size is too small");
        return NULL;
    }
    pAlalc->mCurrent = (unsigned char *)(nStart + nSize);
    return (unsigned char *)nStart;
}

#pragma mark - IPU access

// NTSC-U/C: 0x0030b0b8, PAL: 0x00377548
static void SetIpuControlEnableBit(int nMpeg1) {
    *IPU_CTRL =
        (*IPU_CTRL & ~(unsigned int)kIpuCtrlMpeg1) | ((unsigned int)nMpeg1 << kIpuCtrlMpeg1Shift);
}

// Spins while the IPU is busy without an error, raising the no-data callback every 5001 spins.
static inline int SpinIpuBusy(MpegContext *w) {
    int nSpins = 0;

    if ((*IPU_CTRL & kIpuCtrlBusyOrError) == kIpuCtrlBusy) {
        do {
            if (nSpins++ > kIpuSpinLimit) {
                _sceMpegCbNodata(w->mMpeg);
                nSpins = 0;
            }
        } while ((*IPU_CTRL & kIpuCtrlBusyOrError) == kIpuCtrlBusy);
    }
    return nSpins;
}

// NTSC-U/C: 0x0030c360, PAL: 0x003787f0
static void IssueIpuCommand(MpegContext *w, unsigned int nCommand) {
    *IPU_CMD = nCommand;
    w->mTopStale = g_anIpuCmdTypeTable[nCommand >> kIpuCmdShift];
}

// NTSC-U/C: 0x0030c390, PAL: 0x00378820
static void WaitForIpuIdle(MpegContext *w) {
    (void)SpinIpuBusy(w);
}

// NTSC-U/C: 0x0030c438, PAL: 0x003788c8
// Returns the data of the last command once the IPU has it, or at an error.
static unsigned int WaitForIpuCommandResult(MpegContext *w) {
    unsigned long long nData = *IPU_CMD_DATA;
    int nSpins = 0;

    if ((long long)nData < 0 && (*IPU_CTRL & kIpuCtrlErrorCode) == 0) {
        do {
            if (nSpins++ > kIpuSpinLimit) {
                _sceMpegCbNodata(w->mMpeg);
                nSpins = 0;
            }
            nData = *IPU_CMD_DATA;
            if ((long long)nData >= 0) {
                break;
            }
        } while ((*IPU_CTRL & kIpuCtrlErrorCode) == 0);
    }
    return (unsigned int)nData;
}

// NTSC-U/C: 0x0030c4e8, PAL: 0x00378978
static int DecodeIpuVlc(MpegContext *w, int nTable) {
    unsigned int nCommand;
    unsigned long long nData;
    unsigned long long nTop;
    unsigned int nPosition;
    int nSpins = 0;

    SpinIpuBusy(w);
    nCommand = ((unsigned int)nTable << kIpuVdecTableShift) | kIpuCmdVdec;
    *IPU_CMD = nCommand;
    w->mTopStale = g_anIpuCmdTypeTable[(int)nCommand >> kIpuCmdShift];
    nData = *IPU_CMD_DATA;
    while ((long long)nData < 0) {
        if (nSpins++ > kIpuSpinLimit) {
            _sceMpegCbNodata(w->mMpeg);
            nSpins = 0;
        }
        nData = *IPU_CMD_DATA;
    }
    nTop = *IPU_TOP;
    nPosition = *IPU_BP;
    w->mTopBits = (unsigned int)nTop;
    if ((long long)nTop < 0) {
        w->mTopBitCount = (int)((0u - (nPosition & kIpuBpPositionMask)) & kIpuBpPositionMask);
    } else {
        w->mTopBitCount = kIpuTopBits;
    }
    w->mFault = (int)nData == 0;
    return (short)nData;
}

// NTSC-U/C: 0x0030c660, PAL: 0x00378af0
static unsigned int PeekIpuBits(MpegContext *w, int nBits) {
    if (w->mTopStale != 0 || w->mTopBitCount < nBits) {
        SpinIpuBusy(w);
        *IPU_CMD = kIpuCmdFdec;
        w->mTopStale = g_anIpuCmdTypeTable[kIpuCmdFdecIndex];
        w->mTopBits = WaitForIpuCommandResult(w);
        w->mTopBitCount = kIpuTopBits;
    }
    return w->mTopBits >> ((kIpuTopBits - nBits) & (kIpuTopBits - 1));
}

// NTSC-U/C: 0x0030c768, PAL: 0x00378bf8
static void ConsumeIpuBits(MpegContext *w, int nBits) {
    unsigned int nCommand;

    SpinIpuBusy(w);
    nCommand = (unsigned int)nBits | kIpuCmdFdec;
    *IPU_CMD = nCommand;
    w->mTopStale = g_anIpuCmdTypeTable[nCommand >> kIpuCmdShift];
    w->mTopBits = WaitForIpuCommandResult(w);
    w->mTopBitCount = kIpuTopBits;
}

// NTSC-U/C: 0x0030c858, PAL: 0x00378ce8
static unsigned int ReadMpegBits(MpegContext *w, int nBits) {
    unsigned int nCommand;
    unsigned int nValue;

    SpinIpuBusy(w);
    if (w->mTopStale != 0 || w->mTopBitCount < nBits) {
        *IPU_CMD = kIpuCmdFdec;
        w->mTopStale = g_anIpuCmdTypeTable[kIpuCmdFdecIndex];
        w->mTopBits = WaitForIpuCommandResult(w);
    }
    w->mTopBitCount = kIpuTopBits;
    nValue = w->mTopBits >> ((kIpuTopBits - nBits) & (kIpuTopBits - 1));
    nCommand = (unsigned int)nBits | kIpuCmdFdec;
    *IPU_CMD = nCommand;
    w->mTopStale = g_anIpuCmdTypeTable[nCommand >> kIpuCmdShift];
    w->mTopBits = WaitForIpuCommandResult(w);
    return nValue;
}

// NTSC-U/C: 0x0030c9a8, PAL: 0x00378e38
static void AlignToMpegStartCode(MpegContext *w) {
    int nBits;

    WaitForIpuIdle(w);
    nBits = (int)((0u - *IPU_BP) & kIpuBpByteMask);
    if (nBits != 0) {
        ConsumeIpuBits(w, nBits);
    }
    while (PeekIpuBits(w, 24) != kPacketStartPrefix) {
        ConsumeIpuBits(w, 8);
    }
}

// NTSC-U/C: 0x0030b380, PAL: 0x00377810
static int SetMpegStreamMode3(MpegContext *w) {
    return DecodeIpuVlc(w, kVlcDmVector);
}

#pragma mark - Motion compensation

// NTSC-U/C: 0x0030b3a0, PAL: 0x00377830
static void
mpc___dualPrimeVector(MpegContext *w, int dmv[2][2], const int dmvector[2], int nMvx, int nMvy) {
    if (w->mPictureStructure == kFramePicture) {
        if (w->mTopFieldFirst != 0) {
            dmv[0][0] = ((nMvx + (nMvx > 0)) >> 1) + dmvector[0];
            dmv[0][1] = ((nMvy + (nMvy > 0)) >> 1) + dmvector[1] - 1;
            dmv[1][0] = ((3 * nMvx + (nMvx > 0)) >> 1) + dmvector[0];
            dmv[1][1] = ((3 * nMvy + (nMvy > 0)) >> 1) + dmvector[1] + 1;
        } else {
            dmv[0][0] = ((3 * nMvx + (nMvx > 0)) >> 1) + dmvector[0];
            dmv[0][1] = ((3 * nMvy + (nMvy > 0)) >> 1) + dmvector[1] - 1;
            dmv[1][0] = ((nMvx + (nMvx > 0)) >> 1) + dmvector[0];
            dmv[1][1] = ((nMvy + (nMvy > 0)) >> 1) + dmvector[1] + 1;
        }
    } else {
        dmv[0][0] = ((nMvx + (nMvx > 0)) >> 1) + dmvector[0];
        dmv[0][1] = ((nMvy + (nMvy > 0)) >> 1) + dmvector[1];
        if (w->mPictureStructure == kTopField) {
            --dmv[0][1];
        } else {
            ++dmv[0][1];
        }
    }
}

// Splits the rows of a job at the block boundary of the staged reference.
static inline void
SplitRows(MpegMcDescriptor *pJob, int nTop, int nRows, int nFieldMode, int nHalf, int nBlock) {
    int nAbove;

    if (nTop + (nRows << nFieldMode) < nBlock + 1 - nHalf) {
        pJob->mRows = nRows;
        pJob->mRowsBelow = 0;
        return;
    }
    nAbove = (nBlock >> nFieldMode) - (nTop >> nFieldMode) - nHalf;
    pJob->mRows = nAbove;
    pJob->mRowsBelow = nRows - nAbove;
}

// NTSC-U/C: 0x00309cc8, PAL: 0x00376158
// Queues one prediction of the current macroblock from a reference picture.
static void BuildMpegMotionBlockDescriptor(MpegContext *w,
                                           const MpegPicture *pSource,
                                           int nSourceField,
                                           int nDestField,
                                           int nRowOffset,
                                           int nHeight,
                                           int nX,
                                           int nY,
                                           int nDx,
                                           int nDy,
                                           int nFieldMode,
                                           int nAverage) {
    MpegMcBuffer *pBuffer = &w->mMcBuffers[w->mMcCurrent];
    int nIndex = pBuffer->mPredictionCount;
    MpegMcDescriptor *pLuma = &pBuffer->mLuma[nIndex];
    MpegMcDescriptor *pChroma = &pBuffer->mChroma[nIndex];
    unsigned char *pOutput = w->mMcOutput;
    unsigned char *pStaging;
    unsigned char *pChromaStaging;
    int nLeft = (nDx >> 1) + nX;
    int nTop;
    int nColumn;
    int nRow;
    int nFirst;
    int nHalfX = nDx & 1;
    int nHalfY = nDy & 1;
    int nChromaDx = nDx / 2;
    int nChromaDy = nDy / 2;
    int nChromaLeft;
    int nChromaTop;
    int nChromaColumn;
    int nChromaRow;

    nTop = (nFieldMode != 0 ? (nDy >> 1) << 1 : nDy >> 1) + nY + nRowOffset + nSourceField;
    nColumn = nLeft >> 4;
    nRow = nTop >> 4;
    nFirst = nColumn * pSource->mMbHeight + nRow;
    pLuma->mShift = nLeft - (nColumn << 4);
    pLuma->mDest = pOutput + ((nDestField + nRowOffset) << 5);
    nTop -= nRow << 4;
    SplitRows(pLuma, nTop, nHeight, nFieldMode, nHalfY, 16);
    pStaging = pBuffer->mStaging + nIndex * kMcStagingSize;
    pLuma->mSourceLeft = pStaging + (nTop << 4);
    pLuma->mStride = 16 << nFieldMode;
    pLuma->mSourceRight = pStaging + (nTop << 4) + kMcColumnSize;

    nChromaLeft = (nChromaDx >> 1) + (nX >> 1);
    nChromaTop = (nFieldMode != 0 ? (nChromaDy >> 1) << 1 : nChromaDy >> 1) + (nY >> 1) +
                 (nRowOffset >> 1) + nSourceField;
    nChromaRow = nChromaTop >> 3;
    nChromaColumn = nChromaLeft >> 3;
    pChroma->mShift = nChromaLeft - (nChromaColumn << 3);
    pChroma->mDest = pOutput + kMcChromaOutputOffset + ((nDestField + (nRowOffset >> 1)) << 4);
    nChromaTop -= nChromaRow << 3;
    SplitRows(pChroma, nChromaTop, nHeight >> 1, nFieldMode, nChromaDy & 1, 8);
    pChroma->mStride = 8 << nFieldMode;
    pChromaStaging =
        pStaging + (((nChromaColumn - nColumn) << 1) + (nChromaRow - nRow)) * kMacroblockSize;

    pBuffer->mLumaKernels[nIndex] = g_apfnMpegLumaKernels[(nAverage << 2) | (nHalfX << 1) | nHalfY];
    pBuffer->mChromaKernels[nIndex] =
        g_apfnMpegChromaKernels[(nAverage << 2) | ((nChromaDx & 1) << 1) | (nChromaDy & 1)];
    pChroma->mSourceLeft = pChromaStaging + (nChromaTop << 3) + kMcChromaOffset;
    pChroma->mSourceRight = pChromaStaging + (nChromaTop << 3) + kMcColumnSize + kMcChromaOffset;
    pBuffer->mRefLeft[nIndex] = pSource->mData + nFirst * kMacroblockSize;
    pBuffer->mRefRight[nIndex] = pSource->mData + (nFirst + pSource->mMbHeight) * kMacroblockSize;
    pBuffer->mPredictionCount = nIndex + 1;
}

// NTSC-U/C: 0x003095c0, PAL: 0x00375a50
// Queues every prediction of the current macroblock.
static void mpc___getAllRefs(MpegContext *w,
                             int nX,
                             int nY,
                             int nMbType,
                             int nMotionType,
                             int PMV[2][2][2],
                             int mvfs[2][2],
                             int dmvector[2]) {
    int dmv[2][2];
    MpegPicture *apFields[4];
    int nAverage = 0;
    int nBackward;
    int nBottom;

    w->mMcBuffers[w->mMcCurrent].mPredictionCount = 0;
    if ((nMbType & kMbMotionForward) != 0 || w->mPictureCodingType == kPictureP) {
        if (w->mPictureStructure == kFramePicture) {
            if (nMotionType == kMcFrame || (nMbType & kMbMotionForward) == 0) {
                BuildMpegMotionBlockDescriptor(
                    w, w->mFrames.mForward, 0, 0, 0, 16, nX, nY, PMV[0][0][0], PMV[0][0][1], 0, 0);
            } else if (nMotionType == kMcField) {
                BuildMpegMotionBlockDescriptor(w,
                                               w->mFrames.mForward,
                                               mvfs[0][0],
                                               0,
                                               0,
                                               8,
                                               nX,
                                               nY,
                                               PMV[0][0][0],
                                               PMV[0][0][1] >> 1,
                                               1,
                                               0);
                BuildMpegMotionBlockDescriptor(w,
                                               w->mFrames.mForward,
                                               mvfs[1][0],
                                               1,
                                               0,
                                               8,
                                               nX,
                                               nY,
                                               PMV[1][0][0],
                                               PMV[1][0][1] >> 1,
                                               1,
                                               0);
            } else if (nMotionType == kMcDmv) {
                mpc___dualPrimeVector(w, dmv, dmvector, PMV[0][0][0], PMV[0][0][1] >> 1);
                BuildMpegMotionBlockDescriptor(w,
                                               w->mFrames.mForward,
                                               0,
                                               0,
                                               0,
                                               8,
                                               nX,
                                               nY,
                                               PMV[0][0][0],
                                               PMV[0][0][1] >> 1,
                                               1,
                                               0);
                BuildMpegMotionBlockDescriptor(
                    w, w->mFrames.mForward, 1, 0, 0, 8, nX, nY, dmv[0][0], dmv[0][1], 1, 1);
                BuildMpegMotionBlockDescriptor(w,
                                               w->mFrames.mForward,
                                               1,
                                               1,
                                               0,
                                               8,
                                               nX,
                                               nY,
                                               PMV[0][0][0],
                                               PMV[0][0][1] >> 1,
                                               1,
                                               0);
                BuildMpegMotionBlockDescriptor(
                    w, w->mFrames.mForward, 0, 1, 0, 8, nX, nY, dmv[1][0], dmv[1][1], 1, 1);
            } else {
                _sceMpegErrorFormat(w, "(a) invalid motion_type(%d)-0", (unsigned int)nMotionType);
            }
        } else {
            nBottom = w->mPictureStructure == kBottomField;
            apFields[0] = w->mTopFields.mForward;
            apFields[1] = w->mBottomFields.mForward;
            apFields[2] = w->mTopFields.mBackward;
            apFields[3] = w->mBottomFields.mBackward;
            nBackward = 0;
            if (w->mPictureCodingType == kPictureP && w->mSecondField != 0) {
                nBackward = nBottom != mvfs[0][0];
            }
            if (nMotionType == kMcField || (nMbType & kMbMotionForward) == 0) {
                BuildMpegMotionBlockDescriptor(w,
                                               apFields[(nBackward << 1) + mvfs[0][0]],
                                               0,
                                               0,
                                               0,
                                               16,
                                               nX,
                                               nY,
                                               PMV[0][0][0],
                                               PMV[0][0][1],
                                               0,
                                               0);
            } else if (nMotionType == kMc16x8) {
                BuildMpegMotionBlockDescriptor(w,
                                               apFields[(nBackward << 1) + mvfs[0][0]],
                                               0,
                                               0,
                                               0,
                                               8,
                                               nX,
                                               nY,
                                               PMV[0][0][0],
                                               PMV[0][0][1],
                                               0,
                                               0);
                nBackward = 0;
                if (w->mPictureCodingType == kPictureP && w->mSecondField != 0) {
                    nBackward = nBottom != mvfs[1][0];
                }
                BuildMpegMotionBlockDescriptor(w,
                                               apFields[(nBackward << 1) + mvfs[1][0]],
                                               0,
                                               0,
                                               8,
                                               8,
                                               nX,
                                               nY,
                                               PMV[1][0][0],
                                               PMV[1][0][1],
                                               0,
                                               0);
            } else if (nMotionType == kMcDmv) {
                nBackward = w->mSecondField != 0;
                mpc___dualPrimeVector(w, dmv, dmvector, PMV[0][0][0], PMV[0][0][1]);
                BuildMpegMotionBlockDescriptor(
                    w, apFields[nBottom], 0, 0, 0, 16, nX, nY, PMV[0][0][0], PMV[0][0][1], 0, 0);
                BuildMpegMotionBlockDescriptor(w,
                                               apFields[(nBackward << 1) + !nBottom],
                                               0,
                                               0,
                                               0,
                                               16,
                                               nX,
                                               nY,
                                               dmv[0][0],
                                               dmv[0][1],
                                               0,
                                               1);
            } else {
                _sceMpegErrorFormat(w, "(b) invalid motion_type(%d)-1", (unsigned int)nMotionType);
            }
        }
        nAverage = 1;
    }
    if ((nMbType & kMbMotionBackward) == 0) {
        return;
    }
    if (w->mPictureStructure == kFramePicture) {
        if (nMotionType == kMcFrame) {
            BuildMpegMotionBlockDescriptor(w,
                                           w->mFrames.mBackward,
                                           0,
                                           0,
                                           0,
                                           16,
                                           nX,
                                           nY,
                                           PMV[0][1][0],
                                           PMV[0][1][1],
                                           0,
                                           nAverage);
        } else {
            BuildMpegMotionBlockDescriptor(w,
                                           w->mFrames.mBackward,
                                           mvfs[0][1],
                                           0,
                                           0,
                                           8,
                                           nX,
                                           nY,
                                           PMV[0][1][0],
                                           PMV[0][1][1] >> 1,
                                           1,
                                           nAverage);
            BuildMpegMotionBlockDescriptor(w,
                                           w->mFrames.mBackward,
                                           mvfs[1][1],
                                           1,
                                           0,
                                           8,
                                           nX,
                                           nY,
                                           PMV[1][1][0],
                                           PMV[1][1][1] >> 1,
                                           1,
                                           nAverage);
        }
    } else if (nMotionType == kMcField) {
        BuildMpegMotionBlockDescriptor(w,
                                       mvfs[0][1] != 0 ? w->mBottomFields.mBackward :
                                                         w->mTopFields.mBackward,
                                       0,
                                       0,
                                       0,
                                       16,
                                       nX,
                                       nY,
                                       PMV[0][1][0],
                                       PMV[0][1][1],
                                       0,
                                       nAverage);
    } else if (nMotionType == kMc16x8) {
        BuildMpegMotionBlockDescriptor(w,
                                       mvfs[0][1] != 0 ? w->mBottomFields.mBackward :
                                                         w->mTopFields.mBackward,
                                       0,
                                       0,
                                       0,
                                       8,
                                       nX,
                                       nY,
                                       PMV[0][1][0],
                                       PMV[0][1][1],
                                       0,
                                       nAverage);
        BuildMpegMotionBlockDescriptor(w,
                                       mvfs[1][1] != 0 ? w->mBottomFields.mBackward :
                                                         w->mTopFields.mBackward,
                                       0,
                                       0,
                                       8,
                                       8,
                                       nX,
                                       nY,
                                       PMV[1][1][0],
                                       PMV[1][1][1],
                                       0,
                                       nAverage);
    } else {
        _sceMpegErrorFormat(w, "(c) invalid motion_type(%d)-2", (unsigned int)nMotionType);
    }
}

// NTSC-U/C: 0x0030a0e8, PAL: 0x00376578
// Runs the queued kernels of a macroblock buffer and writes the macroblock into the frame.
static void ReconstructMpegMacroblockPixels(MpegContext *w, int nBuffer) {
    MpegMcBuffer *pBuffer = &w->mMcBuffers[nBuffer];
    int i;

    if (pBuffer->mDmaPending != 0) {
        for (i = 0; i < pBuffer->mPredictionCount; ++i) {
            pBuffer->mLumaKernels[i](&pBuffer->mLuma[i]);
            pBuffer->mChromaKernels[i](&pBuffer->mChroma[i]);
        }
    }
    if (pBuffer->mIntra != 0 && pBuffer->mNotCoded != 0) {
        _sceMpegError(w, "intra && skip MB");
    }
    if (pBuffer->mIntra != 0) {
        sceMpegMcPutBlock(pBuffer->mOutput, pBuffer->mCoefficients);
    } else if (pBuffer->mNotCoded != 0) {
        sceMpegMcPutBlock(pBuffer->mOutput, w->mMcOutput);
    } else {
        sceMpegMcAddBlock(pBuffer->mOutput, w->mMcOutput, pBuffer->mCoefficients);
    }
}

// NTSC-U/C: 0x0030def8, PAL: 0x0037a388
// Stages the reference macroblocks of the current buffer into the scratchpad.
static void BuildMpegInputDmaChainAndKick(MpegContext *w) {
    MpegMcBuffer *pBuffer = &w->mMcBuffers[w->mMcCurrent];
    MpegDmaTag *pTags = (MpegDmaTag *)Uncached(g_aMpegRefDmaChain);
    int nCount = pBuffer->mPredictionCount;
    int nInterrupts;
    int i;

    for (i = 0; i < nCount; ++i) {
        unsigned long long nLastId = i == nCount - 1 ? kDmaTagRefe : kDmaTagRef;

        pTags[0].mTag = ((unsigned long long)PhysicalAddress(pBuffer->mRefLeft[i]) << 32) |
                        ((unsigned long long)kDmaTagRef << kDmaTagIdShift) | kMcColumnSize / 16;
        pTags[1].mTag = ((unsigned long long)PhysicalAddress(pBuffer->mRefRight[i]) << 32) |
                        (nLastId << kDmaTagIdShift) | kMcColumnSize / 16;
        pTags += 2;
    }
    nInterrupts = DIntr();
    __asm__ volatile("sync" ::: "memory");
    *D9_SADR = (unsigned int)(uintptr_t)w->mMcBuffers[w->mMcCurrent].mStaging;
    *D9_TADR = (unsigned int)(uintptr_t)g_aMpegRefDmaChain;
    *D9_QWC = 0;
    *D9_CHCR = kChcrStart | kChcrChain | kChcrFromMemory;
    if (nInterrupts != 0) {
        EIntr();
    }
}

// NTSC-U/C: 0x0030e068, PAL: 0x0037a4f8
// Points the channel from the IPU at the scratchpad for nBytes of decoded block.
static void StartIpuFromDmaTransfer(unsigned char *pDest, int nBytes) {
    int nInterrupts = DIntr();

    *D3_MADR = PhysicalAddress(pDest) | kScratchpadFlag;
    *D3_QWC = (unsigned int)(nBytes >> 4);
    *D3_CHCR = kChcrStart;
    if (nInterrupts != 0) {
        EIntr();
    }
}

// NTSC-U/C: 0x00309370, PAL: 0x00375800
static int DispatchMpegMacroblockDecode(MpegContext *w,
                                        int nMba,
                                        int nMbaInc,
                                        int nMbType,
                                        int nMotionType,
                                        int PMV[2][2][2],
                                        int mvfs[2][2],
                                        int dmvector[2]) {
    int nIntra = nMbType & kMbIntra;
    int nX = nMba % w->mMbWidth;
    int nY = nMba / w->mMbWidth;
    MpegMcBuffer *pBuffer;
    MpegPicture *pCurrent;

    if (nIntra != 0) {
        while ((*D9_CHCR & kChcrStart) != 0) {
        }
        w->mMcBuffers[w->mMcCurrent].mDmaPending = 0;
    } else {
        if ((unsigned int)(nMotionType - 1) >= 3) {
            _sceMpegErrorFormat(w, "Invalid modion type -- ignored(%d)", (unsigned int)nMotionType);
            w->mFault = 1;
            return 0;
        }
        mpc___getAllRefs(w, nX << 4, nY << 4, nMbType, nMotionType, PMV, mvfs, dmvector);
        while ((*D9_CHCR & kChcrStart) != 0) {
        }
        BuildMpegInputDmaChainAndKick(w);
        w->mMcBuffers[w->mMcCurrent].mDmaPending = 1;
    }
    pBuffer = &w->mMcBuffers[w->mMcCurrent];
    pBuffer->mCoded = nMbaInc == 1 && (nMbType & kMbPattern) != 0;
    pBuffer->mIntra = nIntra;
    if (w->mPictureStructure == kFramePicture) {
        pCurrent = w->mFrames.mCurrent;
    } else if (w->mPictureStructure == kBottomField) {
        pCurrent = w->mBottomFields.mCurrent;
    } else {
        pCurrent = w->mTopFields.mCurrent;
    }
    pBuffer->mOutput = pCurrent->mData + (nX * pCurrent->mMbHeight + nY) * kMacroblockSize;
    return 1;
}

// NTSC-U/C: 0x0030b0e0, PAL: 0x00377570
// Waits for the block decode of the IPU. Returns zero after an abort or an error code.
static int WaitForIpuBlockDecode(MpegContext *w) {
    sceMpegCbData cbData;
    unsigned long long nTop;
    unsigned int nPosition;
    int nInterrupts;

    WaitForIpuIdle(w);
    if (*D3_QWC != 0 && (*IPU_CTRL & kIpuCtrlErrorCode) == 0) {
        for (;;) {
            if (*D4_QWC == 0 && (*D4_CHCR & kChcrStart) == 0) {
                cbData.type = sceMpegCbNodata;
                _sceMpegCallback(w->mMpeg, &cbData);
            }
            if (*D3_QWC == 0 || (*IPU_CTRL & kIpuCtrlErrorCode) != 0) {
                break;
            }
        }
    }
    if (w->mAbort != 0) {
        nInterrupts = DIntr();
        *D_ENABLEW = *D_ENABLER | kDmaHoldAll;
        *D4_CHCR = 0;
        *D3_CHCR = 0;
        *D_ENABLEW = *D_ENABLER & ~(unsigned int)kDmaHoldAll;
        if (nInterrupts != 0) {
            EIntr();
        }
        *D3_QWC = 0;
        *D4_QWC = 0;
        *IPU_CTRL = kIpuCtrlReset;
        return 0;
    }
    nTop = *IPU_TOP;
    nPosition = *IPU_BP;
    w->mTopBits = (unsigned int)nTop;
    if ((long long)nTop < 0) {
        w->mTopBitCount = (nPosition & kIpuBpPositionMask) != 0 ?
                              kIpuTopBits - (int)(nPosition & kIpuBpPositionMask) :
                              0;
    } else {
        w->mTopBitCount = kIpuTopBits;
    }
    if ((*IPU_CTRL & kIpuCtrlErrorCode) == 0) {
        return 1;
    }
    _sceMpegError(w, "Error code detected(BDEC)");
    cbData.type = sceMpegCbStopDMA;
    _sceMpegCallback(w->mMpeg, &cbData);
    *IPU_CTRL = kIpuCtrlReset;
    cbData.type = sceMpegCbRestartDMA;
    _sceMpegCallback(w->mMpeg, &cbData);
    nInterrupts = DIntr();
    *D_ENABLEW = *D_ENABLER | kDmaHoldAll;
    *D3_CHCR = 0;
    *D_ENABLEW = *D_ENABLER & ~(unsigned int)kDmaHoldAll;
    if (nInterrupts != 0) {
        EIntr();
    }
    *D3_QWC = 0;
    return 0;
}

#pragma mark - Macroblocks and slices

// NTSC-U/C: 0x0030bfe8, PAL: 0x00378478
static void PredictMpegMotionVectorComponent(
    int *pPred, int nRSize, int nMotionCode, int nResidual, int nFullPel) {
    int nLimit = 16 << nRSize;
    int nVector = nFullPel != 0 ? *pPred >> 1 : *pPred;

    if (nMotionCode > 0) {
        nVector += ((nMotionCode - 1) << nRSize) + nResidual + 1;
        if (nVector >= nLimit) {
            nVector -= nLimit + nLimit;
        }
    } else if (nMotionCode < 0) {
        nVector -= ((-nMotionCode - 1) << nRSize) + nResidual + 1;
        if (nVector < -nLimit) {
            nVector += nLimit + nLimit;
        }
    }
    *pPred = nFullPel != 0 ? nVector << 1 : nVector;
}

// NTSC-U/C: 0x0030c210, PAL: 0x003786a0
static void DecodeMpegMotionVectorPair(MpegContext *w,
                                       int PMV[2],
                                       int dmvector[2],
                                       int nHRSize,
                                       int nVRSize,
                                       int nDmv,
                                       int nMvScale,
                                       int nFullPel) {
    int nCode = DecodeIpuVlc(w, kVlcMotionCode);
    int nResidual = nHRSize != 0 && nCode != 0 ? (int)ReadMpegBits(w, nHRSize) : 0;

    PredictMpegMotionVectorComponent(&PMV[0], nHRSize, nCode, nResidual, nFullPel);
    if (nDmv != 0) {
        dmvector[0] = SetMpegStreamMode3(w);
    }
    nCode = DecodeIpuVlc(w, kVlcMotionCode);
    nResidual = nVRSize != 0 && nCode != 0 ? (int)ReadMpegBits(w, nVRSize) : 0;
    if (nMvScale != 0) {
        PMV[1] >>= 1;
    }
    PredictMpegMotionVectorComponent(&PMV[1], nVRSize, nCode, nResidual, nFullPel);
    if (nMvScale != 0) {
        PMV[1] <<= 1;
    }
    if (nDmv != 0) {
        dmvector[1] = SetMpegStreamMode3(w);
    }
}

// NTSC-U/C: 0x0030c070, PAL: 0x00378500
static void mpc___motionVectors(MpegContext *w,
                                int PMV[2][2][2],
                                int dmvector[2],
                                int mvfs[2][2],
                                int s,
                                int nMotionVectorCount,
                                int nMvFormat,
                                int nHRSize,
                                int nVRSize,
                                int nDmv,
                                int nMvScale) {
    if (nMotionVectorCount == 1) {
        if (nMvFormat == kMvField && nDmv == 0) {
            mvfs[1][s] = mvfs[0][s] = (int)ReadMpegBits(w, 1);
        }
        DecodeMpegMotionVectorPair(w, PMV[0][s], dmvector, nHRSize, nVRSize, nDmv, nMvScale, 0);
        PMV[1][s][0] = PMV[0][s][0];
        PMV[1][s][1] = PMV[0][s][1];
        return;
    }
    mvfs[0][s] = (int)ReadMpegBits(w, 1);
    DecodeMpegMotionVectorPair(w, PMV[0][s], dmvector, nHRSize, nVRSize, nDmv, nMvScale, 0);
    mvfs[1][s] = (int)ReadMpegBits(w, 1);
    DecodeMpegMotionVectorPair(w, PMV[1][s], dmvector, nHRSize, nVRSize, nDmv, nMvScale, 0);
}

// NTSC-U/C: 0x0030b528, PAL: 0x003779b8
static int ReadMpegMacroblockAddressIncrement(MpegContext *w) {
    int nIncrement = 0;
    int nMore;

    do {
        int nCode = DecodeIpuVlc(w, kVlcMacroblockAddressIncrement);

        if (nCode == kMbaiStuffing) {
            nMore = 1;
        } else if (nCode < kMbaiEscape) {
            if (nCode == 0) {
                // The binary peeks before it tests the stream, and takes the stuffing code in an
                // MPEG-2 stream only.
                const unsigned int nPeek = PeekIpuBits(w, 11);
                if (w->mMpeg2 != 0 && nPeek == kMbaiStuffingCode) {
                    ConsumeIpuBits(w, 11);
                    nMore = 1;
                } else {
                    _sceMpegErrorFormat(w,
                                        "Invalid macroblock_address_increment code(0x%08x)",
                                        (unsigned int)nCode);
                    w->mFault = 1;
                    return 1;
                }
            } else {
                nIncrement += nCode;
                nMore = 0;
            }
        } else if (nCode == kMbaiEscape) {
            nIncrement += kMbaiEscapeIncrement;
            nMore = 1;
        } else {
            nIncrement += nCode;
            nMore = 0;
        }
    } while (nMore != 0);
    return nIncrement;
}

// NTSC-U/C: 0x0030bb40, PAL: 0x00377fd0
// Parses the modes and vectors of a coded macroblock and starts the block decode.
static int DecodeMpegMacroblockType(MpegContext *w,
                                    int *pMbType,
                                    int *pMotionType,
                                    int *pDctType,
                                    int PMV[2][2][2],
                                    int mvfs[2][2],
                                    int dmvector[2]) {
    int nMbType;
    int nMotionVectorCount;
    int nMvFormat;
    int nDmv;
    int nMvScale;

    *IPU_CTRL = (*IPU_CTRL & ~(unsigned int)kIpuCtrlPctMask) |
                ((unsigned int)w->mPictureCodingType << kIpuCtrlPctShift);
    nMbType = DecodeIpuVlc(w, kVlcMacroblockType);
    *pMbType = nMbType;
    if (nMbType == 0) {
        _sceMpegError(w, "Invalid macroblock_type code: 0");
        w->mFault = 1;
        return 0;
    }
    if ((nMbType & (kMbMotionForward | kMbMotionBackward)) != 0) {
        if (w->mPictureStructure == kFramePicture && w->mFramePredFrameDct != 0) {
            *pMotionType = kMcFrame;
        } else {
            *pMotionType = (int)ReadMpegBits(w, 2);
        }
    } else if ((nMbType & kMbIntra) != 0 && w->mConcealmentMotionVectors != 0) {
        *pMotionType = w->mPictureStructure == kFramePicture ? kMcFrame : kMcField;
    }
    if (w->mPictureStructure == kFramePicture) {
        nMotionVectorCount = *pMotionType == kMcField ? 2 : 1;
        nMvFormat = *pMotionType == kMcFrame ? kMvFrame : kMvField;
    } else {
        nMotionVectorCount = *pMotionType == kMc16x8 ? 2 : 1;
        nMvFormat = kMvField;
    }
    nDmv = *pMotionType == kMcDmv;
    nMvScale = nMvFormat == kMvField && w->mPictureStructure == kFramePicture;
    *pDctType = w->mPictureStructure == kFramePicture && w->mFramePredFrameDct == 0 &&
                        (*pMbType & (kMbPattern | kMbIntra)) != 0 ?
                    (int)ReadMpegBits(w, 1) :
                    0;
    if ((*pMbType & kMbQuant) != 0) {
        w->mQuantizerScaleCode = (int)ReadMpegBits(w, 5);
    }
    if ((*pMbType & kMbMotionForward) != 0 ||
        ((*pMbType & kMbIntra) != 0 && w->mConcealmentMotionVectors != 0)) {
        if (w->mMpeg2 != 0) {
            mpc___motionVectors(w,
                                PMV,
                                dmvector,
                                mvfs,
                                0,
                                nMotionVectorCount,
                                nMvFormat,
                                w->mFCode[0][0] - 1,
                                w->mFCode[0][1] - 1,
                                nDmv,
                                nMvScale);
        } else {
            DecodeMpegMotionVectorPair(w,
                                       PMV[0][0],
                                       dmvector,
                                       w->mForwardFCode - 1,
                                       w->mForwardFCode - 1,
                                       0,
                                       0,
                                       w->mFullPelForwardVector);
        }
    }
    if (w->mFault != 0) {
        return 0;
    }
    if ((*pMbType & kMbMotionBackward) != 0) {
        if (w->mMpeg2 != 0) {
            mpc___motionVectors(w,
                                PMV,
                                dmvector,
                                mvfs,
                                1,
                                nMotionVectorCount,
                                nMvFormat,
                                w->mFCode[1][0] - 1,
                                w->mFCode[1][1] - 1,
                                0,
                                nMvScale);
        } else {
            DecodeMpegMotionVectorPair(w,
                                       PMV[0][1],
                                       dmvector,
                                       w->mBackwardFCode - 1,
                                       w->mBackwardFCode - 1,
                                       0,
                                       0,
                                       w->mFullPelBackwardVector);
        }
    }
    if (w->mFault != 0) {
        return 0;
    }
    if ((*pMbType & kMbIntra) != 0 && w->mConcealmentMotionVectors != 0) {
        ConsumeIpuBits(w, 1);
    }
    if ((*pMbType & (kMbPattern | kMbIntra)) != 0) {
        StartIpuFromDmaTransfer(w->mMcBuffers[w->mMcCurrent].mCoefficients, 2 * kMacroblockSize);
        WaitForIpuIdle(w);
        IssueIpuCommand(
            w,
            ((unsigned int)(*pMbType & kMbIntra) << kIpuBdecIntraShift) |
                ((unsigned int)w->mQuantizerScaleCode << kIpuBdecQscShift) |
                (((unsigned int)w->mResetDcPredictor << kIpuBdecDcResetShift) | kIpuCmdBdec) |
                ((unsigned int)*pDctType << kIpuBdecDctTypeShift));
    } else {
        w->mMcBuffers[w->mMcCurrent].mNotCoded = 1;
    }
    w->mResetDcPredictor = 0;
    if (w->mFault != 0) {
        return 0;
    }
    if ((*pMbType & kMbIntra) == 0) {
        w->mResetDcPredictor = 1;
    }
    if ((*pMbType & kMbIntra) != 0 && w->mConcealmentMotionVectors == 0) {
        PMV[0][0][0] = PMV[0][0][1] = PMV[1][0][0] = PMV[1][0][1] = 0;
        PMV[0][1][0] = PMV[0][1][1] = PMV[1][1][0] = PMV[1][1][1] = 0;
    }
    if (w->mPictureCodingType == kPictureP && (*pMbType & (kMbMotionForward | kMbIntra)) == 0) {
        PMV[0][0][0] = PMV[0][0][1] = PMV[1][0][0] = PMV[1][0][1] = 0;
        if (w->mPictureStructure == kFramePicture) {
            *pMotionType = kMcFrame;
        } else {
            *pMotionType = kMcField;
            mvfs[0][0] = w->mPictureStructure == kBottomField;
        }
    }
    return 1;
}

// NTSC-U/C: 0x0030ba80, PAL: 0x00377f10
static int HandleMpegSkippedMacroblock(
    MpegContext *w, int PMV[2][2][2], int *pMotionType, int mvfs[2][2], int *pMbType) {
    int nResult = 1;

    w->mMcBuffers[w->mMcCurrent].mNotCoded = 1;
    w->mResetDcPredictor = 1;
    if (w->mPictureCodingType == kPictureP) {
        PMV[0][0][0] = PMV[0][0][1] = PMV[1][0][0] = PMV[1][0][1] = 0;
    }
    if (w->mPictureStructure == kFramePicture) {
        *pMotionType = kMcFrame;
    } else {
        *pMotionType = kMcField;
        mvfs[0][0] = mvfs[0][1] = w->mPictureStructure == kBottomField;
    }
    if (w->mPictureCodingType == kPictureI) {
        _sceMpegError(w, "skiped macroblock in I picure is not allowed");
        nResult = 0;
    }
    *pMbType &= ~kMbIntra;
    return nResult;
}

// NTSC-U/C: 0x0030cf80, PAL: 0x00379410
static void SkipMpegExtraInformation(MpegContext *w) {
    while (ReadMpegBits(w, 1) != 0) {
        ConsumeIpuBits(w, 8);
    }
}

// NTSC-U/C: 0x0030ca28, PAL: 0x00378eb8
// Returns the slice_vertical_position_extension, always zero.
static int ReadMpegSliceQuantizer(MpegContext *w) {
    w->mQuantizerScaleCode = (int)ReadMpegBits(w, 5);
    if (ReadMpegBits(w, 1) != 0) {
        (void)ReadMpegBits(w, 1);
        ConsumeIpuBits(w, 7);
        SkipMpegExtraInformation(w);
    }
    return 0;
}

// NTSC-U/C: 0x0030b758, PAL: 0x00377be8
static int
ParseMpegSliceHeader(MpegContext *w, int nMbaMax, int *pMba, int *pMbaInc, int PMV[2][2][2]) {
    unsigned int nCode;
    int nExtension;
    int nIncrement;

    (void)nMbaMax;
    w->mFault = 0;
    AlignToMpegStartCode(w);
    nCode = PeekIpuBits(w, 32);
    if (nCode - kSliceStartCodeMin >= kSliceStartCodeCount) {
        _sceMpegErrorFormat(w, "slice_start_code(0x%08x) out of range", nCode);
        return kSliceError;
    }
    ConsumeIpuBits(w, 32);
    nExtension = ReadMpegSliceQuantizer(w);
    nIncrement = ReadMpegMacroblockAddressIncrement(w);
    *pMbaInc = nIncrement;
    if (w->mFault != 0) {
        _sceMpegError(w, "_sliceA0(): error happens");
        return kSliceNext;
    }
    *pMba = ((nExtension << 7) + (int)(nCode & 0xff) - 1) * w->mMbWidth + nIncrement - 1;
    *pMbaInc = 1;
    w->mResetDcPredictor = 1;
    PMV[0][0][0] = PMV[0][0][1] = PMV[1][0][0] = PMV[1][0][1] = 0;
    PMV[0][1][0] = PMV[0][1][1] = PMV[1][1][0] = PMV[1][1][1] = 0;
    return kSliceDone;
}

// NTSC-U/C: 0x0030b888, PAL: 0x00377d18
static int DecodeMpegSliceMacroblocks(MpegContext *w, int nMbaMax) {
    // The binary leaves the modes and selectors uninitialised until a macroblock sets them.
    int PMV[2][2][2];
    int mvfs[2][2] = {{0, 0}, {0, 0}};
    int dmvector[2];
    int nMba = 0;
    int nMbaInc = 0;
    int nMbType = 0;
    int nMotionType = 0;
    int nDctType;
    int nResult = ParseMpegSliceHeader(w, nMbaMax, &nMba, &nMbaInc, PMV);

    if (nResult != 0) {
        return nResult;
    }
    w->mFault = 0;
    for (;;) {
        if (nMba >= nMbaMax) {
            return kSliceDone;
        }
        w->mMcBuffers[w->mMcCurrent].mNotCoded = 0;
        if (WaitForIpuBlockDecode(w) == 0) {
            if (w->mAbort != 0) {
                w->mAbort = 0;
                return kSliceAbort;
            }
            return kSliceError;
        }
        if (nMbaInc == 0) {
            if (PeekIpuBits(w, 23) == 0 || w->mFault != 0) {
                w->mFault = 0;
                return kSliceResync;
            }
            nMbaInc = ReadMpegMacroblockAddressIncrement(w);
            if (w->mFault != 0) {
                w->mFault = 0;
                return kSliceNext;
            }
        }
        if (nMba >= nMbaMax) {
            _sceMpegError(w, "Too many macroblocks in picture");
            return kSliceError;
        }
        if (nMbaInc == 1) {
            if (DecodeMpegMacroblockType(
                    w, &nMbType, &nMotionType, &nDctType, PMV, mvfs, dmvector) == 0) {
                w->mFault = 0;
                return kSliceNext;
            }
        } else if (HandleMpegSkippedMacroblock(w, PMV, &nMotionType, mvfs, &nMbType) == 0) {
            w->mFault = 0;
            return kSliceError;
        }
        if (DispatchMpegMacroblockDecode(
                w, nMba, nMbaInc, nMbType, nMotionType, PMV, mvfs, dmvector) == 0) {
            w->mFault = 0;
            return kSliceError;
        }
        if (nMba != 0) {
            ReconstructMpegMacroblockPixels(w, w->mMcCurrent ^ 1);
        }
        ++nMba;
        --nMbaInc;
        w->mMcCurrent ^= 1;
    }
}

// NTSC-U/C: 0x0030b638, PAL: 0x00377ac8
// Decodes the slices of a picture. Returns nonzero when the picture decoded.
static int DecodeMpegPictureSlices(MpegContext *w) {
    int nMbaMax;
    int nResult;

    w->mMcCurrent = 0;
    nMbaMax = w->mMbWidth * w->mMbHeight;
    w->mPictureResetWord = 0;
    if (w->mPictureStructure != kFramePicture) {
        nMbaMax >>= 1;
    }
    do {
        nResult = DecodeMpegSliceMacroblocks(w, nMbaMax);
    } while (nResult == kSliceNext || nResult == kSliceResync);
    WaitForIpuIdle(w);
    if (WaitForIpuBlockDecode(w) == 0) {
        if (w->mAbort != 0) {
            w->mAbort = 0;
            nResult = kSliceAbort;
        } else {
            nResult = kSliceError;
        }
    }
    while ((*D9_CHCR & kChcrStart) != 0) {
    }
    if (nResult == kSliceDone) {
        ReconstructMpegMacroblockPixels(w, w->mMcCurrent == 0);
    }
    if ((unsigned int)(nResult - 1) < 2) {
        _sceMpegError(w, "= Skip to the next picture =");
    }
    return nResult == kSliceDone;
}

#pragma mark - Sequence and picture headers

// NTSC-U/C: 0x00308b70, PAL: 0x00375000
static void SetPlaneRectDimensions(MpegPicture *pPicture, int nWidth, int nHeight) {
    pPicture->mMbWidth = nWidth >> 4;
    pPicture->mMbHeight = nHeight >> 4;
    pPicture->mWidth = nWidth;
    pPicture->mHeight = nHeight;
}

// NTSC-U/C: 0x00308f60, PAL: 0x003753f0
// Points the three frames at the buffers, and each frame's fields at its two halves.
static void InitPlaneDmaAddressSlots(MpegPicture *pFrame0,
                                     MpegPicture *pFrame1,
                                     MpegPicture *pFrame2,
                                     MpegPicture *pTop0,
                                     MpegPicture *pTop1,
                                     MpegPicture *pTop2,
                                     MpegPicture *pBottom0,
                                     MpegPicture *pBottom1,
                                     MpegPicture *pBottom2,
                                     unsigned char *pBuffer0,
                                     unsigned char *pBuffer1,
                                     unsigned char *pBuffer2,
                                     int nWidth,
                                     int nHeight) {
    int nFieldOffset = nWidth * nHeight / 512 * kMacroblockSize;

    pFrame0->mData = Uncached(pBuffer0);
    pFrame1->mData = Uncached(pBuffer1);
    pFrame2->mData = Uncached(pBuffer2);
    pTop0->mData = Uncached(pBuffer0);
    pTop1->mData = Uncached(pBuffer1);
    pTop2->mData = Uncached(pBuffer2);
    pBottom0->mData = Uncached(pBuffer0 + nFieldOffset);
    pBottom1->mData = Uncached(pBuffer1 + nFieldOffset);
    pBottom2->mData = Uncached(pBuffer2 + nFieldOffset);
}

// NTSC-U/C: 0x00309040, PAL: 0x003754d0
static void
UploadQuantMatrixToIpu(MpegContext *w, unsigned int nCommand, const unsigned char *pMatrix) {
    sceMpegCbData cbData;
    int nInterrupts;

    cbData.type = sceMpegCbStopDMA;
    _sceMpegCallback(w->mMpeg, &cbData);
    WaitForIpuIdle(w);
    *IPU_CMD = kIpuCmdBclr;
    WaitForIpuIdle(w);
    nInterrupts = DIntr();
    *D4_MADR = PhysicalAddress(pMatrix);
    *D4_QWC = kQuantMatrixQuadwords;
    *D4_CHCR = kChcrStart | kChcrFromMemory;
    if (nInterrupts != 0) {
        EIntr();
    }
    IssueIpuCommand(w, nCommand);
    WaitForIpuIdle(w);
    cbData.type = sceMpegCbRestartDMA;
    _sceMpegCallback(w->mMpeg, &cbData);
}

// NTSC-U/C: 0x00308cb8, PAL: 0x00375148
// Sizes the decoder for the sequence and carves its three frame buffers on a size change.
static void SetupMpegPictureBuffers(sceMpeg *mp) {
    MpegContext *w = mp->sys;
    int nWidth;
    int nHeight;
    unsigned int nSize;

    if (w->mMpeg2 == 0) {
        w->mPictureStructure = kFramePicture;
        w->mFramePredFrameDct = 1;
        w->mMatrixCoefficients = 5;
        w->mProgressiveSequence = 1;
        w->mChromaFormat = 1;
        w->mProgressiveFrame = 1;
    }
    w->mMbWidth = (w->mHorizontalSize + 15) >> 4;
    if (w->mMpeg2 != 0 && w->mProgressiveSequence == 0) {
        w->mMbHeight = ((w->mVerticalSize + 31) >> 5) << 1;
    } else {
        w->mMbHeight = (w->mVerticalSize + 15) >> 4;
    }
    nHeight = w->mMbHeight << 4;
    nWidth = w->mMbWidth << 4;
    if (nWidth == mp->width && nHeight == mp->height) {
        return;
    }
    mp->height = nHeight;
    mp->width = nWidth;
    nSize = (unsigned int)(nWidth * (nHeight * kMacroblockSize)) >> 8;
    _sceMpegAlalcFree(&w->mAlalc);
    w->mFrameBuffers[0] = _sceMpegAlalcAlloc(w, &w->mAlalc, nSize, kFrameBufferAlign);
    w->mFrameBuffers[1] = _sceMpegAlalcAlloc(w, &w->mAlalc, nSize, kFrameBufferAlign);
    w->mFrameBuffers[2] = _sceMpegAlalcAlloc(w, &w->mAlalc, nSize, kFrameBufferAlign);
    InitPlaneDmaAddressSlots(&w->mPictures[0],
                             &w->mPictures[1],
                             &w->mPictures[2],
                             &w->mPictures[3],
                             &w->mPictures[4],
                             &w->mPictures[5],
                             &w->mPictures[6],
                             &w->mPictures[7],
                             &w->mPictures[8],
                             w->mFrameBuffers[0],
                             w->mFrameBuffers[1],
                             w->mFrameBuffers[2],
                             nWidth,
                             nHeight);
    SetPlaneRectDimensions(&w->mPictures[0], nWidth, nHeight);
    SetPlaneRectDimensions(&w->mPictures[1], nWidth, nHeight);
    SetPlaneRectDimensions(&w->mPictures[2], nWidth, nHeight);
    SetPlaneRectDimensions(&w->mPictures[3], nWidth, nHeight >> 1);
    SetPlaneRectDimensions(&w->mPictures[4], nWidth, nHeight >> 1);
    SetPlaneRectDimensions(&w->mPictures[5], nWidth, nHeight >> 1);
    SetPlaneRectDimensions(&w->mPictures[6], nWidth, nHeight >> 1);
    SetPlaneRectDimensions(&w->mPictures[7], nWidth, nHeight >> 1);
    SetPlaneRectDimensions(&w->mPictures[8], nWidth, nHeight >> 1);
}

// NTSC-U/C: 0x00309120, PAL: 0x003755b0
static void ParseMpegSequenceExtension(MpegContext *w) {
    unsigned int nBits;
    unsigned int nBitRateExtension;
    unsigned int nHorizontalExtension;
    unsigned int nVerticalExtension;
    unsigned int nProfileAndLevel;
    unsigned int nVbvExtension;

    w->mMpeg2 = 1;
    SetIpuControlEnableBit(0);
    nBits = ReadMpegBits(w, 28);
    nBitRateExtension = (nBits >> 1) & 0xfff;
    nVerticalExtension = (nBits >> 13) & 3;
    nHorizontalExtension = (nBits >> 15) & 3;
    w->mChromaFormat = (int)((nBits >> 17) & 3);
    if (w->mChromaFormat != 1) {
        _sceMpegError(w, "_chroma_format needs to be 1: 420");
    }
    w->mProgressiveSequence = (int)((nBits >> 19) & 1);
    nProfileAndLevel = nBits >> 20;
    nVbvExtension = ReadMpegBits(w, 16) >> 8;
    if (nProfileAndLevel != 0x48 && nProfileAndLevel != 0x58 && nProfileAndLevel != 0x44) {
        _sceMpegError(w, "Unsupported profile/level");
    }
    w->mHorizontalSize = (int)((nHorizontalExtension << 12) | (w->mHorizontalSize & 0xfff));
    w->mVerticalSize = (int)((nVerticalExtension << 12) | (w->mVerticalSize & 0xfff));
    w->mBitRate += (int)(nBitRateExtension << 18);
    w->mVbvBufferSize += (int)(nVbvExtension << 10);
}

// NTSC-U/C: 0x00309250, PAL: 0x003756e0
static void ParseMpegSequenceRateInfo(MpegContext *w) {
    (void)ReadMpegBits(w, 3);
    if (ReadMpegBits(w, 1) != 0) {
        (void)ReadMpegBits(w, 8);
        (void)ReadMpegBits(w, 8);
        w->mMatrixCoefficients = (int)ReadMpegBits(w, 8);
    }
    w->mSequenceDisplayHorizontalSize = (int)ReadMpegBits(w, 14);
    (void)ReadMpegBits(w, 1);
    w->mSequenceDisplayVerticalSize = (int)ReadMpegBits(w, 14);
}

// NTSC-U/C: 0x003092e0, PAL: 0x00375770
static void HandleMpegSequenceScalableExtUnsupported(MpegContext *w) {
    _sceMpegError(w, "_sequenceScalableExtension() is not implemented");
}

// NTSC-U/C: 0x003092f0, PAL: 0x00375780
static void HandleMpegUnknownExtension(MpegContext *w) {
    _sceMpegError(w, "Unknown Extension");
}

// NTSC-U/C: 0x00309300, PAL: 0x00375790
static void HandleMpegPictureSpatialScalableExtUnsupported(MpegContext *w) {
    _sceMpegError(w, "_pictureSpatialScalableExtension is not supported");
}

// NTSC-U/C: 0x00309310, PAL: 0x003757a0
static void HandleMpegPictureTemporalScalableExtUnsupported(MpegContext *w) {
    _sceMpegError(w, "_pictureTemporalScalableExtension is not supported");
}

// Loads one quantiser matrix from the bitstream.
static inline void LoadMatrixFromStream(MpegContext *w, unsigned int nCommand) {
    WaitForIpuIdle(w);
    IssueIpuCommand(w, nCommand);
    WaitForIpuIdle(w);
}

// NTSC-U/C: 0x0030d0e0, PAL: 0x00379570
static void LoadMpegQuantizerMatrices(MpegContext *w) {
    w->mLoadIntraQuantiserMatrix = (int)ReadMpegBits(w, 1);
    if (w->mLoadIntraQuantiserMatrix != 0) {
        LoadMatrixFromStream(w, kIpuCmdSetIqIntra);
    }
    w->mLoadNonIntraQuantiserMatrix = (int)ReadMpegBits(w, 1);
    if (w->mLoadNonIntraQuantiserMatrix != 0) {
        LoadMatrixFromStream(w, kIpuCmdSetIqNonIntra);
    }
    if (ReadMpegBits(w, 1) != 0) {
        _sceMpegError(w, "load_chroma_intra_quantizer_matrix == 1");
    }
    if (ReadMpegBits(w, 1) != 0) {
        _sceMpegError(w, "load_chroma_non_intra_quantizer_matrix == 1");
    }
}

// NTSC-U/C: 0x0030d2a0, PAL: 0x00379730
static void SkipMpegCopyrightExtension(MpegContext *w) {
    (void)ReadMpegBits(w, 1);
    (void)ReadMpegBits(w, 8);
    (void)ReadMpegBits(w, 1);
    (void)ReadMpegBits(w, 7);
    (void)ReadMpegBits(w, 1);
    (void)ReadMpegBits(w, 20);
    (void)ReadMpegBits(w, 1);
    (void)ReadMpegBits(w, 22);
    (void)ReadMpegBits(w, 1);
    (void)ReadMpegBits(w, 22);
}

// NTSC-U/C: 0x0030d1a8, PAL: 0x00379638
static void ParseMpegPictureDisplayExtension(MpegContext *w) {
    int nOffsets;
    int i;

    if (w->mProgressiveSequence != 0) {
        if (w->mRepeatFirstField != 0) {
            nOffsets = w->mTopFieldFirst != 0 ? 3 : 2;
        } else {
            nOffsets = 1;
        }
    } else if (w->mPictureStructure != kFramePicture) {
        nOffsets = 1;
    } else {
        nOffsets = w->mRepeatFirstField != 0 ? 3 : 2;
    }
    for (i = 0; i < nOffsets; ++i) {
        w->mPictureFrameCentreHorizontalOffset[i] = (int)ReadMpegBits(w, 16);
        (void)ReadMpegBits(w, 1);
        w->mPictureFrameCentreVerticalOffset[i] = (int)ReadMpegBits(w, 16);
        (void)ReadMpegBits(w, 1);
    }
}

static inline void SetIpuControlBit(unsigned int nBit, unsigned int nShift, unsigned int nValue) {
    *IPU_CTRL = (*IPU_CTRL & ~nBit) | (nValue << nShift);
}

// NTSC-U/C: 0x0030cd90, PAL: 0x00379220
static void ParseMpegPictureCodingExtension(MpegContext *w) {
    w->mFCode[0][0] = (int)ReadMpegBits(w, 4);
    w->mFCode[0][1] = (int)ReadMpegBits(w, 4);
    w->mFCode[1][0] = (int)ReadMpegBits(w, 4);
    w->mFCode[1][1] = (int)ReadMpegBits(w, 4);
    SetIpuControlBit(kIpuCtrlIdpMask, kIpuCtrlIdpShift, ReadMpegBits(w, 2));
    w->mPictureStructure = (int)ReadMpegBits(w, 2);
    if (w->mFirstFieldStructure == 0) {
        w->mFirstFieldStructure = w->mPictureStructure;
    }
    w->mTopFieldFirst = (int)ReadMpegBits(w, 1);
    w->mFramePredFrameDct = (int)ReadMpegBits(w, 1);
    w->mConcealmentMotionVectors = (int)ReadMpegBits(w, 1);
    SetIpuControlBit(kIpuCtrlQst, kIpuCtrlQstShift, ReadMpegBits(w, 1));
    SetIpuControlBit(kIpuCtrlIntraVlc, kIpuCtrlIntraVlcShift, ReadMpegBits(w, 1));
    SetIpuControlBit(kIpuCtrlAlternateScan, kIpuCtrlAlternateScanShift, ReadMpegBits(w, 1));
    w->mRepeatFirstField = (int)ReadMpegBits(w, 1);
    (void)ReadMpegBits(w, 1);
    w->mProgressiveFrame = (int)ReadMpegBits(w, 1);
    if (ReadMpegBits(w, 1) != 0) {
        (void)ReadMpegBits(w, 1);
        (void)ReadMpegBits(w, 3);
        (void)ReadMpegBits(w, 1);
        (void)ReadMpegBits(w, 7);
        (void)ReadMpegBits(w, 8);
    }
}

// NTSC-U/C: 0x0030cc68, PAL: 0x003790f8
static void ParseMpegPictureExtensions(MpegContext *w) {
    void (*const apfnExtensions[kExtensionIdCount])(MpegContext *) = {
        HandleMpegUnknownExtension,
        ParseMpegSequenceExtension,
        ParseMpegSequenceRateInfo,
        LoadMpegQuantizerMatrices,
        SkipMpegCopyrightExtension,
        HandleMpegSequenceScalableExtUnsupported,
        HandleMpegUnknownExtension,
        ParseMpegPictureDisplayExtension,
        ParseMpegPictureCodingExtension,
        HandleMpegPictureSpatialScalableExtUnsupported,
        HandleMpegPictureTemporalScalableExtUnsupported,
    };
    unsigned int nCode;
    unsigned int nId;

    AlignToMpegStartCode(w);
    for (;;) {
        nCode = PeekIpuBits(w, 32);
        if (nCode == kExtensionStartCode) {
            ConsumeIpuBits(w, 32);
            nId = ReadMpegBits(w, 4);
            if (nId >= kExtensionIdCount) {
                nId = 0;
            }
            apfnExtensions[nId](w);
            AlignToMpegStartCode(w);
        } else if (nCode == kUserDataStartCode) {
            ConsumeIpuBits(w, 32);
            AlignToMpegStartCode(w);
        } else {
            return;
        }
    }
}

// NTSC-U/C: 0x00308b90, PAL: 0x00375020
static void ParseMpegSequenceHeader(MpegContext *w) {
    unsigned int nBits;

    w->mFirstFieldStructure = 0;
    nBits = ReadMpegBits(w, 32);
    w->mHorizontalSize = (int)(nBits >> 20);
    w->mVerticalSize = (int)((nBits >> 8) & 0xfff);
    if (w->mVerticalSize > kMaxVerticalSize) {
        _sceMpegError(w, "vertical size > 2800");
    }
    nBits = ReadMpegBits(w, 30);
    w->mBitRate = (int)(nBits >> 12);
    w->mVbvBufferSize = (int)((nBits >> 1) & 0x3ff);
    w->mLoadIntraQuantiserMatrix = (int)ReadMpegBits(w, 1);
    if (w->mLoadIntraQuantiserMatrix != 0) {
        LoadMatrixFromStream(w, kIpuCmdSetIqIntra);
    } else {
        UploadQuantMatrixToIpu(w, kIpuCmdSetIqIntra, g_abDefaultIntraQuantMatrix);
    }
    w->mLoadNonIntraQuantiserMatrix = (int)ReadMpegBits(w, 1);
    if (w->mLoadNonIntraQuantiserMatrix != 0) {
        LoadMatrixFromStream(w, kIpuCmdSetIqNonIntra);
    } else {
        UploadQuantMatrixToIpu(w, kIpuCmdSetIqNonIntra, g_abDefaultNonIntraQuantMatrix);
    }
    ParseMpegPictureExtensions(w);
    SetupMpegPictureBuffers(w->mMpeg);
}

// NTSC-U/C: 0x0030d040, PAL: 0x003794d0
static void ParseMpegGroupOfPicturesHeader(MpegContext *w) {
    w->mForceBrokenLink = 0;
    w->mNewGop = 1;
    w->mGopPictureBase = w->mMaxPictureNumber + 1;
    (void)ReadMpegBits(w, 1);
    (void)ReadMpegBits(w, 5);
    (void)ReadMpegBits(w, 6);
    (void)ReadMpegBits(w, 1);
    (void)ReadMpegBits(w, 6);
    (void)ReadMpegBits(w, 6);
    w->mClosedGop = (int)ReadMpegBits(w, 1);
    w->mBrokenLink = (int)ReadMpegBits(w, 1);
    ParseMpegPictureExtensions(w);
}

// NTSC-U/C: 0x0030cfc8, PAL: 0x00379458
static void UpdateMpegPictureDisplayOrder(MpegContext *w, int nTemporalReference) {
    int nWrapped = 0;

    // Yes, the binary only wraps for a negative temporal reference, which ten bits never hold.
    if (w->mPictureCodingType != kPictureB && nTemporalReference != 0) {
        if (nTemporalReference < 0) {
            nWrapped = w->mNewGop == 0;
        }
        w->mNewGop = 0;
    }
    w->mPictureNumber = w->mGopPictureBase + nTemporalReference;
    if (nWrapped != 0) {
        w->mPictureNumber = w->mGopPictureBase + nTemporalReference + 1024;
    }
    if (w->mMaxPictureNumber < w->mPictureNumber) {
        w->mMaxPictureNumber = w->mPictureNumber;
    }
}

// NTSC-U/C: 0x0030cba0, PAL: 0x00379030
static void ParseMpegPictureHeader(MpegContext *w) {
    int nTemporalReference = (int)ReadMpegBits(w, 10);

    w->mPictureCodingType = (int)ReadMpegBits(w, 3);
    (void)ReadMpegBits(w, 16);
    if (w->mPictureCodingType == kPictureP || w->mPictureCodingType == kPictureB) {
        w->mFullPelForwardVector = (int)ReadMpegBits(w, 1);
        w->mForwardFCode = (int)ReadMpegBits(w, 3);
    }
    if (w->mPictureCodingType == kPictureB) {
        w->mFullPelBackwardVector = (int)ReadMpegBits(w, 1);
        w->mBackwardFCode = (int)ReadMpegBits(w, 3);
    }
    SkipMpegExtraInformation(w);
    ParseMpegPictureExtensions(w);
    UpdateMpegPictureDisplayOrder(w, nTemporalReference);
}

// NTSC-U/C: 0x0030ca90, PAL: 0x00378f20
// Parses headers up to the next picture. Returns its coding type, or 0 at the sequence end.
static int ParseMpegToNextPicture(MpegContext *w) {
    sceMpegCbData cbData;

    for (;;) {
        AlignToMpegStartCode(w);
        switch (ReadMpegBits(w, 32)) {
        case kSequenceHeaderCode:
            ParseMpegSequenceHeader(w);
            break;
        case kGroupStartCode:
            ParseMpegGroupOfPicturesHeader(w);
            break;
        case kPictureStartCode:
            ParseMpegPictureHeader(w);
            cbData.timeStamp.type = sceMpegCbTimeStamp;
            cbData.timeStamp.dts = -1;
            cbData.timeStamp.pts = -1;
            _sceMpegCallback(w->mMpeg, &cbData);
            w->mDts = cbData.timeStamp.dts;
            w->mPts = cbData.timeStamp.pts;
            return w->mPictureCodingType;
        case kSequenceEndCode:
            return 0;
        default:
            break;
        }
    }
}

#pragma mark - Picture output

// NTSC-U/C: 0x0030daa0, PAL: 0x00379f30
static void sceMpegGetPictureStamps(MpegContext *w,
                                    const MpegPicture *pPicture,
                                    long long *pPts,
                                    long long *pDts,
                                    unsigned long long *pFlags) {
    if (w->mUseDefaultPtsGap != 0) {
        long long nPts = pPicture->mPts;

        if (nPts >= 0 || w->mLastPts < 0) {
            *pPts = nPts;
        } else {
            long long nFields = (int)w->mDisplayFieldCount;
            int nOdd = w->mOddGapPictures;

            *pPts = w->mLastPts + (int)((w->mDefaultPtsGap * (unsigned long long)nFields) >> 1) +
                    (int)((unsigned long long)(nFields & 1) * (w->mDefaultPtsGap & 1) *
                          (unsigned long long)(nOdd & 1));
            if ((unsigned long long)(nFields & 1) * (w->mDefaultPtsGap & 1) != 0) {
                w->mOddGapPictures = nOdd + 1;
            }
        }
    } else {
        *pPts = pPicture->mPts;
    }
    if (w->mPendingPtsState == kPendingPtsReady && w->mPendingPts >= 0) {
        *pPts = w->mPendingPts;
        w->mPendingPtsState = 0;
        w->mPendingPts = -1;
    }
    *pDts = pPicture->mDts;
    *pFlags = (unsigned long long)pPicture->mPictureCodingType |
              ((unsigned long long)pPicture->mPictureStructure << 3) |
              ((unsigned long long)pPicture->mRepeatFirstField << 5) |
              ((unsigned long long)pPicture->mTopFieldFirst << 6) |
              ((unsigned long long)pPicture->mProgressiveFrame << 7) |
              ((unsigned long long)pPicture->mProgressiveSequence << 8);
}

// NTSC-U/C: 0x0030d748, PAL: 0x00379bd8
static int CheckMpegOutputBufferCapacity(MpegContext *w, const MpegPicture *pPicture) {
    char szMessage[kErrorMessageSize];
    int nFits;

    if (w->mOutputHeight != 0) {
        nFits = !(w->mOutputWidth < pPicture->mWidth) && !(w->mOutputHeight < pPicture->mHeight);
    } else {
        nFits = !(w->mOutputMacroblocks < pPicture->mMbWidth * pPicture->mMbHeight);
    }
    if (nFits == 0) {
        sprintf(szMessage,
                "Too small buffer size for %dx%d picture\n",
                pPicture->mWidth,
                pPicture->mHeight);
        _sceMpegError(w, szMessage);
    }
    return nFits;
}

// NTSC-U/C: 0x0030d7e8, PAL: 0x00379c78
// Copies the decoded macroblock columns to the output through the scratchpad.
static void CopyMpegMacroblocksToFrameViaSpr(MpegContext *w, const MpegPicture *pPicture) {
    unsigned int nSource = PhysicalAddress(pPicture->mData);
    unsigned int nDest = PhysicalAddress(w->mPictureAddress);
    unsigned int nColumnDest;
    int nColumnBytes;
    int nDestStride;
    int nPasses;
    int nPass;
    int nColumn;
    int nInterrupts;

    if (w->mPictureStructure == kFramePicture || w->mOutputHeight == 0) {
        nColumnBytes = pPicture->mMbHeight * kMacroblockSize;
        nDestStride =
            w->mOutputHeight != 0 ? (w->mOutputHeight >> 4) * kMacroblockSize : nColumnBytes;
        nPasses = 1;
    } else {
        nDestStride = (w->mOutputHeight >> 4) * (kMacroblockSize / 2);
        nColumnBytes = (pPicture->mMbHeight >> 1) * kMacroblockSize;
        nPasses = 2;
    }
    for (nPass = 0; nPass < nPasses; ++nPass) {
        nColumnDest = nDest;
        for (nColumn = 0; nColumn < pPicture->mMbWidth; ++nColumn) {
            nInterrupts = DIntr();
            *D9_SADR = 0;
            *D9_MADR = nSource;
            *D9_QWC = (unsigned int)(nColumnBytes >> 4);
            *D9_CHCR = kChcrStart | kChcrFromMemory;
            if (nInterrupts != 0) {
                EIntr();
            }
            nSource += (unsigned int)nColumnBytes;
            while ((*D9_CHCR & kChcrStart) != 0) {
            }
            nInterrupts = DIntr();
            *D8_SADR = 0;
            *D8_MADR = nColumnDest;
            *D8_QWC = (unsigned int)(nColumnBytes >> 4);
            *D8_CHCR = kChcrStart;
            if (nInterrupts != 0) {
                EIntr();
            }
            while ((*D8_CHCR & kChcrStart) != 0) {
            }
            while (*D8_QWC != 0) {
            }
            nColumnDest += (unsigned int)nDestStride;
        }
        // The second field goes to the second half of the output.
        nDest += (unsigned int)(w->mOutputMacroblocks * (kMacroblockSize / 2));
    }
}

// NTSC-U/C: 0x0030da78, PAL: 0x00379f08
static void SetMpegPictureReady(MpegContext *w) {
    if (w->mOutputState != kOutputShown) {
        w->mOutputState = kOutputShown;
        w->mFrameCountBase = w->mPictureCount;
    }
    w->mPictureReady = 1;
}

// NTSC-U/C: 0x0030e0f0, PAL: 0x0037a580
static void IssueIpuCscAndReadResult(MpegContext *w, unsigned char *pDest, int nMacroblocks) {
    sceMpegCbData cbData;
    int nInterrupts;

    while ((int)*IPU_CTRL < 0) {
    }
    nInterrupts = DIntr();
    *D3_MADR = PhysicalAddress(pDest);
    *D3_QWC = (unsigned int)(nMacroblocks << 6);
    *D3_CHCR = kChcrStart;
    if (nInterrupts != 0) {
        EIntr();
    }
    IssueIpuCommand(w, (unsigned int)nMacroblocks | kIpuCmdCsc);
    cbData.type = sceMpegCbBackground;
    _sceMpegCallback(w->mMpeg, &cbData);
    while ((*D3_CHCR & kChcrStart) != 0) {
    }
    while ((int)*IPU_CTRL < 0) {
    }
}

// The progress of a chunked colour conversion, which the DMA handler advances.
typedef struct {
    volatile int mChunksDone;
    int mError;
    int mMacroblocks;
    unsigned int mNextDest;
    int mChunks;
} CscProgress;

// NTSC-U/C: 0x0030e218, PAL: 0x0037a6a8
// Starts the next chunk of a colour conversion when the channel from the IPU finishes one.
static int csc___ch3dmaCSC(int nChannel, void *pArg, void *pAddress) {
    CscProgress *pProgress = pArg;

    (void)nChannel;
    (void)pAddress;
    *D_STAT = kDStatFromIpu;
    ++pProgress->mChunksDone;
    if (*D3_QWC != 0 || (*D3_CHCR & kChcrStart) != 0) {
        pProgress->mError = 1;
        ExitHandler();
        return 0;
    }
    if (pProgress->mChunksDone < pProgress->mChunks - 1) {
        *D3_MADR = pProgress->mNextDest;
        *D3_QWC = kCscChunkQuadwords;
        *D3_CHCR = kChcrStart;
        *IPU_CMD = kIpuCmdCsc | kCscChunkMacroblocks;
        pProgress->mNextDest = (pProgress->mNextDest + kCscChunkBytes) & kPhysicalAddressMask;
    } else if (pProgress->mChunksDone == pProgress->mChunks - 1) {
        pProgress->mMacroblocks -= (pProgress->mChunksDone << 10) - pProgress->mChunksDone;
        *D3_MADR = pProgress->mNextDest;
        *D3_QWC = (unsigned int)(pProgress->mMacroblocks << 6);
        *D3_CHCR = kChcrStart;
        *IPU_CMD = (unsigned int)pProgress->mMacroblocks | kIpuCmdCsc;
    }
    ExitHandler();
    return 0;
}

// NTSC-U/C: 0x0030e360, PAL: 0x0037a7f0
// Converts more macroblocks than one CSC command takes, in chunks of 1023.
static void csc___doCSC2(MpegContext *w, unsigned char *pDest, int nMacroblocks) {
    CscProgress progress;
    sceMpegCbData cbData;
    int nHandler;
    int nInterrupts;
    int nError;

    progress.mChunks = nMacroblocks / kCscChunkMacroblocks + 1;
    progress.mMacroblocks = nMacroblocks;
    progress.mNextDest = ((unsigned int)(uintptr_t)pDest + kCscChunkBytes) & kPhysicalAddressMask;
    progress.mError = 0;
    progress.mChunksDone = 0;
    while ((int)*IPU_CTRL < 0) {
    }
    nHandler = AddDmacHandler2(DMAC_FROM_IPU, csc___ch3dmaCSC, 0, &progress);
    *D_STAT = kDStatFromIpu;
    EnableDmac(DMAC_FROM_IPU);
    nInterrupts = DIntr();
    *D3_MADR = PhysicalAddress(pDest);
    *D3_QWC = kCscChunkQuadwords;
    *D3_CHCR = kChcrStart;
    if (nInterrupts != 0) {
        EIntr();
    }
    *IPU_CMD = kIpuCmdCsc | kCscChunkMacroblocks;
    cbData.type = sceMpegCbBackground;
    _sceMpegCallback(w->mMpeg, &cbData);
    nError = progress.mError; // Yes, the binary reads the error flag before the wait.
    while (progress.mChunksDone < progress.mChunks) {
    }
    if (nError != 0) {
        _sceMpegError(w, "CSC handler error\n");
    }
    while ((int)*IPU_CTRL < 0) {
    }
    DisableDmac(DMAC_FROM_IPU);
    RemoveDmacHandler(DMAC_FROM_IPU, nHandler);
}

// The remainder of a transfer to the IPU, which the DMA handler sends in chunks.
typedef struct {
    unsigned int mQuadwords;
    unsigned int mSource;
} ToIpuProgress;

// NTSC-U/C: 0x0030e520, PAL: 0x0037a9b0
static int csc___ch4dma(int nChannel, void *pArg, void *pAddress) {
    ToIpuProgress *pProgress = pArg;

    (void)nChannel;
    (void)pAddress;
    *D_STAT = kDStatToIpu;
    if (pProgress->mQuadwords == 0) {
        ExitHandler();
        return 1;
    }
    if (pProgress->mQuadwords > kToIpuChunkQuadwords) {
        *D4_MADR = pProgress->mSource;
        *D4_QWC = kToIpuChunkQuadwords;
        *D4_CHCR = kChcrStart | kChcrFromMemory;
        pProgress->mSource = (pProgress->mSource + kToIpuChunkBytes) & kPhysicalAddressMask;
        pProgress->mQuadwords -= kToIpuChunkQuadwords;
    } else {
        *D4_MADR = pProgress->mSource;
        *D4_QWC = pProgress->mQuadwords;
        *D4_CHCR = kChcrStart | kChcrFromMemory;
        pProgress->mQuadwords = 0;
    }
    ExitHandler();
    return 0;
}

// NTSC-U/C: 0x0030e5f8, PAL: 0x0037aa88
// Feeds a decoded picture back through the IPU to colour convert it into the output.
static void BlitMpegFrameViaIpuCsc(MpegContext *w, const MpegPicture *pPicture) {
    int nMacroblocks = pPicture->mMbWidth * pPicture->mMbHeight;
    sceMpegCbData cbData;
    ToIpuProgress progress;
    int nDirect;
    int nHandler;
    int nInterrupts;

    cbData.type = sceMpegCbStopDMA;
    _sceMpegCallback(w->mMpeg, &cbData);
    if ((*IPU_CTRL & kIpuCtrlErrorCode) != 0) {
        *IPU_CTRL = kIpuCtrlReset;
    }
    nDirect = nMacroblocks < kCscDirectLimit;
    while ((int)*IPU_CTRL < 0) {
    }
    IssueIpuCommand(w, kIpuCmdBclr);
    while ((int)*IPU_CTRL < 0) {
    }
    progress.mSource = PhysicalAddress(pPicture->mData);
    progress.mQuadwords = (unsigned int)(nMacroblocks * kMacroblockToIpuQuadwords);
    if (progress.mQuadwords > kToIpuChunkQuadwords) {
        nHandler = AddDmacHandler2(DMAC_TO_IPU, csc___ch4dma, 0, &progress);
        *D_STAT = kDStatToIpu;
        EnableDmac(DMAC_TO_IPU);
        nInterrupts = DIntr();
        *D4_MADR = progress.mSource;
        *D4_QWC = kToIpuChunkQuadwords;
        *D4_CHCR = kChcrStart | kChcrFromMemory;
        if (nInterrupts != 0) {
            EIntr();
        }
        progress.mSource = (progress.mSource + kToIpuChunkBytes) & kPhysicalAddressMask;
        progress.mQuadwords -= kToIpuChunkQuadwords;
        if (nDirect != 0) {
            IssueIpuCscAndReadResult(w, w->mPictureAddress, nMacroblocks);
        } else {
            csc___doCSC2(w, w->mPictureAddress, nMacroblocks);
        }
        DisableDmac(DMAC_TO_IPU);
        RemoveDmacHandler(DMAC_TO_IPU, nHandler);
    } else {
        nInterrupts = DIntr();
        *D4_MADR = PhysicalAddress(pPicture->mData);
        *D4_QWC = progress.mQuadwords;
        *D4_CHCR = kChcrStart | kChcrFromMemory;
        if (nInterrupts != 0) {
            EIntr();
        }
        progress.mQuadwords = 0;
        if (nDirect != 0) {
            IssueIpuCscAndReadResult(w, w->mPictureAddress, nMacroblocks);
        } else {
            csc___doCSC2(w, w->mPictureAddress, nMacroblocks);
        }
    }
    cbData.type = sceMpegCbRestartDMA;
    _sceMpegCallback(w->mMpeg, &cbData);
}

static inline void WritePicture(MpegContext *w, const MpegPicture *pPicture) {
    if (w->mConvertColours != 0) {
        BlitMpegFrameViaIpuCsc(w, pPicture);
    } else {
        CopyMpegMacroblocksToFrameViaSpr(w, pPicture);
    }
}

// NTSC-U/C: 0x0030dc30, PAL: 0x0037a0c0
static void DisplayMpegFramePicture(MpegContext *w, MpegPicture *pPicture, int nFrame) {
    sceMpeg *mp = w->mMpeg;

    (void)nFrame;
    sceMpegGetPictureStamps(w, pPicture, &mp->pts, &mp->dts, &mp->flags);
    mp = w->mMpeg;
    w->mLastPts = (int)mp->pts;
    w->mDisplayFieldCount = g_anMpegDisplayFieldCount[(int)(mp->flags >> 5) & 15];
    w->mDisplayHorizontalSize = pPicture->mDisplayHorizontalSize;
    w->mDisplayVerticalSize = pPicture->mDisplayVerticalSize;
    w->mFrameCentreHorizontalOffset[0] = pPicture->mFrameCentreHorizontalOffset[0];
    w->mFrameCentreHorizontalOffset[1] = pPicture->mFrameCentreHorizontalOffset[1];
    w->mFrameCentreHorizontalOffset[2] = pPicture->mFrameCentreHorizontalOffset[2];
    w->mFrameCentreVerticalOffset[0] = pPicture->mFrameCentreVerticalOffset[0];
    w->mFrameCentreVerticalOffset[1] = pPicture->mFrameCentreVerticalOffset[1];
    w->mFrameCentreVerticalOffset[2] = pPicture->mFrameCentreVerticalOffset[2];
    if (CheckMpegOutputBufferCapacity(w, pPicture) != 0 && pPicture->mDecoded == 1) {
        WritePicture(w, pPicture);
        SetMpegPictureReady(w);
    }
}

// NTSC-U/C: 0x0030dd40, PAL: 0x0037a1d0
static void
DisplayMpegFieldPicturePair(MpegContext *w, MpegPicture *pTop, MpegPicture *pBottom, int nFrame) {
    sceMpeg *mp;
    MpegPicture *pFirst;
    MpegPicture *pSecond;
    unsigned long long nTopFieldFirst = 0;

    (void)nFrame;
    if (w->mPictureStructure == kBottomField) {
        pFirst = pTop;
        pSecond = pBottom;
        nTopFieldFirst = 64;
    } else {
        pFirst = pBottom;
        pSecond = pTop;
    }
    mp = w->mMpeg;
    sceMpegGetPictureStamps(w, pFirst, &mp->pts, &mp->dts, &mp->flags);
    mp = w->mMpeg;
    w->mDisplayFieldCount = 1;
    w->mLastPts = (int)mp->pts;
    sceMpegGetPictureStamps(w, pSecond, &mp->pts2nd, &mp->dts2nd, &mp->flags2nd);
    mp = w->mMpeg;
    w->mDisplayFieldCount = 1;
    w->mLastPts = (int)mp->pts2nd;
    mp->flags |= nTopFieldFirst;
    mp->flags2nd |= nTopFieldFirst;
    w->mDisplayHorizontalSize = pFirst->mDisplayHorizontalSize;
    w->mDisplayVerticalSize = pFirst->mDisplayVerticalSize;
    w->mFrameCentreHorizontalOffset[0] = pFirst->mFrameCentreHorizontalOffset[0];
    w->mFrameCentreHorizontalOffset[1] = pSecond->mFrameCentreHorizontalOffset[1];
    w->mFrameCentreVerticalOffset[0] = pFirst->mFrameCentreVerticalOffset[0];
    w->mFrameCentreVerticalOffset[1] = pSecond->mFrameCentreVerticalOffset[1];
    if (CheckMpegOutputBufferCapacity(w, pTop) != 0 && pTop->mDecoded == 1 &&
        pBottom->mDecoded == 1) {
        // The top field picture spans the frame for the output.
        pTop->mMbHeight <<= 1;
        WritePicture(w, pTop);
        pTop->mMbHeight >>= 1;
        SetMpegPictureReady(w);
    }
}

// NTSC-U/C: 0x0030d490, PAL: 0x00379920
// Rotates the reference pictures for the new picture and records its header state. Returns
// nonzero when the references the picture needs are decoded.
static int SetupMpegReferenceFrameBuffers(MpegContext *w, int nSecondField) {
    int nReferences = w->mPictureStructure == kFramePicture ? 2 : 4;
    int nReady = 0;
    MpegPicture *pCurrent = NULL;
    MpegPicture *pSwap;
    MpegPicture *pOther;

    if (w->mPictureCodingType == kPictureB) {
        w->mFrames.mCurrent = w->mFrames.mAux;
        w->mTopFields.mCurrent = w->mTopFields.mAux;
        w->mBottomFields.mCurrent = w->mBottomFields.mAux;
        if (w->mDecodeCounts[0] + w->mDecodeCounts[1] < nReferences) {
            w->mForceBrokenLink = 0;
            w->mBrokenLink = 0;
            w->mClosedGop = 0;
        }
        if ((w->mForceBrokenLink != 0 || w->mBrokenLink != 0) && w->mClosedGop == 0) {
            w->mFrames.mForward->mDecoded = 0;
            w->mTopFields.mForward->mDecoded = 0;
            w->mBottomFields.mForward->mDecoded = 0;
        }
        w->mForceBrokenLink = 0;
        w->mBrokenLink = 0;
        if (w->mPictureStructure == kFramePicture) {
            if (w->mFrames.mForward->mDecoded == 1 || w->mClosedGop != 0) {
                nReady = w->mFrames.mBackward->mDecoded == 1;
            }
        } else if ((w->mTopFields.mForward->mDecoded == 1 &&
                    w->mBottomFields.mForward->mDecoded == 1) ||
                   w->mClosedGop != 0) {
            nReady =
                w->mTopFields.mBackward->mDecoded == 1 && w->mBottomFields.mBackward->mDecoded == 1;
        }
    } else {
        if (nSecondField == 0) {
            pSwap = w->mFrames.mForward;
            w->mFrames.mForward = w->mFrames.mBackward;
            w->mFrames.mBackward = pSwap;
            pSwap = w->mTopFields.mForward;
            w->mTopFields.mForward = w->mTopFields.mBackward;
            w->mTopFields.mBackward = pSwap;
            pSwap = w->mBottomFields.mForward;
            w->mBottomFields.mForward = w->mBottomFields.mBackward;
            w->mBottomFields.mBackward = pSwap;
        }
        w->mFrames.mCurrent = w->mFrames.mBackward;
        w->mTopFields.mCurrent = w->mTopFields.mBackward;
        w->mBottomFields.mCurrent = w->mBottomFields.mBackward;
        if (w->mPictureStructure == kFramePicture) {
            nReady = w->mPictureCodingType != kPictureP || w->mFrames.mForward->mDecoded == 1;
        } else {
            pOther = w->mPictureStructure == kTopField ? w->mBottomFields.mBackward :
                                                         w->mTopFields.mBackward;
            if (w->mPictureCodingType != kPictureP ||
                (nSecondField != 0 && pOther->mDecoded == 1)) {
                nReady = 1;
            } else {
                nReady = w->mTopFields.mForward->mDecoded == 1 &&
                         w->mBottomFields.mForward->mDecoded == 1;
            }
        }
    }
    if (w->mPictureStructure == kBottomField) {
        pCurrent = w->mBottomFields.mCurrent;
    } else if (w->mPictureStructure == kTopField) {
        pCurrent = w->mTopFields.mCurrent;
    } else if (w->mPictureStructure == kFramePicture) {
        pCurrent = w->mFrames.mCurrent;
    }
    // Yes, the binary writes through a null picture for an unknown picture_structure.
    pCurrent->mDecoded = 0;
    pCurrent->mPts = w->mPts;
    pCurrent->mPictureCodingType = w->mPictureCodingType;
    pCurrent->mDts = w->mDts;
    pCurrent->mPictureStructure = w->mPictureStructure;
    pCurrent->mProgressiveSequence = w->mProgressiveSequence;
    pCurrent->mProgressiveFrame = w->mProgressiveFrame;
    pCurrent->mTopFieldFirst = w->mTopFieldFirst;
    pCurrent->mRepeatFirstField = w->mRepeatFirstField;
    pCurrent->mFrameCentreHorizontalOffset[0] = w->mPictureFrameCentreHorizontalOffset[0];
    pCurrent->mFrameCentreHorizontalOffset[1] = w->mPictureFrameCentreHorizontalOffset[1];
    pCurrent->mFrameCentreHorizontalOffset[2] = w->mPictureFrameCentreHorizontalOffset[2];
    pCurrent->mFrameCentreVerticalOffset[0] = w->mPictureFrameCentreVerticalOffset[0];
    pCurrent->mFrameCentreVerticalOffset[1] = w->mPictureFrameCentreVerticalOffset[1];
    pCurrent->mFrameCentreVerticalOffset[2] = w->mPictureFrameCentreVerticalOffset[2];
    pCurrent->mDisplayHorizontalSize = w->mSequenceDisplayHorizontalSize;
    pCurrent->mDisplayVerticalSize = w->mSequenceDisplayVerticalSize;
    return nReady;
}

// NTSC-U/C: 0x0030d330, PAL: 0x003797c0
static int DecodeMpegPictureToFrameBuffer(MpegContext *w) {
    MpegPicture *pCurrent;
    int nDecoded;

    if (w->mPictureStructure == kFramePicture && w->mSecondField != 0) {
        _sceMpegError(w, "odd number of field pictures");
        w->mSecondField = 0;
    }
    if (w->mPictureStructure == kBottomField) {
        pCurrent = w->mBottomFields.mCurrent;
    } else if (w->mPictureStructure == kTopField) {
        pCurrent = w->mTopFields.mCurrent;
    } else {
        pCurrent = w->mFrames.mCurrent;
        if (w->mPictureStructure != kFramePicture) {
            _sceMpegError(w, "unknown picture sutructure");
        }
    }
    nDecoded = DecodeMpegPictureSlices(w);
    if (nDecoded != 0) {
        pCurrent->mDecoded = 1;
    }
    return nDecoded;
}

// NTSC-U/C: 0x0030d400, PAL: 0x00379890
// Outputs the reference picture that precedes the new one in display order.
static void OutputDecodedMpegPicture(MpegContext *w, int nFrame, int nPending) {
    if (nPending != 0) {
        if (w->mPictureStructure == kFramePicture) {
            DisplayMpegFramePicture(w,
                                    w->mPictureCodingType == kPictureB ? w->mFrames.mAux :
                                                                         w->mFrames.mForward,
                                    nFrame - 1);
        } else if (w->mPictureCodingType == kPictureB) {
            DisplayMpegFieldPicturePair(w, w->mTopFields.mAux, w->mBottomFields.mAux, nFrame - 1);
        } else {
            DisplayMpegFieldPicturePair(
                w, w->mTopFields.mForward, w->mBottomFields.mForward, nFrame - 1);
        }
    }
    if (w->mPendingPtsState == kPendingPtsArmed) {
        w->mPendingPtsState = kPendingPtsReady;
    }
}

#pragma mark - Decoding loop

// NTSC-U/C: 0x00308a08, PAL: 0x00374e98
// Stops the IPU and scratchpad channels and resets the IPU.
static int mpeg___clearEach(MpegContext *w) {
    int nInterrupts;

    w->mTopStale = 1;
    w->mResetDcPredictor = 0;
    nInterrupts = DIntr();
    *D_ENABLEW = *D_ENABLER | kDmaHoldAll;
    *D3_CHCR = 0;
    *D4_CHCR = 0;
    *D9_CHCR = 0;
    *D_ENABLEW = *D_ENABLER & ~(unsigned int)kDmaHoldAll;
    if (nInterrupts != 0) {
        EIntr();
    }
    *D3_QWC = 0;
    *D4_QWC = 0;
    *D9_QWC = 0;
    *IPU_CTRL = kIpuCtrlReset;
    return sceIpuSync(0, 0);
}

// NTSC-U/C: 0x00308918, PAL: 0x00374da8
static void ClearRetryFlagAndEnableIpu(MpegContext *w) {
    w->mMpeg2 = 0;
    SetIpuControlEnableBit(1);
}

// NTSC-U/C: 0x003089a8, PAL: 0x00374e38
// Places the two macroblock buffers in the scratchpad.
static void InitDecoderScratchpadPointers(MpegContext *w) {
    SetIpuControlEnableBit(1);
    w->mMcBuffers[0].mStaging = (unsigned char *)0x70000000;
    w->mMcBuffers[0].mCoefficients = (unsigned char *)0x70001800;
    w->mMcBuffers[1].mStaging = (unsigned char *)0x70001b00;
    w->mMcBuffers[1].mCoefficients = (unsigned char *)0x70003300;
    w->mMcCurrent = 0;
}

// NTSC-U/C: 0x00308928, PAL: 0x00374db8
static void EmitDecodedMpegFrame(MpegContext *w) {
    int nFrame = w->mPictureCount;

    if (w->mSecondField != 0) {
        _sceMpegError(w, "the second field is missing");
    } else if (w->mPictureStructure == kFramePicture) {
        DisplayMpegFramePicture(w, w->mFrames.mBackward, nFrame - 1);
    } else {
        DisplayMpegFieldPicturePair(
            w, w->mTopFields.mBackward, w->mBottomFields.mBackward, nFrame - 1);
    }
    w->mSecondField = 0;
}

// NTSC-U/C: 0x003088b0, PAL: 0x00374d40
// Outputs the last reference picture at the end of the stream.
static int FlushPendingMpegFrame(sceMpeg *mp) {
    MpegContext *w = mp->sys;

    if (w->mPendingOutput == 0 || w->mOutputState == kOutputIdle) {
        return 0;
    }
    EmitDecodedMpegFrame(w);
    mp->frameCount = w->mPictureCount - w->mFrameCountBase;
    w->mPendingOutput = 0;
    return 1;
}

// NTSC-U/C: 0x003085f0, PAL: 0x00374a80
// Decodes or skips a frame picture.
static int DecodeNextMpegFrame(sceMpeg *mp, int nCount, int nLimit) {
    MpegContext *w = mp->sys;
    int nSkipped = 0;
    int nResult;

    if (nLimit == -1 || nCount < nLimit) {
        if (w->mOutputState == kOutputIdle) {
            mp->frameCount = 0;
            w->mOutputState = kOutputDecoding;
        }
        nResult =
            SetupMpegReferenceFrameBuffers(w, 0) != 0 && DecodeMpegPictureToFrameBuffer(w) != 0;
    } else {
        nResult = SetupMpegReferenceFrameBuffers(w, 0);
        nSkipped = 1;
        _sceMpegCbNodata(mp);
    }
    OutputDecodedMpegPicture(w, w->mPictureCount, w->mPendingOutput);
    // Yes, the binary toggles the field flag here, though only frame pictures arrive.
    if (w->mPictureStructure != kFramePicture && nSkipped == 0) {
        w->mSecondField = w->mSecondField == 0;
    }
    mp->frameCount = w->mPictureCount - w->mFrameCountBase;
    if (w->mSecondField == 0) {
        ++w->mPictureCount;
        ++w->mPendingOutput;
    }
    return nResult;
}

// NTSC-U/C: 0x00308750, PAL: 0x00374be0
// Decodes or skips both fields of a field picture pair. Returns -1 when the second field does
// not follow, with its header already parsed.
static int PrepareNextMpegFrame(sceMpeg *mp, int nCount, int nLimit) {
    MpegContext *w = mp->sys;
    int nDecode = 0;
    int nResult = 0;

    w->mSecondField = 0;
    if (nLimit == -1 || nCount < nLimit) {
        nDecode = 1;
    }
    if (w->mOutputState == kOutputIdle) {
        mp->frameCount = 0;
        w->mOutputState = kOutputDecoding;
    }
    if (SetupMpegReferenceFrameBuffers(w, 0) != 0 && nDecode != 0) {
        DecodeMpegPictureToFrameBuffer(w);
    }
    w->mSecondField = 1;
    if (ParseMpegToNextPicture(w) == 0) {
        FlushPendingMpegFrame(mp);
        w->mIsEnd = 1;
        return 0;
    }
    if (w->mPictureStructure != (w->mFirstFieldStructure == kTopField ? kBottomField : kTopField)) {
        return -1;
    }
    if (SetupMpegReferenceFrameBuffers(w, 1) != 0 && nDecode != 0) {
        nResult = DecodeMpegPictureToFrameBuffer(w) != 0;
    }
    OutputDecodedMpegPicture(w, w->mPictureCount, w->mPendingOutput);
    w->mSecondField = 0;
    mp->frameCount = w->mPictureCount - w->mFrameCountBase;
    ++w->mPictureCount;
    ++w->mPendingOutput;
    if (nDecode == 0) {
        _sceMpegCbNodata(mp);
    }
    return nResult;
}

// NTSC-U/C: 0x00308708, PAL: 0x00374b98
static int mpeg___decodeOrSkip(sceMpeg *mp, int nCount, int nLimit) {
    if (mp->sys->mPictureStructure == kFramePicture) {
        return DecodeNextMpegFrame(mp, nCount, nLimit);
    }
    return PrepareNextMpegFrame(mp, nCount, nLimit);
}

// NTSC-U/C: 0x00308480, PAL: 0x00374910
// Decodes pictures until one is output or the stream ends.
static int RunImageDecodeStateMachine(sceMpeg *mp) {
    MpegContext *w = mp->sys;
    int nType = 1;
    int nResult = 0;

    w->mIsEnd = 0;
    if (((uintptr_t)w->mPictureAddress & kPictureAlignMask) != 0) {
        _sceMpegErrorFormat(w,
                            "image buffer needs to be aligned to 64byte boundary(0x%08x)",
                            (unsigned int)(uintptr_t)w->mPictureAddress);
        return -1;
    }
    w->mPictureReady = 0;
    do {
        // A field pair that broke off left the header of the next picture parsed.
        if (nResult != -1) {
            do {
                nType = ParseMpegToNextPicture(w);
            } while (nType != 0 && w->mPictureStructure != w->mFirstFieldStructure &&
                     w->mMpeg2 != 0);
        }
        switch (nType) {
        case 0:
            FlushPendingMpegFrame(mp);
            w->mIsEnd = 1;
            break;
        case kPictureI:
            w->mDecodeCounts[2] = 0;
            w->mDecodeCounts[1] = 0;
            w->mDecodeCounts[0] = 0;
            nResult = mpeg___decodeOrSkip(mp, w->mDecodeCounts[0], w->mDecodeLimits[0]);
            ++w->mDecodeCounts[0];
            break;
        case kPictureP:
            nResult = mpeg___decodeOrSkip(mp, w->mDecodeCounts[1], w->mDecodeLimits[1]);
            ++w->mDecodeCounts[1];
            break;
        case kPictureB:
        case kPictureD:
            nResult = mpeg___decodeOrSkip(mp, w->mDecodeCounts[2], w->mDecodeLimits[2]);
            ++w->mDecodeCounts[2];
            break;
        default:
            break;
        }
    } while (w->mPictureReady == 0 && w->mIsEnd == 0);
    return 1;
}

// NTSC-U/C: 0x00309320, PAL: 0x003757b0
static int SaveMpegPictureDataDmaChannel(sceMpeg *mp, void *cbdata, void *data) {
    (void)cbdata;
    (void)data;
    sceIpuStopDMA(&mp->sys->mDmaEnv);
    return 1;
}

// NTSC-U/C: 0x00309348, PAL: 0x003757d8
static int ResetMpegStreamSubField4c(sceMpeg *mp, void *cbdata, void *data) {
    (void)cbdata;
    (void)data;
    sceIpuRestartDMA(&mp->sys->mDmaEnv);
    return 1;
}

#pragma mark - Library interface

int sceMpegInit(void) {
    int nInterrupts = DIntr();

    *D_ENABLEW = *D_ENABLER | kDmaHoldAll;
    *D3_CHCR &= ~(unsigned int)kChcrStart;
    *D4_CHCR &= ~(unsigned int)kChcrStart;
    *D_ENABLEW = *D_ENABLER & ~(unsigned int)kDmaHoldAll;
    if (nInterrupts != 0) {
        EIntr();
    }
    *D3_QWC = 0;
    *D4_QWC = 0;
    sceIpuInit();
    return 1;
}

int sceMpegCreate(sceMpeg *mp, unsigned char *work_area, int work_area_size) {
    MpegContext *w;
    unsigned int nAvailable;

    memset(work_area, 0, (size_t)work_area_size);
    w = (MpegContext *)(((uintptr_t)work_area + kAlignMask) & ~(uintptr_t)kAlignMask);
    nAvailable = (unsigned int)work_area_size - (unsigned int)((unsigned char *)w - work_area);
    if (nAvailable < kContextSize) {
        // Yes, the binary reports through a context that may extend past the work area.
        _sceMpegError(w, "The size of work area is too small");
        return 0;
    }
    mp->sys = w;
    _sceMpegAlalcInit(
        &w->mAlalc, (unsigned char *)w + kContextSize, (int)(nAvailable - kContextSize));
    mp->width = 0;
    mp->height = 0;
    mp->frameCount = 0;
    mp->pts = -1;
    mp->dts = -1;
    mp->flags = 0;
    mp->pts2nd = -1;
    mp->dts2nd = -1;
    mp->flags2nd = 0;
    w->mFrameCentreHorizontalOffset[0] = 0;
    w->mFrameCentreHorizontalOffset[1] = 0;
    w->mFrameCentreHorizontalOffset[2] = 0;
    w->mFrameCentreVerticalOffset[0] = 0;
    w->mFrameCentreVerticalOffset[1] = 0;
    w->mFrameCentreVerticalOffset[2] = 0;
    w->mDisplayHorizontalSize = 0;
    w->mDisplayVerticalSize = 0;
    w->mFirstFieldStructure = 0;
    w->mPictureAddress = NULL;
    w->mOutputWidth = 0;
    w->mOutputHeight = 0;
    w->mOutputMacroblocks = 0;
    w->mForceBrokenLink = 0;
    w->mPendingPtsState = 0;
    w->mSlots[sceMpegCbError].mCallback = NULL;
    w->mSlots[sceMpegCbNodata].mCallback = NULL;
    w->mSlots[sceMpegCbBackground].mCallback = NULL;
    w->mSlots[sceMpegCbTimeStamp].mCallback = NULL;
    w->mSlots[sceMpegCbStr].mCallback = NULL;
    w->mPendingPts = -1;
    w->mSlots[sceMpegCbStopDMA].mCallback = SaveMpegPictureDataDmaChannel;
    w->mSlots[sceMpegCbRestartDMA].mCallback = ResetMpegStreamSubField4c;
    w->mStreamTable =
        (StreamEntry *)_sceMpegAlalcAlloc(w, &w->mAlalc, kStreamTableSize, kStreamTableAlign);
    w->mStreamCount = 0;
    w->mFrameBuffers[0] = NULL;
    w->mFrameBuffers[1] = NULL;
    w->mFrameBuffers[2] = NULL;
    w->mUseDefaultPtsGap = 0;
    w->mDefaultPtsGap = 0;
    w->mLastPts = -1;
    w->mDisplayFieldCount = 0;
    w->mOddGapPictures = 0;
    w->mFrameCountBase = 0;
    w->mDecodeLimits[0] = -1;
    w->mDecodeLimits[1] = -1;
    w->mDecodeLimits[2] = -1;
    w->mMpeg = mp;
    w->mConvertColours = 1;
    InitDecoderScratchpadPointers(w);
    sceMpegReset(mp);
    sceMpegClearRefBuff(mp);
    w->mFrames.mForward = &w->mPictures[0];
    w->mFrames.mBackward = &w->mPictures[1];
    w->mFrames.mAux = &w->mPictures[2];
    w->mTopFields.mForward = &w->mPictures[3];
    w->mTopFields.mBackward = &w->mPictures[4];
    w->mTopFields.mAux = &w->mPictures[5];
    w->mBottomFields.mForward = &w->mPictures[6];
    w->mBottomFields.mBackward = &w->mPictures[7];
    w->mBottomFields.mAux = &w->mPictures[8];
    _sceMpegAlalcSetDynamic(&w->mAlalc);
    w->mMaxPictureNumber = -1;
    w->mNewGop = 0;
    w->mMcOutput = (unsigned char *)0x70003600;
    w->mGopPictureBase = 0;
    return 1;
}

int sceMpegDelete(sceMpeg *mp) {
    (void)mp;
    return 1;
}

int sceMpegGetPicture(sceMpeg *mp, void *rgb32, int mbcount) {
    MpegContext *w = mp->sys;

    w->mConvertColours = 1;
    w->mPictureAddress = Uncached(rgb32);
    w->mOutputMacroblocks = mbcount;
    w->mOutputWidth = 0;
    w->mOutputHeight = 0;
    return RunImageDecodeStateMachine(mp);
}

int sceMpegIsEnd(sceMpeg *mp) {
    return mp->sys->mIsEnd;
}

int sceMpegReset(sceMpeg *mp) {
    MpegContext *w = mp->sys;

    w->mIsEnd = 0;
    w->mPendingOutput = 0;
    w->mOutputState = kOutputIdle;
    mp->frameCount = 0;
    w->mFrameCountBase = 0;
    w->mLastPts = -1;
    mpeg___clearEach(w);
    w->mPictureCount = 0;
    ClearRetryFlagAndEnableIpu(w);
    return 1;
}

int sceMpegClearRefBuff(sceMpeg *mp) {
    MpegContext *w = mp->sys;

    if (w->mFrames.mForward != NULL) {
        w->mFrames.mForward->mDecoded = 0;
    }
    if (w->mTopFields.mForward != NULL) {
        w->mTopFields.mForward->mDecoded = 0;
    }
    if (w->mBottomFields.mForward != NULL) {
        w->mBottomFields.mForward->mDecoded = 0;
    }
    if (w->mFrames.mBackward != NULL) {
        w->mFrames.mBackward->mDecoded = 0;
    }
    if (w->mTopFields.mBackward != NULL) {
        w->mTopFields.mBackward->mDecoded = 0;
    }
    if (w->mBottomFields.mBackward != NULL) {
        w->mBottomFields.mBackward->mDecoded = 0;
    }
    return 1;
}

sceMpegCallback sceMpegAddCallback(sceMpeg *mp, int type, sceMpegCallback func, void *data) {
    MpegSlot *pSlot = &mp->sys->mSlots[type];
    sceMpegCallback pfnPrevious = pSlot->mCallback;

    pSlot->mData = data;
    pSlot->mCallback = func;
    return pfnPrevious;
}
