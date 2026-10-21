#include "game/setanimaction.h"

#include "os/debug.h"

SetAnimAction::SetAnimAction(DataArray *pAction) {
    mAnim = FindObject<RndAnimatable>(pAction, pAction->Sym(1));
    mFlags = 0;
    if (pAction->FindFloat("scale", &mScale, false)) {
        mFlags |= kFlagScale;
        CheckFirstFilter(pAction, RndAnimatable::kFilterScaleOffset, "kScaleOffset");
    }
    if (pAction->FindFloat("offset", &mOffset, false)) {
        mFlags |= kFlagOffset;
        CheckFirstFilter(pAction, RndAnimatable::kFilterScaleOffset, "kScaleOffset");
    }
    if (pAction->FindFloat("min", &mMin, false)) {
        mFlags |= kFlagMin;
        CheckFirstFilter(pAction, RndAnimatable::kFilterMinMaxLoop, "kMinMaxLoop");
    }
    if (pAction->FindFloat("max", &mMax, false)) {
        mFlags |= kFlagMax;
        CheckFirstFilter(pAction, RndAnimatable::kFilterMinMaxLoop, "kMinMaxLoop");
    }
}

inline void SetAnimAction::CheckFirstFilter(DataArray *pAction, int nType, const char *pszType) {
    if (!mAnim->GetFilters().empty() && mAnim->GetFilters().front()->Type() == nType) {
        return;
    }
    DebugWarn("First filter not '%s' in animatable '%s' (file %s, line %d)",
              pszType,
              pAction->Sym(1),
              pAction->GetFile(),
              pAction->GetLine());
}

void SetAnimAction::Exec() {
    if ((mFlags & kFlagScale) != 0) {
        RndAnimatable::ScaleOffset *pFilter =
            static_cast<RndAnimatable::ScaleOffset *>(mAnim->GetFilters().front());
        if ((mFlags & kFlagOffset) == 0) {
            pFilter->SetOffset((mAnim->GetFrame() * (pFilter->GetScale() - mScale)) +
                               pFilter->GetOffset());
        }
        pFilter->SetScale(mScale);
    }
    if ((mFlags & kFlagOffset) != 0) {
        static_cast<RndAnimatable::ScaleOffset *>(mAnim->GetFilters().front())->SetOffset(mOffset);
    }
    if ((mFlags & kFlagMin) != 0) {
        static_cast<RndAnimatable::MinMaxLoop *>(mAnim->GetFilters().front())->SetMin(mMin);
    }
    if ((mFlags & kFlagMax) != 0) {
        static_cast<RndAnimatable::MinMaxLoop *>(mAnim->GetFilters().front())->SetMax(mMax);
    }
}
