#include "met/avatarpanel.h"

#include "game/avatarcam.h"
#include "met/metagameutil.h"
#include "os/joypad.h"
#include "os/string.h"
#include "rnd/manager.h"

namespace {

// The controller and the stick that turn the Freq.
constexpr int kSpinPad = 0;
constexpr int kSpinStick = 1;

// The share of the distance to the stick the eased position covers each frame is its inverse.
constexpr float kSpinEasing = 5.0f;

// The stick position turns the Freq by twice as many radians.
constexpr float kSpinTurnScale = 2.0f;

} // namespace

AvatarPanel::AvatarPanel(DataArray *pData, const char *pszDir) : FreqPanel(pData, pszDir) {
    mSpinControl = 0;
    mAvatar = nullptr;
    mSpinX = 0.0f;
    mSpinY = 0.0f;
    mData->FindBool("spin_control", &mSpinControl, false);
}

AvatarPanel::~AvatarPanel() {
}

void AvatarPanel::FinishLoad() {
    if (mLoaded) {
        return;
    }
    FreqPanel::FinishLoad();
    mPostView =
        dynamic_cast<Rnd::View *>(Rnd::TheManager.Find(FormatString("%s_post_avatar.view", mName)));
    mView->RemoveDraw(mPostView);
    mFreqMesh =
        dynamic_cast<Rnd::Mesh *>(Rnd::TheManager.Find(FormatString("%s_freq.mesh", mName)));
}

void AvatarPanel::Draw() {
    FreqPanel::Draw();
    if (mAvatar != nullptr) {
        if (mSpinControl != 0 && mState == kStateShown) {
            const JoypadStick &stick = JoypadGetState(kSpinPad)->mSticks[kSpinStick];
            mSpinX += (stick.mX - mSpinX) / kSpinEasing;
            mSpinY += (stick.mY - mSpinY) / kSpinEasing;
            SetAvatarViewAngles(0.0f, mSpinX * kSpinTurnScale);
        }
        DrawAvatarOnMesh(mAvatar, mFreqMesh);
    }
    mPostView->Draw();
}

void AvatarPanel::Enter(bool bForce, float fTime) {
    FreqPanel::Enter(bForce, fTime);
    SetAvatarViewAngles(0.0f, 0.0f);
    mSpinX = 0.0f;
    mSpinY = 0.0f;
}

void AvatarPanel::Exit(bool bForce, float fTime) {
    FreqPanel::Exit(bForce, fTime);
    if (mAvatar != nullptr) {
        mAvatar->ReleasePlayer();
        mAvatar = nullptr;
    }
    SetAvatarViewAngles(0.0f, 0.0f);
    mSpinX = 0.0f;
    mSpinY = 0.0f;
}

void AvatarPanel::SetAvatar(AvatarPartSet *pAvatar) {
    if (pAvatar == mAvatar) {
        return;
    }
    if (mAvatar != nullptr) {
        mAvatar->ReleasePlayer();
    }
    mAvatar = pAvatar;
}
