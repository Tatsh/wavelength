#include "synth_s/synth.h"

#include <kernel.h>
#include <libsd.h>
#include <sif.h>
#include <sysclib.h>

#include "synth_s/bank.h"
#include "synth_s/ioputil.h"
#include "synth_s/midieventqueue.h"
#include "synth_s/random.h"
#include "synth_s/synthlock.h"
#include "synth_s/vibrato.h"

namespace {

// Status nibbles of the channel messages the synthesiser acts on.
enum MidiStatus {
    kStatusNoteOff = 0x80,
    kStatusNoteOn = 0x90,
    kStatusControlChange = 0xb0,
    kStatusProgramChange = 0xc0,
    kStatusPitchBend = 0xe0,
    kStatusSystem = 0xf0,
};

constexpr unsigned char kStatusTypeMask = 0xf0;
constexpr unsigned char kStatusChannelMask = 0x0f;
constexpr unsigned char kDataByteFlag = 0x80;

// Controllers the synthesiser acts on.
enum MidiController {
    kControllerBankSelect = 0,
    kControllerVolume = 7,
    kControllerPan = 10,
    kControllerMonophonic = 13,
    kControllerPriority = 16,
    kControllerExpression = 17,
    kControllerDetune = 18,
    kControllerStereo = 19,
    kControllerEffectMode = 75,
    kControllerEffectDepthLeft = 76,
    kControllerEffectDepthRight = 77,
    kControllerEffectDelay = 78,
    kControllerEffectFeedback = 79,
    kControllerEnableUpdate = 82,
    kControllerDisableUpdate = 83,
    kControllerTranspose = 91,
    kControllerBusMode = 93,
    kControllerBus = 94,
    kControllerResetAll = 121,
    kControllerAllNotesOff = 123,
};

// The pitch bend wheel the second argument of SetControllerEnable() selects.
constexpr unsigned char kPitchBendController = 0xe0;

// SetEffectParam() parameters.
enum EffectParam {
    kEffectParamMode,
    kEffectParamDepthLeft,
    kEffectParamDepthRight,
    kEffectParamDelay,
    kEffectParamFeedback,
};

// SetVoiceMix() modes.
enum VoiceMixMode {
    kMixDry = 0,
    kMixWet = 1,
    kMixDryAndWet = 2,
    kMixNone = 3,
};

// Channel::mBus values.
enum ChannelBus {
    kBusEither = 0,
    kBusCore0 = 1,
    kBusCore1 = 2,
};

// SampleDesc::mSusMode values and the ADSR2 bits they select.
enum SustainMode {
    kSustainLinearUp = 0,
    kSustainLinearDown = 1,
    kSustainExponentialUp = 2,
    kSustainExponentialDown = 3,
};

constexpr int kAnyCore = -1;
constexpr int kMidiMax = 127;
constexpr unsigned char kMidiCenter = 64;
constexpr unsigned char kControllerOn = 64;
constexpr unsigned char kUpdateEnabled = 127;
constexpr unsigned char kDefaultVolume = 100;
constexpr unsigned char kDefaultExpression = 100;
constexpr unsigned char kDefaultPriority = 10;
constexpr int kPanOffset = 128;
constexpr int kTicksPerVoiceUpdate = 10;
constexpr unsigned int kNewestAge = 0x7fffffff;
constexpr unsigned int kOldestValidAge = 0x7ffffffe;
constexpr int kRandomSeed = 123;

// Steps of a 14-bit controller value.
constexpr int kFourteenBitRange = 16383;
// Pitch bend spans 24 semitones each way, in cents.
constexpr int kPitchBendCentsRange = 4800;
constexpr int kPitchBendCentsOffset = 2400;
// Transposition spans 96 semitones in cents, scaled from a 7-bit value.
constexpr int kTransposeScale = 1228800;
constexpr int kTransposeCentsOffset = 4800;
constexpr int kCentsPerSemitone = 100;

// Delay units of a timed message, system clock cycles per millisecond less one.
constexpr unsigned int kDelayUnitCycles = 36863;

// ADSR register fields.
constexpr unsigned short kAttackExponential = 0x8000;
constexpr unsigned short kReleaseExponential = 0x20;
constexpr unsigned short kSustainModeBits[] = {0x0000, 0x4000, 0x8000, 0xc000};
constexpr unsigned int kAttackRateMask = 0x7f;
constexpr unsigned int kDecayRateMask = 0x0f;
constexpr unsigned int kSusLevelMask = 0x0f;
constexpr unsigned int kSusRateMask = 0x7f;
constexpr unsigned int kReleaseRateMask = 0x1f;

// Vibrato phase of the second layer of a stereo pair.
constexpr int kDetunePhase = 25;
// Vibrato tables a voice reads.
enum VibratoTable {
    kVibratoFast = 0,
    kVibratoSlow = 1,
};

// SPU2 mixer and reverb setup.
constexpr unsigned short kCore0MixSettings = 0xff0;
constexpr unsigned short kCore1MixSettings = 0xffc;
constexpr unsigned short kMasterVolume = 0x3fff;
constexpr unsigned int kSpuTop = 0x1fffff;
constexpr int kEffectAreaShift = 17;
constexpr int kEffectModeController = 0x100;

// The banks' SPU2 areas start here and are sized in 64-byte units.
constexpr unsigned int kBankSpuBase = 0x5010;
constexpr int kBankSizeShift = 6;

// Every bank record arrives after a word the loader skips.
constexpr int kRecordPrefixSize = 4;

constexpr int kSpuChannel = 1;
constexpr unsigned int kTransferEnded = 1;
constexpr int kRetryMicroseconds = 1000;

constexpr int kNotifyBufferSize = 64;
constexpr int kNotifySize = 16;

const char kTickLockName[] = "TickCallbackFunc";

// Reverb depth registers for each controller value.
// NTSC-U/C: 0x00009b00
const int kEffectDepthTable[] = {
    -32768, -32252, -31736, -31220, -30704, -30188, -29672, -29156, -28640, -28124, -27608, -27092,
    -26576, -26060, -25544, -25028, -24512, -23996, -23480, -22964, -22448, -21932, -21416, -20900,
    -20384, -19868, -19352, -18836, -18320, -17804, -17288, -16772, -16256, -15740, -15224, -14708,
    -14192, -13676, -13160, -12644, -12128, -11612, -11096, -10580, -10064, -9548,  -9032,  -8516,
    -8000,  -7484,  -6968,  -6452,  -5936,  -5420,  -4904,  -4388,  -3872,  -3356,  -2840,  -2324,
    -1808,  -1292,  -776,   -260,   0,      772,    1288,   1804,   2320,   2836,   3352,   3868,
    4384,   4900,   5416,   5932,   6448,   6964,   7480,   7996,   8512,   9028,   9544,   10060,
    10576,  11092,  11608,  12124,  12640,  13156,  13672,  14188,  14704,  15220,  15736,  16252,
    16768,  17284,  17800,  18316,  18832,  19348,  19864,  20380,  20896,  21412,  21928,  22444,
    22960,  23476,  23992,  24508,  25024,  25540,  26056,  26572,  27088,  27604,  28120,  28636,
    29152,  29668,  30184,  30700,  31216,  31732,  32248,  32764,
};

// The entry value of a voice register.
inline unsigned short VoiceEntry(int core, int voice, unsigned short reg) {
    return core | SD_VOICE(voice) | reg;
}

// Send an empty notification block to an EE address.
inline void NotifyEe(void *address) {
    unsigned char buffer[kNotifyBufferSize];
    sceSifDmaData dma;
    int state;
    memset(buffer, 0, sizeof buffer);
    memset(&dma, 0, sizeof dma);
    dma.data = buffer;
    dma.addr = address;
    dma.size = kNotifySize;
    dma.mode = 0;
    CpuSuspendIntr(&state);
    sceSifSetDma(&dma, 1);
    CpuResumeIntr(state);
}

} // namespace

// NTSC-U/C: 0x00006a00
int Synth::sTickCount;
// NTSC-U/C: 0x00009d00
void *Synth::sNotifyAddress;
// NTSC-U/C: 0x00009d04
int Synth::sNotifyPending;
// NTSC-U/C: 0x00009d08
void *Synth::sEffectNotifyAddress;
// NTSC-U/C: 0x00009d0c
int Synth::sEffectNotifyPending;
// NTSC-U/C: 0x00009d10
int Synth::sEffectThreadId;
// NTSC-U/C: 0x00009d14
unsigned char Synth::sEffectsApplying;
// NTSC-U/C: 0x00014198
Channel Synth::sChannels[kNumChannels];
// NTSC-U/C: 0x00014418
Voice Synth::sVoices[kNumCores][kVoicesPerCore];
// NTSC-U/C: 0x00014ad8
int Synth::sRetiredVoiceCount;
// NTSC-U/C: 0x00014adc
Voice Synth::sRetiredVoices[kMaxRetiredVoices];
// NTSC-U/C: 0x000151e4
unsigned char Synth::sVoiceMixDirty;
// NTSC-U/C: 0x000151e5
unsigned char Synth::sEffectsDirty;
// NTSC-U/C: 0x000151e6
unsigned char Synth::sMono;
// NTSC-U/C: 0x000151e7
unsigned char Synth::sSurroundDisabled;
// NTSC-U/C: 0x000151e8
int Synth::sVibratoTick;
// NTSC-U/C: 0x000151ec
unsigned int Synth::sNoteSerial;
// NTSC-U/C: 0x000151f0
int Synth::sTransferBusy;
// NTSC-U/C: 0x000151f4
int Synth::sTransferCancel;
// NTSC-U/C: 0x00015228
int Synth::sVoicesInUse[kNumCores];
// NTSC-U/C: 0x00015230
unsigned int Synth::sKeyOnMask[kNumCores];
// NTSC-U/C: 0x00015238
unsigned int Synth::sKeyOffMask[kNumCores];
// NTSC-U/C: 0x00015240
unsigned int Synth::sVoiceMixDryLeft[kNumCores];
// NTSC-U/C: 0x00015248
unsigned int Synth::sVoiceMixDryRight[kNumCores];
// NTSC-U/C: 0x00015250
unsigned int Synth::sVoiceMixWetLeft[kNumCores];
// NTSC-U/C: 0x00015258
unsigned int Synth::sVoiceMixWetRight[kNumCores];
// NTSC-U/C: 0x00015260
sceSdEffectAttr Synth::sEffectAttr[kNumCores];
// NTSC-U/C: 0x000152b0
unsigned char Synth::sInitFlag;

void Synth::TickThread() {
    for (;;) {
        SleepThread();
        SynthLock::Lock(kTickLockName);
        if (sTickCount == kTicksPerVoiceUpdate) {
            UpdateVoiceStates();
            sTickCount = 0;
        }
        ProcessDueEvents();
        UpdateVibrato();
        ++sTickCount;
        if (sNotifyPending != 0) {
            NotifyEe(sNotifyAddress);
            sNotifyPending = 0;
        }
        if (sEffectNotifyPending != 0) {
            NotifyEe(sEffectNotifyAddress);
            sEffectNotifyPending = 0;
        }
        SynthLock::Unlock(kTickLockName);
    }
}

void Synth::EffectThread() {
    for (;;) {
        SleepThread();
        ApplyEffects();
    }
}

int Synth::ProcessMidiMessage(unsigned char status, unsigned char data1, unsigned char data2) {
    if ((status & kStatusTypeMask) == kStatusSystem) {
        return 0;
    }
    const unsigned char channel = status & kStatusChannelMask;
    switch (status & kStatusTypeMask) {
    case kStatusNoteOn:
        NoteOn(data1, data2, channel);
        break;
    case kStatusNoteOff:
        NoteOff(data1, channel);
        break;
    case kStatusProgramChange:
        ProgramChange(channel, data1);
        break;
    case kStatusControlChange:
        switch (data1) {
        case kControllerBankSelect:
            BankSelect(channel, data2);
            break;
        case kControllerVolume:
            SetVolume(channel, data2);
            break;
        case kControllerExpression:
            SetExpression(channel, data2);
            break;
        case kControllerPan:
            SetPan(channel, data2);
            break;
        case kControllerPriority:
            SetPriority(channel, data2);
            break;
        case kControllerMonophonic:
            SetMonophonic(channel, data2);
            break;
        case kControllerEnableUpdate:
            SetControllerEnable(channel, data2, kMidiMax);
            break;
        case kControllerDisableUpdate:
            SetControllerEnable(channel, data2, 0);
            break;
        case kControllerDetune:
            SetDetune(channel, data2);
            break;
        case kControllerStereo:
            SetStereo(channel, data2);
            break;
        case kControllerTranspose: {
            const int cents =
                ((data2 * kTransposeScale) / kFourteenBitRange) - kTransposeCentsOffset;
            SetTranspose(channel, cents / kCentsPerSemitone);
            break;
        }
        case kControllerBusMode:
            SetBusMode(channel, data2);
            break;
        case kControllerBus:
            SetBus(channel, data2);
            break;
        case kControllerAllNotesOff:
            AllNotesOff(channel);
            break;
        case kControllerResetAll:
            ResetAllControllers(channel);
            break;
        case kControllerEffectMode:
            SetEffectParam(channel, data2, kEffectParamMode);
            break;
        case kControllerEffectDepthLeft:
            SetEffectParam(channel, data2, kEffectParamDepthLeft);
            break;
        case kControllerEffectDepthRight:
            SetEffectParam(channel, data2, kEffectParamDepthRight);
            break;
        case kControllerEffectDelay:
            SetEffectParam(channel, data2, kEffectParamDelay);
            break;
        case kControllerEffectFeedback:
            SetEffectParam(channel, data2, kEffectParamFeedback);
            break;
        default:
            break;
        }
        break;
    case kStatusPitchBend: {
        const int bend = data1 | (data2 << 7);
        const int cents =
            ((bend * kPitchBendCentsRange) / kFourteenBitRange) - kPitchBendCentsOffset;
        const int semitones = cents / kCentsPerSemitone;
        SetPitchBend(channel, semitones, cents - (semitones * kCentsPerSemitone));
        break;
    }
    default:
        break;
    }
    return 0;
}

void Synth::ProcessDueEvents() {
    // NTSC-U/C: 0x00025650
    static SysClock now;
    // NTSC-U/C: 0x00025658
    static MidiEvent event;
    // NTSC-U/C: 0x0002565c
    static MidiEvent message;
    GetSystemTime(&now);
    ClearKeyMasks();
    while (MidiEventQueue::Pop(&event, now.low, now.hi)) {
        message = event;
        ProcessMidiMessage(message.mStatus, message.mData1, message.mData2);
    }
    CommitKeys();
}

int Synth::QueueTimedEvents(const MidiEvent *events, unsigned int count) {
    // NTSC-U/C: 0x00025660
    static SysClock now;
    // NTSC-U/C: 0x00025668
    static MidiEvent message;
    // NTSC-U/C: 0x0002566c
    static unsigned int dueLow;
    // NTSC-U/C: 0x00025670
    static unsigned int dueHigh;
    // NTSC-U/C: 0x00025674
    static unsigned int delay;
    GetSystemTime(&now);
    for (unsigned int i = 0; i < count; ++i) {
        const MidiEvent event = events[i];
        message = event;
        delay = message.mDelay * kDelayUnitCycles;
        dueLow = now.low;
        dueHigh = now.hi;
        if (~dueLow < delay) {
            ++dueHigh;
            dueLow = delay + 1 + dueLow; // Yes, retail adds one more cycle when the low word wraps.
        } else {
            dueLow += delay;
        }
        MidiEventQueue::Push(event, dueLow, dueHigh, MidiEventQueue::kPrimaryQueue);
    }
    return 0;
}

int Synth::ProcessMidiStream(const unsigned char *data, unsigned int size) {
    if (size == 0) {
        return 0;
    }
    unsigned int i = 0;
    do {
        const unsigned char status = data[i++];
        if ((status & kStatusTypeMask) == kStatusSystem) {
            return 0;
        }
        const bool noteOn = (status & kStatusTypeMask) == kStatusNoteOn;
        const unsigned char noteOff = (status & kStatusChannelMask) | kStatusNoteOff;
        // Data byte pairs follow under running status until the next status byte. Retail does not
        // check the size here.
        while ((data[i] & kDataByteFlag) == 0) {
            const unsigned char data1 = data[i++];
            const unsigned char data2 = data[i++];
            if (noteOn && (data2 == 0)) {
                ProcessMidiMessage(noteOff, data1, 0);
            } else {
                ProcessMidiMessage(status, data1, data2);
            }
        }
    } while (i < size);
    return 0;
}

void Synth::SetMono(bool mono) {
    sMono = mono;
}

void Synth::SetSurround(bool enabled) {
    sSurroundDisabled = !enabled;
}

void Synth::Init() {
    Random::Seed(kRandomSeed);
    for (int i = 0; i < Bank::kMaxBanks; ++i) {
        Bank::sBanks[i].Init();
        Bank::sBanks[i].mSpuAddress = 0;
        Bank::sBanks[i].mSpuSize = 0;
    }
    for (int i = 0; i < kNumChannels; ++i) {
        sChannels[i].Init();
    }
    for (int core = 0; core < kNumCores; ++core) {
        for (int voice = 0; voice < kVoicesPerCore; ++voice) {
            sVoices[core][voice].Init();
        }
    }
    for (int i = 0; i < kMaxRetiredVoices; ++i) {
        sRetiredVoices[i].Init();
    }
    ResetSpu();
}

void Synth::ResetSpu() {
    sceSdInit(0);
    SpuSetParam(SD_P_MMIX | SD_CORE_0, kCore0MixSettings);
    SpuSetParam(SD_P_MMIX | SD_CORE_1, kCore1MixSettings);
    for (int core = 0; core < kNumCores; ++core) {
        // Each core's reverb area ends at the top of its half of the top 256 KiB.
        sceSdSetAddr(SD_A_EEA | core, kSpuTop - ((SD_CORE_1 - core) << kEffectAreaShift));
        sceSdSetCoreAttr(core | SD_C_EFFECT_ENABLE, 1);
        SpuSetParam(core | SD_P_MVOLL, kMasterVolume);
        SpuSetParam(core | SD_P_MVOLR, kMasterVolume);
    }
    for (int core = 0; core < kNumCores; ++core) {
        sEffectAttr[core].mode = 0;
        sEffectAttr[core].depth_L = 0;
        sEffectAttr[core].depth_R = 0;
        sEffectAttr[core].delay = 0;
        sEffectAttr[core].feedback = 0;
    }
    sEffectsDirty = 1;
    sEffectsApplying = 1;
    ApplyEffects();
    for (int core = 0; core < kNumCores; ++core) {
        sVoiceMixWetRight[core] = 0;
        sVoiceMixWetLeft[core] = 0;
        sVoiceMixDryRight[core] = 0;
        sVoiceMixDryLeft[core] = 0;
    }
    sVoiceMixDirty = 1;
    CommitVoiceMix();
    for (int core = 0; core < kNumCores; ++core) {
        sVoicesInUse[core] = 0;
    }
    sceSdSetCoreAttr(SD_C_SPDIF_MODE, 0);
}

void Synth::RetireVoice(const Voice *voice) {
    int slot = -1;
    if (sRetiredVoices[0].mState == kVoiceFree) {
        slot = 0;
    } else {
        for (int i = 1; i < kMaxRetiredVoices; ++i) {
            if (sRetiredVoices[i].mState == kVoiceFree) {
                slot = i;
                break;
            }
        }
    }
    // Yes, with every record in use retail writes slot -1, before the start of the list.
    sRetiredVoices[slot] = *voice;
    sRetiredVoices[slot].mSampleDesc = nullptr;
    ++sRetiredVoiceCount;
}

void Synth::UpdateVibrato() {
    for (int core = 0; core < kNumCores; ++core) {
        for (int index = 0; index < kVoicesPerCore; ++index) {
            const Voice &voice = sVoices[core][index];
            if ((voice.mState < kVoiceKeyOn) || (voice.mState > kVoiceReleased) ||
                (voice.mVibratoPhase < 0)) {
                continue;
            }
            int pitch = voice.mPitch;
            if (voice.mVibratoTable == kVibratoFast) {
                pitch += Vibrato::GetFast(sVibratoTick, voice.mVibratoPhase);
            } else if (voice.mVibratoTable == kVibratoSlow) {
                pitch += Vibrato::GetSlow(sVibratoTick, voice.mVibratoPhase);
            }
            SpuSetParam(VoiceEntry(core, index, SD_VP_PITCH), pitch);
        }
    }
    sVibratoTick = (sVibratoTick == (Vibrato::kCycleTicks - 1)) ? 0 : (sVibratoTick + 1);
}

void Synth::UpdateVoiceStates() {
    unsigned int ended[kNumCores];
    ended[SD_CORE_0] = sceSdGetSwitch(SD_S_ENDX | SD_CORE_0);
    ended[SD_CORE_1] = sceSdGetSwitch(SD_S_ENDX | SD_CORE_1);
    for (int core = 0; core < kNumCores; ++core) {
        for (int index = 0; index < kVoicesPerCore; ++index) {
            Voice &voice = sVoices[core][index];
            if (voice.mState == kVoicePlaying) {
                if (((1u << index) & ended[core]) == 0) {
                    continue;
                }
                const Sample *sample = voice.mSampleDesc->mSample;
                const bool looping =
                    (sample->mLoopStart < sample->mLoopEnd) ? (sample->mLoopEnd != 0) : false;
                if (looping) {
                    continue;
                }
                RetireVoice(&voice);
            } else if (voice.mState == kVoiceReleased) {
                if (sceSdGetParam(VoiceEntry(core, index, SD_VP_ENVX)) != 0) {
                    continue;
                }
            } else {
                continue;
            }
            voice.Init();
            --sVoicesInUse[core];
        }
    }
}

int Synth::CombinePan(unsigned char channelPan, unsigned char programPan, unsigned char samplePan) {
    int pan = channelPan + (programPan - kPanOffset) + samplePan;
    if (pan < 0) {
        pan = 0;
    } else if (pan > kMidiMax) {
        pan = kMidiMax;
    }
    return pan;
}

Voice *Synth::AllocVoice(int *core, int *voice, signed char bus) {
    int steal[kNumCores] = {-1, -1};
    Voice *result = nullptr;
    int target = bus;
    if (target == kAnyCore) {
        target = !(sVoicesInUse[SD_CORE_0] < sVoicesInUse[SD_CORE_1]);
    }
    if (target == SD_CORE_0) {
        const int index = Voice::FindFree(sVoices[SD_CORE_0], kVoicesPerCore, &steal[SD_CORE_0]);
        if (index >= 0) {
            *core = SD_CORE_0;
            *voice = index;
            result = &sVoices[SD_CORE_0][index];
            ++sVoicesInUse[SD_CORE_0];
        }
    }
    if (result != nullptr) {
        return result;
    }
    if (target == SD_CORE_1) {
        const int index = Voice::FindFree(sVoices[SD_CORE_1], kVoicesPerCore, &steal[SD_CORE_1]);
        if (index >= 0) {
            *core = target;
            *voice = index;
            result = &sVoices[SD_CORE_1][index];
            ++sVoicesInUse[SD_CORE_1];
        }
    }
    if (result != nullptr) {
        return result;
    }
    Voice *candidate0 = (steal[SD_CORE_0] >= 0) ? &sVoices[SD_CORE_0][steal[SD_CORE_0]] : nullptr;
    Voice *candidate1 = (steal[SD_CORE_1] >= 0) ? &sVoices[SD_CORE_1][steal[SD_CORE_1]] : nullptr;
    bool useCore1 = true;
    if (candidate0 != nullptr) {
        if (candidate1 == nullptr) {
            useCore1 = false;
        } else if (candidate0->mPriority < candidate1->mPriority) {
            useCore1 = false;
        } else if (candidate1->mPriority < candidate0->mPriority) {
            useCore1 = true;
        } else if (candidate0->mAge < candidate1->mAge) {
            useCore1 = false;
        } else if (candidate1->mAge < candidate0->mAge) {
            useCore1 = true;
        } else {
            return nullptr; // Retail steals neither of two equal candidates.
        }
    }
    if (useCore1) {
        result = candidate1;
        *core = SD_CORE_1;
        *voice = steal[SD_CORE_1];
    } else {
        result = candidate0;
        *core = SD_CORE_0;
        *voice = steal[SD_CORE_0];
    }
    return result;
}

void Synth::ClearKeyMasks() {
    sKeyOnMask[SD_CORE_1] = 0;
    sKeyOnMask[SD_CORE_0] = 0;
    sKeyOffMask[SD_CORE_1] = 0;
    sKeyOffMask[SD_CORE_0] = 0;
}

void Synth::KeyOn(unsigned int core0Mask, unsigned int core1Mask) {
    for (int index = 0; index < kVoicesPerCore; ++index) {
        const unsigned int bit = 1u << index;
        if ((core0Mask & bit) != 0) {
            sVoices[SD_CORE_0][index].mState = kVoicePlaying;
        }
        if ((core1Mask & bit) != 0) {
            sVoices[SD_CORE_1][index].mState = kVoicePlaying;
        }
    }
    sceSdSetSwitch(SD_S_KON | SD_CORE_0, core0Mask);
    sceSdSetSwitch(SD_S_KON | SD_CORE_1, core1Mask);
}

void Synth::KeyOff(unsigned int core0Mask, unsigned int core1Mask) {
    sceSdSetSwitch(SD_S_KOFF | SD_CORE_0, core0Mask);
    sceSdSetSwitch(SD_S_KOFF | SD_CORE_1, core1Mask);
}

void Synth::CommitKeys() {
    for (int core = 0; core < kNumCores; ++core) {
        if ((sKeyOffMask[core] & sKeyOnMask[core]) != 0) {
            sKeyOffMask[core] = 0;
        }
    }
    CommitVoiceMix();
    CommitEffects();
    KeyOff(sKeyOffMask[SD_CORE_0], sKeyOffMask[SD_CORE_1]);
    KeyOn(sKeyOnMask[SD_CORE_0], sKeyOnMask[SD_CORE_1]);
}

bool Synth::ChannelHasProgram(unsigned char channel) {
    bool hasProgram = true;
    const Channel &state = sChannels[channel];
    Bank &bank = Bank::sBanks[state.mBank];
    if ((state.mProgram == Channel::kNoProgram) || (bank.GetProgram(state.mProgram) == nullptr)) {
        hasProgram = false;
    }
    return hasProgram;
}

void Synth::NoteOn(unsigned char note, unsigned char velocity, unsigned char channel) {
    if (velocity == 0) {
        NoteOff(note, channel);
        return;
    }
    Channel &state = sChannels[channel];
    if (state.mBank == Channel::kNoBank) {
        return;
    }
    Bank *bank = &Bank::sBanks[state.mBank];
    if (!ChannelHasProgram(channel)) {
        return;
    }
    if (state.mMonophonic != 0) {
        AllNotesOff(channel);
    }
    BankProgram *program = bank->GetProgram(state.mProgram);
    for (int descIndex = 0; descIndex < program->mNumSampleDescs; ++descIndex) {
        BankSampleDesc *sampleDesc = bank->GetSampleDesc(program, descIndex);
        const Sample *sample = sampleDesc->GetSample();
        if ((note < sampleDesc->mLowKeymap) || (sampleDesc->mHighKeymap < note)) {
            continue;
        }
        int layers = 1;
        int layerPan = -1;
        if (state.mStereo != 0) {
            layers = 2;
            layerPan = 0;
        }
        for (int layer = 0; layer < layers; ++layer, ++layerPan) {
            const unsigned int busSelect =
                (state.mBusMode == Channel::kBusModeFromSampleDesc) ? sampleDesc->mBus : state.mBus;
            signed char bus = kAnyCore;
            if (busSelect == kBusEither) {
                bus = kAnyCore;
            } else if (busSelect == kBusCore0) {
                bus = SD_CORE_0;
            } else if (busSelect == kBusCore1) {
                bus = SD_CORE_1;
            }
            int core;
            int index;
            Voice *voice = AllocVoice(&core, &index, bus);
            if (core == SD_CORE_0) {
                sKeyOnMask[SD_CORE_0] |= 1u << index;
            } else if (core == SD_CORE_1) {
                sKeyOnMask[SD_CORE_1] |= 1u << index;
            }
            SetVoiceBus(core, index, sampleDesc, &state);
            voice->mChannel = channel;
            voice->mNote = note;
            voice->mBank = state.mBank;
            voice->mAge = sNoteSerial;
            voice->mState = kVoiceKeyOn;
            voice->mPriority = state.mPriority;
            voice->mSampleDesc = sampleDesc;
            voice->mVelocity = velocity;

            int pan = 0;
            if (sMono != 0) {
                pan = CombinePan(kMidiCenter, kMidiCenter, kMidiCenter);
            } else if (layerPan == -1) {
                pan = CombinePan(state.mPan, program->mPan, sampleDesc->mPan);
            } else if (layerPan == 0) {
                pan = CombinePan(0, 0, 0);
            } else if (layerPan == 1) {
                pan = CombinePan(kMidiMax, kMidiMax, kMidiMax);
            }
            const signed char surround =
                (sSurroundDisabled != 0) ? 0 : (sampleDesc->mSurround != 0);
            unsigned int left;
            unsigned int right;
            Voice::CalculateVolume(velocity,
                                   bank->mHeader.mVolume,
                                   program->mVolume,
                                   sampleDesc->mVolume,
                                   state.mVolume,
                                   state.mExpression,
                                   pan,
                                   &left,
                                   &right,
                                   surround);
            SpuSetParam(VoiceEntry(core, index, SD_VP_VOLL), left);
            SpuSetParam(VoiceEntry(core, index, SD_VP_VOLR), right);
            sceSdSetAddr(VoiceEntry(core, index, SD_VA_SSA), bank->mSpuAddress + sample->mOffset);

            const unsigned short attackMode =
                (sampleDesc->mAttackMode != 0) ? kAttackExponential : 0;
            const unsigned short releaseMode =
                (sampleDesc->mReleaseMode != 0) ? kReleaseExponential : 0;
            unsigned int sustainMode = 0xffffffff; // Retail sets every bit for an unknown mode.
            if (sampleDesc->mSusMode <= kSustainExponentialDown) {
                sustainMode = kSustainModeBits[sampleDesc->mSusMode];
            }
            const unsigned int adsr1 = attackMode |
                                       ((sampleDesc->mAttackRate & kAttackRateMask) << 8) |
                                       ((sampleDesc->mDecayRate & kDecayRateMask) << 4) |
                                       (sampleDesc->mSusLevel & kSusLevelMask);
            const unsigned int adsr2 = sustainMode | ((sampleDesc->mSusRate & kSusRateMask) << 6) |
                                       releaseMode | (sampleDesc->mReleaseRate & kReleaseRateMask);
            SpuSetParam(VoiceEntry(core, index, SD_VP_ADSR1), adsr1);
            SpuSetParam(VoiceEntry(core, index, SD_VP_ADSR2), adsr2);

            const unsigned int pitch = Voice::CalculatePitch(
                note,
                sampleDesc->mBaseKey,
                state.mPitchBendSemitones + program->mTranspose + sampleDesc->mTranspose +
                    state.mTranspose,
                state.mPitchBendCents + program->mFineTranspose + sampleDesc->mFineTranspose,
                sample->mSampleRate);
            voice->mPitch = pitch;
            int vibrato = 0;
            if (state.mDetune != 0) {
                voice->mVibratoTable = layer;
                if (layer == 0) {
                    voice->mVibratoPhase = 0;
                    vibrato = Vibrato::GetFast(sVibratoTick, 0);
                } else if (voice->mVibratoTable == kVibratoSlow) {
                    voice->mVibratoPhase = kDetunePhase;
                    vibrato = Vibrato::GetSlow(sVibratoTick, kDetunePhase);
                }
            }
            SpuSetParam(VoiceEntry(core, index, SD_VP_PITCH), pitch + vibrato);
        }
    }
    if (++sNoteSerial == kNewestAge) {
        sNoteSerial = 0;
    }
}

void Synth::NoteOff(unsigned char note, unsigned char channel) {
    unsigned int oldest = kNewestAge;
    for (int i = 0; i < kMaxRetiredVoices; ++i) {
        const Voice &retired = sRetiredVoices[i];
        if ((retired.mAge < oldest) && (retired.mState != kVoiceFree) &&
            (retired.mChannel == channel) && (retired.mNote == note)) {
            oldest = retired.mAge;
        }
    }
    for (int core = 0; core < kNumCores; ++core) {
        for (int index = 0; index < kVoicesPerCore; ++index) {
            const Voice &voice = sVoices[core][index];
            if ((voice.mChannel == channel) && (voice.mNote == note) && (voice.mAge < oldest) &&
                ((voice.mState == kVoiceKeyOn) || (voice.mState == kVoicePlaying))) {
                oldest = voice.mAge;
            }
        }
    }
    if (oldest > kOldestValidAge) {
        return;
    }
    for (int core = 0; core < kNumCores; ++core) {
        for (int index = 0; index < kVoicesPerCore; ++index) {
            Voice &voice = sVoices[core][index];
            if (voice.mAge != oldest) {
                continue;
            }
            if (voice.mState == kVoiceKeyOn) {
                voice.Init();
                sKeyOnMask[core] &= ~(1u << index);
                --sVoicesInUse[core];
            } else if ((voice.mState == kVoicePlaying) && (voice.mChannel == channel) &&
                       (voice.mNote == note)) {
                sKeyOffMask[core] |= 1u << index;
                voice.mState = kVoiceReleased;
            }
        }
    }
    for (int i = 0; i < kMaxRetiredVoices; ++i) {
        if (sRetiredVoices[i].mAge == oldest) {
            sRetiredVoices[i].Init();
            --sRetiredVoiceCount;
        }
    }
}

void Synth::AllNotesOff(unsigned char channel) {
    for (int core = 0; core < kNumCores; ++core) {
        for (int index = 0; index < kVoicesPerCore; ++index) {
            Voice &voice = sVoices[core][index];
            if (voice.mChannel != channel) {
                continue;
            }
            if (voice.mState == kVoicePlaying) {
                sKeyOffMask[core] |= 1u << index;
                voice.mState = kVoiceReleased;
            } else if (voice.mState == kVoiceKeyOn) {
                sKeyOnMask[core] &= ~(1u << index);
                voice.Init();
                --sVoicesInUse[core];
            }
        }
    }
    // Yes, retail forgets the retired voices of every channel.
    for (int i = 0; i < kMaxRetiredVoices; ++i) {
        sRetiredVoices[i].Init();
        sRetiredVoiceCount = 0;
    }
}

void Synth::SelectProgram(unsigned short channel, unsigned short program) {
    Channel &state = sChannels[channel];
    if (state.mBank == Channel::kNoBank) {
        return;
    }
    Bank &bank = Bank::sBanks[state.mBank];
    if (bank.mLoaded != 1) {
        return;
    }
    bool found = false;
    for (int i = 0; i < bank.mHeader.mNumPrograms; ++i) {
        if (bank.mPrograms[i]->mProgram == program) {
            found = true;
            state.mProgram = i;
            break;
        }
    }
    if (!found) {
        state.mProgram = Channel::kNoProgram;
    }
}

void Synth::ProgramChange(unsigned char channel, unsigned short program) {
    const Channel &state = sChannels[channel];
    if ((state.mBank >= 0) && (state.mProgram >= 0) &&
        (Bank::sBanks[state.mBank].mPrograms[state.mProgram]->mProgram == program)) {
        return;
    }
    SelectProgram(channel, program);
}

void Synth::BankSelect(unsigned char channel, unsigned short bank) {
    Channel &state = sChannels[channel];
    const int previous = state.mBank;
    if ((previous >= 0) && (Bank::sBanks[previous].mHeader.mId == bank)) {
        return;
    }
    const int found = Bank::Find(bank);
    if (found < 0) {
        return;
    }
    state.mBank = found;
    // Yes, retail looks the program up in the bank the channel left.
    if ((previous < 0) || (state.mProgram < 0)) {
        return;
    }
    SelectProgram(channel, Bank::sBanks[previous].GetProgram(state.mProgram)->mProgram);
}

void Synth::UpdateChannelVolume(unsigned char channel) {
    for (int core = 0; core < kNumCores; ++core) {
        for (int index = 0; index < kVoicesPerCore; ++index) {
            Voice &voice = sVoices[core][index];
            if ((voice.mChannel != channel) || (voice.mState < kVoiceKeyOn) ||
                (voice.mState > kVoiceReleased)) {
                continue;
            }
            if (sChannels[voice.mChannel].mBank == Channel::kNoBank) {
                voice.Dump();
            }
            unsigned int left;
            unsigned int right;
            voice.ComputeVolume(&left, &right);
            SpuSetParam(VoiceEntry(core, index, SD_VP_VOLL), left);
            SpuSetParam(VoiceEntry(core, index, SD_VP_VOLR), right);
        }
    }
}

void Synth::SetPan(unsigned char channel, unsigned char pan) {
    Channel &state = sChannels[channel];
    if (state.mPan == pan) {
        return;
    }
    state.mPan = pan;
    if (state.mControllerEnable[kEnablePan] >= kControllerOn) {
        UpdateChannelVolume(channel);
    }
}

void Synth::SetVolume(unsigned char channel, unsigned char volume) {
    Channel &state = sChannels[channel];
    if (state.mVolume == volume) {
        return;
    }
    state.mVolume = volume;
    if (state.mControllerEnable[kEnableVolume] >= kControllerOn) {
        UpdateChannelVolume(channel);
    }
}

void Synth::SetExpression(unsigned char channel, unsigned char expression) {
    Channel &state = sChannels[channel];
    if (state.mExpression == expression) {
        return;
    }
    state.mExpression = expression;
    if (state.mControllerEnable[kEnableExpression] >= kControllerOn) {
        UpdateChannelVolume(channel);
    }
}

void Synth::SetPitchBend(unsigned char channel, signed char semitones, signed char cents) {
    Channel &state = sChannels[channel];
    if ((state.mPitchBendSemitones == semitones) && (state.mPitchBendCents == cents)) {
        return;
    }
    state.mPitchBendSemitones = semitones;
    state.mPitchBendCents = cents;
    if (state.mControllerEnable[kEnablePitchBend] < kControllerOn) {
        return;
    }
    for (int core = 0; core < kNumCores; ++core) {
        for (int index = 0; index < kVoicesPerCore; ++index) {
            const Voice &voice = sVoices[core][index];
            if ((voice.mChannel == channel) && (voice.mState >= kVoiceKeyOn) &&
                (voice.mState <= kVoiceReleased)) {
                SpuSetParam(VoiceEntry(core, index, SD_VP_PITCH), voice.ComputePitch());
            }
        }
    }
}

void Synth::SetTranspose(unsigned char channel, signed char semitones) {
    Channel &state = sChannels[channel];
    state.SetTranspose(semitones);
    if (state.mControllerEnable[kEnableTranspose] < kControllerOn) {
        return;
    }
    for (int core = 0; core < kNumCores; ++core) {
        for (int index = 0; index < kVoicesPerCore; ++index) {
            const Voice &voice = sVoices[core][index];
            if ((voice.mChannel == channel) && (voice.mState >= kVoiceKeyOn) &&
                (voice.mState <= kVoiceReleased)) {
                SpuSetParam(VoiceEntry(core, index, SD_VP_PITCH), voice.ComputePitch());
            }
        }
    }
}

void Synth::SetPriority(unsigned char channel, unsigned char priority) {
    sChannels[channel].mPriority = priority;
}

void Synth::SetDetune(unsigned char channel, unsigned char detune) {
    sChannels[channel].mDetune = detune;
}

void Synth::SetStereo(unsigned char channel, unsigned char stereo) {
    sChannels[channel].mStereo = stereo;
}

void Synth::SetMonophonic(unsigned char channel, unsigned char monophonic) {
    sChannels[channel].mMonophonic = monophonic;
}

void Synth::SetControllerEnable(unsigned char channel,
                                unsigned char controller,
                                unsigned char value) {
    int index = -1;
    switch (controller) {
    case kControllerPan:
        index = kEnablePan;
        break;
    case kPitchBendController:
        index = kEnablePitchBend;
        break;
    case kControllerVolume:
        index = kEnableVolume;
        break;
    case kControllerExpression:
        index = kEnableExpression;
        break;
    case kControllerTranspose:
        index = kEnableTranspose;
        break;
    default:
        break;
    }
    const unsigned char enable = (value >= kControllerOn) ? kUpdateEnabled : 0;
    Channel &state = sChannels[channel];
    if (index < 0) {
        // Retail writes the byte before the switches, the high byte of the monophonic switch.
        state.mMonophonic =
            (state.mMonophonic & 0x00ffffffu) | (static_cast<unsigned int>(enable) << 24);
        return;
    }
    state.mControllerEnable[index] = enable;
}

void Synth::SetBusMode(unsigned char channel, unsigned char busMode) {
    sChannels[channel].mBusMode = busMode;
}

void Synth::SetBus(unsigned char channel, unsigned char bus) {
    sChannels[channel].mBus = bus;
}

void Synth::ResetAllControllers(unsigned char channel) {
    SetPan(channel, kMidiCenter);
    SetVolume(channel, kDefaultVolume);
    SetExpression(channel, kDefaultExpression);
    SetPriority(channel, kDefaultPriority);
    SetBusMode(channel, Channel::kBusModeFromSampleDesc);
    SetBus(channel, kBusEither);
    SetTranspose(channel, 0);
    SetPitchBend(channel, 0, 0);
    SetDetune(channel, 0);
    SetStereo(channel, 0);
    for (int i = 0; i < kNumControllerEnables; ++i) {
        sChannels[channel].mControllerEnable[i] = kUpdateEnabled;
    }
}

int Synth::LoadBankHeader(const unsigned char *data, unsigned short bank) {
    Bank &target = Bank::sBanks[bank];
    target.mHeader.Unpack(data + kRecordPrefixSize);
    (void)Bank::Find(target.mHeader.mId); // Yes, retail discards the lookup.
    return 0;
}

int Synth::LoadPrograms(const unsigned char *data, short size, unsigned short bank) {
    Bank &target = Bank::sBanks[bank];
    int index = 0;
    for (int offset = 0; offset < size; ++index) {
        BankProgram *program;
        Bank::NewProgram(&program);
        offset += kRecordPrefixSize + program->Unpack(data + offset + kRecordPrefixSize);
        target.SetProgram(program, index);
    }
    return 0;
}

int Synth::LoadSampleDescs(const unsigned char *data, short size, unsigned short bank) {
    Bank &target = Bank::sBanks[bank];
    int index = 0;
    for (int offset = 0; offset < size; ++index) {
        BankSampleDesc *sampleDesc;
        Bank::NewSampleDesc(&sampleDesc);
        offset += kRecordPrefixSize + sampleDesc->Unpack(data + offset + kRecordPrefixSize);
        target.SetSampleDesc(sampleDesc, index);
    }
    return 0;
}

int Synth::LoadSamples(const unsigned char *data, short size, unsigned short bank) {
    Bank &target = Bank::sBanks[bank];
    int index = 0;
    Sample *sample = nullptr;
    for (int offset = 0; offset < size; ++index) {
        sample = nullptr;
        Bank::NewSample(&sample);
        offset += kRecordPrefixSize + sample->Unpack(data + offset + kRecordPrefixSize);
        target.SetSample(sample, index);
    }
    return 0;
}

int Synth::TransferDone(int, void *) {
    sTransferBusy = 0;
    sNotifyPending = 1;
    sceSdSetTransIntrHandler(kSpuChannel, nullptr, nullptr);
    return 0;
}

int Synth::EffectsApplied() {
    sTransferBusy = 0;
    if (sEffectNotifyAddress != nullptr) {
        sEffectNotifyPending = 1;
    }
    return 0;
}

int Synth::TransferSampleData(unsigned char *data,
                              int size,
                              unsigned int offset,
                              unsigned short bank) {
    sTransferBusy = 1;
    while (sceSdVoiceTransStatus(kSpuChannel, SD_TRANS_STATUS_CHECK) != kTransferEnded) {
        DelayThread(kRetryMicroseconds);
    }
    if ((sceSdSetTransIntrHandler(kSpuChannel, TransferDone, nullptr) == nullptr) &&
        (sTransferCancel == 0)) {
        do {
            DelayThread(kRetryMicroseconds);
            if (sceSdSetTransIntrHandler(kSpuChannel, TransferDone, nullptr) != nullptr) {
                break;
            }
        } while (sTransferCancel == 0);
    }
    sceSdVoiceTrans(kSpuChannel,
                    SD_TRANS_MODE_WRITE | SD_TRANS_BY_DMA,
                    data,
                    Bank::sBanks[bank].mSpuAddress + offset,
                    size);
    sTransferCancel = 0;
    return 0;
}

int Synth::SetBankSpuLayout(const unsigned char *data) {
    // The sizes are words of an RPC buffer at any alignment.
    struct __attribute__((packed)) BankSize {
        int mSize;
    };
    const auto *sizes = static_cast<const BankSize *>(static_cast<const void *>(data));
    // Retail computes every size twice and discards the first pass.
    unsigned int address = kBankSpuBase;
    for (int i = 0; i < Bank::kMaxBanks; ++i) {
        int size = sizes[i].mSize;
        if (size < 0) {
            size = Bank::sBanks[i].mSpuSize;
        } else {
            size <<= kBankSizeShift;
        }
        Bank::sBanks[i].mSpuAddress = address;
        Bank::sBanks[i].mSpuSize = size;
        address += size;
    }
    return 0;
}

int Synth::SetBankLoaded(unsigned short bank) {
    Bank::sBanks[bank].mLoaded = 1;
    return 0;
}

int Synth::UnloadBank(unsigned short bank) {
    for (int core = 0; core < kNumCores; ++core) {
        for (int index = 0; index < kVoicesPerCore; ++index) {
            Voice &voice = sVoices[core][index];
            if ((voice.mBank != bank) || (voice.mState == kVoiceFree) ||
                (voice.mState > kVoiceReleased)) {
                continue;
            }
            SpuSetParam(VoiceEntry(core, index, SD_VP_VOLL), 0);
            SpuSetParam(VoiceEntry(core, index, SD_VP_VOLR), 0);
            sKeyOnMask[core] &= ~(1u << index);
            voice.Init();
            --sVoicesInUse[core];
        }
    }
    for (int i = 0; i < kNumChannels; ++i) {
        if (sChannels[i].mBank == bank) {
            sChannels[i].mBank = Channel::kNoBank;
            sChannels[i].mProgram = Channel::kNoProgram;
        }
    }
    Bank &target = Bank::sBanks[bank];
    target.mLoaded = 0;
    target.Clear();
    return 0;
}

void Synth::SetVoiceMix(int core, int voice, int mode) {
    if (sEffectsApplying != 0) {
        mode = kMixDry;
    }
    int wet = -1;
    int dry = -1;
    switch (mode) {
    case kMixDry:
        wet = 0;
        dry = 1;
        break;
    case kMixWet:
        wet = 1;
        dry = 0;
        break;
    case kMixDryAndWet:
        wet = 1;
        dry = 1;
        break;
    case kMixNone:
        wet = 0;
        dry = 0;
        break;
    default:
        break;
    }
    const unsigned int bit = 1u << voice;
    if (dry != 0) {
        sVoiceMixDryLeft[core] |= bit;
        sVoiceMixDryRight[core] |= bit;
    } else {
        sVoiceMixDryLeft[core] &= ~bit;
        sVoiceMixDryRight[core] &= ~bit;
    }
    if (wet != 0) {
        sVoiceMixWetLeft[core] |= bit;
        sVoiceMixWetRight[core] |= bit;
    } else {
        sVoiceMixWetLeft[core] &= ~bit;
        sVoiceMixWetRight[core] &= ~bit;
    }
    sVoiceMixDirty = 1;
}

void Synth::CommitVoiceMix() {
    if (sVoiceMixDirty == 0) {
        return;
    }
    for (int core = 0; core < kNumCores; ++core) {
        sceSdSetSwitch(core | SD_S_VMIXL, sVoiceMixDryLeft[core]);
        sceSdSetSwitch(core | SD_S_VMIXR, sVoiceMixDryRight[core]);
        sceSdSetSwitch(core | SD_S_VMIXEL, sVoiceMixWetLeft[core]);
        sceSdSetSwitch(core | SD_S_VMIXER, sVoiceMixWetRight[core]);
    }
    sVoiceMixDirty = 0;
}

void Synth::CommitEffects() {
    if ((sEffectsDirty != 0) && (sEffectsApplying == 0)) {
        sEffectsApplying = 1;
        WakeupThread(sEffectThreadId);
    }
}

void Synth::ApplyEffects() {
    sTransferBusy = 1;
    for (int core = 0; core < kNumCores; ++core) {
        while (sceSdSetEffectAttr(core, &sEffectAttr[core]) != 0) {
            DelayThread(kRetryMicroseconds);
        }
        while (sceSdClearEffectWorkArea(core, kSpuChannel, sEffectAttr[core].mode) != 0) {
            DelayThread(kRetryMicroseconds);
        }
    }
    sEffectsDirty = 0;
    EffectsApplied();
    sEffectsApplying = 0;
}

void Synth::SetVoiceBus(int core,
                        int voice,
                        const BankSampleDesc *sampleDesc,
                        const Channel *channel) {
    int mode = channel->mBusMode;
    if (mode == Channel::kBusModeFromSampleDesc) {
        mode = sampleDesc->mBusMode;
    }
    SetVoiceMix(core, voice, mode);
}

void Synth::SetEffectParam(unsigned char channel, unsigned char value, unsigned char param) {
    if ((channel == 0) || ((channel - 1) >= kNumCores)) {
        return;
    }
    const int core = (channel != 1);
    sceSdEffectAttr &attr = sEffectAttr[core];
    switch (param) {
    case kEffectParamMode:
        attr.mode = value | kEffectModeController;
        break;
    case kEffectParamDepthLeft:
        attr.depth_L = kEffectDepthTable[value];
        SpuSetParam(core | SD_P_EVOLL, kEffectDepthTable[value]);
        return;
    case kEffectParamDepthRight:
        attr.depth_R = kEffectDepthTable[value];
        SpuSetParam(core | SD_P_EVOLR, kEffectDepthTable[value]);
        return;
    case kEffectParamDelay:
        attr.delay = value;
        break;
    case kEffectParamFeedback:
        attr.feedback = value;
        break;
    default:
        return;
    }
    sEffectsDirty = 1;
}
