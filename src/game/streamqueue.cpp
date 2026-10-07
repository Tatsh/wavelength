#include "game/streamqueue.h"

#include <cstring>

StreamQueue::~StreamQueue() {
    for (auto *pStream : mStreams) {
        delete pStream;
    }
}

void StreamQueue::Stop() {
    if (mStreams.begin() != mStreams.end()) {
        mStreams.front()->Stop();
    }
}

void StreamQueue::Enqueue(const char *pszName, bool bSkipIfBusy) {
    if (!mStreams.empty() && bSkipIfBusy) {
        return;
    }
    mStreams.push_back(new StreamTask(pszName, mDirectory.c_str()));
    if (mStreams.size() == 1) {
        mStreams.front()->Start();
    }
}

void StreamQueue::Poll() {
    if (mStreams.empty()) {
        return;
    }
    if (mStreams.front()->Poll() == Task::kStateRunning) {
        return;
    }
    delete mStreams.front();
    mStreams.erase(mStreams.begin());
    if (!mStreams.empty()) {
        mStreams.front()->Start();
    }
}

void StreamQueue::SetPaused(bool bPaused) {
    if (!mStreams.empty()) {
        mStreams.front()->SetPaused(bPaused);
    }
}

void StreamQueue::NotifyWhenDone(Task *pTask, const char *pszName) {
    auto it = mStreams.begin();
    while (it != mStreams.end() && strcmp((*it)->mName, pszName) != 0) {
        ++it;
    }
    if (it == mStreams.end()) {
        ReportDone(pTask);
    } else {
        (*it)->SetWaitingTask(pTask);
    }
}
