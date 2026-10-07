#include "game/waittimetask.h"

#include "os/scheduler.h"

namespace {

constexpr float kMillisecondsPerSecond = 1000.0f;

} // namespace

WaitTimeTask::WaitTimeTask(float fSeconds)
    : mDuration(fSeconds * kMillisecondsPerSecond), mEndTime(0.0f) {
}

void WaitTimeTask::OnStart() {
    mEndTime = TheSongScheduler.mTime + mDuration;
}

void WaitTimeTask::OnPoll() {
    if (mEndTime <= TheSongScheduler.mTime) {
        Finish(true);
    }
}
