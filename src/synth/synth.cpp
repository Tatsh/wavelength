#include "synth/synth.h"

#include "os/debug.h"

Synth *TheSynth;

void Synth::Destroy() {
    TheSynth->Terminate();
    delete TheSynth;
    TheSynth = nullptr;
}

int Synth::Unsupported() {
    DebugWarn("talk to denny");
    return 0;
}

void Synth::VirtualSlot3() {
}

Synth::~Synth() {
}

int Synth::VirtualSlot10() {
    return 0;
}

void Synth::SetStream([[maybe_unused]] StreamPlayer *pStream) {
}

void Synth::VirtualSlot12() {
}

int Synth::VirtualSlot13() {
    return 0;
}

int Synth::VirtualSlot18() {
    return 1;
}

void Synth::VirtualSlot21() {
}

void Synth::VirtualSlot22() {
}

void Synth::SetLag([[maybe_unused]] float fLag) {
}

float Synth::GetLag() {
    return 0.0f;
}

void Synth::SetSoftFxSweep([[maybe_unused]] float fPosition) {
}

void Synth::VirtualSlot26([[maybe_unused]] float fValue) {
}

void Synth::SetSoftFxFilter([[maybe_unused]] const SoftFxFilter &filter) {
}

void Synth::SetOutputLevel([[maybe_unused]] float fLevel) {
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
