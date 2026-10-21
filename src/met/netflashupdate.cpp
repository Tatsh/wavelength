#include "met/netflashupdate.h"

#include "os/string.h"
#include "rnd/rndmanager.h"

namespace {

constexpr char kTitlesViewFormat[] = "%s_titles.view";
constexpr char kRefreshTextFormat[] = "%s_refresh.txt";
constexpr char kPanelMeshFormat[] = "%s_panel.mesh";
constexpr char kMatFormat[] = "panel_%s.mat";
constexpr char kHiliteMatFormat[] = "panel_%s_hi.mat";

constexpr int kDefaultMaxFlashes = 4;
constexpr float kDefaultFlashInterval = 30.0f;

template <typename T>
T *FindObject(const char *pszFormat, const char *pszName) {
    return dynamic_cast<T *>(TheManager.Find(FormatString(pszFormat, pszName)));
}

} // namespace

NetFlashUpdate::NetFlashUpdate() {
    mMaxFlashes = kDefaultMaxFlashes;
    mFlashInterval = kDefaultFlashInterval;
    mHilite = 0;
    mFlashing = 0;
    mFlashes = 0;
    mFlashTime = 0.0f;
}

void NetFlashUpdate::StopFlash() {
    mFlashes = 0;
    mFlashing = 0;
    mFlashTime = 0.0f;
    if (mHilite == 1) {
        mPanelMesh->SetMat(mHiliteMat);
    } else {
        mPanelMesh->SetMat(mMat);
    }
}

void NetFlashUpdate::InitFlash(const char *pszPanel, const char *pszKind) {
    mTitles = FindObject<RndView>(kTitlesViewFormat, pszPanel);
    mTitles->SetShowing(true);
    mRefresh = FindObject<RndText>(kRefreshTextFormat, pszPanel);
    mRefresh->SetShowing(false);
    mPanelMesh = FindObject<RndMesh>(kPanelMeshFormat, pszPanel);
    mMat = FindObject<RndMat>(kMatFormat, pszKind);
    mHiliteMat = FindObject<RndMat>(kHiliteMatFormat, pszKind);
}

void NetFlashUpdate::HideFlash() {
    StopFlash();
    mRefresh->SetShowing(false);
    mTitles->SetShowing(true);
}

void NetFlashUpdate::StartFlash(float fTime, bool bHilite) {
    mFlashTime = fTime;
    mFlashing = 1;
    mHilite = bHilite ? 1 : 0;
    mRefresh->SetShowing(true);
    mTitles->SetShowing(false);
}

void NetFlashUpdate::PollFlash(float fTime) {
    if (mFlashing == 0 || mFlashTime == 0.0f || !(mFlashTime + mFlashInterval < fTime)) {
        return;
    }
    ++mFlashes;
    if (mPanelMesh->GetMat() == mMat) {
        mPanelMesh->SetMat(mHiliteMat);
    } else {
        mPanelMesh->SetMat(mMat);
    }
    if (mFlashes < mMaxFlashes) {
        mFlashTime = fTime + mFlashInterval;
    } else {
        mFlashes = 0;
        StopFlash();
    }
}
