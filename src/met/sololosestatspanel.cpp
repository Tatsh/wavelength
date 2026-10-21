#include "met/sololosestatspanel.h"

#include "game/gamedb.h"
#include "game/songstats.h"
#include "os/string.h"
#include "ui/uicomponent.h"

namespace {

constexpr char kProgressComponent[] = "progress";
constexpr char kPercentFormat[] = "%d%%";

constexpr int kLocalPlayer = 0;
constexpr float kPercent = 100.0f;

} // namespace

void SoloLoseStatsPanel::Refresh() {
    SoloGameStatsPanel::Refresh();
    SongStats record;
    // Yes, the binary fetches the record and never reads it.
    TheGameDb->GetProfile(kLocalPlayer)
        ->GetSongStats(TheGameDb->GetSong().c_str(), TheGameDb->GetSkillLevel(), &record);
    FindComponent(kProgressComponent, false)
        ->SetText(
            FormatString(kPercentFormat, static_cast<int>(TheGameDb->GetProgress() * kPercent)));
}
