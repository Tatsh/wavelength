#include "met/netendgamescreen.h"

#include "met/metagame.h"
#include "os/joypad.h"
#include "os/system.h"

namespace {

constexpr float kNoEndTime = 0.0f;

} // namespace

NetEndGameScreen::NetEndGameScreen(DataArray *pData) : MultiEndGameScreen(pData), mEndTime(0.0f) {
}

void NetEndGameScreen::Poll(float fTime) {
    MultiEndGameScreen::Poll(fTime);
    if (kNoEndTime < mEndTime && mEndTime < SystemMs()) {
        mEndTime = kNoEndTime;
        TheMetagame.ShowEndGameScreens(Metagame::kDialogActionEnd);
    }
}

bool NetEndGameScreen::DispatchPriv(Message *pMsg) {
    const int nType = pMsg->Type();
    if (nType == g_nUIComponentSelectMsgType) {
        return HandleSelect(static_cast<UIComponentSelectMsg *>(pMsg));
    }
    if (nType == g_nUITransitionCompleteMsgType) {
        return HandleTransitionComplete(static_cast<UITransitionCompleteMsg *>(pMsg));
    }
    return MultiEndGameScreen::DispatchPriv(pMsg);
}

bool NetEndGameScreen::HandleSelect(UIComponentSelectMsg *pMsg) {
    if (pMsg->mButton == kPadCross) {
        mEndTime = kNoEndTime;
        TheMetagame.ShowEndGameScreens(Metagame::kDialogActionEnd);
    }
    return UIScreen::HandleSelect(pMsg);
}

bool NetEndGameScreen::HandleTransitionComplete(UITransitionCompleteMsg *pMsg) {
    float fWaitMs = 0.0f;
    SystemConfig()->FindArray("metagame", false)->FindFloat("net_endgame_wait_ms", &fWaitMs, true);
    mEndTime = SystemMs() + fWaitMs;
    return FreqScreen::HandleTransitionComplete(pMsg);
}
