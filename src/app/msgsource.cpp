#include "app/msgsource.h"

#include <algorithm>

#include "app/msgsink.h"
#include "msg/message.h"

MsgSource::MsgSource() : mDispatchDepth(0) {
}

MsgSource::~MsgSource() {
}

void MsgSource::AddSink(MsgSink *pSink) {
    if (std::find(mSinks.begin(), mSinks.end(), pSink) == mSinks.end()) {
        mSinks.push_back(pSink);
    }
}

void MsgSource::RemoveSink(MsgSink *pSink) {
    for (auto it = mSinks.begin(); it != mSinks.end(); ++it) {
        if (*it == pSink) {
            if (mDispatchDepth != 0) {
                *it = nullptr;
            } else {
                mSinks.erase(it);
            }
            return;
        }
    }
}

void MsgSource::Send(Message *pMsg) {
    ++mDispatchDepth;
    for (auto it = mSinks.begin(); it != mSinks.end();) {
        if (*it != nullptr) {
            (*it)->Dispatch(pMsg);
            ++it;
        } else {
            it = mSinks.erase(it);
        }
    }
    --mDispatchDepth;
}

bool MsgSource::SendUntilHandled(Message *pMsg) {
    ++mDispatchDepth;
    bool bHandled = false;
    for (auto it = mSinks.begin(); it != mSinks.end();) {
        if (*it != nullptr) {
            bHandled = (*it)->Dispatch(pMsg);
            ++it;
            if (bHandled) {
                break;
            }
        } else {
            it = mSinks.erase(it);
        }
    }
    --mDispatchDepth;
    return bHandled;
}
