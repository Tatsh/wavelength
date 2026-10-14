#include "netflow/nettask.h"

#include <cstring>

#include "netflow/netmultitask.h"
#include "netflow/periodictask.h"
#include "netflow/tasklist.h"
#include "os/system.h"

// The unit's static initialiser at NTSC-U/C: 0x0026a958, PAL: 0x002744e8, and its global
// constructor and destructor at NTSC-U/C: 0x0026a9e8, PAL: 0x00274578, and NTSC-U/C: 0x0026aa08,
// PAL: 0x00274598, construct and destroy the list.
std::list<NetTaskTag> NetTask::sTags;

int NetTask::sNextTag = 1;

void NetTask::MakeTag(NetTask *pTask, char *pszTag) {
    NetTaskTag tag;
    tag.mTask = pTask;
    tag.mTag << sNextTag;
    sTags.push_back(tag);
    strcpy(pszTag, tag.mTag.c_str());
    ++sNextTag;
}

void NetTask::RemoveTag(NetTask *pTask) {
    for (auto it = sTags.begin(); it != sTags.end(); ++it) {
        if (it->mTask == pTask) {
            sTags.erase(it);
            break;
        }
    }
}

void NetTask::OnStop() {
    RemoveTag(this);
}

void NetTask::Finish(bool bSucceeded, int nResult) {
    if (mParent != nullptr && !bSucceeded) {
        mParent->mResult = nResult;
    }
    Task::Finish(bSucceeded);
}

NetTask *NetTask::FindTag(const char *pszTag) {
    for (const NetTaskTag &tag : sTags) {
        if (tag.mTag == pszTag) {
            return tag.mTask;
        }
    }
    return nullptr;
}

void NetMultiTask::Add(NetTask *pTask) {
    pTask->mParent = this;
    SerialTasks::Add(pTask);
}

void NetMultiTask::Start(MsgSink *pSink) {
    mSink = pSink;
    Task::Start();
}

void NetMultiTask::Notify(Message *pMsg) {
    if (mSink != nullptr) {
        mSink->Dispatch(pMsg); // The binary discards the result.
    }
}

PeriodicTask::PeriodicTask(float fPeriod) : mTask(nullptr), mPeriod(fPeriod), mNextTime(kNever) {
}

PeriodicTask::~PeriodicTask() {
    if (mTask != nullptr) {
        mTask->Stop();
        delete mTask;
    }
}

void PeriodicTask::SetTask(Task *pTask) {
    mTask = pTask;
}

int PeriodicTask::Poll() {
    if (mTask == nullptr) {
        return Task::kStateIdle;
    }
    const float fNow = SystemMs();
    if (mNextTime < fNow) {
        mNextTime = kNever;
        mTask->Start();
    }
    const int nState = mTask->Poll();
    if (nState == Task::kStateDone || nState == Task::kStateFailed) {
        mTask->Stop();
        mNextTime = fNow + mPeriod;
    }
    return nState;
}

void PeriodicTask::RunNow() {
    mNextTime = 0.0f;
}

void PeriodicTask::Suspend() {
    mNextTime = kNever;
}

void TaskList::Add(Task *pTask) {
    mTasks.push_back(pTask);
    pTask->Start();
}

TaskList::~TaskList() {
    for (Task *pTask : mTasks) {
        pTask->Stop();
        delete pTask;
    }
    mTasks.clear();
}

void TaskList::Poll() {
    auto it = mTasks.begin();
    while (it != mTasks.end()) {
        if ((*it)->Poll() != Task::kStateRunning) {
            (*it)->Stop();
            delete *it;
            it = mTasks.erase(it);
        } else {
            ++it;
        }
    }
}
