#include "synth_s/synthlock.h"

// NTSC-U/C: 0x00015290
int SynthLock::sSemaphore = -1;
// NTSC-U/C: 0x00015294
SemaParam SynthLock::sSemaParam;

void SynthLock::Create() {
    sSemaParam.initCount = 1;
    sSemaParam.maxCount = 1;
    sSemaParam.option = 0;
    sSemaParam.attr = 0;
    sSemaphore = CreateSema(&sSemaParam);
}

void SynthLock::Destroy() {
    DeleteSema(sSemaphore);
}

void SynthLock::Lock(const char *) {
    WaitSema(sSemaphore);
}

void SynthLock::Unlock(const char *) {
    SignalSema(sSemaphore);
}
