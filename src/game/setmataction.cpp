#include "game/setmataction.h"

#include "os/debug.h"

namespace {

const char *const kBlendTags[] = {"blend1", "blend2", "blend3"};
const char *const kTexTags[] = {"tex1", "tex2", "tex3"};

} // namespace

SetMatAction::SetMatAction(DataArray *pAction) {
    mMat = FindObject<Rnd::Mat>(pAction, pAction->Sym(1));
    mFlags = 0;
    if (pAction->FindColor("base_color", &mBaseColor, false)) {
        mFlags |= kFlagBaseColor;
    }
    if (pAction->FindColor("light_color", &mLightColor, false)) {
        mFlags |= kFlagLightColor;
    }
    if (pAction->FindColor("edge_color", &mEdgeColor, false)) {
        mFlags |= kFlagEdgeColor;
    }
    if (pAction->FindFloat("alpha", &mAlpha, false)) {
        mFlags |= kFlagAlpha;
    }
    for (int i = 0; i < kNumStages; ++i) {
        if (pAction->FindInt(kBlendTags[i], &mBlend[i], false)) {
            mFlags |= kFlagBlend1 << i;
            CheckStage(pAction, i + 1);
        }
    }
    for (int i = 0; i < kNumStages; ++i) {
        const char *pszTex;
        if (pAction->FindSymbol(kTexTags[i], &pszTex, false)) {
            mTex[i] = FindObject<Rnd::Tex>(pAction, pszTex);
            mFlags |= kFlagTex1 << i;
            CheckStage(pAction, i + 1);
        }
    }
}

inline void SetMatAction::CheckStage(DataArray *pAction, int nStage) {
    if (static_cast<int>(mMat->mStages.size()) >= nStage) {
        return;
    }
    DebugWarn("Mat '%s' doesn't have stage %d (file %s, line %d)",
              pAction->Sym(1),
              nStage,
              pAction->mFile,
              pAction->mLine);
}

void SetMatAction::Exec() {
    if ((mFlags & kFlagLightColor) != 0) {
        mMat->SetLightColor(mLightColor);
    }
    if ((mFlags & kFlagEdgeColor) != 0) {
        mMat->SetEdgeColor(mEdgeColor);
    }
    if ((mFlags & kFlagBaseColor) != 0) {
        mMat->SetBaseColor(mBaseColor);
    }
    if ((mFlags & kFlagAlpha) != 0) {
        mMat->SetAlpha(mAlpha);
    }
    for (int i = 0; i < kNumStages; ++i) {
        if ((mFlags & (kFlagBlend1 << i)) != 0) {
            mMat->mStages[i].mBlend = static_cast<Rnd::Mat::BlendMode>(mBlend[i]);
        }
    }
    for (int i = 0; i < kNumStages; ++i) {
        if ((mFlags & (kFlagTex1 << i)) != 0) {
            mMat->mStages[i].SetTex(mTex[i]);
        }
    }
}
