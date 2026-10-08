#include "game/setbluraction.h"

SetBlurAction::SetBlurAction(DataArray *pAction) {
    mBlur = FindObject<Rnd::Blur>(pAction, pAction->Sym(1));
    mFlags = 0;
    if (pAction->FindInt("length", &mLength, false)) {
        mFlags |= kFlagLength;
    }
    if (pAction->FindInt("rate", &mRate, false)) {
        mFlags |= kFlagRate;
    }
    if (pAction->FindFloat("falloff", &mFalloff, false)) {
        mFlags |= kFlagFalloff;
    }
}

void SetBlurAction::Exec() {
    if ((mFlags & kFlagLength) != 0) {
        mBlur->SetLength(mLength);
    }
    if ((mFlags & kFlagRate) != 0) {
        mBlur->SetRate(mRate);
    }
    if ((mFlags & kFlagFalloff) != 0) {
        mBlur->mFalloff = mFalloff;
    }
}
