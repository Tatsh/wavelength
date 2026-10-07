#include "met/netflashupdate.h"

#include "os/string.h"
#include "rnd/manager.h"

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
    return dynamic_cast<T *>(Rnd::TheManager.Find(FormatString(pszFormat, pszName)));
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
    mTitles = FindObject<Rnd::View>(kTitlesViewFormat, pszPanel);
    mTitles->SetShowing(true);
    mRefresh = FindObject<Rnd::Text>(kRefreshTextFormat, pszPanel);
    mRefresh->SetShowing(false);
    mPanelMesh = FindObject<Rnd::Mesh>(kPanelMeshFormat, pszPanel);
    mMat = FindObject<Rnd::Mat>(kMatFormat, pszKind);
    mHiliteMat = FindObject<Rnd::Mat>(kHiliteMatFormat, pszKind);
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
    if (mPanelMesh->mMat == mMat) {
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
