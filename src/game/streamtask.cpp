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
    (void)TheSynth->VirtualSlot10(); // Yes, the binary discards this call's result.
    const char *pszPath = FormatString(kStreamPathFormat, mDirectory, mName);
    mPlayer = new StreamPlayer(0, pszPath, false);
    TheSynth->SetStream(mPlayer);
    mPlayer->SetPlaying(true);
}

void StreamTask::OnStop() {
    if (mPlayer != nullptr) {
        mPlayer->SetPlaying(false);
    }
    TheSynth->SetStream(nullptr);
    delete mPlayer;
    mPlayer = nullptr;
}

void StreamTask::SetPaused(bool bPaused) {
    mPlayer->SetPlaying(!bPaused);
}

void StreamTask::OnPoll() {
    if (mPlayer->IsPlaying()) {
        return;
    }
    TheSynth->SetStream(nullptr);
    delete mPlayer;
    mPlayer = nullptr;
    Finish(true);
    if (mWaitingTask != nullptr) {
        TaskDoneNotifier::ReportDone(mWaitingTask);
    }
    mWaitingTask = nullptr;
}
