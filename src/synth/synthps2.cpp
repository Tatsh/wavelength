#include "synth/synthps2.h"

#include <algorithm>
#include <math.h>

#include <eekernel.h>
#include <sifcmd.h>
#include <sifrpc.h>

#include "os/debug.h"
#include "os/file.h"
#include "os/system.h"
#include "os/timer.h"
#include "script/dataarray.h"
#include "synth/bankfileio.h"
#include "synth/bankloadqj.h"
#include "synth/callbackqj.h"
#include "synth/deactivatebankqj.h"
#include "synth/enabledisablesoftfxqj.h"
#include "synth/partitionqj.h"
#include "synth/setupcorefxqj.h"
#include "synth/streamplayer.h"

namespace {

// The client of each IOP module, indexing g_aSynthClients and g_anSynthServerIds.
constexpr int kSynthModule = 0;
constexpr int kSoftFxModule = 1;
constexpr int kNumModules = 2;

// The busy-wait between attempts to bind a module, in loop iterations.
constexpr int kBindDelayLoops = 10000;

// Results of sceSifCheckStatRpc().
constexpr int kRpcIdle = 0;
constexpr int kRpcBusy = 1;

// The size of each read of the audio data file.
constexpr int kAudioBufferSize = 0xf800;

// The unit of GetBankBlockCount().
constexpr unsigned int kBankBlockSize = 64;

constexpr int kNoSlot = -1;
constexpr float kNoTime = -1.0f;

// The defaults of the software effect, a swept filter that is off.
constexpr int kDefaultFreqCeiling = 10000;
constexpr int kDefaultFreqFloor = 500;
constexpr int kDefaultInitFreq = 1000;
constexpr float kDefaultResonance = 0.5f;

// The format GetStreamFormat() reports.
constexpr int kStreamRate = 28351;
constexpr int kStreamBlock = 64;

// MIDI.
constexpr unsigned char kStatusControlChange = 0xb0;
constexpr unsigned char kStatusTypeMask = 0xf0;
constexpr unsigned char kChannelMask = 0x0f;
constexpr unsigned char kControllerVolume = 7;
constexpr unsigned char kControllerSoftFxSweep = 12;
constexpr unsigned char kControllerOutputLevel = 15;
constexpr unsigned char kControllerVolumeTarget = 17;
constexpr float kControllerScale = 1.0f / 127.0f;
constexpr int kMaxDelay = 255;

// Byte positions of a packed message.
constexpr int kData1Shift = 8;
constexpr int kData2Shift = 16;
constexpr int kDelayShift = 24;
constexpr unsigned int kByteMask = 0xff;

// A channel volume moves when it is further than this from its target.
constexpr float kVolumeStep = 0.5f;

// The output level is sent as a fraction of this.
constexpr float kFullVolume = 16383.0f;

// The software effect parameters are fixed point with 13 fractional bits.
constexpr float kFixedOne = 8192.0f;
constexpr double kFixedRound = 0.5;

// The software effect runs at 48 kHz.
constexpr float kSampleRate = 48000.0f;
constexpr double kSampleRateDouble = 48000.0;
constexpr double kTwoPi = 6.2831854820251465;

// The swept filter.
constexpr float kSweepPoleLinear = 3.6f;
constexpr float kSweepPoleSquare = 1.6f;
constexpr float kSweepDamping = 1.386249f;

// The stutter.
constexpr double kStutterRateScale = 0.9;
constexpr int kStutterFeedback = 32;

// The filter of each channel.
constexpr double kRightFrequencyScale = 0.6;
constexpr double kRightResonanceScale = 1.2;
constexpr float kModulationFloorStart = 0.5f;
constexpr float kModulationFloorTop = 2200.0f;
constexpr float kModulationResonanceStart = 0.75f;
constexpr float kModulationResonanceSlope = 4.0f;

// The SetParams command carries the parameters of both channels.
constexpr int kSoftFxParamsSize = 64;

// The size argument of a command with one word.
constexpr int kWordSize = 4;

// One cache line the IOP writes a word into.
constexpr int kCacheLineSize = 64;

struct alignas(kCacheLineSize) IopNotify {
    int mValue;
    unsigned char mRest[kCacheLineSize - sizeof(int)];
};

// The SIF DMA that sends a command reads it from the stack, which needs a quadword boundary.
constexpr int kQwordSize = 16;

struct alignas(kQwordSize) SynthInitCommand {
    IopNotify *mTransferDone;
    IopNotify *mCoreFxDone;
    int mVerbose;
};

struct alignas(kQwordSize) SoftFxInitCommand {
    int mVerbose;
    IopNotify *mStreamStatus;
};

// This file's copies of kBankExtension and kAudioDataExtension, g_abSynthSweepFilter, and
// g_abSynthBankLoadTimer are constructed by one routine at 0x00250ed0. The routine also destroys
// the two strings. The image's static construction and destruction entries at 0x00250f70 and
// 0x00250f90 call it.
// NTSC-U/C: 0x00250ed0, PAL: 0x00259880
// NTSC-U/C: 0x00250f70, PAL: 0x00259920
// NTSC-U/C: 0x00250f90, PAL: 0x00259940

// NTSC-U/C: 0x0043c870
OnePoleFilter g_abSynthSweepFilter;

// NTSC-U/C: 0x0043c880
Timer g_abSynthBankLoadTimer;

// NTSC-U/C: 0x0043c8c0
IopNotify g_abSynthTransferPending;

// NTSC-U/C: 0x0043c900
IopNotify g_abSynthCoreFxPending;

// NTSC-U/C: 0x0043c940
IopNotify g_abSynthStreamStatus;

// NTSC-U/C: 0x0043c980
alignas(kCacheLineSize) sceSifClientData g_aSynthClients[kNumModules];

// NTSC-U/C: 0x003b0a88, PAL: 0x0042f920
IopNotify *g_pSynthTransferPending = &g_abSynthTransferPending;

// NTSC-U/C: 0x003b0a8c, PAL: 0x0042f924
IopNotify *g_pSynthCoreFxPending = &g_abSynthCoreFxPending;

// NTSC-U/C: 0x003b0a90, PAL: 0x0042f928
IopNotify *g_pSynthStreamStatus = &g_abSynthStreamStatus;

// NTSC-U/C: 0x003b0a98, PAL: 0x0042f930
unsigned int g_anSynthServerIds[kNumModules] = {0x75433178, 0x75433776};

// NTSC-U/C: 0x003b0aa0, PAL: 0x0042f938
int g_nSynthLoadStatePolls;

inline void WriteNotify(IopNotify *pNotify, int nValue) {
    pNotify->mValue = nValue;
    SyncDCache(pNotify, &pNotify->mRest[sizeof(pNotify->mRest) - 1]);
}

inline int ReadNotify(IopNotify *pNotify) {
    SyncDCache(pNotify, &pNotify->mRest[sizeof(pNotify->mRest) - 1]);
    InvalidDCache(pNotify, &pNotify->mRest[sizeof(pNotify->mRest) - 1]);
    return pNotify->mValue;
}

inline unsigned int PackMessage(unsigned char nStatus,
                                unsigned char nData1,
                                unsigned char nData2,
                                unsigned char nDelay) {
    return nStatus | (nData1 << kData1Shift) | (nData2 << kData2Shift) | (nDelay << kDelayShift);
}

inline bool IsController(unsigned char nStatus, unsigned char nData1, unsigned char nController) {
    return ((nStatus & kStatusTypeMask) == kStatusControlChange) && (nData1 == nController);
}

inline int ToFixed(float fValue) {
    return static_cast<int>(static_cast<double>(fValue * kFixedOne) + kFixedRound);
}

} // namespace

SynthPS2::SynthPS2()
    : mLoadSlot(kNoSlot), mBankFile(nullptr), mLoadState(kLoadIdle), mInitialized(0), mLagMs(0.0f),
      mNowMs(kNoTime), mLastMs(kNoTime), mTimeUpdated(0), mStream(nullptr), mSoftFxEnabled(0),
      mSoftFxSweep(0.0f), mSoftFxModulation(0.0f), mOutputLevel(1.0f), mGotAudioData(0),
      mMoreAudioData(1), mAudioDataOffset(0) {
    mFilter.mFreqCeiling = kDefaultFreqCeiling;
    mFilter.mFreqFloor = kDefaultFreqFloor;
    mFilter.mType = SoftFxFilter::kTypeOff;
    mFilter.mFrequency = 0.0f;
    for (SoftFxFilter::Tap &tap : mFilter.mTaps) {
        tap = {};
    }
    mFilter.mParam.mInitFreq = kDefaultInitFreq;
    mFilter.mMix.mResonance = kDefaultResonance;

    mBankLoaded.resize(kNumBankSlots);
    mCommands = std::make_unique<BufStream>(mCommandBuffer.mBytes, kCommandBufferSize, true);
    mSoftFxCommands = std::make_unique<BufStream>(mSoftFxBuffer.mBytes, kCommandBufferSize, true);
    WriteNotify(g_pSynthTransferPending, 0);
    WriteNotify(g_pSynthCoreFxPending, 0);
    WriteNotify(g_pSynthStreamStatus, 0);

    mChannelVolumes.resize(kNumChannels, OnePoleFilter());
    float fSmoothing; // Left unset when the entry is missing, as in the binary.
    SystemConfig()->FindArray("synth", false)->FindFloat("volume_smoothing", &fSmoothing, true);
    for (int i = 0; i < kNumChannels; ++i) {
        mChannelVolumes[i].SetCoefficient(fSmoothing);
    }

    mAudioData.reserve(kAudioBufferSize);
    mAudioData.resize(0);
}

SynthPS2::~SynthPS2() = default;

int SynthPS2::ReadStreamStatus() {
    return ReadNotify(g_pSynthStreamStatus);
}

void SynthPS2::ClearStreamStatus() {
    WriteNotify(g_pSynthStreamStatus, 0);
}

bool SynthPS2::IsBankLoaded(unsigned short nSlot) {
    bool bLoaded = false;
    if (mBankLoaded[nSlot]) {
        bLoaded = !HasPendingLoad(nSlot);
    }
    return bLoaded;
}

int SynthPS2::IsBankPresent(unsigned short nSlot) {
    int nPresent = 0;
    if (mBankLoaded[nSlot]) {
        nPresent = 1;
    } else if (HasPendingLoad(nSlot)) {
        nPresent = 1;
    }
    return nPresent;
}

int SynthPS2::Terminate() {
    (void)IsRpcBusy(); // Yes, the binary discards the check.
    (void)sceSifCallRpc(&g_aSynthClients[kSynthModule],
                        kCommandTerminate,
                        0,
                        nullptr,
                        0,
                        nullptr,
                        0,
                        nullptr,
                        nullptr);
    return 0;
}

void SynthPS2::SendCommand(int nCommand, const void *pData, int nSize) {
    (void)IsRpcBusy(); // Yes, the binary discards the check.
    BufStream *pStream = mCommands.get();
    pStream->WriteEndian(&nCommand, sizeof(nCommand));
    pStream->WriteEndian(&nSize, sizeof(nSize));
    (void)mCommands->Fail();
    mCommands->Write(pData, nSize);
    (void)mCommands->Fail();
}

void SynthPS2::SetLag(float fLag) {
    mLagMs = fLag;
}

float SynthPS2::GetLag() {
    return mLagMs;
}

int SynthPS2::Init() {
    SystemConfig()->FindArray("synth", true)->FindFloat("lag_ms", &mLagMs, true);
    mMidiQueue.reserve(200);
    const int nStatus = InitIop();
    sceSifInitCmd();
    mInitialized = 1;
    return nStatus;
}

int SynthPS2::InitIop() {
    String unusedName; // Yes, the binary builds two strings it never uses.
    String unusedPath;

    for (int i = 0; i < kNumModules; ++i) {
        do {
            if (sceSifBindRpc(&g_aSynthClients[i], g_anSynthServerIds[i], 0) < 0) {
                DebugPrint("could not bind with module id: %d.\n", g_anSynthServerIds[i]);
                DebugWarn("could not bind rpc.");
            }
            for (int nDelay = 0; nDelay < kBindDelayLoops; ++nDelay) {
                __asm__ volatile("nop");
            }
        } while (g_aSynthClients[i].serve == nullptr);
    }

    (void)IsRpcBusy(); // Yes, the binary discards the check.
    SynthInitCommand synthInit;
    synthInit.mTransferDone = &g_abSynthTransferPending;
    synthInit.mCoreFxDone = &g_abSynthCoreFxPending;
    synthInit.mVerbose = 0;
    DataArray *pConfig = SystemConfig()->FindArray("synth", false);
    pConfig->FindBool("iop_verbose", &synthInit.mVerbose, true);
    SoftFxInitCommand softFxInit; // mVerbose stays unset when the entry is missing.
    softFxInit.mStreamStatus = &g_abSynthStreamStatus;
    pConfig->FindBool("iop_verbose", &softFxInit.mVerbose, true);
    (void)sceSifCallRpc(&g_aSynthClients[kSynthModule],
                        kCommandInit,
                        0,
                        &synthInit,
                        sizeof(synthInit),
                        nullptr,
                        0,
                        nullptr,
                        nullptr);
    (void)sceSifCallRpc(&g_aSynthClients[kSoftFxModule],
                        kSoftFxCommandInit,
                        0,
                        &softFxInit,
                        sizeof(softFxInit),
                        nullptr,
                        0,
                        nullptr,
                        nullptr);
    return 0;
}

void SynthPS2::UpdateVolumes() {
    for (int i = 0; i < kNumChannels; ++i) {
        OnePoleFilter &volume = mChannelVolumes[i];
        if (kVolumeStep < fabsf(volume.GetTarget() - volume.GetValue())) {
            const float fVolume = volume.Update();
            mMidiQueue.push_back(
                PackMessage(kStatusControlChange | i,
                            kControllerVolume,
                            static_cast<unsigned char>(static_cast<int>(fVolume + kVolumeStep)),
                            0));
        }
    }
}

bool SynthPS2::IsRpcBusy() {
    const int nStatus = sceSifCheckStatRpc(&g_aSynthClients[kSynthModule].rpcd);
    if (nStatus == kRpcBusy) {
        return true;
    }
    if (nStatus != kRpcIdle) {
        DebugWarn("Bad value.");
    }
    return false;
}

bool SynthPS2::IsSoftFxRpcBusy() {
    const int nStatus = sceSifCheckStatRpc(&g_aSynthClients[kSoftFxModule].rpcd);
    if (nStatus == kRpcBusy) {
        return true;
    }
    if (nStatus != kRpcIdle) {
        DebugWarn("Bad value.");
    }
    return false;
}

bool SynthPS2::IsTransferDone() {
    return ReadNotify(g_pSynthTransferPending) == 0;
}

void SynthPS2::SetTransferPending() {
    WriteNotify(g_pSynthTransferPending, 1);
}

bool SynthPS2::IsCoreFxDone() {
    return ReadNotify(g_pSynthCoreFxPending) == 0;
}

void SynthPS2::SetCoreFxPending() {
    WriteNotify(g_pSynthCoreFxPending, 1);
}

int SynthPS2::PollBankLoad() {
    const auto finishLoad = [this]() {
        mBankFile.reset();
        mLoadSlot = kNoSlot;
        mLoadState = kLoadIdle;
        g_nSynthLoadStatePolls = 0;
        mAudioDataOffset = 0;
        mMoreAudioData = 1;
        mGotAudioData = 0;
        mAudioData.resize(0);
        ProcessJobs();
    };

    int nBusy = 1;
    switch (mLoadState) {
    case kLoadIdle:
        nBusy = 0;
        g_nSynthLoadStatePolls = 0;
        break;
    case kLoadBankHeader:
        if (!mBankFile->ReadChunk(mChunk) && !IsRpcBusy()) {
            (void)SendBankHeader(mChunk, mLoadSlot);
            mLoadState = kFindPrograms;
            g_nSynthLoadStatePolls = 0;
            (void)PollBankLoad();
        }
        break;
    case kFindPrograms:
        mBankFile->FindChunk(BankFileIO::kInstrumentChunk);
        (void)mBankFile->ReadChunk(mChunk);
        mLoadState = kLoadPrograms;
        g_nSynthLoadStatePolls = 0;
        break;
    case kLoadPrograms:
        if (!mBankFile->ReadChunk(mChunk) && !IsRpcBusy()) {
            (void)SendPrograms(mChunk, mLoadSlot);
            mLoadState = kFindSamples;
            g_nSynthLoadStatePolls = 0;
            (void)PollBankLoad();
        }
        break;
    case kFindSamples:
        mBankFile->FindChunk(BankFileIO::kSampleChunk);
        (void)mBankFile->ReadChunk(mChunk);
        mLoadState = kLoadSamples;
        g_nSynthLoadStatePolls = 0;
        break;
    case kLoadSampleDescs:
        if (!mBankFile->ReadChunk(mChunk) && !IsRpcBusy()) {
            (void)SendSampleDescs(mChunk, mLoadSlot);
            mLoadState = kOpenAudioData;
            g_nSynthLoadStatePolls = 0;
            (void)PollBankLoad();
        }
        break;
    case kOpenAudioData:
        mBankFile->OpenAudioData(kAudioBufferSize);
        mLoadState = kTransferAudioData;
        g_nSynthLoadStatePolls = 0;
        break;
    case kLoadSamples:
        if (!mBankFile->ReadChunk(mChunk) && !IsRpcBusy()) {
            (void)SendSamples(mChunk, mLoadSlot);
            mLoadState = kFindSampleDescs;
            g_nSynthLoadStatePolls = 0;
            (void)PollBankLoad();
        }
        break;
    case kFindSampleDescs:
        mBankFile->FindChunk(BankFileIO::kSampleDescriptionChunk);
        (void)mBankFile->ReadChunk(mChunk);
        mLoadState = kLoadSampleDescs;
        g_nSynthLoadStatePolls = 0;
        break;
    case kTransferAudioData:
        if (mGotAudioData == 0) {
            mMoreAudioData = mBankFile->GetAudioData(mAudioData, &mAudioDataOffset, &mGotAudioData);
        }
        if (!IsRpcBusy() && IsTransferDone()) {
            if (mGotAudioData) {
                (void)SendSampleData(mAudioData, mAudioDataOffset, mLoadSlot);
                g_nSynthLoadStatePolls = 0;
                mGotAudioData = 0;
                mAudioData.resize(0);
            }
            if (mMoreAudioData == 0) {
                mLoadState = kMarkLoaded;
                g_nSynthLoadStatePolls = 0;
            }
        }
        break;
    case kMarkLoaded:
        if (!IsRpcBusy() && IsTransferDone()) {
            (void)SendBankLoaded(static_cast<unsigned short>(mLoadSlot));
            mLoadState = kFinishLoad;
            g_nSynthLoadStatePolls = 0;
        }
        break;
    case kFinishLoad:
        if (!IsRpcBusy()) {
            mBankLoaded[mLoadSlot] = true;
            finishLoad();
        }
        break;
    case kAbortLoad:
        mBankLoaded[mLoadSlot] = false;
        finishLoad();
        break;
    default:
        break;
    }
    ++g_nSynthLoadStatePolls;
    return nBusy;
}

int SynthPS2::SendBankLoaded(unsigned short nSlot) {
    (void)mBankLoaded[nSlot]; // Yes, the binary looks the flag up and discards it.
    SendCommand(kCommandSetBankLoaded, &nSlot, sizeof(nSlot));
    return 0;
}

int SynthPS2::SendUnloadBank(unsigned short nSlot) {
    SendCommand(kCommandUnloadBank, &nSlot, sizeof(nSlot));
    return 0;
}

int SynthPS2::UnloadBank(unsigned short nSlot) {
    if (!HasPendingLoad(nSlot)) {
        if (mBankLoaded[nSlot]) {
            mBankLoaded[nSlot] = false;
            mJobs.push_back(std::make_unique<DeactivateBankQJ>(nSlot));
        }
        return 0;
    }

    int nIndex = 0;
    auto it = mJobs.begin();
    for (; it != mJobs.end(); ++it) {
        const BankLoadQJ *pLoad = dynamic_cast<BankLoadQJ *>(it->get());
        if ((pLoad != nullptr) && (pLoad->GetSlot() == nSlot)) {
            break;
        }
        ++nIndex;
    }
    if ((mLoadState != kLoadIdle) && (nIndex == 0)) {
        mLoadState = kAbortLoad;
        mJobs.push_back(std::make_unique<DeactivateBankQJ>(nSlot));
    } else {
        (void)it->release();
        mJobs.erase(it); // Yes, the job itself is not deleted.
    }
    return 0;
}

int SynthPS2::GetBankBlockCount(const String &file) {
    const String audioFile = BankFileIO::AudioDataFilename(file);
    const int bTrace = FileIsTraceEnabled();
    FileSetTraceEnabled(0);
    std::unique_ptr<File> pFile(File::New(audioFile.c_str(), File::kModeRead, 0));
    const unsigned int nSize = static_cast<unsigned int>(pFile->Size());
    pFile.reset();
    FileSetTraceEnabled(bTrace);
    unsigned int nBlocks = nSize / kBankBlockSize;
    if ((nSize % kBankBlockSize) != 0) {
        ++nBlocks;
    }
    return static_cast<int>(nBlocks);
}

int SynthPS2::StartBankLoad(unsigned short nSlot, const String &file) {
    (void)mBankLoaded[nSlot]; // Yes, the binary looks the flag up and discards it.
    mLoadSlot = nSlot;
    mBankFile = std::make_unique<BankFileIO>(file, BankFileIO::kRead);
    (void)!*mBankFile; // Yes, the binary discards the check.
    mBankFile->FindChunk(BankFileIO::kBankChunk);
    (void)mBankFile->ReadChunk(mChunk);
    mLoadState = kLoadBankHeader;
    g_abSynthBankLoadTimer.Reset();
    g_abSynthBankLoadTimer.Start();
    return 0;
}

int SynthPS2::LoadBank(unsigned short nSlot, const String &file, bool bSkipPending) {
    if (bSkipPending && HasPendingLoad(nSlot)) {
        return 0;
    }
    mJobs.push_back(std::make_unique<BankLoadQJ>(nSlot, file));
    return 0;
}

int SynthPS2::SendBankData(const std::vector<unsigned char> &data,
                           int nBank,
                           int nCommand,
                           int nOffset) {
    (void)IsRpcBusy(); // Yes, the binary discards the check.
    BankDataCommand header;
    header.mBank = nBank;
    header.mOffset = nOffset;
    mCommandBuffer.mLastHeader = header; // Yes, the binary overwrites the buffer's first bytes.
    const int nSize = static_cast<int>(data.size() + sizeof(header));
    BufStream *pStream = mCommands.get();
    pStream->WriteEndian(&nCommand, sizeof(nCommand));
    pStream->WriteEndian(&nSize, sizeof(nSize));
    (void)mCommands->Fail();
    mCommands->Write(&header, sizeof(header));
    (void)mCommands->Fail();
    mCommands->Write(data.data(), static_cast<int>(data.size()));
    (void)mCommands->Fail();
    return 0;
}

int SynthPS2::SendBankHeader(const std::vector<unsigned char> &data, int nBank) {
    (void)SendBankData(data, nBank, kCommandLoadBankHeader, 0);
    return 0;
}

int SynthPS2::SendPrograms(const std::vector<unsigned char> &data, int nBank) {
    (void)SendBankData(data, nBank, kCommandLoadPrograms, 0);
    return 0;
}

int SynthPS2::SendSampleDescs(const std::vector<unsigned char> &data, int nBank) {
    (void)SendBankData(data, nBank, kCommandLoadSampleDescs, 0);
    return 0;
}

int SynthPS2::SendSamples(const std::vector<unsigned char> &data, int nBank) {
    (void)SendBankData(data, nBank, kCommandLoadSamples, 0);
    return 0;
}

int SynthPS2::SendMessage(unsigned char nByte0,
                          unsigned char nByte1,
                          unsigned char nByte2,
                          int nByte3) {
    const int nDelay = std::min(std::max(0, nByte3), kMaxDelay);
    const unsigned int nMessage =
        PackMessage(nByte0, nByte1, nByte2, static_cast<unsigned char>(nDelay));
    if (IsController(nByte0, nByte1, kControllerSoftFxSweep)) {
        SetSweepFromController(nByte2);
    } else if (IsController(nByte0, nByte1, kControllerOutputLevel)) {
        SetLevelFromController(nByte2);
    } else if (IsController(nByte0, nByte1, kControllerVolumeTarget)) {
        mChannelVolumes[nByte0 & kChannelMask].SetTarget(static_cast<float>(nByte2));
    } else {
        mMidiQueue.push_back(nMessage);
    }
    return 0;
}

int SynthPS2::SendPackedMessage(unsigned int nPacked) {
    return SendMessage(static_cast<unsigned char>(nPacked & kByteMask),
                       static_cast<unsigned char>((nPacked >> kData1Shift) & kByteMask),
                       static_cast<unsigned char>((nPacked >> kData2Shift) & kByteMask),
                       static_cast<int>((nPacked >> kDelayShift) & kByteMask));
}

void SynthPS2::UpdateTime() {
    const float fNowMs = SystemMs();
    if (!mMidiQueue.empty()) {
        AdjustDelays(static_cast<int>(mNowMs - fNowMs));
    }
    mTimeUpdated = 1;
    mLastMs = mNowMs;
    mNowMs = fNowMs;
}

int SynthPS2::Poll() {
    if (!IsRpcBusy()) {
        UpdateVolumes();
        (void)mCommands->Fail();
        ProcessJobs();
        (void)mCommands->Fail();
        (void)PollBankLoad();
        (void)mCommands->Fail();
        (void)FlushMidi();
        (void)mCommands->Fail();
        if (mCommands->Tell() > 0) {
            (void)sceSifCallRpc(&g_aSynthClients[kSynthModule],
                                kCommandConcatenation,
                                SIF_RPCM_NOWAIT,
                                mCommandBuffer.mBytes,
                                mCommands->Tell(),
                                nullptr,
                                0,
                                nullptr,
                                nullptr);
        }
        mCommands->Seek(0, BinStream::kSeekBegin);
    }

    if (IsSoftFxRpcBusy()) {
        mTimeUpdated = 0;
        return 0;
    }
    PollStream();
    (void)mSoftFxCommands->Fail();
    UpdateSoftFx();
    (void)mSoftFxCommands->Fail();
    if (mSoftFxCommands->Tell() > 0) {
        (void)sceSifCallRpc(&g_aSynthClients[kSoftFxModule],
                            kSoftFxCommandConcatenation,
                            SIF_RPCM_NOWAIT,
                            mSoftFxBuffer.mBytes,
                            mSoftFxCommands->Tell(),
                            nullptr,
                            0,
                            nullptr,
                            nullptr);
    }
    mSoftFxCommands->Seek(0, BinStream::kSeekBegin);
    mTimeUpdated = 0;
    return 0;
}

void SynthPS2::ProcessJobs() {
    if (mJobs.empty()) {
        return;
    }
    const bool bBusy = IsRpcBusy();
    QueuedJob *pJob = mJobs.front().get();
    switch (pJob->GetType()) {
    case QueuedJob::kTypePartition: {
        PartitionQJ *pPartition = dynamic_cast<PartitionQJ *>(pJob);
        if (bBusy) {
            return;
        }
        if (pPartition->GetStarted() == 1) {
            mJobs.pop_front();
        } else {
            (void)SendPartition(pPartition->GetBlocks());
            pPartition->SetStarted(1);
        }
        break;
    }
    case QueuedJob::kTypeDeactivateBank: {
        DeactivateBankQJ *pDeactivate = dynamic_cast<DeactivateBankQJ *>(pJob);
        if (bBusy) {
            return;
        }
        if (pDeactivate->GetStarted() == 1) {
            mJobs.pop_front();
        } else {
            (void)SendUnloadBank(pDeactivate->GetSlot());
            pDeactivate->SetStarted(1);
        }
        break;
    }
    case QueuedJob::kTypeBankLoad: {
        BankLoadQJ *pLoad = dynamic_cast<BankLoadQJ *>(pJob);
        if (mLoadState != kLoadIdle) {
            return;
        }
        if (pLoad->GetStarted() == 1) {
            pLoad->GetTimer().Stop();
            mJobs.pop_front();
        } else {
            pLoad->GetTimer().Reset();
            pLoad->GetTimer().Start();
            (void)StartBankLoad(pLoad->GetSlot(), pLoad->GetFile());
            pLoad->SetStarted(1);
        }
        break;
    }
    case QueuedJob::kTypeEnableDisableSoftFx: {
        EnableDisableSoftFxQJ *pSoftFx = dynamic_cast<EnableDisableSoftFxQJ *>(pJob);
        if (pSoftFx->GetStarted() != 1) {
            SendSoftFxEnable(pSoftFx->GetEnable());
            pSoftFx->SetStarted(1);
        } else if (pSoftFx->GetWaitPolls() == 0) {
            mJobs.pop_front();
        } else {
            pSoftFx->SetWaitPolls(pSoftFx->GetWaitPolls() - 1);
        }
        break;
    }
    case QueuedJob::kTypeSetupCoreFx: {
        SetupCoreFxQJ *pCoreFx = dynamic_cast<SetupCoreFxQJ *>(pJob);
        if (pCoreFx->GetStarted() != 1) {
            SetCoreFxPending();
            SendMessageList(pCoreFx->GetMessages());
            pCoreFx->SetStarted(1);
        } else if (pCoreFx->GetWaitPolls() > 0) {
            pCoreFx->SetWaitPolls(pCoreFx->GetWaitPolls() - 1);
        } else if (IsCoreFxDone()) {
            mJobs.pop_front();
        } else {
            // Yes, the count goes on below zero.
            pCoreFx->SetWaitPolls(pCoreFx->GetWaitPolls() - 1);
        }
        break;
    }
    case QueuedJob::kTypeCallback: {
        const CallbackQJ *pCallback = dynamic_cast<CallbackQJ *>(pJob);
        pCallback->GetCallback()(pCallback->GetArg());
        mJobs.pop_front();
        break;
    }
    default:
        break;
    }
}

void SynthPS2::AdjustDelays(int nBias) {
    for (unsigned int &nMessage : mMidiQueue) {
        const int nDelay =
            std::min(std::max(0, static_cast<int>(nMessage >> kDelayShift) + nBias), kMaxDelay);
        nMessage = (nMessage & ~(kByteMask << kDelayShift)) |
                   (static_cast<unsigned int>(nDelay) << kDelayShift);
    }
}

int SynthPS2::FlushMidi() {
    if (mMidiQueue.empty()) {
        return 0;
    }
    if (IsRpcBusy()) {
        return 0;
    }
    if (mTimeUpdated) {
        const float fNowMs = SystemMs();
        AdjustDelays(
            static_cast<int>(static_cast<double>(mLagMs - (fNowMs - mLastMs)) + kFixedRound));
    }
    SendCommand(kCommandQueueEvents,
                mMidiQueue.data(),
                static_cast<int>(mMidiQueue.size() * sizeof(unsigned int)));
    mMidiQueue.clear();
    return 1;
}

int SynthPS2::EnableSoftFx() {
    mJobs.push_back(std::make_unique<EnableDisableSoftFxQJ>(1));
    return 0;
}

void SynthPS2::QueueCallback(Callback pfnCallback, int nArg) {
    mJobs.push_back(std::make_unique<CallbackQJ>(pfnCallback, nArg));
}

void SynthPS2::SendSoftFxEnable(int nEnable) {
    BufStream *pStream = mSoftFxCommands.get();
    if (nEnable) {
        const int nCommand = kSoftFxCommandStart;
        pStream->WriteEndian(&nCommand, sizeof(nCommand));
        const int nSize = 0;
        pStream->WriteEndian(&nSize, sizeof(nSize));
    } else {
        const int nCommand = kSoftFxCommandStop;
        pStream->WriteEndian(&nCommand, sizeof(nCommand));
        const int nSize = 0;
        pStream->WriteEndian(&nSize, sizeof(nSize));
    }
    (void)mSoftFxCommands->Fail();
    mSoftFxEnabled = nEnable;
}

int SynthPS2::DisableSoftFx() {
    mJobs.push_back(std::make_unique<EnableDisableSoftFxQJ>(0));
    return 0;
}

int SynthPS2::GetStreamFormat(int *pnRate, int *pnBlock) {
    *pnRate = kStreamRate;
    *pnBlock = kStreamBlock;
    return 0;
}

int SynthPS2::SendPartition(const std::vector<int> &blocks) {
    SendCommand(
        kCommandSetBankLayout, blocks.data(), static_cast<int>(blocks.size() * sizeof(int)));
    return 0;
}

int SynthPS2::Partition(const std::vector<int> &blocks) {
    mJobs.push_back(std::make_unique<PartitionQJ>(blocks));
    return 0;
}

void SynthPS2::SetStream(StreamPlayer *pStream) {
    mStream = pStream;
    SendStreamData(nullptr, 0);
}

void SynthPS2::SendStreamData(const void *pData, int nSize) {
    BufStream *pStream = mSoftFxCommands.get();
    const int nCommand = kSoftFxCommandStreamData;
    pStream->WriteEndian(&nCommand, sizeof(nCommand));
    pStream->WriteEndian(&nSize, sizeof(nSize));
    (void)mSoftFxCommands->Fail();
    mSoftFxCommands->Write(pData, nSize);
    (void)mSoftFxCommands->Fail();
    ClearStreamStatus();
}

void SynthPS2::PollStream() {
    if (mStream != nullptr) {
        mStream->Poll();
    }
}

void SynthPS2::SetSoftFxModulation(float fValue) {
    mSoftFxModulation = fValue;
}

void SynthPS2::SetSoftFxSweep(float fPosition) {
    switch (mFilter.mType) {
    case SoftFxFilter::kTypeOff:
    case SoftFxFilter::kTypeStream:
    case SoftFxFilter::kTypeBypass:
        break;
    case SoftFxFilter::kTypeSweptFilter:
    case SoftFxFilter::kTypeStutter:
    case SoftFxFilter::kTypeFilter:
    case SoftFxFilter::kTypeChorus:
    case SoftFxFilter::kTypeFilterVariant:
    case SoftFxFilter::kTypeDistort:
    case SoftFxFilter::kTypeDistortedFilter:
        mSoftFxSweep = fPosition;
        break;
    default:
        DebugNotify(FormatString("Bad filter type param: type = %i.\n", mFilter.mType));
        break;
    }
}

void SynthPS2::SetSoftFxFilter(const SoftFxFilter &filter) {
    BufStream *pStream = mSoftFxCommands.get();
    const int nCommand = kSoftFxCommandSetEffect;
    pStream->WriteEndian(&nCommand, sizeof(nCommand));
    const int nSize = kWordSize;
    pStream->WriteEndian(&nSize, sizeof(nSize));
    const int nType = filter.mType;
    pStream->WriteEndian(&nType, sizeof(nType));
    (void)mSoftFxCommands->Fail();
    SetOutputLevel(1.0f);

    switch (filter.mType) {
    case SoftFxFilter::kTypeSweptFilter:
    case SoftFxFilter::kTypeFilter:
    case SoftFxFilter::kTypeFilterVariant:
    case SoftFxFilter::kTypeDistortedFilter: {
        const float fRatio =
            static_cast<float>(filter.mParam.mInitFreq) / static_cast<float>(filter.mFreqCeiling);
        mSoftFxSweep = fRatio;
        if (1.0f < fRatio) {
            mSoftFxSweep = 1.0f;
        } else if (static_cast<double>(fRatio) < 0.01) {
            mSoftFxSweep = 0.01f;
        }
        break;
    }
    case SoftFxFilter::kTypeDistort:
        mSoftFxSweep = 0.0f;
        break;
    case SoftFxFilter::kTypeChorus:
        mSoftFxSweep = 0.0f;
        for (int i = 0; i < kNumTaps; ++i) {
            const SoftFxFilter::Tap &tap = filter.mTaps[i];
            Lfo &lfo = mTapOscillators[i];
            lfo.SetWaveform(tap.mLfo);
            lfo.SetPeriod((tap.mOscMin + tap.mOscMax) * 0.5f);
            lfo.SetPhase(tap.mPhase);
            lfo.Stop();
            lfo.Start();
        }
        break;
    default:
        break;
    }
    mFilter = filter;
}

void SynthPS2::UpdateSoftFx() {
    if (mSoftFxEnabled == 0) {
        return;
    }

    BufStream *pStream = mSoftFxCommands.get();
    const int nCommand = kSoftFxCommandSetParams;
    const int nSize = kSoftFxParamsSize;
    SoftFxParams params;
    switch (mFilter.mType) {
    case SoftFxFilter::kTypeOff:
    case SoftFxFilter::kTypeStream:
    case SoftFxFilter::kTypeBypass:
        return;
    case SoftFxFilter::kTypeSweptFilter:
    case SoftFxFilter::kTypeFilter: {
        g_abSynthSweepFilter.SetTarget(mSoftFxSweep);
        const float fSweep = g_abSynthSweepFilter.Update();
        float fFrequency = (fSweep + 1.0f) * 0.5f * static_cast<float>(mFilter.mFreqCeiling);
        const float fFloor = static_cast<float>(mFilter.mFreqFloor);
        if (fFrequency < fFloor) {
            fFrequency = fFloor;
        }
        mFilter.mFrequency = fFrequency;
        const float fOmega = fFrequency / kSampleRate * 2.0f;
        const float fPole = fOmega * kSweepPoleLinear - fOmega * kSweepPoleSquare * fOmega - 1.0f;
        const float fCutoff = (fPole + 1.0f) * 0.5f;
        float fResonance = mFilter.mMix.mResonance * expf((1.0f - fCutoff) * kSweepDamping);
        params.mMode = 0;
        params.mCutoff = 0;
        params.mFeedback = 0;
        params.mLevel = 0;
        params.mLevel = static_cast<short>(ToFixed(fPole));
        params.mCutoff = static_cast<short>(ToFixed(fCutoff));
        params.mMode = (1.0f <= fResonance) ? 1 : 0;
        if (params.mMode) {
            fResonance = 1.0f / fResonance;
        }
        params.mFeedback = static_cast<short>(ToFixed(fResonance));
        pStream->WriteEndian(&nCommand, sizeof(nCommand));
        pStream->WriteEndian(&nSize, sizeof(nSize));
        break;
    }
    case SoftFxFilter::kTypeFilterVariant:
    case SoftFxFilter::kTypeDistortedFilter: {
        pStream->WriteEndian(&nCommand, sizeof(nCommand));
        pStream->WriteEndian(&nSize, sizeof(nSize));
        const float fModulation = (mSoftFxModulation + 1.0f) * 0.5f;
        for (int nChannel = 0; nChannel < kNumTaps; ++nChannel) {
            g_abSynthSweepFilter.SetTarget(mSoftFxSweep);
            float fSweep = (g_abSynthSweepFilter.Update() + 1.0f) * 0.5f;
            if (nChannel == 1) {
                fSweep = static_cast<float>(static_cast<double>(fSweep) * kRightFrequencyScale);
            }
            float fFloor = static_cast<float>(mFilter.mFreqFloor);
            if (kModulationFloorStart <= fModulation) {
                fFloor = fFloor + (fModulation - kModulationFloorStart) * 2.0f *
                                      (kModulationFloorTop - fFloor);
            }
            float fFrequency = fSweep * static_cast<float>(mFilter.mFreqCeiling);
            if (fFrequency < fFloor) {
                fFrequency = fFloor;
            }
            mFilter.mFrequency = fFrequency;
            float fResonance = mFilter.mMix.mResonance;
            if (nChannel == 1) {
                fResonance =
                    static_cast<float>(static_cast<double>(fResonance) * kRightResonanceScale);
            }
            if (kModulationResonanceStart <= fModulation) {
                fResonance = fResonance - (fModulation - kModulationResonanceStart) *
                                              kModulationResonanceSlope * (fResonance - 1.0f);
            }
            const float fOmega =
                static_cast<float>(static_cast<double>(fFrequency) * kTwoPi / kSampleRateDouble);
            const double dOmega = fOmega;
            double dDamping = static_cast<double>(fResonance) + 0.5 / (dOmega + 1.0);
            dDamping = dDamping + dDamping;
            const float fDamping = static_cast<float>(1.0 - dOmega / ((dDamping + dOmega) - 2.0));
            const float fCos = cosf(fOmega);
            const float fDampingSquared = fDamping * fDamping;
            const float fCoefficient = static_cast<float>(
                (static_cast<double>(fDampingSquared) + 1.0) -
                (static_cast<double>(fCos) * 2.0) * static_cast<double>(fDamping));
            params.mMode = 0;
            params.mCutoff = 0;
            params.mFeedback = 0;
            params.mLevel = 0;
            params.mFeedback = ToFixed(fDampingSquared);
            params.mLevel = ToFixed(fCoefficient);
            mSoftFxCommands->Write(&params, sizeof(params));
        }
        return;
    }
    case SoftFxFilter::kTypeStutter: {
        const float fSweep = (mSoftFxSweep + 1.0f) * 0.5f;
        const float fRate = static_cast<float>(static_cast<double>(fSweep) * kStutterRateScale);
        params.mMode = 0;
        params.mCutoff = 0;
        params.mFeedback = 0;
        params.mLevel = 0;
        params.mLevel = static_cast<short>(ToFixed(fRate));
        params.mFeedback = kStutterFeedback;
        pStream->WriteEndian(&nCommand, sizeof(nCommand));
        pStream->WriteEndian(&nSize, sizeof(nSize));
        break;
    }
    case SoftFxFilter::kTypeDistort: {
        const float fSweep = (mSoftFxSweep + 1.0f) * 0.5f;
        params.mMode = 0;
        params.mCutoff = 0;
        params.mFeedback = 0;
        short nLevel = static_cast<short>(static_cast<int>((1.0f - fSweep) * kFullVolume));
        if (nLevel < 0) {
            nLevel = -nLevel;
        }
        params.mLevel = nLevel;
        params.mFeedback = static_cast<short>(ToFixed(mFilter.mMix.mFeedback));
        pStream->WriteEndian(&nCommand, sizeof(nCommand));
        pStream->WriteEndian(&nSize, sizeof(nSize));
        break;
    }
    case SoftFxFilter::kTypeChorus: {
        const float fSweep = (mSoftFxSweep + 1.0f) * 0.5f;
        params.mMode = 0;
        params.mCutoff = 0;
        params.mFeedback = 0;
        params.mLevel = 0;
        params.mFeedback = static_cast<short>(ToFixed(mFilter.mMix.mFeedback));
        params.mLevel = static_cast<short>(ToFixed(mFilter.mParam.mAttenuation));
        for (int i = 0; i < kNumTaps; ++i) {
            const SoftFxFilter::Tap &tap = mFilter.mTaps[i];
            Lfo &lfo = mTapOscillators[i];
            const float fFastest = 1.0f / tap.mOscMax;
            lfo.SetPeriod((1.0f / tap.mOscMin - fFastest) * fSweep + fFastest);
            const float fValue = lfo.Evaluate();
            params.mTaps[i].mDelay = static_cast<int>(
                static_cast<float>(tap.mDelayMin) +
                (fValue + 1.0f) * 0.5f * static_cast<float>(tap.mDelayMax - tap.mDelayMin));
            params.mTaps[i].mGain = static_cast<short>(ToFixed(tap.mVolume));
        }
        pStream->WriteEndian(&nCommand, sizeof(nCommand));
        pStream->WriteEndian(&nSize, sizeof(nSize));
        break;
    }
    default:
        DebugNotify(FormatString("Bad softfx type: ignoring. type = %i.\n", mFilter.mType));
        return;
    }
    mSoftFxCommands->Write(&params, sizeof(params));
    mSoftFxCommands->Write(&params, sizeof(params));
}

void SynthPS2::SetSoftFxInput(int nInput) {
    BufStream *pStream = mSoftFxCommands.get();
    const int nCommand = kSoftFxCommandRouteInput;
    pStream->WriteEndian(&nCommand, sizeof(nCommand));
    const int nSize = kWordSize;
    pStream->WriteEndian(&nSize, sizeof(nSize));
    mSoftFxCommands->WriteEndian(&nInput, sizeof(nInput));
}

void SynthPS2::SetBusToCoreFx(int nOn) {
    BufStream *pStream = mSoftFxCommands.get();
    const int nCommand = kSoftFxCommandRouteVoices;
    pStream->WriteEndian(&nCommand, sizeof(nCommand));
    const int nSize = kWordSize;
    pStream->WriteEndian(&nSize, sizeof(nSize));
    mSoftFxCommands->WriteEndian(&nOn, sizeof(nOn));
}

void SynthPS2::SetBusToSoftFx([[maybe_unused]] int nOn) {
}

void SynthPS2::SendMessages(const std::vector<unsigned int> &messages) {
    mJobs.push_back(std::make_unique<SetupCoreFxQJ>(messages));
}

void SynthPS2::SendMessageList(const std::vector<unsigned int> &messages) {
    const int nCount = static_cast<int>(messages.size());
    for (int i = 0; i < nCount; ++i) {
        (void)SendPackedMessage(messages[i]);
    }
}

void SynthPS2::SetSweepFromController(unsigned char nValue) {
    SetSoftFxSweep(static_cast<float>(nValue) * kControllerScale * 2.0f - 1.0f);
}

void SynthPS2::SetLevelFromController(unsigned char nValue) {
    SetOutputLevel(static_cast<float>(nValue) * kControllerScale);
}

void SynthPS2::SetOutputLevel(float fLevel) {
    mOutputLevel = fLevel;
    const unsigned int nVolume = static_cast<unsigned int>(fLevel * kFullVolume);
    BufStream *pStream = mSoftFxCommands.get();
    const int nCommand = kSoftFxCommandSetVolume;
    pStream->WriteEndian(&nCommand, sizeof(nCommand));
    const int nSize = kWordSize;
    pStream->WriteEndian(&nSize, sizeof(nSize));
    mSoftFxCommands->WriteEndian(&nVolume, sizeof(nVolume));
}

float SynthPS2::GetOutputLevel() {
    return mOutputLevel;
}

bool SynthPS2::HasPendingLoad(unsigned short nSlot) {
    bool bPending = false;
    for (const auto &pJob : mJobs) {
        const BankLoadQJ *pLoad = dynamic_cast<const BankLoadQJ *>(pJob.get());
        if ((pLoad != nullptr) && (pLoad->GetSlot() == nSlot)) {
            bPending = true;
            break;
        }
    }
    return bPending;
}

int SynthPS2::GetStreamStatus() {
    return ReadStreamStatus();
}

void SynthPS2::SetSurround(bool bSurround) {
    const int nSurround = bSurround ? 1 : 0;
    SendCommand(kCommandSurround, &nSurround, sizeof(nSurround));
}

void SynthPS2::SetMono(bool bMono) {
    const int nMono = bMono ? 1 : 0;
    SendCommand(kCommandMono, &nMono, sizeof(nMono));
    BufStream *pStream = mSoftFxCommands.get();
    const int nCommand = kSoftFxCommandSetMonoStream;
    pStream->WriteEndian(&nCommand, sizeof(nCommand));
    const int nSize = kWordSize;
    pStream->WriteEndian(&nSize, sizeof(nSize));
    pStream->WriteEndian(&nMono, sizeof(nMono));
    (void)mSoftFxCommands->Fail();
}

void SynthPS2::ResetSpu() {
    SendCommand(kCommandResetSpu, nullptr, 0);
}
