#include "synth/synth.h"

#include "os/debug.h"
#include "os/system.h"
#include "script/dataarray.h"
#include "synth/synthnull.h"
#include "synth/synthps2.h"

std::unique_ptr<Synth> TheSynth;

void Synth::Create() {
    int bUseNullSynth = 0;
    SystemConfig()->FindArray("synth", true)->FindInt("use_null_synth", &bUseNullSynth, false);
    if (bUseNullSynth) {
        TheSynth = std::make_unique<SynthNull>();
    } else {
        TheSynth = std::make_unique<SynthPS2>();
    }
    (void)TheSynth->Init(); // Yes, the binary discards the status.
}

void Synth::Destroy() {
    (void)TheSynth->Terminate(); // Yes, the binary discards the status.
    TheSynth.reset();
}

int Synth::Unsupported() {
#ifndef VIDEO_STANDARD_PAL
    DebugWarn("talk to denny");
#endif
    return 0;
}

void Synth::ResetSpu() {
}

Synth::~Synth() {
}

StreamPlayer *Synth::GetStream() {
    return nullptr;
}

void Synth::SetStream([[maybe_unused]] StreamPlayer *pStream) {
}

void Synth::SendStreamData([[maybe_unused]] const void *pData, [[maybe_unused]] int nSize) {
}

int Synth::GetStreamStatus() {
    return 0;
}

int Synth::IsBankPresent([[maybe_unused]] unsigned short nSlot) {
    return 1;
}

void Synth::SetSurround([[maybe_unused]] bool bSurround) {
}

void Synth::SetMono([[maybe_unused]] bool bMono) {
}

void Synth::SetLag([[maybe_unused]] float fLag) {
}

float Synth::GetLag() {
    return 0.0f;
}

void Synth::SetSoftFxSweep([[maybe_unused]] float fPosition) {
}

void Synth::SetSoftFxModulation([[maybe_unused]] float fValue) {
}

void Synth::SetSoftFxFilter([[maybe_unused]] const SoftFxFilter &filter) {
}

void Synth::SetOutputLevel([[maybe_unused]] float fLevel) {
}

float Synth::GetOutputLevel() {
    return 1.0f;
}

void Synth::SetSoftFxInput([[maybe_unused]] int nInput) {
}

void Synth::SetBusToCoreFx([[maybe_unused]] int nOn) {
}

void Synth::SetBusToSoftFx([[maybe_unused]] int nOn) {
}

void Synth::VirtualSlot35() {
}

void Synth::SendMessages([[maybe_unused]] const std::vector<unsigned int> &messages) {
}

void Synth::QueueCallback([[maybe_unused]] Callback pfnCallback, [[maybe_unused]] int nArg) {
}
