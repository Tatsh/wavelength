#include "met/netlpadgamepanel.h"

#include <cstdio>
#include <cstring>

#include "game/gamedb.h"
#include "game/songentry.h"
#include "met/metagameutil.h"
#include "netflow/netlaunchpad.h"
#include "os/locale.h"
#include "os/string.h"
#include "rnd/font.h"
#include "rnd/manager.h"
#include "rnd/mesh.h"
#include "rnd/text.h"
#include "ui/uicomponent.h"

namespace {

constexpr char kNameTextFormat[] = "%s_0%d.txt";
constexpr char kRankMeshFormat[] = "%s_r_0%d.mesh";
constexpr char kRankTextFormat[] = "%s_rank_0%d.txt";
constexpr char kColorMeshFormat[] = "%s_0%dc.mesh";
constexpr char kReadyFont[] = "neuro_1_drop.font";
constexpr char kNotReadyFont[] = "neuro_1_drop_grey.font";
constexpr char kNewbieToken[] = "newbie";
constexpr char kRankFormat[] = "%d";
constexpr char kRankToken[] = "fn_h_lpad_d_rank_01";

constexpr char kGenreComponent[] = "genre";
constexpr char kBpmComponent[] = "bpm";
constexpr char kBpmToken[] = "JUST_BPM";
constexpr char kModeComponent[] = "mode";
constexpr char kRemixModeFormat[] = "%s";
constexpr char kModeFormat[] = "%s: %s";
constexpr char kRemixNameComponent[] = "remix_name";
constexpr char kPowerupComponent[] = "powerup";
constexpr char kPowerupToken[] = "NET_LPAD_POWERUP";
constexpr char kNoPowerupToken[] = "powerup_na";
constexpr char kReadOnlyComponent[] = "read_only";
constexpr char kNoText[] = "";

constexpr int kReady = 1;
constexpr int kRankTextSize = 32;

template <typename T>
T *FindObject(const char *pszName) {
    return dynamic_cast<T *>(Rnd::TheManager.Find(pszName));
}

} // namespace

void NetLPadGamePanel::Enter(bool bForce, float fTime) {
    FreqPanel::Enter(bForce, fTime);
    RefreshGame();
}

void NetLPadGamePanel::Update(std::list<NetLaunchpadPlayer> *pPlayers) {
    ShowPlayers(mName, pPlayers);
}

void NetLPadGamePanel::ShowPlayers(const char *pszPanel, std::list<NetLaunchpadPlayer> *pPlayers) {
    if (TheNetLaunchpad == nullptr) {
        return;
    }

    auto player = pPlayers->begin();
    for (int nRow = 1; nRow <= kNumPlayerRows; ++nRow) {
        String name;
        if (pPlayers->size() < static_cast<unsigned int>(nRow)) {
            FindObject<Rnd::Mesh>(FormatString(kColorMeshFormat, pszPanel, nRow))
                ->SetShowing(false);
            name = FormatString(kNameTextFormat, pszPanel, nRow);
            FindObject<Rnd::Text>(name.c_str())->SetText(kNoText);
        } else {
            name = FormatString(kNameTextFormat, pszPanel, nRow);
            Rnd::Text *pName = FindObject<Rnd::Text>(name.c_str());
            pName->SetText(player->mPlayer.mName.c_str());
            pName->SetFont(
                FindObject<Rnd::Font>(player->mReady == kReady ? kReadyFont : kNotReadyFont));
            FindObject<Rnd::Mesh>(FormatString(kRankMeshFormat, pszPanel, nRow))
                ->SetMat(FindRankMaterial(player->mPlayer.mRankIcon));

            name = FormatString(kRankTextFormat, pszPanel, nRow);
            Rnd::Text *pRank = FindObject<Rnd::Text>(name.c_str());
            char szRank[kRankTextSize];
            if (player->mPlayer.IsNewbie()) {
                std::strcpy(szRank, TheLocale.Localize(kNewbieToken, true));
            } else {
                std::sprintf(szRank, kRankFormat, player->mPlayer.mRank);
            }
            pRank->SetText(FormatString(TheLocale.Localize(kRankToken, true), szRank));

            Rnd::Mesh *pColor =
                FindObject<Rnd::Mesh>(FormatString(kColorMeshFormat, pszPanel, nRow));
            const char *pszColor = GetPlayerColorName(player->mDifficulty);
            pColor->SetShowing(true);
            pColor->SetMat(FindColorMaterial(pszColor));
        }
        if (player != pPlayers->end()) {
            ++player;
        }
    }
}

void NetLPadGamePanel::RefreshGame() {
    SetBandPicture(TheGameDb->mSong.c_str(), true, false, false);
    const SongEntry entry{TheGameDb->FindSong(TheGameDb->mSong.c_str())};
    FindComponent(kGenreComponent, false)->SetText(entry.GetGenreName());

    UIComponent *pBpm = FindComponent(kBpmComponent, false);
    if (TheGameDb->mLoadRemix != 0) {
        pBpm->SetText(kNoText);
    } else {
        pBpm->SetText(
            FormatString(TheLocale.Localize(kBpmToken, true), static_cast<int>(entry.GetBpm())));
    }

    const char *pszMode = TheGameDb->GetModeName(TheGameDb->mRuleSet);
    const char *pszDifficulty =
        TheGameDb->GetDifficultyName(TheGameDb->mSkillLevel, TheGameDb->mRuleSet);
    UIComponent *pMode = FindComponent(kModeComponent, false);
    if (TheGameDb->mRuleSet == GameDb::kRuleSetRemix) {
        pMode->SetText(FormatString(kRemixModeFormat, pszMode));
    } else {
        pMode->SetText(FormatString(kModeFormat, pszMode, pszDifficulty));
    }

    UIComponent *pRemixName = FindComponent(kRemixNameComponent, false);
    if (TheGameDb->mLoadRemix != 0) {
        pRemixName->SetText(TheGameDb->mRemixName.c_str());
    } else {
        pRemixName->SetText(kNoText);
    }

    UIComponent *pPowerup = FindComponent(kPowerupComponent, false);
    const String format(TheLocale.Localize(kPowerupToken, true));
    if (TheGameDb->mRuleSet == GameDb::kRuleSetGame) {
        pPowerup->SetText(
            FormatString(format.c_str(), TheGameDb->GetPowerupName(TheGameDb->mPowerupLevel)));
    } else {
        pPowerup->SetText(FormatString(format.c_str(), TheLocale.Localize(kNoPowerupToken, true)));
    }

    FindComponent(kReadOnlyComponent, false)
        ->SetShowing(TheGameDb->mLoadRemix != 0 && TheGameDb->mRemixReadOnly != 0);
}
