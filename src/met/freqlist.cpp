#include "met/freqlist.h"

#include "game/gamedb.h"
#include "game/remixinfo.h"
#include "game/songentry.h"
#include "met/metagameutil.h"
#include "met/songpicpanel.h"
#include "os/locale.h"
#include "os/string.h"
#include "synth/fxmidi.h"
#include "ui/uilabel.h"
#include "ui/uimanager.h"

namespace {

constexpr char kCreatorLabelFormat[] = "creator_%d";

} // namespace

FreqList::FreqList(DataArray *pData, const char *pszPanel) : UIList(pData, pszPanel) {
}

void FreqList::ScrollUp() {
    const int nSelected = mSelected;
    UIList::ScrollUp();
    if (nSelected != mSelected) {
        FxMidi::PlayMenuUp();
    }
}

void FreqList::ScrollDown() {
    const int nSelected = mSelected;
    UIList::ScrollDown();
    if (nSelected != mSelected) {
        FxMidi::PlayMenuDown();
    }
}

void RemixInfo::ShowDetails(const char *pszPanel) const {
    SongPicPanel *pPicPanel = static_cast<SongPicPanel *>(TheUI.FindPanel(pszPanel, false));

    UILabel *pLabel = dynamic_cast<UILabel *>(TheUI.FindComponent(pszPanel, "genre", false));
    SongEntry entry{TheGameDb->FindSong(mSong)};
    pLabel->SetText(FormatGenreTempo(entry, this));

    pLabel = dynamic_cast<UILabel *>(TheUI.FindComponent(pszPanel, "date", false));
    String date;
    mDate.FormatDateTime(date);
    pLabel->SetText(date.c_str());

    pLabel = dynamic_cast<UILabel *>(TheUI.FindComponent(pszPanel, "rating", false));
    const String rating(TheLocale.Localize("remix_rating", true));
    if (mPlayable) {
        pLabel->SetText(FormatString(
            rating.c_str(), TheGameDb->GetDifficultyName(mSkillLevel, GameDb::kRuleSetGame)));
    } else {
        pLabel->SetText(FormatString(rating.c_str(), TheLocale.Localize("skill_unplayable", true)));
    }

    pPicPanel->SetBandPicture(entry.GetName(), true, false, false);

    for (int i = 0; i < kRemixInfoCreatorCount; ++i) {
        pLabel = dynamic_cast<UILabel *>(
            TheUI.FindComponent(pszPanel, FormatString(kCreatorLabelFormat, i + 1), false));
        pLabel->SetText(mCreators[i]);
    }

    pLabel = dynamic_cast<UILabel *>(TheUI.FindComponent(pszPanel, "read_only", false));
    pLabel->SetShowing(mReadOnly != 0);
}
