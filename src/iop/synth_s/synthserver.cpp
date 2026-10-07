#include "synth_s/synthserver.h"

#include <kernel.h>
#include <sysclib.h>

#include "synth_s/bank.h"
#include "synth_s/ioputil.h"
#include "synth_s/midievent.h"
#include "synth_s/synth.h"
#include "synth_s/synthlock.h"
#include "synth_s/ticktimer.h"

namespace {

constexpr int kEffectThreadPriority = 20;
constexpr int kEnabled = 1;

const char kConcatenationLockName[] = "kSynthCommandConcatenation";

} // namespace

// NTSC-U/C: 0x000069fc
int SynthServer::sConcatenating;
// NTSC-U/C: 0x000155f0
sceSifQueueData SynthServer::sQueue;
// NTSC-U/C: 0x00015608
sceSifServeData SynthServer::sServer;
// NTSC-U/C: 0x00015650
alignas(16) unsigned char SynthServer::sBuffer[kBufferSize];

void SynthServer::Thread() {
    sceSifSetRpcQueue(&sQueue, GetThreadId());
    sceSifRegisterRpc(&sServer, kServerId, Dispatch, sBuffer, nullptr, nullptr, &sQueue);
    sceSifRpcLoop(&sQueue);
}

void SynthServer::Remove() {
    if (sceSifRemoveRpc(&sServer, &sQueue) != nullptr) {
        sceSifRemoveRpcQueue(&sQueue);
    }
}

void *SynthServer::Dispatch(unsigned int command, void *buffer, int size) {
    unsigned char *data = static_cast<unsigned char *>(buffer);
    int result = 0;
    switch (command) {
    case kSynthCommandInit: {
        SynthInitCommand init;
        memcpy(&init, data, sizeof init);
        result = Initialize(&init);
        break;
    }
    case kSynthCommandTerminate:
        result = Terminate();
        break;
    case kSynthCommandSetBankLayout:
        result = Synth::SetBankSpuLayout(data);
        break;
    case kSynthCommandSetBankLoaded: {
        unsigned short bank;
        memcpy(&bank, data, sizeof bank);
        result = Synth::SetBankLoaded(bank);
        break;
    }
    case kSynthCommandUnloadBank: {
        unsigned short bank;
        memcpy(&bank, data, sizeof bank);
        result = Synth::UnloadBank(bank);
        break;
    }
    case kSynthCommandTransferSamples: {
        BankDataCommand header;
        memcpy(&header, data, sizeof header);
        result = Synth::TransferSampleData(data + sizeof header,
                                           size - static_cast<int>(sizeof header),
                                           header.mOffset,
                                           header.mBank);
        break;
    }
    case kSynthCommandLoadBankHeader: {
        BankDataCommand header;
        memcpy(&header, data, sizeof header);
        result = Synth::LoadBankHeader(data + sizeof header, header.mBank);
        break;
    }
    case kSynthCommandLoadPrograms: {
        BankDataCommand header;
        memcpy(&header, data, sizeof header);
        result = Synth::LoadPrograms(data + sizeof header, size - sizeof header, header.mBank);
        break;
    }
    case kSynthCommandLoadSampleDescs: {
        BankDataCommand header;
        memcpy(&header, data, sizeof header);
        result = Synth::LoadSampleDescs(data + sizeof header, size - sizeof header, header.mBank);
        break;
    }
    case kSynthCommandLoadSamples: {
        BankDataCommand header;
        memcpy(&header, data, sizeof header);
        result = Synth::LoadSamples(data + sizeof header, size - sizeof header, header.mBank);
        break;
    }
    case kSynthCommandQueueEvents:
        result = Synth::QueueTimedEvents(static_cast<const MidiEvent *>(buffer),
                                         static_cast<unsigned int>(size) / sizeof(MidiEvent));
        break;
    case kSynthCommandMidi:
        Synth::ClearKeyMasks();
        result = Synth::ProcessMidiStream(data, size);
        Synth::CommitKeys();
        break;
    case kSynthCommandConcatenation: {
        sConcatenating = 1;
        SynthLock::Lock(kConcatenationLockName);
        for (int offset = 0; offset < size;) {
            const auto *header =
                static_cast<const SynthCommandHeader *>(static_cast<const void *>(data + offset));
            const int innerSize = header->mSize;
            Dispatch(header->mCommand, data + offset + sizeof(SynthCommandHeader), innerSize);
            offset += sizeof(SynthCommandHeader) + innerSize;
        }
        result = 0;
        SynthLock::Unlock(kConcatenationLockName);
        sConcatenating = 0;
        break;
    }
    case kSynthCommandMono: {
        int mono;
        memcpy(&mono, data, sizeof mono);
        Synth::SetMono(mono == kEnabled);
        break;
    }
    case kSynthCommandSurround: {
        int surround;
        memcpy(&surround, data, sizeof surround);
        Synth::SetSurround(surround == kEnabled);
        break;
    }
    case kSynthCommandResetSpu:
        Synth::ResetSpu();
        break;
    default:
        break;
    }
    // Yes, retail returns the status word as the reply pointer.
    return reinterpret_cast<void *>(result);
}

int SynthServer::Initialize(const SynthInitCommand *command) {
    Synth::sEffectThreadId = CreateSynthThread(Synth::EffectThread, kEffectThreadPriority);
    StartThread(Synth::sEffectThreadId, 0);
    Synth::sNotifyAddress = command->mNotifyAddress;
    Synth::sEffectNotifyAddress = command->mEffectNotifyAddress;
    Synth::sInitFlag = command->mFlag;
    Bank::InitPools();
    Synth::Init();
    TickTimer::Begin(Synth::TickThread);
    return 0;
}

int SynthServer::Terminate() {
    TickTimer::End();
    Bank::ReleasePools();
    return 0;
}
