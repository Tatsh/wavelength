#include "met/solowinstatspanel.h"

#include "game/gamedb.h"
#include "game/playerprofile.h"
#include "game/songrecord.h"
#include "met/metagameutil.h"
#include "os/locale.h"
#include "os/string.h"
#include "rnd/manager.h"
#include "rnd/mesh.h"
#include "rnd/view.h"

namespace {

constexpr char kScoreLabel[] = "score";
constexpr char kBlastedLabel[] = "blasted";
constexpr char kFullMixLabel[] = "full_mix";
constexpr char kStreakLabel[] = "streak";
constexpr char kScoreBestTitle[] = "score_hi_title";
constexpr char kBlastedBestTitle[] = "blasted_hi_title";
constexpr char kFullMixBestTitle[] = "full_mix_hi_title";
constexpr char kScoreBestLabel[] = "score_hi";
constexpr char kBlastedBestLabel[] = "blasted_hi";
constexpr char kFullMixBestLabel[] = "full_mix_hi";

constexpr char kNumberFormat[] = "%d";
constexpr char kPercentFormat[] = "%d%%";
constexpr char kStreakFormat[] = "%d ";
constexpr char kFullMixToken[] = "solo_stats_full_mix";
constexpr char kNoText[] = "";
constexpr char kGradeMeshFormat[] = "%s_grade.mesh";
constexpr char kViewFormat[] = "%s.view";

constexpr float kPercent = 100.0f;

// The share of the song played that earns a medal.
constexpr int kFullSong = 100;

} // namespace

SoloWinStatsPanel::SoloWinStatsPanel(DataArray *pData, const char *pszDir)
    : SoloGameStatsPanel(pData, pszDir), mReveal() {
}

void SoloWinStatsPanel::Unload() {
    FreqPanel::Unload();
    mReveal.Clear();
}

void SoloWinStatsPanel::Exit(bool bForce, float fTime) {
    FreqPanel::Exit(bForce, fTime);
    mReveal.Stop();
}

void SoloWinStatsPanel::Refresh() {
    SoloGameStatsPanel::Refresh();
    SongRecord record;
    TheGameDb->GetProfile(0)->GetSongRecord(
        TheGameDb->mSong.c_str(), TheGameDb->mSkillLevel, &record);

    SetLabel(kScoreLabel, FormatString(kNumberFormat, TheGameDb->GetPlayerScore(0)));
    SetLabel(kBlastedLabel,
             FormatString(kPercentFormat, static_cast<int>(TheGameDb->GetEnergized() * kPercent)));
    const char *pszFullMixFormat = TheLocale.Localize(kFullMixToken, true);
    SetLabel(kFullMixLabel, FormatString(pszFullMixFormat, TheGameDb->GetFullMixBars()));
    SetLabel(kStreakLabel, FormatString(kStreakFormat, TheGameDb->GetBestStreak()));

    if (TheGameDb->mLoadRemix) {
        SetLabel(kScoreBestTitle, kNoText);
        SetLabel(kBlastedBestTitle, kNoText);
        SetLabel(kFullMixBestTitle, kNoText);
        SetLabel(kScoreBestLabel, kNoText);
        SetLabel(kBlastedBestLabel, kNoText);
        SetLabel(kFullMixBestLabel, kNoText);
    } else {
        SetLabel(kScoreBestLabel, FormatString(kNumberFormat, record.mScore));
        SetLabel(kBlastedBestLabel, FormatString(kPercentFormat, record.mBlasted));
        SetLabel(kFullMixBestLabel, FormatString(pszFullMixFormat, record.mFullMixBars));
    }

    const bool bShowGrade = !TheGameDb->mLoadRemix;
    Rnd::Mesh *pGrade =
        dynamic_cast<Rnd::Mesh *>(Rnd::TheManager.Find(FormatString(kGradeMeshFormat, mName)));
    pGrade->SetShowing(bShowGrade);
    if (bShowGrade) {
        PlayerProfile *pProfile = TheGameDb->GetProfile(0);
        const int nMedal = pProfile->GetMedalForScore(TheGameDb->mSong.c_str(),
                                                      TheGameDb->mSkillLevel,
                                                      TheGameDb->GetPlayerScore(0),
                                                      kFullSong);
        pGrade->SetMat(FindGradeMaterial(nMedal, true));
    }

    mReveal.SetAnim(
        dynamic_cast<Rnd::View *>(Rnd::TheManager.Find(FormatString(kViewFormat, mName))));
}

void SoloWinStatsPanel::Poll(float fTime) {
    FreqPanel::Poll(fTime);
    mReveal.Poll(fTime);
}

void SoloWinStatsPanel::StartAnim(float fTime) {
    mReveal.Start(fTime);
}
