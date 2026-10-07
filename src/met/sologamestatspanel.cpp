#include "met/sologamestatspanel.h"

#include "game/gamedb.h"
#include "game/songentry.h"
#include "rnd/text.h"
#include "ui/uilabel.h"

namespace {

constexpr char kBandLabel[] = "band";
constexpr char kSongLabel[] = "song";
constexpr char kSkillLabel[] = "skill";

} // namespace

SoloGameStatsPanel::SoloGameStatsPanel(DataArray *pData, const char *pszDir)
    : FreqPanel(pData, pszDir) {
}

void SoloGameStatsPanel::Refresh() {
    const SongEntry entry{TheGameDb->FindSong(TheGameDb->mSong.c_str())};
    SetLabel(kBandLabel, entry.GetArtistShort());
    SetLabel(kSongLabel, entry.GetTitleShort());
    SetLabel(kSkillLabel, TheGameDb->GetDifficultyName());
}

void SoloGameStatsPanel::SetLabel(const char *pszComponent, const char *pszText) {
    UILabel *pLabel = dynamic_cast<UILabel *>(FindComponent(pszComponent, false));
    pLabel->SetText(pszText);
    pLabel->mText->UpdateCursors();
}
