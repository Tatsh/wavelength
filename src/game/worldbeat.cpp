#include "game/worldbeat.h"

#include "game/triggermgr.h"

namespace {

// The index that marks no event at or after tick 0.
constexpr int kNoEvent = -1;

} // namespace

WorldBeat::WorldBeatCmd::WorldBeatCmd(Scheduler *pScheduler,
                                      int nLengthTicks,
                                      std::vector<int> *pEvents,
                                      char cLetter)
    : mScheduler(pScheduler), mLengthTicks(nLengthTicks), mEvents(pEvents), mLetter(cLetter),
      mFirst(0), mIndex(0), mLoop(0) {
    const int nCount = static_cast<int>(pEvents->size());
    while (mFirst < nCount && (*pEvents)[mFirst] < 0) {
        ++mFirst;
    }
    if (mFirst >= nCount) {
        mFirst = kNoEvent;
    }
}

void WorldBeat::WorldBeatCmd::Execute() {
    TheTriggerMgr.BeatEvent(mLetter);
    ++mIndex;
    if (static_cast<unsigned int>(mIndex) >= mEvents->size() || mLengthTicks < (*mEvents)[mIndex]) {
        if (mFirst == kNoEvent) {
            return;
        }
        mIndex = mFirst;
        ++mLoop;
    }
    mScheduler->PostAt(this, mLoop * mLengthTicks + (*mEvents)[mIndex], false);
}

WorldBeat::WorldBeat(Scheduler *pScheduler, WorldTrack *pTrack, int nLengthTicks)
    : mScheduler(pScheduler), mTrack(pTrack), mCommands(WorldTrack::kListCount, Ptr<Command>()) {
    for (int i = 0; i < WorldTrack::kListCount; ++i) {
        const char cLetter = static_cast<char>(WorldTrack::kFirstLetter + i);
        Command *pCommand =
            new WorldBeatCmd(pScheduler, nLengthTicks, pTrack->GetEvents(cLetter), cLetter);
        mCommands[i] = Ptr<Command>(pCommand);
    }
}

WorldBeat::~WorldBeat() {
    Stop();
}

void WorldBeat::Start() {
    for (int i = 0; i < WorldTrack::kListCount; ++i) {
        const std::vector<int> *pEvents =
            mTrack->GetEvents(static_cast<char>(WorldTrack::kFirstLetter + i));
        if (!pEvents->empty()) {
            mScheduler->PostAt(mCommands[i].Get(), pEvents->front(), false);
        }
    }
}

void WorldBeat::Stop() {
    for (unsigned int i = 0; i < mCommands.size(); ++i) {
        mScheduler->Cancel(mCommands[i].Get());
    }
}
