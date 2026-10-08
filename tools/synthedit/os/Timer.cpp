#include "os/Timer.h"

#include <vector>

#include <windows.h>

#include "os/System.h"
#include "utl/Data.h"

namespace {

const int kNumPads = 4;
const DWORD kCalibrationMs = 100;
const float kMsPerSecond = 1000.0f;
const float kSecondsPerMs = 0.001f;

// 0x10040c7c
bool gTimerFlag0;

// 0x10040c7d
bool gTimerFlag1;

// 0x10038b24
bool gTimerFlag2 = true;

// 0x10040c88
std::vector<Timer> gTimers;

// 0x10040ca0
float gFrameMs;

} // namespace

unsigned int gLastCycleCount;
unsigned int gDeltaCycles;
int gCalibrateCount;
float gMsPerCycle;
unsigned __int64 gTotalCycles;
float gSystemTime;

int ReturnZero() {
    return 0;
}

void TimerPoll() {
    ReturnZero();
    for (int i = 0; i < kNumPads; ++i) {
        ReturnZero(); // The binary passes the pad index, which the merged routine ignores.
    }
    gTimerFlag0 = false;
    gTimerFlag1 = false;
    gTimerFlag2 = true;
    gSystemTime = TimerUpdateMs() * kSecondsPerMs;
}

void TimerTerminate() {
    for (int i = 0; i < kNumPads; ++i) {
        ReturnZero(); // The binary passes the pad index, which the merged routine ignores.
    }
}

void TimerInit() {
    LARGE_INTEGER counterStart;
    LARGE_INTEGER counterEnd;
    LARGE_INTEGER frequency;
    QueryPerformanceCounter(&counterStart);
    const unsigned int cyclesStart = ReadTimeStampCounter();
    Sleep(kCalibrationMs);
    const unsigned int cycles = ReadTimeStampCounter() - cyclesStart;
    QueryPerformanceCounter(&counterEnd);
    QueryPerformanceFrequency(&frequency);
    const __int64 counts = counterEnd.QuadPart - counterStart.QuadPart;
    gMsPerCycle = static_cast<float>(counts) * kMsPerSecond /
                  static_cast<float>(frequency.QuadPart) / static_cast<int>(cycles);
    if (--gCalibrateCount == 0) {
        gDeltaCycles += ReadTimeStampCounter() - gLastCycleCount;
    }
    gFrameMs = static_cast<float>(static_cast<__int64>(gDeltaCycles)) * gMsPerCycle;
    gDeltaCycles = 0;
    gCalibrateCount = 1;
    gLastCycleCount = ReadTimeStampCounter();
    gTotalCycles = 0;
}

void TimerConfigInit() {
    DataArray *timers = SystemConfig()->FindArray("timer", true);
    gTimers.resize(timers->Size() - 1);
    for (int i = 1; i < timers->Size(); ++i) {
        gTimers[i - 1].mName = timers->Sym(i);
    }
}
