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

void Synth::VirtualSlot11() {
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

void Synth::VirtualSlot23() {
}

int Synth::VirtualSlot24() {
    return 0;
}

void Synth::VirtualSlot25() {
}

void Synth::VirtualSlot26() {
}

void Synth::VirtualSlot27() {
}

void Synth::SetOutputLevel([[maybe_unused]] float fLevel) {
}
