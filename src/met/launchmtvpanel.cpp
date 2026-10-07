#include "met/launchmtvpanel.h"

#include "game/gamedb.h"
#include "game/songentry.h"
#include "ui/uicomponent.h"
#include "ui/uimanager.h"

namespace {

constexpr char kBandPanel[] = "d_band";
constexpr char kTitleLabel[] = "01";
constexpr char kArtistLabel[] = "02";
constexpr char kRecordLabel[] = "03";

} // namespace

void LaunchMTVPanel::Enter(bool bForce, float fTime) {
    const SongEntry entry{TheGameDb->FindSong(TheGameDb->mSong.c_str())};
    TheUI.FindComponent(kBandPanel, kTitleLabel, false)->SetText(entry.GetTitleShort());
    TheUI.FindComponent(kBandPanel, kArtistLabel, false)->SetText(entry.GetArtistShort());
    TheUI.FindComponent(kBandPanel, kRecordLabel, false)->SetText(entry.GetLabel());
    FreqPanel::Enter(bForce, fTime);
}
