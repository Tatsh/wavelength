#include "os/serialtasks.h"

namespace {

// The index of mCurrent while no task runs.
constexpr int kNoCurrent = -1;

} // namespace

SerialTasks::~SerialTasks() = default;

void SerialTasks::Add(Task *pTask) {
    int nCurrent = kNoCurrent;
    if (GetState() == kStateRunning) {
        nCurrent = mCurrent - mTasks.begin();
    }
    mTasks.emplace_back(pTask);
    if (nCurrent != kNoCurrent) {
        mCurrent = mTasks.begin() + nCurrent;
    }
}

void SerialTasks::OnStart() {
    mCurrent = mTasks.begin();
    if (mTasks.size() != 0) {
        (*mCurrent)->Start();
    } else {
        Finish(true);
    }
}

void SerialTasks::OnStop() {
    if (mTasks.size() != 0 && mCurrent != mTasks.end()) {
        (*mCurrent)->Stop();
    }
}

void SerialTasks::OnPoll() {
    const int nState = (*mCurrent)->Poll();
    if (nState == kStateDone) {
        ++mCurrent;
        if (mCurrent == mTasks.end()) {
            Finish(true);
        } else {
            (*mCurrent)->Start();
        }
    } else if (nState == kStateFailed) {
        Finish(false);
    }
}
