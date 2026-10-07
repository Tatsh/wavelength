#include "softfx_s/softfx.h"

#include <kernel.h>
#include <libsd.h>
#include <sifrpc.h>
#include <sysclib.h>

#include "runtime/runtime.h"
#include "softfx_s/effects.h"
#include "softfx_s/lock.h"
#include "softfx_s/periodictimer.h"

namespace {

constexpr unsigned short kModuleVersion = 0x0109;
constexpr char kUnloadToken[] = "other";
constexpr char kConcatenationLockName[] = "kSynthCommandConcatenation";

constexpr int kRpcThreadPriority = 11;
constexpr int kProcessingThreadPriority = 10;
constexpr int kThreadStackSize = 0x800;
constexpr int kRpcBufferSize = 0x10000;

// Captured frames cycle through three slots.
constexpr int kFrameCount = 3;
// Interrupts the capture runs for after a stop.
constexpr int kStopInterrupts = 4;
constexpr unsigned short kDefaultInputVolume = 0x3fff;
// Bit of the core 0 interrupt in the cores an SPU2 interrupt reports.
constexpr int kCore0Interrupt = 1;
constexpr short kTransferChannel = 0;
constexpr int kSdErrorTransferBusy = -210;
constexpr int kSdErrorInvalidArgument = -100;

// SPU2 addresses of the capture. The interrupt address lies near the end of the half just filled,
// and each transfer reads or writes one CaptureFrame starting at the filled half.
constexpr unsigned int kIrqFirstHalf = 0x29f0;
constexpr unsigned int kIrqSecondHalf = 0x2ff0;
constexpr unsigned int kCaptureFirstHalf = 0x2800;
constexpr unsigned int kCaptureSecondHalf = 0x2a00;
constexpr unsigned int kOutputFirstHalf = 0x4000;
constexpr unsigned int kOutputSecondHalf = 0x4200;

// Core 0 mixer bits that the routing commands switch.
constexpr unsigned short kMixInputMask = 0x00f0;
constexpr unsigned short kMixInputLowPair = 0x0030;
constexpr unsigned short kMixInputHighPair = 0x00c0;
constexpr unsigned short kMixInputBothPairs = 0x00f0;
constexpr unsigned short kMixVoiceBits = 0x0300;

// Values of the kSoftFxCommandRouteInput argument.
enum InputRoute {
    kInputRouteLowPair = 0,
    kInputRouteHighPair = 1,
    kInputRouteBothPairs = 2,
};

constexpr char kVoiceTransErrorMessage[] =
    "************sceSdBlockTrans() error ::SCESD_EINVALID_ARGUMENT.************\n";
constexpr char kCaptureCoreErrorMessage[] =
    "What up? Callback but no valid core? gActiveCores = %i.\n";

// NTSC-U/C: 0x00003b68
int g_nRpcThread = KE_ERROR;

// NTSC-U/C: 0x00003b70
volatile int g_nProcessDepth = 0;

// NTSC-U/C: 0x00003b74
volatile int g_nCaptureHalf = 0;

// NTSC-U/C: 0x00003b78
volatile bool g_bTransferIdle = true;

// NTSC-U/C: 0x00003b7c
volatile int g_nFrameSlot = 0;

// NTSC-U/C: 0x00003b80
volatile int g_nProcessingThread = KE_ERROR;

// NTSC-U/C: 0x00003b88
bool g_bCapturing = false;

// NTSC-U/C: 0x00003b8c
int g_nStopCountdown = kStopInterrupts;

// NTSC-U/C: 0x00003b90
alignas(16) CaptureFrame g_frames[kFrameCount];

// NTSC-U/C: 0x000065c0
unsigned char g_initOption;

// NTSC-U/C: 0x00006830
sceSifQueueData g_rpcQueue;

// NTSC-U/C: 0x00006848
sceSifServeData g_rpcServer;

// NTSC-U/C: 0x00006890
unsigned char g_rpcBuffer[kRpcBufferSize];

// NTSC-U/C: 0x00003460, PAL: 0x00003460
void SetSdParam(unsigned short entry, unsigned short value) {
    sceSdSetParam(entry, value);
}

// NTSC-U/C: 0x00000c90, PAL: 0x00000c90
int WriteDoneHandler([[maybe_unused]] int channel, [[maybe_unused]] void *data) {
    g_bTransferIdle = true;
    g_nFrameSlot = g_nFrameSlot - 1;
    if (g_nFrameSlot < 0) {
        g_nFrameSlot = g_nFrameSlot + kFrameCount;
    }
    return 0;
}

// NTSC-U/C: 0x00000860, PAL: 0x00000860
void StartWriteBack() {
    int slot = g_nFrameSlot - 1;
    if (slot < 0) {
        slot += kFrameCount;
    }
    sceSdSetTransIntrHandler(kTransferChannel, WriteDoneHandler, nullptr);
    sceSdVoiceTrans(kTransferChannel,
                    SD_TRANS_MODE_WRITE,
                    &g_frames[slot],
                    g_nCaptureHalf != 0 ? kOutputSecondHalf : kOutputFirstHalf,
                    sizeof(CaptureFrame));
}

// NTSC-U/C: 0x00000c6c, PAL: 0x00000c6c
int ReadDoneHandler([[maybe_unused]] int channel, [[maybe_unused]] void *data) {
    StartWriteBack();
    return 0;
}

// NTSC-U/C: 0x0000064c, PAL: 0x0000064c
int CaptureInterruptHandler(int activeCores, [[maybe_unused]] void *data) {
    if (!g_bCapturing) {
        if (g_nStopCountdown == 0) {
            return 0;
        }
        --g_nStopCountdown;
    }
    bool toggleHalf = true;
    if (g_bTransferIdle) {
        if (activeCores != kCore0Interrupt) {
            Kprintf(kCaptureCoreErrorMessage, activeCores);
            toggleHalf = false;
        } else if (static_cast<int>(sceSdVoiceTransStatus(kTransferChannel,
                                                          SD_TRANS_STATUS_CHECK)) == activeCores) {
            sceSdSetTransIntrHandler(kTransferChannel, ReadDoneHandler, nullptr);
            g_nCaptureHalf = sceSdGetAddr(SD_CORE_0 | SD_A_IRQA) != kIrqFirstHalf;
            const int result =
                sceSdVoiceTrans(kTransferChannel,
                                SD_TRANS_MODE_READ,
                                &g_frames[g_nFrameSlot],
                                g_nCaptureHalf != 0 ? kCaptureSecondHalf : kCaptureFirstHalf,
                                sizeof(CaptureFrame));
            if (result != kSdErrorTransferBusy && g_nProcessingThread > 0) {
                toggleHalf = false;
                if (result == kSdErrorInvalidArgument) {
                    Kprintf(kVoiceTransErrorMessage);
                } else {
                    g_bTransferIdle = false;
                    if (iWakeupThread(g_nProcessingThread) != KE_OK) {
                        return 0; // The binary leaves the interrupt disarmed here.
                    }
                }
            }
        }
    }
    if (toggleHalf) {
        g_nCaptureHalf = g_nCaptureHalf ^ 1;
    }
    sceSdSetCoreAttr(SD_CORE_0 | SD_C_IRQ_ENABLE, 0);
    sceSdSetSpu2IntrHandler(CaptureInterruptHandler, nullptr);
    sceSdSetAddr(SD_CORE_0 | SD_A_IRQA, g_nCaptureHalf != 0 ? kIrqFirstHalf : kIrqSecondHalf);
    sceSdSetCoreAttr(SD_CORE_0 | SD_C_IRQ_ENABLE, 1);
    return 0;
}

// NTSC-U/C: 0x000008f0, PAL: 0x000008f0
void ProcessFrame() {
    g_nProcessDepth = g_nProcessDepth + 1;
    int current = g_nFrameSlot - 2;
    if (current < 0) {
        current += kFrameCount;
    }
    int previous = g_nFrameSlot - 1;
    if (previous < 0) {
        previous += kFrameCount;
    }
    if (g_nCaptureHalf == 0) {
        g_frames[current].mOverlap = g_frames[previous].mRight;
    } else if (g_nCaptureHalf == 1) {
        g_frames[current].mOverlap = g_frames[previous].mLeft;
    }
    SampleBlock *left = &g_frames[current].mLeft;
    SampleBlock *right = &g_frames[current].mRight;
    if (!g_bCapturing) {
        EffectSilence(left, right, left, right);
    } else {
        switch (g_nEffect) {
        case kEffectSilence:
            EffectSilence(left, right, left, right);
            break;
        case kEffectStream:
            EffectStream(left, right, left, right);
            break;
        case kEffectSweptFilter:
        case kEffectFilter:
        case kEffectFilterVariant:
            EffectResonantFilter(left, right, left, right);
            break;
        case kEffectStutter:
            EffectStutter(left, right, left, right);
            break;
        case kEffectBypass:
            EffectBypass(left, right, left, right);
            break;
        case kEffectEcho:
            EffectEcho(left, right, left, right);
            break;
        case kEffectTrackingEcho:
            EffectTrackingEcho(left, right, left, right);
            break;
        case kEffectDistortedFilter:
            EffectDistortedFilter(left, right, left, right);
            break;
        default:
            break;
        }
    }
    g_nProcessDepth = g_nProcessDepth - 1;
}

// NTSC-U/C: 0x00000c14, PAL: 0x00000c14
void DataProcessingThreadHandler() {
    g_nProcessingThread = GetThreadId();
    for (;;) {
        SleepThread();
        AcquireLock(__func__);
        ProcessFrame();
        ReleaseLock(__func__);
    }
}

// NTSC-U/C: 0x00000480, PAL: 0x00000480
int StartCapture() {
    if (!g_bCapturing) {
        g_nCaptureHalf = 0;
        g_nFrameSlot = 0;
        sceSdSetCoreAttr(SD_CORE_0 | SD_C_IRQ_ENABLE, 0);
        sceSdSetSpu2IntrHandler(CaptureInterruptHandler, nullptr);
        sceSdSetAddr(SD_CORE_0 | SD_A_IRQA, g_nCaptureHalf != 0 ? kIrqSecondHalf : kIrqFirstHalf);
        sceSdSetCoreAttr(SD_CORE_0 | SD_C_IRQ_ENABLE, 1);
        memset(g_frames, 0, sizeof(g_frames));
        g_bCapturing = true;
    }
    return 0;
}

// NTSC-U/C: 0x0000054c, PAL: 0x0000054c
int StopCapture() {
    if (g_bCapturing) {
        g_bCapturing = false;
        g_nStopCountdown = kStopInterrupts;
    }
    return 0;
}

// NTSC-U/C: 0x00000580, PAL: 0x00000580
int SetEffect(int effect) {
    g_nEffect = effect;
    return 0;
}

// NTSC-U/C: 0x00000590, PAL: 0x00000590
void SetInputVolume(unsigned short volume) {
    SetSdParam(SD_CORE_0 | SD_P_BVOLL, volume);
    SetSdParam(SD_CORE_0 | SD_P_BVOLR, volume);
}

// NTSC-U/C: 0x000005c8, PAL: 0x000005c8
int Initialize(const InitArgs *args) {
    sifcmd_4();
    g_initOption = args->mOption;
    StartThread(CreateThreadWithPriority(DataProcessingThreadHandler, kProcessingThreadPriority),
                0);
    SetInputVolume(kDefaultInputVolume);
    SetStreamStatusAddress(args->mStreamStatus);
    g_nEffect = kEffectSilence;
    StartCapture();
    StopCapture();
    return 0;
}

// NTSC-U/C: 0x00000644, PAL: 0x00000644
int Shutdown() {
    return 0;
}

// NTSC-U/C: 0x000001f4, PAL: 0x000001f4
void *RpcHandler(unsigned int command, void *data, int size) {
    switch (command) {
    case kSoftFxCommandInit: {
        InitArgs args;
        memcpy(&args, data, sizeof(args));
        Initialize(&args);
        break;
    }
    case kSoftFxCommandShutdown:
        Shutdown();
        break;
    case kSoftFxCommandStart:
        StartCapture();
        break;
    case kSoftFxCommandStop:
        StopCapture();
        break;
    case kSoftFxCommandStreamData:
        StreamData(data, size);
        break;
    case kSoftFxCommandSetEffect: {
        int effect;
        memcpy(&effect, data, sizeof(effect));
        SetEffect(effect);
        break;
    }
    case kSoftFxCommandSetParams: {
        int state;
        CpuSuspendIntr(&state);
        SetEffectParams(static_cast<const EffectParams *>(data));
        CpuResumeIntr(state);
        break;
    }
    case kSoftFxCommandSetVolume: {
        int volume;
        memcpy(&volume, data, sizeof(volume));
        SetInputVolume(static_cast<unsigned short>(volume));
        break;
    }
    case kSoftFxCommandRouteInput: {
        int route;
        memcpy(&route, data, sizeof(route));
        auto mix =
            static_cast<unsigned short>(sceSdGetParam(SD_CORE_0 | SD_P_MMIX) & ~kMixInputMask);
        if (route == kInputRouteHighPair) {
            mix |= kMixInputHighPair;
        } else if (route == kInputRouteLowPair) {
            mix |= kMixInputLowPair;
        } else if (route == kInputRouteBothPairs) {
            mix |= kMixInputBothPairs;
        }
        SetSdParam(SD_CORE_0 | SD_P_MMIX, mix);
        break;
    }
    case kSoftFxCommandRouteVoices: {
        int enable;
        memcpy(&enable, data, sizeof(enable));
        auto mix =
            static_cast<unsigned short>(sceSdGetParam(SD_CORE_0 | SD_P_MMIX) & ~kMixVoiceBits);
        if (enable != 0) {
            mix |= kMixVoiceBits;
        }
        SetSdParam(SD_CORE_0 | SD_P_MMIX, mix);
        break;
    }
    case kSoftFxCommandSetMonoStream: {
        int mono;
        memcpy(&mono, data, sizeof(mono));
        g_bMonoStream = mono != 0;
        break;
    }
    case kSoftFxCommandConcatenation: {
        AcquireLock(kConcatenationLockName);
        auto *bytes = static_cast<unsigned char *>(data);
        for (int offset = 0; offset < size;) {
            CommandHeader header;
            memcpy(&header, bytes + offset, sizeof(header));
            RpcHandler(header.mCommand, bytes + offset + sizeof(header), header.mSize);
            offset += sizeof(header) + header.mSize;
        }
        ReleaseLock(kConcatenationLockName);
        break;
    }
    default:
        break;
    }
    return nullptr; // Every command returns zero, so the reply is always empty.
}

// NTSC-U/C: 0x00000148, PAL: 0x00000148
void RpcServerThread() {
    sceSifSetRpcQueue(&g_rpcQueue, GetThreadId());
    sceSifRegisterRpc(
        &g_rpcServer, kSoftFxRpcServer, RpcHandler, g_rpcBuffer, nullptr, nullptr, &g_rpcQueue);
    sceSifRpcLoop(&g_rpcQueue);
}

// NTSC-U/C: 0x000001b4, PAL: 0x000001b4
[[maybe_unused]] void RemoveRpcServer() {
    if (sceSifRemoveRpc(&g_rpcServer, &g_rpcQueue) != nullptr) {
        sceSifRemoveRpcQueue(&g_rpcQueue);
    }
}

// NTSC-U/C: 0x00000000, PAL: 0x00000000
int ModuleStart() {
    RunGlobalConstructors();
    EnableIntr(INUM_DMA_4);
    EnableIntr(INUM_DMA_7);
    sceSifInitRpc(0);
    CreateLock();
    InitEffects();
    ThreadParam param;
    param.attr = TH_C;
    param.entry = RpcServerThread;
    param.initPriority = kRpcThreadPriority;
    param.option = 0;
    param.stackSize = kThreadStackSize;
    g_nRpcThread = CreateThread(&param);
    if (g_nRpcThread <= 0) {
        return NO_RESIDENT_END;
    }
    StartThread(g_nRpcThread, 0);
    return REMOVABLE_RESIDENT_END;
}

// NTSC-U/C: 0x000000a0, PAL: 0x000000a0
int ModuleStop([[maybe_unused]] int argc, char **argv) {
    RunGlobalDestructors();
    // The binary reads argv from a register the destructors are free to overwrite.
    if (strcmp(argv[0], kUnloadToken) != 0) {
        return NO_RESIDENT_END;
    }
    if (TerminateThread(g_nRpcThread) != KE_OK) {
        return REMOVABLE_RESIDENT_END;
    }
    if (DeleteThread(g_nRpcThread) == KE_OK) {
        return NO_RESIDENT_END;
    }
    return REMOVABLE_RESIDENT_END;
}

} // namespace

// NTSC-U/C: 0x00003b60
ModuleInfo Module = {"softFX", kModuleVersion};

// NTSC-U/C: 0x00003b84
int g_nEffect = kEffectSilence;

int start(int argc, char **argv) {
    if (argc < 0) {
        return ModuleStop(-argc, argv);
    }
    return ModuleStart();
}
