#include "game/gemcursor.h"

namespace {

constexpr int kInvalidIndex = -1;
// A tick later than every gem.
constexpr int kNoGemTick = 0x20000000;

} // namespace

GemCursor::GemCursor(PlayMap *pPlayMap, CatchTrackData *pData, int nTick)
    : mData(pData), mPlayMap(pPlayMap) {
    const int nMappedTick = mPlayMap->MapTick(nTick);
    mOffset = nTick - nMappedTick;
    mIndex = mData->FindGem(nMappedTick);
    if (mIndex == kInvalidIndex || mData->GetGem(mIndex)->mTick != nMappedTick) {
        int nIsLast;
        AdvanceToTick(nTick, false, &nIsLast);
    }
}

GemCursor::GemCursor() {
    mData = nullptr;
    mIndex = kInvalidIndex;
    mPlayMap = nullptr;
    mOffset = 0;
}

GemCursor::GemCursor(const GemCursor &other)
    : mData(other.mData), mPlayMap(other.mPlayMap), mIndex(other.mIndex), mOffset(other.mOffset) {
}

GemCursor &GemCursor::operator=(const GemCursor &other) {
    if (this != &other) {
        mData = other.mData;
        mPlayMap = other.mPlayMap;
        mIndex = other.mIndex;
        mOffset = other.mOffset;
    }
    return *this;
}

bool GemCursor::IsValid() const {
    return mIndex != kInvalidIndex;
}

int GemCursor::GetTick() const {
    return mData->GetGem(mIndex)->mTick + mOffset;
}

int GemCursor::GetLane() const {
    return mData->GetGem(mIndex)->mLane;
}

Muse *GemCursor::GetMuse() const {
    return mData->GetGem(mIndex)->mMuse.Get();
}

void GemCursor::Advance() {
    (void)IsValid(); // Yes, the binary discards this call's result.
    int nIsLast;
    AdvanceToTick(GetTick(), true, &nIsLast);
}

void GemCursor::AdvanceToTick(int nTick, bool bSkipCurrent, int *pIsLast) {
    int nChangeTick;
    int nEnd;
    int nNextStart;
    int nNextLength;
    *pIsLast = 0;
    mPlayMap->GetSegment(nTick, &nChangeTick, &nEnd, &nNextStart, &nNextLength, pIsLast);
    if (bSkipCurrent) {
        ++mIndex;
    }

    int nGemTick;
    if (mIndex == kInvalidIndex || mIndex >= mData->GetNumGems()) {
        mIndex = kInvalidIndex;
        nGemTick = kNoGemTick;
    } else {
        nGemTick = GetTick();
    }
    if (nGemTick < nChangeTick) {
        return;
    }

    // The gem plays after the current stretch. The search continues in the stretches that follow.
    mOffset += nEnd - nNextStart;
    mIndex = mData->FindGem(nNextStart);
    int nFoundTick = mIndex != kInvalidIndex ? mData->GetGem(mIndex)->mTick : kNoGemTick;
    while (nFoundTick < nNextStart || nFoundTick >= nNextStart + nNextLength) {
        if (*pIsLast != 0) {
            mIndex = kInvalidIndex;
            return;
        }
        mPlayMap->GetSegment(nChangeTick, &nChangeTick, &nEnd, &nNextStart, &nNextLength, pIsLast);
        mOffset += nEnd - nNextStart;
        mIndex = mData->FindGem(nNextStart);
        nFoundTick = mIndex != kInvalidIndex ? mData->GetGem(mIndex)->mTick : kNoGemTick;
    }
}

void GemCursor::AdvanceToLane(int nLane) {
    int nFirstIndex = kInvalidIndex;
    (void)IsValid(); // Yes, the binary discards this call's result.
    do {
        int nIsLast;
        AdvanceToTick(GetTick(), true, &nIsLast);
        if (nIsLast != 0 && nFirstIndex == kInvalidIndex) {
            nFirstIndex = mIndex;
        } else if (mIndex == nFirstIndex) {
            mIndex = kInvalidIndex;
        }
    } while (IsValid() && mData->GetGem(mIndex)->mLane != nLane);
}

bool GemCursor::operator==(const GemCursor &other) const {
    if (!IsValid()) {
        return !other.IsValid();
    }
    return mData == other.mData && mIndex == other.mIndex;
}

GemCursor GemCursor::Next() {
    Advance();
    return *this;
}
