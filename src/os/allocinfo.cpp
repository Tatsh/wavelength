#include "os/allocinfo.h"

#include <string.h>

#include "os/backtrace.h"

namespace {

constexpr int kCaptureSize = 28;

// The frames of the capture itself and of the tracker routines above it.
constexpr int kSkippedFrames = 4;

} // namespace

void AllocInfo::CaptureStack() {
    unsigned int frames[kCaptureSize];
    CaptureStackFrames(frames, kCaptureSize);
    mStack[0] = frames[kSkippedFrames];
    if (mStack[0] != 0) {
        for (int i = 1; i < kStackSize; ++i) {
            mStack[i] = frames[kSkippedFrames + i];
            if (mStack[i] == 0) {
                break;
            }
        }
    }
    mStack[kStackSize - 1] = 0;
}

void AllocInfo::Print(PrnStream &stream) const {
    if (mPoolTag != nullptr) {
        stream.Printf("p \"%s\" %d ", mPoolTag, mPoolSize);
    } else if (mTag != nullptr) {
        stream.Printf("m \"%s\" %d ", mTag, mSizeRequested);
    }
    for (int i = 0; mStack[i] != 0;) {
        stream.Printf("%d ", mStack[i]);
        ++i;
        if (!(i < kStackSize)) {
            break;
        }
    }
}

bool AllocInfo::Less(const AllocInfo *pFirst, const AllocInfo *pSecond) {
    if (pFirst->mSizeRequested != pSecond->mSizeRequested) {
        return pFirst->mSizeRequested < pSecond->mSizeRequested;
    }
    if (pFirst->mPoolSize != pSecond->mPoolSize) {
        return pFirst->mPoolSize < pSecond->mPoolSize;
    }
    if (pFirst->mStack[0] != pSecond->mStack[0]) {
        return static_cast<int>(pFirst->mStack[0]) < static_cast<int>(pSecond->mStack[0]);
    }
    for (int i = 0; pFirst->mStack[i] != 0;) {
        ++i;
        if (!(i < kStackSize)) {
            break;
        }
        if (pFirst->mStack[i] != pSecond->mStack[i]) {
            return static_cast<int>(pFirst->mStack[i]) < static_cast<int>(pSecond->mStack[i]);
        }
    }
    if (pFirst->mTag != nullptr && pSecond->mTag != nullptr) {
        const int nCompare = strcmp(pFirst->mTag, pSecond->mTag);
        if (nCompare != 0) {
            return nCompare == 1;
        }
    }
    if (pFirst->mPoolTag == nullptr || pSecond->mPoolTag == nullptr) {
        return false;
    }
    return strcmp(pFirst->mPoolTag, pSecond->mPoolTag) == 1;
}
