#include "netflow/clocksynctask.h"

#include <algorithm>

#include "msg/timerequestpkt.h"
#include "netflow/joinpadrt.h"
#include "os/system.h"

namespace {

// The part of the round trip the answer of the host takes to arrive.
constexpr float kReturnShare = 0.5f;

} // namespace

void ClockSyncTask::OnStart() {
    mSamples.clear();
    mCountdown = 0;
}

void ClockSyncTask::OnPoll() {
    if (mCountdown <= 0) {
        TimeRequestPkt pkt(LaunchpadRT::kHostId, SystemMs());
        TheLaunchpadRT->Send(&pkt);
        mCountdown = kRequestInterval;
    }
    --mCountdown;
}

void ClockSyncTask::AddSample(float fRequestTime, float fHostTime) {
    if (GetState() != kStateRunning) {
        return;
    }
    const float fNow = SystemMs();
    Sample sample;
    sample.mRoundTrip = fNow - fRequestTime;
    sample.mOffset = (fHostTime + sample.mRoundTrip * kReturnShare) - fNow;
    mSamples.push_back(sample);
    if (mSamples.size() >= kSampleCount) {
        FinishSync();
    }
}

void ClockSyncTask::FinishSync() {
    std::sort(mSamples.begin(), mSamples.end());
    float fRoundTrip = 0.0f;
    for (unsigned int i = 0; i < mSamples.size(); ++i) {
        fRoundTrip += mSamples[i].mRoundTrip;
    }
    const float fOffset = mSamples[mSamples.size() / 2].mOffset;
    fRoundTrip /= static_cast<float>(mSamples.size());
    Finish(true);
    TheJoinpadRT->OnClockSynced(fOffset, fRoundTrip);
}
