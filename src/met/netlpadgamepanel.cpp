#include "met/netlpadgamepanel.h"

#include <cstdio>
#include <cstring>

#include "game/gamedb.h"
#include "game/songentry.h"
#include "met/metagameutil.h"
#include "netflow/netlaunchpad.h"
#include "os/locale.h"
#include "os/string.h"
#include "rnd/rndfont.h"
#include "rnd/rndmanager.h"
#include "rnd/rndmesh.h"
#include "rnd/rndtext.h"
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
    return dynamic_cast<T *>(TheManager.Find(pszName));
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
            FindObject<RndMesh>(FormatString(kColorMeshFormat, pszPanel, nRow))->SetShowing(false);
            name = FormatString(kNameTextFormat, pszPanel, nRow);
            FindObject<RndText>(name.c_str())->SetText(kNoText);
        } else {
            name = FormatString(kNameTextFormat, pszPanel, nRow);
            RndText *pName = FindObject<RndText>(name.c_str());
            pName->SetText(player->mPlayer.mName.c_str());
            pName->SetFont(
                FindObject<RndFont>(player->mReady == kReady ? kReadyFont : kNotReadyFont));
            FindObject<RndMesh>(FormatString(kRankMeshFormat, pszPanel, nRow))
                ->SetMat(FindRankMaterial(player->mPlayer.mRankIcon));

            name = FormatString(kRankTextFormat, pszPanel, nRow);
            RndText *pRank = FindObject<RndText>(name.c_str());
            char szRank[kRankTextSize];
            if (player->mPlayer.IsNewbie()) {
                std::strcpy(szRank, TheLocale.Localize(kNewbieToken, true));
            } else {
                std::sprintf(szRank, kRankFormat, player->mPlayer.mRank);
            }
            pRank->SetText(FormatString(TheLocale.Localize(kRankToken, true), szRank));

            RndMesh *pColor = FindObject<RndMesh>(FormatString(kColorMeshFormat, pszPanel, nRow));
            const char *pszColor = GetPlayerColorName(player->mColor);
            pColor->SetShowing(true);
            pColor->SetMat(FindColorMaterial(pszColor));
        }
        if (player != pPlayers->end()) {
            ++player;
        }
    }
}

void NetLPadGamePanel::RefreshGame() {
    SetBandPicture(TheGameDb->GetSong().c_str(), true, false, false);
    const SongEntry entry{TheGameDb->FindSong(TheGameDb->GetSong().c_str())};
    FindComponent(kGenreComponent, false)->SetText(entry.GetGenreName());

    UIComponent *pBpm = FindComponent(kBpmComponent, false);
    if (TheGameDb->GetLoadRemix() != 0) {
        pBpm->SetText(kNoText);
    } else {
        pBpm->SetText(
            FormatString(TheLocale.Localize(kBpmToken, true), static_cast<int>(entry.GetBpm())));
    }

    const char *pszMode = TheGameDb->GetModeName(TheGameDb->GetRuleSet());
    const char *pszDifficulty =
        TheGameDb->GetDifficultyName(TheGameDb->GetSkillLevel(), TheGameDb->GetRuleSet());
    UIComponent *pMode = FindComponent(kModeComponent, false);
    if (TheGameDb->GetRuleSet() == GameDb::kRuleSetRemix) {
        pMode->SetText(FormatString(kRemixModeFormat, pszMode));
    } else {
        pMode->SetText(FormatString(kModeFormat, pszMode, pszDifficulty));
    }

    UIComponent *pRemixName = FindComponent(kRemixNameComponent, false);
    if (TheGameDb->GetLoadRemix() != 0) {
        pRemixName->SetText(TheGameDb->GetRemixName().c_str());
    } else {
        pRemixName->SetText(kNoText);
    }

    UIComponent *pPowerup = FindComponent(kPowerupComponent, false);
    const String format(TheLocale.Localize(kPowerupToken, true));
    if (TheGameDb->GetRuleSet() == GameDb::kRuleSetGame) {
        pPowerup->SetText(
            FormatString(format.c_str(), TheGameDb->GetPowerupName(TheGameDb->GetPowerupLevel())));
    } else {
        pPowerup->SetText(FormatString(format.c_str(), TheLocale.Localize(kNoPowerupToken, true)));
    }

    FindComponent(kReadOnlyComponent, false)
        ->SetShowing(TheGameDb->GetLoadRemix() != 0 && TheGameDb->GetRemixReadOnly() != 0);
}
