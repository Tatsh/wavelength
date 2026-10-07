#include "game/pitchtrackriffs.h"

#include "os/scheduler.h"

namespace {

constexpr int kNumSlots = 3;

} // namespace

PitchTrackRiffs::PitchTrackRiffs(PitchTrackRiffData *pRiffData, PlayMap *pPlayMap)
    : mRiffData(pRiffData), mPlayMap(pPlayMap) {
}

PitchTrackRiffs::~PitchTrackRiffs() {
}

Muse *PitchTrackRiffs::GetRiff(int nSlot) {
    return GetRiff(nSlot, TheSongScheduler.mTick);
}

Muse *PitchTrackRiffs::GetRiff(int nSlot, int nTick) {
    Muse *riffs[kNumSlots];
    // Yes, the binary does not check for a tick before the first set of riffs.
    mRiffData->GetRiffs(
        mRiffData->FindRiffs(mPlayMap->MapTick(nTick)), &riffs[0], &riffs[1], &riffs[2]);
    return riffs[nSlot];
}
