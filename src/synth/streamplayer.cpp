#include "synth/streamplayer.h"

#include "os/system.h"
#include "synth/synth.h"

namespace {

constexpr float kNoEndMs = -1.0f;

// NTSC-U/C: 0x003b0aa8
float g_flStreamEndDelayMs = 640.0f;

} // namespace

StreamPlayer::StreamPlayer(int nChannel, const char *pszFile, bool bAudio)
    : mChannel(nChannel), mPlaying(0), mRun(nullptr), mEndMs(kNoEndMs), mEndSent(0) {
    mBuffer = new FileStreamBuffer(pszFile, bAudio);
    if (mBuffer->IsClosed()) {
        delete mBuffer;
        mBuffer = new FileStreamBuffer("SoundFX/default.str", 0);
    }
}

StreamPlayer::~StreamPlayer() {
    delete mBuffer;
    mBuffer = nullptr;
}

void StreamPlayer::Poll() {
    if (!IsPlaying()) {
        return;
    }
    if (IsFinished()) {
        SetPlaying(false);
    }
    const int nRoom = TheSynth->GetStreamStatus();
    if (nRoom == 0) {
        return;
    }
    mBuffer->CommitReadRun(&mRun);
    int nBytes = nRoom;
    mRun = mBuffer->AcquireReadRun(nRoom, &nBytes);
    if (mRun != nullptr) {
        TheSynth->SendStreamData(mRun, nBytes);
        mEndMs = kNoEndMs;
        return;
    }
    if (mBuffer->IsEof() && mEndMs < 0.0f) {
        mEndMs = SystemMs() + g_flStreamEndDelayMs;
    }
    if (0.0f < mEndMs && mEndMs < SystemMs() && mEndSent == 0) {
        TheSynth->SendStreamData(nullptr, 0);
        mEndSent = 1;
    }
}

bool StreamPlayer::IsFinished() {
    return mBuffer->IsEof() && (mEndSent != 0);
}

void StreamPlayer::SetPlaying(bool bPlaying) {
    mPlaying = bPlaying;
}

bool StreamPlayer::IsPlaying() const {
    return mPlaying != 0;
}
