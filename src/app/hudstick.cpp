#include "app/hudstick.h"

#include "os/string.h"
#include "rnd/rndmanager.h"

HudStick::HudStick(char chHud, RndAnimatable *pAnims, RndView *pParent) {
    mMesh = dynamic_cast<RndMesh *>(TheManager.Find(FormatString("HUD%cr_stick.mesh", chHud)));
    mMats[kDirectionInOut] = dynamic_cast<RndMat *>(TheManager.Find("HUD in_out.mat"));
    mMats[kDirectionUpDown] = dynamic_cast<RndMat *>(TheManager.Find("HUD up_dn.mat"));
    pAnims->AddAnim(dynamic_cast<RndAnimatable *>(TheManager.Find("HUD in_out.mnm")));
    pAnims->AddAnim(dynamic_cast<RndAnimatable *>(TheManager.Find("HUD up_dn.mnm")));
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
    mMesh->mDirty = 1;
    // The binary also copies an unset fourth word, which is left as it was here.
    mMesh->mLocalXfm.mTranslation.x = pPosition->x;
    mMesh->mLocalXfm.mTranslation.y = 0.0f;
    mMesh->mLocalXfm.mTranslation.z = pPosition->y;
    mMesh->SetShowing(true);
    mMesh->SetMat(mMats[nDirection]);
}
