#include "game/setgeneratoraction.h"

namespace {

// The frames SetPath() receives for the whole length of the path.
constexpr float kWholePath = -1.0f;

} // namespace

SetGeneratorAction::SetGeneratorAction(DataArray *pAction) {
    mGenerator = FindObject<Rnd::Generator>(pAction, pAction->Sym(1));
    mFlags = 0;
    const char *pszPath;
    if (pAction->FindSymbol("path", &pszPath, false)) {
        mFlags |= kFlagPath;
        mPath = FindObject<Rnd::TransAnim>(pAction, pszPath);
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
        mGenerator->SetRateGen(mRate.x, mRate.y);
    }
    if ((mFlags & kFlagScale) != 0) {
        mGenerator->SetScaleGen(mScale.x, mScale.y);
    }
    if ((mFlags & kFlagPathVar) != 0) {
        mGenerator->SetPathVarMax(mPathVar.x, mPathVar.y, mPathVar.z);
    }
}
