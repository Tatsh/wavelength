#include "synth_s/ticktimer.h"

#include <kernel.h>

#include "synth_s/ioputil.h"
#include "synth_s/synthlock.h"

namespace {

constexpr unsigned int kTickMicroseconds = 6000;
constexpr int kTickThreadPriority = 12;

} // namespace

// NTSC-U/C: 0x00025680
TickTimer TickTimer::sTimer;
// NTSC-U/C: 0x0002568c
int TickTimer::sThreadId;

unsigned int TickTimer::Handler(void *timer) {
    TickTimer *tickTimer = static_cast<TickTimer *>(timer);
    iWakeupThread(tickTimer->mThreadId);
    return tickTimer->mCompare;
}

int TickTimer::Init() {
    SysClock clock;
    USec2SysClock(kTickMicroseconds, &clock);
    mCompare = clock.low;
    mTimerId = AllocHardTimer(TC_SYSCLOCK, TIMER_SIZE_32, TIMER_PRESCALE_1);
    SetTimerHandler(mTimerId, mCompare, Handler, this);
    SetupHardTimer(mTimerId, TC_SYSCLOCK, TM_NO_GATE, TIMER_PRESCALE_1);
    return 0;
}

int TickTimer::Start() {
    StartHardTimer(mTimerId);
    return 0;
}

int TickTimer::Free() {
    FreeHardTimer(mTimerId);
    return 0;
}

int TickTimer::Stop() {
    StopHardTimer(mTimerId);
    return 0;
}

void TickTimer::Begin(void (*entry)()) {
    const int thread = CreateSynthThread(entry, kTickThreadPriority);
    sThreadId = thread;
    sTimer.mThreadId = thread;
    StartThread(thread, 0);
    sTimer.Init();
    sTimer.Start();
}

void TickTimer::End() {
    sTimer.Stop();
    sTimer.Free();
    SynthLock::Destroy();
}
