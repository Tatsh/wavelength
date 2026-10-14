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
        mGenerator->mRateGenLow = mRate.x;
        mGenerator->mRateGenHigh = mRate.y;
    }
    if ((mFlags & kFlagScale) != 0) {
        mGenerator->mScaleGenLow = mScale.x;
        mGenerator->mScaleGenHigh = mScale.y;
    }
    if ((mFlags & kFlagPathVar) != 0) {
        mGenerator->mPathVarMax[0] = mPathVar.x;
        mGenerator->mPathVarMax[1] = mPathVar.y;
        mGenerator->mPathVarMax[2] = mPathVar.z;
    }
}
