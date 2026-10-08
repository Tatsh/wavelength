#include "app/ovysongpos.h"

#include <algorithm>

#include "app/overlay.h"
#include "game/gamedb.h"
#include "os/string.h"
#include "rnd/manager.h"

namespace {

// The row of a transform that holds the translation.
constexpr int kXfmRowTranslation = 3;

// The places of the ends of the bar.
constexpr float kStart = 0.0f;
constexpr float kEnd = 1.0f;

// The animation of the future part runs in milliseconds.
constexpr float kMsPerSecond = 1000.0f;

// The head-up display prefix of an online game that is not a remix.
constexpr char kOnlineHudPrefix[] = "HUD1";

} // namespace

Vector3 OvySongPos::sStartPos{0.0f, -90.0f, -3.7e-5f};
Vector3 OvySongPos::sEndPos{0.0f, 149.0f, -3.7e-5f};
int OvySongPos::sNumLabels = 0;

OvySongPos::OvySongPos(Rnd::View *pHudView) : HideablePanel(nullptr, nullptr, false) {
    bool bLetterbox = false;
    const char *pszHud;
    if (TheGameDb->mCommunity == GameDb::kCommunityOnline &&
        TheGameDb->mRuleSet != GameDb::kRuleSetRemix) {
        bLetterbox = true;
        pszHud = kOnlineHudPrefix;
    } else {
        pszHud = Overlay::sHudPrefix;
    }
    const char *pszSlide;
    if (TheGameDb->mRuleSet == GameDb::kRuleSetRemix &&
        TheGameDb->mCommunity != GameDb::kCommunitySolo) {
        pszSlide = "HUDmr songpos.tnm";
    } else {
        pszSlide = FormatString("%s songpos.tnm", pszHud);
    }
    SetObjects(pszSlide, nullptr, false);
    mView =
        dynamic_cast<Rnd::View *>(Rnd::TheManager.Find(FormatString("%s songpos.view", pszHud)));
    mCheckpointMesh = dynamic_cast<Rnd::Mesh *>(
        Rnd::TheManager.Find(FormatString("%s songpos chpt.mesh", pszHud)));
    mLabelTemplate = dynamic_cast<Rnd::Text *>(Rnd::TheManager.Find("HUDr songpos chpt.txt"));
    mFuture = dynamic_cast<Rnd::Mesh *>(
        Rnd::TheManager.Find(FormatString("%s songpos future.mesh", pszHud)));
    mShip = dynamic_cast<Rnd::Drawable *>(
        Rnd::TheManager.Find(FormatString("%s songpos ship.mesh", pszHud)));
    mFutureAnim = dynamic_cast<Rnd::Animatable *>(
        Rnd::TheManager.Find(FormatString("%s songpos future.msnm", pszHud)));
    mCheckpointMesh->RemoveDraw(mLabelTemplate);
    mCheckpointMesh->RemoveTrans(mLabelTemplate);
    mLabelTemplate->SetText("");
    if (TheGameDb->mRuleSet != GameDb::kRuleSetRemix) {
        mLabelTemplate = nullptr;
    }
    pHudView->RemoveDraw(mView);
    mView->RemoveDraw(mCheckpointMesh);
    mView->RemoveDraw(mShip);
    mView->SetShowing(true);
    if (bLetterbox) {
        auto *pLetterbox = dynamic_cast<Rnd::Transformable *>(
            Rnd::TheManager.Find(FormatString("%s letterbox scale all.view", Overlay::sHudPrefix)));
        pLetterbox->AddTrans(mView);
    }
    Reset();
}

void OvySongPos::AddCheckpoint(float fPos, const char *pszLabel) {
    auto it = std::lower_bound(mCheckpoints.begin(), mCheckpoints.end(), fPos);
    if (it != mCheckpoints.end() && it->mPos == fPos) {
        return;
    }
    it = mCheckpoints.insert(it, Section{fPos, nullptr});
    if (pszLabel == nullptr || mLabelTemplate == nullptr) {
        return;
    }
    it->mText = dynamic_cast<Rnd::Text *>(Rnd::TheManager.Create(
        Rnd::g_textClassName.mStr, FormatString("HUDr songpos ckpt %0xd.txt", ++sNumLabels)));
    it->mText->Copy(mLabelTemplate, 0);
    it->mText->SetText(pszLabel);
}

void OvySongPos::ClearCheckpoints() {
    if (mLabelTemplate == nullptr) {
        mCheckpoints.clear();
        return;
    }
    for (auto it = mCheckpoints.begin(); it != mCheckpoints.end();) {
        delete it->mText;
        it->mText = nullptr;
        it = mCheckpoints.erase(it);
    }
}

void OvySongPos::Reset() {
    ClearCheckpoints();
    SetFuture(0.0f);
}

void OvySongPos::SetFuture(float fFuture) {
    mFutureAnim->SetFrame(fFuture * kMsPerSecond);
}

void OvySongPos::Poll() {
    HideablePanel::Poll();
}

void OvySongPos::Draw() {
    if (mSlide.mValue == 0.0f) {
        return;
    }
    static_cast<Rnd::Drawable *>(mView)->Draw();
    for (const Section &checkpoint : mCheckpoints) {
        Vector3 position;
        if (checkpoint.mPos == kStart) {
            position = sStartPos;
        } else if (checkpoint.mPos == kEnd) {
            position = sEndPos;
        } else {
            const float fRest = kEnd - checkpoint.mPos;
            position = sEndPos;
            position.x = sEndPos.x * checkpoint.mPos + sStartPos.x * fRest;
            position.y = sEndPos.y * checkpoint.mPos + sStartPos.y * fRest;
            position.z = sEndPos.z * checkpoint.mPos + sStartPos.z * fRest;
        }
        float (&translation)[Rnd::kXfmRowFloatCount] =
            mCheckpointMesh->mLocalXfm[kXfmRowTranslation];
        translation[0] = position.x;
        translation[1] = position.y;
        translation[2] = position.z;
        translation[3] = position.w;
        mCheckpointMesh->mDirty = 1;
        mCheckpointMesh->UpdateWorldXfm(mFuture, 0);
        mCheckpointMesh->Draw();
        if (checkpoint.mText != nullptr) {
            checkpoint.mText->UpdateWorldXfm(mCheckpointMesh, 1);
            checkpoint.mText->Draw();
        }
    }
    mShip->Draw();
}
