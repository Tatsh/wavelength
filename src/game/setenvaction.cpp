#include "game/setenvaction.h"

SetEnvAction::SetEnvAction(DataArray *pAction) {
    mEnviron = FindObject<RndEnviron>(pAction, pAction->Sym(1));
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
    const float flStart = (mFlags & kFlagFogStart) != 0 ? mFogStart : mEnviron->GetFogStart();
    const float flEnd = (mFlags & kFlagFogEnd) != 0 ? mFogEnd : mEnviron->GetFogEnd();
    const float flDensity =
        (mFlags & kFlagFogDensity) != 0 ? mFogDensity : mEnviron->GetFogDensity();
    const Color &color = (mFlags & kFlagFogColor) != 0 ? mFogColor : mEnviron->GetFogColor();
    mEnviron->SetFogStart(flStart);
    mEnviron->SetFogEnd(flEnd);
    mEnviron->SetFogDensity(flDensity);
    mEnviron->GetFogColor() = color;
}
