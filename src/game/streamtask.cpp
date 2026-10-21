#include "game/streamtask.h"

#include "os/string.h"
#include "os/taskdonenotifier.h"
#include "synth/synth.h"

namespace {

constexpr char kStreamPathFormat[] = "%s%s";

} // namespace

StreamTask::StreamTask(const char *pszName, const char *pszDirectory)
    : mWaitingTask(nullptr), mPlayer(nullptr), mName(pszName), mDirectory(pszDirectory) {
}

void StreamTask::SetWaitingTask(Task *pTask) {
    mWaitingTask = pTask;
}

void StreamTask::OnStart() {
#ifndef VIDEO_STANDARD_PAL
    (void)TheSynth->GetStream(); // Yes, the binary discards this call's result.
#endif
    const char *pszPath = FormatString(kStreamPathFormat, mDirectory, mName);
    mPlayer = std::make_unique<StreamPlayer>(0, pszPath, false);
    TheSynth->SetStream(mPlayer.get());
    mPlayer->SetPlaying(true);
}

void StreamTask::OnStop() {
    if (mPlayer != nullptr) {
        mPlayer->SetPlaying(false);
    }
    TheSynth->SetStream(nullptr);
    mPlayer.reset();
}

void StreamTask::SetPaused(bool bPaused) {
    mPlayer->SetPlaying(!bPaused);
}

void StreamTask::OnPoll() {
    if (mPlayer->IsPlaying()) {
        return;
    }
    TheSynth->SetStream(nullptr);
    mPlayer.reset();
    Finish(true);
    if (mWaitingTask != nullptr) {
        TaskDoneNotifier::ReportDone(mWaitingTask);
    }
    mWaitingTask = nullptr;
}
