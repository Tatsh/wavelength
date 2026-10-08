#include "app/hudstick.h"

#include "os/string.h"
#include "rnd/manager.h"

namespace {

// The row of a transform that holds the translation.
constexpr int kXfmRowTranslation = 3;

// The components of a row of a transform.
enum XfmColumn { kXfmColumnX = 0, kXfmColumnY = 1, kXfmColumnZ = 2 };

} // namespace

HudStick::HudStick(char chHud, Rnd::Animatable *pAnims, Rnd::View *pParent) {
    mMesh =
        dynamic_cast<Rnd::Mesh *>(Rnd::TheManager.Find(FormatString("HUD%cr_stick.mesh", chHud)));
    mMats[kDirectionInOut] = dynamic_cast<Rnd::Mat *>(Rnd::TheManager.Find("HUD in_out.mat"));
    mMats[kDirectionUpDown] = dynamic_cast<Rnd::Mat *>(Rnd::TheManager.Find("HUD up_dn.mat"));
    pAnims->AddAnim(dynamic_cast<Rnd::Animatable *>(Rnd::TheManager.Find("HUD in_out.mnm")));
    pAnims->AddAnim(dynamic_cast<Rnd::Animatable *>(Rnd::TheManager.Find("HUD up_dn.mnm")));
    pParent->AddTrans(mMesh);
    Reset();
}

void HudStick::Reset() {
    Hide();
}

void HudStick::Hide() {
    mMesh->SetShowing(false);
}

void HudStick::Show(int nDirection, const Vector2 *pPosition) {
    float translation[Rnd::kXfmRowFloatCount]; // Yes, the binary copies the unset fourth word.
    translation[kXfmColumnX] = pPosition->x;
    translation[kXfmColumnY] = 0.0f;
    translation[kXfmColumnZ] = pPosition->y;
    mMesh->mDirty = 1;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wuninitialized"
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
    for (int i = 0; i < Rnd::kXfmRowFloatCount; ++i) {
        mMesh->mLocalXfm[kXfmRowTranslation][i] = translation[i];
    }
#pragma GCC diagnostic pop
    mMesh->SetShowing(true);
    mMesh->SetMat(mMats[nDirection]);
}
