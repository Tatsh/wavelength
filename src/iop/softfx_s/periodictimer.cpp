#include "softfx_s/periodictimer.h"

#include <kernel.h>

#include "softfx_s/lock.h"

namespace {

constexpr unsigned int kTickMicroseconds = 6000;
constexpr int kThreadStackSize = 0x800;
constexpr int kTickThreadPriority = 12;

// NTSC-U/C: 0x00016890
PeriodicTimer g_tickTimer;

// NTSC-U/C: 0x0001689c
int g_nTickThread;

} // namespace

unsigned int PeriodicTimer::Handler(void *common) {
    auto *timer = static_cast<PeriodicTimer *>(common);
    iWakeupThread(timer->mThread);
    return timer->mInterval;
}

int CreateThreadWithPriority(void (*entry)(), int priority) {
    ThreadParam param;
    param.attr = TH_C;
    param.entry = entry;
    param.initPriority = priority;
    param.stackSize = kThreadStackSize;
    param.option = 0;
    return CreateThread(&param);
}

int PeriodicTimer::Init() {
    SysClock clock;
    USec2SysClock(kTickMicroseconds, &clock);
    mInterval = clock.low;
    const int timer = AllocHardTimer(TC_SYSCLOCK, TIMER_SIZE_32, TIMER_PRESCALE_1);
    mTimer = timer;
    SetTimerHandler(timer, mInterval, Handler, this);
    SetupHardTimer(timer, TC_SYSCLOCK, TM_NO_GATE, TIMER_PRESCALE_1);
    return 0;
}

int PeriodicTimer::Start() {
    StartHardTimer(mTimer);
    return 0;
}

int PeriodicTimer::Free() {
    FreeHardTimer(mTimer);
    return 0;
}

int PeriodicTimer::Stop() {
    StopHardTimer(mTimer);
    return 0;
}

void StartTickThread(void (*entry)()) {
    const int thread = CreateThreadWithPriority(entry, kTickThreadPriority);
    g_nTickThread = thread;
    g_tickTimer.mThread = thread;
    StartThread(thread, 0);
    g_tickTimer.Init();
    g_tickTimer.Start();
}

void StopTickThread() {
    g_tickTimer.Stop();
    g_tickTimer.Free();
    DeleteLock();
}
