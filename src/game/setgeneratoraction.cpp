#include "game/setgeneratoraction.h"

namespace {

// The frames SetPath() receives for the whole length of the path.
constexpr float kWholePath = -1.0f;

} // namespace

SetGeneratorAction::SetGeneratorAction(DataArray *pAction) {
    mGenerator = FindObject<RndGenerator>(pAction, pAction->Sym(1));
    mFlags = 0;
    const char *pszPath;
    if (pAction->FindSymbol("path", &pszPath, false)) {
        mFlags |= kFlagPath;
        mPath = FindObject<RndTransAnim>(pAction, pszPath);
    }
    if (pAction->FindVector("rate", &mRate, false)) {
        mFlags |= kFlagRate;
    }
    if (pAction->FindVector("scale", &mScale, false)) {
        mFlags |= kFlagScale;
    }
    if (pAction->FindVector("path_var", &mPathVar, false)) {
        mFlags |= kFlagPathVar;
    }
}

void SetGeneratorAction::Exec() {
    if ((mFlags & kFlagPath) != 0) {
        mGenerator->SetPath(mPath, kWholePath, kWholePath);
    }
    if ((mFlags & kFlagRate) != 0) {
        mGenerator->SetRateGenLow(mRate.x);
        mGenerator->SetRateGenHigh(mRate.y);
    }
    if ((mFlags & kFlagScale) != 0) {
        mGenerator->SetScaleGenLow(mScale.x);
        mGenerator->SetScaleGenHigh(mScale.y);
    }
    if ((mFlags & kFlagPathVar) != 0) {
        mGenerator->SetPathVarMax(0, mPathVar.x);
        mGenerator->SetPathVarMax(1, mPathVar.y);
        mGenerator->SetPathVarMax(2, mPathVar.z);
    }
}
