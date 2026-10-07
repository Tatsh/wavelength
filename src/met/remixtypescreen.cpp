#include "met/remixtypescreen.h"

#include <cstring>

#include "game/gamedb.h"
#include "game/remixinfo.h"
#include "met/metagame.h"
#include "os/joypad.h"
#include "os/string.h"
#include "ui/uicomponent.h"
#include "ui/uimanager.h"

namespace {

// The longest name and player name strncpy() copies into a RemixInfo.
constexpr size_t kNameCopyLength = 29;
constexpr size_t kPlayerCopyLength = 15;

} // namespace

RemixTypeScreen::RemixTypeScreen(DataArray *pData) : FreqScreen(pData) {
}

void RemixTypeScreen::Enter(UIScreen *pPrevScreen, float fTime) {
    FreqScreen::Enter(pPrevScreen, fTime);
}

bool RemixTypeScreen::DispatchPriv(Message *pMsg) {
    if (pMsg->Type() == g_nUIComponentSelectMsgType) {
        return HandleSelect(static_cast<UIComponentSelectMsg *>(pMsg));
    }
    return FreqScreen::DispatchPriv(pMsg);
}

bool RemixTypeScreen::HandleSelect(UIComponentSelectMsg *pMsg) {
    if (pMsg->mButton != kPadCross) {
        return UIScreen::HandleSelect(pMsg);
    }

    String button(pMsg->mComponent->mName);
    TheGameDb->SetTutorial(0);
    if (button == "new") {
        TheGameDb->SetPracticeMode(false);
        TheGameDb->SetTutorial(0);
        TheGameDb->SetRemixBuffer(0);
        RemixInfo record;
        strncpy(record.mName, "Untitled", kNameCopyLength);
        record.mSource = RemixInfo::kSourceNone;
        record.mDataSize = 0;
        for (int i = 0; i < TheGameDb->GetNumPlayers(); ++i) {
            strncpy(record.mCreators[i], TheGameDb->GetPlayerName(i), kPlayerCopyLength);
        }
        record.mDate.ReadClock();
        record.mReadOnly = false;
        TheGameDb->SetRemix(&record);
    } else if (button == "load") {
        TheGameDb->SetRemixActive(1);
    } else if (button == "training") {
        RemixInfo record;
        TheGameDb->SetRemix(&record);
        TheGameDb->SetPracticeMode(true);
        TheGameDb->SetTutorial(1);
        TheGameDb->SetRemixActive(1);
        TheMetagame.mSelectedArena = "No arena";
        TheGameDb->SetSong("TUT2");
        TheUI.GotoScreen("pre_solo_r_tut2launchseq");
    }
    TheMetagame.ShowUnlockedArenas();
    return UIScreen::HandleSelect(pMsg);
}
