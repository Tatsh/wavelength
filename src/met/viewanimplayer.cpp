#include "met/viewanimplayer.h"

#include "rnd/cursor.h"
#include "synth/fxmidi.h"

ViewAnimPlayer::ViewAnimPlayer() : mStartMs(0.0f), mAnim(nullptr) {
}

void ViewAnimPlayer::SetAnim(Rnd::Animatable *pAnim) {
    mAnim = pAnim;
    pAnim->SetFrame(0.0f);
    mEndFrame = mAnim->ChildrenEndFrame();
    mCursorMoving = 0;
}

void ViewAnimPlayer::Start(float fNowMs) {
    mAnim->SetFrame(0.0f);
    mStartMs = fNowMs;
}

void ViewAnimPlayer::Stop() {
    mStartMs = 0.0f;
    mCursorMoving = 0;
    FxMidi::StopCursorLoop();
}

void ViewAnimPlayer::Poll(float fNowMs) {
    if (mStartMs == 0.0f || mAnim == nullptr) {
        return;
    }
    const float fFrame = fNowMs - mStartMs;
    mAnim->SetFrame(fFrame);
    if (mEndFrame < fFrame) {
        Stop();
        return;
    }
    int nMoving = 0;
    for (Rnd::Animatable *pChild : mAnim->mAnims) {
        Rnd::Cursor *pCursor = pChild != nullptr ? dynamic_cast<Rnd::Cursor *>(pChild) : nullptr;
        if (pCursor == nullptr) {
            continue;
        }
        const float fCursorFrame = pCursor->mFilteredFrame;
        if (0.0f <= fCursorFrame && fCursorFrame <= pCursor->FilteredFrameEnd()) {
            nMoving = 1;
        }
    }
    if (nMoving == mCursorMoving) {
        return;
    }
    mCursorMoving = nMoving;
    if (nMoving != 0) {
        FxMidi::PlayCursorLoop();
    } else {
        FxMidi::StopCursorLoop();
    }
}

bool ViewAnimPlayer::IsPlaying() {
    return mStartMs != 0.0f;
}

void ViewAnimPlayer::Clear() {
    Stop();
    mAnim = nullptr;
}
