#include "met/sololosetipspanel.h"

#include <cstring>

#include "game/gamedb.h"
#include "math/rand.h"
#include "os/debug.h"
#include "os/locale.h"
#include "os/system.h"
#include "rnd/manager.h"
#include "ui/uimanager.h"

namespace {

constexpr char kMetagameEntry[] = "metagame";
constexpr char kLoseTipsEntry[] = "lose_tips";
constexpr char kFillerEntry[] = "filler";
constexpr char kProgressEntry[] = "percent_done";
constexpr char kBadConditionFormat[] = " Bad lose tip condition: %s";
constexpr char kAlwaysItem[] = "always";
constexpr char kTipPanel[] = "s_g_end_lose_tip";
constexpr char kTipComponent[] = "tip";
constexpr char kTipView[] = "s_g_end_lose_tip.view";

constexpr int kLocalPlayer = 0;
constexpr int kAnySkillLevel = 4;

// The layout of a `percent_done` entry.
constexpr int kProgressSkillLevels = 1;
constexpr int kProgressLow = 2;
constexpr int kProgressHigh = 3;
constexpr int kProgressFirstTip = 4;

// The layout of a tip.
constexpr int kTipItem = 0;
constexpr int kTipToken = 1;

constexpr int kFirstFillerTip = 1;
constexpr int kFirstCondition = 1;

} // namespace

SoloLoseTipsPanel::SoloLoseTipsPanel(DataArray *pData, const char *pszDir)
    : FreqPanel(pData, pszDir), mProgressTipNext(1) {
}

void SoloLoseTipsPanel::Enter(bool bForce, float fTime) {
    DataArray *pTips =
        SystemConfig()->FindArray(kMetagameEntry, true)->FindArray(kLoseTipsEntry, true);
    const char *pszToken = PickTip(pTips);
    mTip = TheUI.FindComponent(kTipPanel, kTipComponent, false);
    mTip->SetText(TheLocale.Localize(pszToken, true));
    mTipAnim.SetAnim(dynamic_cast<Rnd::View *>(Rnd::TheManager.Find(kTipView)));
    mTipAnim.Start(fTime + mEnterMs);
    FreqPanel::Enter(bForce, fTime);
    mTip->SetShowing(false);
}

void SoloLoseTipsPanel::Unload() {
    FreqPanel::Unload();
    mTipAnim.Clear();
}

void SoloLoseTipsPanel::Poll(float fTime) {
    FreqPanel::Poll(fTime);
    mTipAnim.Poll(fTime);
    if (mState == kStateShown && !mTipAnim.IsPlaying()) {
        mTip->SetShowing(true);
    }
}

const char *SoloLoseTipsPanel::PickTip(DataArray *pTips) {
    const int nSkillLevel = TheGameDb->mSkillLevel;
    if (mProgressTipNext == 0) {
        return PickFillerTip(pTips->FindArray(kFillerEntry, true), nSkillLevel);
    }
    if (mProgressTipNext != 1) {
        return nullptr;
    }

    const float fProgress = TheGameDb->GetProgress();
    for (int i = kFirstCondition; i < pTips->Size(); ++i) {
        DataArray *pEntry = pTips->Array(i);
        const char *pszCondition = pEntry->Sym(0);
        if (std::strcmp(pszCondition, kProgressEntry) == 0) {
            const char *pszToken = PickProgressTip(pEntry, nSkillLevel, fProgress);
            if (pszToken != nullptr) {
                return pszToken;
            }
        } else if (std::strcmp(pszCondition, kFillerEntry) != 0) {
            DebugWarn(kBadConditionFormat, pszCondition);
        }
    }
    return nullptr;
}

const char *SoloLoseTipsPanel::PickFillerTip(DataArray *pFiller, int nSkillLevel) {
    DataArray *pTip;
    do {
        pTip = pFiller->Array(RandomInt(kFirstFillerTip, pFiller->Size()));
        if (IsTipAvailable(pTip, nSkillLevel)) {
            mProgressTipNext = 1;
        } else {
            pTip = nullptr;
        }
    } while (pTip == nullptr);
    return pTip->Sym(kTipToken);
}

const char *
SoloLoseTipsPanel::PickProgressTip(DataArray *pEntry, int nSkillLevel, float fProgress) {
    DataArray *pSkillLevels = pEntry->Array(kProgressSkillLevels);
    bool bMatch;
    if (pSkillLevels->Int(0) == kAnySkillLevel) {
        bMatch = true;
    } else {
        int i = 0;
        while (i < pSkillLevels->Size() && pSkillLevels->Int(i) != nSkillLevel) {
            ++i;
        }
        bMatch = i != pSkillLevels->Size();
    }
    if (!bMatch || !(pEntry->Float(kProgressLow) <= fProgress) ||
        !(fProgress <= pEntry->Float(kProgressHigh))) {
        return nullptr;
    }

    DataArray *pTip;
    do {
        pTip = pEntry->Array(RandomInt(kProgressFirstTip, pEntry->Size()));
        if (IsTipAvailable(pTip, nSkillLevel)) {
            mProgressTipNext = 0;
        } else {
            pTip = nullptr;
        }
    } while (pTip == nullptr);
    return pTip->Sym(kTipToken);
}

bool SoloLoseTipsPanel::IsTipAvailable(DataArray *pTip, int nSkillLevel) {
    if (std::strcmp(pTip->Sym(kTipItem), kAlwaysItem) == 0) {
        return true;
    }
    return TheGameDb->GetProfile(kLocalPlayer)->IsUnlocked(pTip->Sym(kTipItem), nSkillLevel);
}
