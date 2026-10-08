#include "gfx/tnltrackrange.h"

namespace {

constexpr char kNoTrack = -1;
constexpr float kNoTick = -1.0f;

} // namespace

TnlTrackRange::TnlTrackRange() {
    mEndTrack = kNoTrack;
    mEndTick = kNoTick;
    mFirstTrack = kNoTrack;
    mStartTick = kNoTick;
}

TnlTrackRange::~TnlTrackRange() = default;

void TnlTrackRange::Clear() {
    mFirstTrack = kNoTrack;
    mStartTick = kNoTick;
    mEndTrack = kNoTrack;
    mEndTick = kNoTick;
}

void TnlTrackRange::Expand(char nFirstTrack, char nEndTrack, float flStartTick, float flEndTick) {
    if (mFirstTrack >= mEndTrack) {
        mEndTick = flEndTick;
        mFirstTrack = nFirstTrack;
        mEndTrack = nEndTrack;
        mStartTick = flStartTick;
        return;
    }
    if (nFirstTrack < mFirstTrack) {
        mFirstTrack = nFirstTrack;
    }
    if (flStartTick < mStartTick) {
        mStartTick = flStartTick;
    }
    if (mEndTrack < nEndTrack) {
        mEndTrack = nEndTrack;
    }
    if (mEndTick < flEndTick) {
        mEndTick = flEndTick;
    }
}

bool TnlTrackRange::Overlaps(char nTrack, float flStartTick, float flEndTick) const {
    if (nTrack < mFirstTrack || !(nTrack < mEndTrack)) {
        return false;
    }
    if (mStartTick <= flStartTick) {
        return flStartTick < mEndTick;
    }
    return mStartTick < flEndTick;
}

bool TnlTrackRange::Contains(char nTrack, float flTick) const {
    if (nTrack < mFirstTrack || !(nTrack < mEndTrack)) {
        return false;
    }
    return mStartTick <= flTick && flTick < mEndTick;
}
