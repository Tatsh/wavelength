#include "met/multifreqselectscreen.h"

#include <cstring>

#include "game/gamedb.h"
#include "met/freqselpanel.h"
#include "os/joypad.h"
#include "os/string.h"
#include "ui/uicomponent.h"
#include "ui/uimanager.h"

namespace {

constexpr char kPanelFormat[] = "m_g_s_f_%dpl_0%d";

} // namespace

MultiFreqSelectScreen::MultiFreqSelectScreen(DataArray *pData) : FreqScreen(pData) {
    pData->FindInt("num_players", &mNumPlayers, false);
}

void MultiFreqSelectScreen::Enter(UIScreen *pPrevScreen, float fTime) {
    FreqScreen::Enter(pPrevScreen, fTime);
    mConfirmed.clear();
    mConfirmed.resize(mNumPlayers);
    // Yes, the binary clears every bit again after the resize.
    for (int i = 0; i < mNumPlayers; ++i) {
        mConfirmed[i] = false;
    }
}

bool MultiFreqSelectScreen::DispatchPriv(Message *pMsg) {
    const int nType = pMsg->Type();
    if (nType == g_nUIComponentSelectStartMsgType) {
        return HandleSelectStart(static_cast<UIComponentSelectStartMsg *>(pMsg));
    }
    if (nType == g_nUIComponentSelectMsgType) {
        return HandleSelect(static_cast<UIComponentSelectMsg *>(pMsg));
    }
    if (nType == g_nJoypadInputMsgType) {
        return HandleJoypad(static_cast<JoypadInputMsg *>(pMsg));
    }
    return FreqScreen::DispatchPriv(pMsg);
}

bool MultiFreqSelectScreen::HandleSelectStart(UIComponentSelectStartMsg *pMsg) {
    const int nPad = pMsg->mPad;
    if (nPad >= mNumPlayers) {
        return false;
    }
    const char *pszPanel = FormatString(kPanelFormat, TheGameDb->GetNumPlayers(), nPad + 1);
    auto *pPanel = dynamic_cast<FreqSelPanel *>(TheUI.FindPanel(pszPanel, false));
    return pPanel->HandleSelectStart(pMsg);
}

bool MultiFreqSelectScreen::HandleSelect(UIComponentSelectMsg *pMsg) {
    bool bHandled = false;
    if (pMsg->mButton != kPadCross) {
        return bHandled;
    }
    const char *pszName = pMsg->mComponent->mName;
    const int nDigit = pszName[std::strlen(pszName) - 1];
    const int nPlayer = nDigit - '1';
    if (nPlayer < mNumPlayers) {
        const char *pszPanel = FormatString(kPanelFormat, TheGameDb->GetNumPlayers(), nDigit - '0');
        auto *pPanel = dynamic_cast<FreqSelPanel *>(TheUI.FindPanel(pszPanel, false));
        bHandled = pPanel->HandleSelect(pMsg);
        mConfirmed[nPlayer] = true;
        CheckDone();
    }
    return bHandled;
}
