#include "game/setanimaction.h"

#include "os/debug.h"

SetAnimAction::SetAnimAction(DataArray *pAction) {
    mAnim = FindObject<Rnd::Animatable>(pAction, pAction->Sym(1));
    mFlags = 0;
    if (pAction->FindFloat("scale", &mScale, false)) {
        mFlags |= kFlagScale;
        CheckFirstFilter(pAction, Rnd::Animatable::kFilterScaleOffset, "kScaleOffset");
    }
    if (pAction->FindFloat("offset", &mOffset, false)) {
        mFlags |= kFlagOffset;
        CheckFirstFilter(pAction, Rnd::Animatable::kFilterScaleOffset, "kScaleOffset");
    }
    if (pAction->FindFloat("min", &mMin, false)) {
        mFlags |= kFlagMin;
        CheckFirstFilter(pAction, Rnd::Animatable::kFilterMinMaxLoop, "kMinMaxLoop");
    }
    if (pAction->FindFloat("max", &mMax, false)) {
        mFlags |= kFlagMax;
        CheckFirstFilter(pAction, Rnd::Animatable::kFilterMinMaxLoop, "kMinMaxLoop");
    }
}

inline void SetAnimAction::CheckFirstFilter(DataArray *pAction, int nType, const char *pszType) {
    if (!mAnim->mFilters.empty() && mAnim->mFilters.front()->Type() == nType) {
        return;
    }
    DebugWarn("First filter not '%s' in animatable '%s' (file %s, line %d)",
              pszType,
              pAction->Sym(1),
              pAction->mFile,
              pAction->mLine);
}

void SetAnimAction::Exec() {
    if ((mFlags & kFlagScale) != 0) {
        Rnd::Animatable::ScaleOffset *pFilter =
            static_cast<Rnd::Animatable::ScaleOffset *>(mAnim->mFilters.front());
        if ((mFlags & kFlagOffset) == 0) {
            pFilter->mOffset = (mAnim->mFrame * (pFilter->mScale - mScale)) + pFilter->mOffset;
        }
        pFilter->mScale = mScale;
    }
    if ((mFlags & kFlagOffset) != 0) {
        static_cast<Rnd::Animatable::ScaleOffset *>(mAnim->mFilters.front())->mOffset = mOffset;
    }
    if ((mFlags & kFlagMin) != 0) {
        static_cast<Rnd::Animatable::MinMaxLoop *>(mAnim->mFilters.front())->mMin = mMin;
    }
    if ((mFlags & kFlagMax) != 0) {
        static_cast<Rnd::Animatable::MinMaxLoop *>(mAnim->mFilters.front())->mMax = mMax;
    }
}
