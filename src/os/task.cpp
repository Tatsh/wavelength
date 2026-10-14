#include "os/task.h"

Task::~Task() {
}

void Task::Finish(bool bSucceeded) {
    const int nOldState = mState;
    mState = bSucceeded ? kStateDone : kStateFailed;
    if (nOldState == kStateRunning) {
        OnStop();
    }
}

int Task::Poll() {
    if (mState == kStateRunning) {
        OnPoll();
    }
    return mState;
}

int Task::GetState() const {
    return mState;
}

void Task::Start() {
    if (mState == kStateRunning) {
        return;
    }
    mState = kStateRunning;
    OnStart();
}

void Task::Stop() {
    if (mState == kStateRunning) {
        OnStop();
    }
    mState = kStateIdle;
}
