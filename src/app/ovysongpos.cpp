#include "app/ovysongpos.h"

#include <algorithm>

#include "app/overlay.h"
#include "game/gamedb.h"
#include "os/string.h"
#include "rnd/rndmanager.h"

namespace {

// The places of the ends of the bar.
constexpr float kStart = 0.0f;
constexpr float kEnd = 1.0f;

// The animation of the future part runs in milliseconds.
constexpr float kMsPerSecond = 1000.0f;

// The head-up display prefix of an online game that is not a remix.
constexpr char kOnlineHudPrefix[] = "HUD1";

} // namespace

// Yes, the binary leaves the fourth words of these places at zero.
Vector3 OvySongPos::sStartPos{0.0f, -90.0f, -3.7e-5f, 0.0f};
Vector3 OvySongPos::sEndPos{0.0f, 149.0f, -3.7e-5f, 0.0f};
int OvySongPos::sNumLabels = 0;

OvySongPos::OvySongPos(RndView *pHudView) : HideablePanel(nullptr, nullptr, false) {
    bool bLetterbox = false;
    const char *pszHud;
    if (TheGameDb->GetCommunity() == GameDb::kCommunityOnline &&
        TheGameDb->GetRuleSet() != GameDb::kRuleSetRemix) {
        bLetterbox = true;
        pszHud = kOnlineHudPrefix;
    } else {
        pszHud = Overlay::sHudPrefix;
    }
    const char *pszSlide;
    if (TheGameDb->GetRuleSet() == GameDb::kRuleSetRemix &&
        TheGameDb->GetCommunity() != GameDb::kCommunitySolo) {
        pszSlide = "HUDmr songpos.tnm";
    } else {
        pszSlide = FormatString("%s songpos.tnm", pszHud);
    }
    SetObjects(pszSlide, nullptr, false);
    mView = dynamic_cast<RndView *>(TheManager.Find(FormatString("%s songpos.view", pszHud)));
    mCheckpointMesh =
        dynamic_cast<RndMesh *>(TheManager.Find(FormatString("%s songpos chpt.mesh", pszHud)));
    mLabelTemplate = dynamic_cast<RndText *>(TheManager.Find("HUDr songpos chpt.txt"));
    mFuture =
        dynamic_cast<RndMesh *>(TheManager.Find(FormatString("%s songpos future.mesh", pszHud)));
    mShip =
        dynamic_cast<RndDrawable *>(TheManager.Find(FormatString("%s songpos ship.mesh", pszHud)));
    mFutureAnim = dynamic_cast<RndAnimatable *>(
        TheManager.Find(FormatString("%s songpos future.msnm", pszHud)));
    mCheckpointMesh->RemoveDraw(mLabelTemplate);
    mCheckpointMesh->RemoveTrans(mLabelTemplate);
    mLabelTemplate->SetText("");
    if (TheGameDb->GetRuleSet() != GameDb::kRuleSetRemix) {
        mLabelTemplate = nullptr;
    }
    pHudView->RemoveDraw(mView);
    mView->RemoveDraw(mCheckpointMesh);
    mView->RemoveDraw(mShip);
    mView->SetShowing(true);
    if (bLetterbox) {
        auto *pLetterbox = dynamic_cast<RndTransformable *>(
            TheManager.Find(FormatString("%s letterbox scale all.view", Overlay::sHudPrefix)));
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
    it->mText.reset(dynamic_cast<RndText *>(TheManager.Create(
        RndText::sClassName, FormatString("HUDr songpos ckpt %0xd.txt", ++sNumLabels))));
    it->mText->Copy(mLabelTemplate, 0);
    it->mText->SetText(pszLabel);
}

void OvySongPos::ClearCheckpoints() {
    mCheckpoints.clear();
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
    if (mSlide.GetValue() == 0.0f) {
        return;
    }
    static_cast<RndDrawable *>(mView)->Draw();
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
        mCheckpointMesh->mDirty = 1;
        mCheckpointMesh->mLocalXfm.mTranslation = position;
        mCheckpointMesh->UpdateWorldXfm(mFuture, 0);
        mCheckpointMesh->Draw();
        if (checkpoint.mText != nullptr) {
            checkpoint.mText->UpdateWorldXfm(mCheckpointMesh, 1);
            checkpoint.mText->Draw();
        }
    }
    mShip->Draw();
}
