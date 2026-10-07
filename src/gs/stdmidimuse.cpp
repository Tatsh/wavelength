#include "gs/stdmidimuse.h"

#include "synth/synth.h"

namespace {

constexpr unsigned char kStatusControlChange = 0xb0;
constexpr unsigned char kStatusProgramChange = 0xc0;
constexpr unsigned char kStatusChannelPressure = 0xd0;
constexpr float kRound = 0.5f;

} // namespace

void StdMidiMuse::StdMidiCommand::Execute() {
    TheSynth->SendMessage(
        mMuse->mStatus,
        mMuse->mData1,
        mMuse->mData2,
        static_cast<int>(mScheduler->mTime - mScheduler->mPrevFrameTime + kRound));
    mScheduler = nullptr;
}

void StdMidiMuse::StdMidiCommand::Cancel() {
    if (mScheduler != nullptr) {
        mScheduler->Cancel(this);
        mScheduler = nullptr;
    }
}

StdMidiMuse::StdMidiMuse(unsigned char nStatus, unsigned char nData1, unsigned char nData2)
    : mStatus(nStatus), mData1(nData1), mData2(nData2), mCmd(new StdMidiCommand(this)) {
}

StdMidiMuse::~StdMidiMuse() {
    Stop();
}

void StdMidiMuse::Play(Scheduler *pScheduler) {
    StdMidiCommand *pCmd = mCmd.operator->();
    pCmd->Cancel();
    pCmd->mScheduler = pScheduler;
    pScheduler->PostIn(pCmd, 0, false);
}

void StdMidiMuse::PlayFrom(Scheduler *pScheduler, int nOffset) {
    StdMidiCommand *pCmd = mCmd.operator->();
    if (nOffset >= 1) {
        nOffset = 0;
    }
    pCmd->Cancel();
    pCmd->mScheduler = pScheduler;
    pScheduler->PostIn(pCmd, -nOffset, false);
}

void StdMidiMuse::PlayWindow(Scheduler *pScheduler, int nStart, int nEnd) {
    if (!(nStart < nEnd)) {
        return;
    }
    StdMidiCommand *pCmd = mCmd.operator->();
    if (nStart >= 1) {
        nStart = 0;
    }
    pCmd->Cancel();
    pCmd->mScheduler = pScheduler;
    pScheduler->PostIn(pCmd, -nStart, false);
}

void StdMidiMuse::Stop() {
    mCmd->Cancel();
}

bool StdMidiMuse::IsPlaying() {
    return mCmd.Get()->mScheduler != nullptr;
}

Muse *StdMidiMuse::Clone() {
    return new StdMidiMuse(mStatus, mData1, mData2);
}

void StdMidiMuse::SetNoteCB(NoteCB *) {
}

StdMidiMuse *StdMidiMuse::NewProgramChange(unsigned char nChannel, unsigned char nProgram) {
    return new StdMidiMuse(nChannel | kStatusProgramChange, nProgram, 0);
}

StdMidiMuse *StdMidiMuse::NewChannelPressure(unsigned char nChannel, unsigned char nPressure) {
    return new StdMidiMuse(nChannel | kStatusChannelPressure, nPressure, 0);
}

StdMidiMuse *StdMidiMuse::NewControlChange(unsigned char nChannel,
                                           unsigned char nController,
                                           unsigned char nValue) {
    return new StdMidiMuse(nChannel | kStatusControlChange, nController, nValue);
}
