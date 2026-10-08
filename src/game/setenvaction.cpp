#include "game/setenvaction.h"

SetEnvAction::SetEnvAction(DataArray *pAction) {
    mEnviron = FindObject<Rnd::Environ>(pAction, pAction->Sym(1));
    mFlags = 0;
    if (pAction->FindFloat("fog_start", &mFogStart, false)) {
        mFlags |= kFlagFogStart;
    }
    if (pAction->FindFloat("fog_end", &mFogEnd, false)) {
        mFlags |= kFlagFogEnd;
    }
    if (pAction->FindFloat("fog_density", &mFogDensity, false)) {
        mFlags |= kFlagFogDensity;
    }
    if (pAction->FindColor("fog_color", &mFogColor, false)) {
        mFlags |= kFlagFogColor;
    }
}

void SetEnvAction::Exec() {
    if (mFlags == 0) {
        return;
    }
    const float flStart = (mFlags & kFlagFogStart) != 0 ? mFogStart : mEnviron->mFogStart;
    const float flEnd = (mFlags & kFlagFogEnd) != 0 ? mFogEnd : mEnviron->mFogEnd;
    const float flDensity = (mFlags & kFlagFogDensity) != 0 ? mFogDensity : mEnviron->mFogDensity;
    const Color &color = (mFlags & kFlagFogColor) != 0 ? mFogColor : mEnviron->mFogColor;
    mEnviron->mFogStart = flStart;
    mEnviron->mFogEnd = flEnd;
    mEnviron->mFogDensity = flDensity;
    mEnviron->mFogColor = color;
}
