#include "utl/MsgSource.h"

#include <algorithm>

#include "os/Debug.h"

MsgSource::MsgSource() : mGraphBuilt(0) {
}

MsgSource::~MsgSource() {
}

void MsgSource::AddSink(MsgSink *sink) {
    if (std::find(mSinks.begin(), mSinks.end(), sink) == mSinks.end()) {
        mSinks.insert(mSinks.end(), sink);
    }
}

void MsgSource::RemoveSink(MsgSink *sink) {
    std::list<MsgSink *>::iterator it = std::find(mSinks.begin(), mSinks.end(), sink);
    if (it == mSinks.end()) {
        ASSERT(it != mSinks.end());
        return;
    }
    if (mGraphBuilt) {
        *it = NULL;
        return;
    }
    mSinks.erase(it);
}
