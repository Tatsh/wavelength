#include "gs/notemusebase.h"

#include "synth/synth.h"

namespace {

constexpr unsigned char kStatusNoteOff = 0x80;
constexpr unsigned char kStatusNoteOn = 0x90;
constexpr unsigned char kNoteOffVelocity = 64;
constexpr float kRound = 0.5f;

// The milliseconds into the current pump of a scheduler. They time a message within the frame.
int FrameOffsetMs(const Scheduler *pScheduler) {
    return static_cast<int>(pScheduler->mTime - pScheduler->mPrevFrameTime + kRound);
}

} // namespace

void NoteMuseBase::NoteCommand::Execute() {
    if (!mMuse->mSounding) {
        NoteOn();
    } else {
        NoteOff();
        mScheduler = nullptr;
    }
}

void NoteMuseBase::NoteCommand::Cancel() {
    if (mMuse->mSounding) {
        NoteOff();
    }
    if (mScheduler != nullptr) {
        mScheduler->Cancel(this);
        mScheduler = nullptr;
    }
}

void NoteMuseBase::NoteCommand::NoteOff() {
    TheSynth->SendMessage(mMuse->mChannel | kStatusNoteOff,
                          mMuse->mNote,
                          kNoteOffVelocity,
                          FrameOffsetMs(mScheduler));
    mMuse->mSounding = 0;
}

void NoteMuseBase::NoteCommand::NoteOn() {
    TheSynth->SendMessage(
        mMuse->mChannel | kStatusNoteOn, mMuse->mNote, mMuse->mVelocity, FrameOffsetMs(mScheduler));
    mMuse->mSounding = 1;
    if (mMuse->mNoteCB != nullptr) {
        mMuse->mNoteCB->OnNote(mMuse->mNote, mMuse->mDuration);
    }
}

NoteMuseBase::NoteMuseBase(unsigned char nNote,
                           unsigned char nVelocity,
                           int nDuration,
                           unsigned char nChannel)
    : mNote(nNote), mVelocity(nVelocity), mChannel(nChannel), mSounding(0), mDuration(nDuration),
      mCmd(new NoteCommand(this)), mNoteCB(nullptr) {
}

NoteMuseBase::~NoteMuseBase() {
    Stop();
    mCmd->Cancel();
}

void NoteMuseBase::Play(Scheduler *pScheduler) {
    NoteCommand *pCmd = mCmd.operator->();
    const int nDuration = pCmd->mMuse->mDuration;
    pCmd->Cancel();
    pScheduler->PostIn(pCmd, 0, false);
    pScheduler->PostIn(pCmd, nDuration, false);
    pCmd->mScheduler = pScheduler;
}

void NoteMuseBase::PlayFrom(Scheduler *pScheduler, int nOffset) {
    NoteCommand *pCmd = mCmd.operator->();
    const int nDuration = pCmd->mMuse->mDuration;
    if (nOffset > 0) {
        return;
    }
    pCmd->Cancel();
    pScheduler->PostIn(pCmd, -nOffset, false);
    pScheduler->PostIn(pCmd, nDuration - nOffset, false);
    pCmd->mScheduler = pScheduler;
}

void NoteMuseBase::PlayWindow(Scheduler *pScheduler, int nStart, int nEnd) {
    if (nStart < nEnd) {
        // The binary does not cut the note at the end of the window.
        NoteMuseBase::PlayFrom(pScheduler, nStart);
    }
}

void NoteMuseBase::Stop() {
    mCmd->Cancel();
}

bool NoteMuseBase::IsPlaying() {
    return mCmd.Get()->mScheduler != nullptr;
}

void NoteMuseBase::SetNoteCB(NoteCB *pNoteCB) {
    mNoteCB = pNoteCB;
}
