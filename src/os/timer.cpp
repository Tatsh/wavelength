#include "os/timer.h"

#include <cstring>
#include <vector>

#include "os/system.h"
#include "script/dataarray.h"

namespace {

// NTSC-U/C: 0x0028d3b0, PAL: 0x00296d90 (static initialiser)
// NTSC-U/C: 0x0028d460, PAL: 0x00296e40 (constructor call)
// NTSC-U/C: 0x0028d480, PAL: 0x00296e60 (destructor call)
// NTSC-U/C: 0x00491a00
std::vector<Timer> gTimers;

} // namespace

// NTSC-U/C: 0x00491a10
Timer gSystemTimer;

// NTSC-U/C: 0x003b2260
unsigned long long gSystemCycles;

// NTSC-U/C: 0x003b2258
float gSystemCycles2Ms;

void TimerSleep(int nMs) {
    const float fEnd = SystemMs() + static_cast<float>(nMs);
    while (SystemMs() < fEnd) {
    }
}

Timer *Timer::Find(const char *pszName) {
    for (auto &timer : gTimers) {
        if (strcmp(timer.mName, pszName) == 0) {
            return &timer;
        }
    }
    return nullptr;
}

void TimerInit() {
    gSystemCycles2Ms = 1.0F / static_cast<float>(kCyclesPerMillisecond);
    gSystemTimer.Stop();
    gSystemTimer.Reset();
    gSystemTimer.Start();
    gSystemCycles = 0;
}

void TimerLoadNames() {
    const DataArray *pConfig = SystemConfig()->FindArray("timer", true);
    gTimers.resize(pConfig->Size() - 1, Timer());
    for (int i = 1; i < pConfig->Size(); ++i) {
        gTimers[i - 1].mName = pConfig->Sym(i);
    }
}

void TimerTerminate() {
}
