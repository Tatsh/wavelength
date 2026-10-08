#include "met/cardsconnectedscreen.h"

#include <cstring>

#include "memcard/mcmanager.h"
#include "memcard/memcardtask.h"
#include "met/choosememcardscreen.h"
#include "met/mcdialogpanel.h"
#include "ui/uimanager.h"

namespace {

constexpr char kDialogPanel[] = "mem_cards_connected_dlg";
constexpr char kChooseScreen[] = "o_memcard";

// The memory card slots of the console.
constexpr int kNumSlots = 2;

} // namespace

CardsConnectedScreen::CardsConnectedScreen(DataArray *pData) : ErrorScreen(pData) {
    mNumSlots = kNumSlots;
    mAdvance = 0;
}

void CardsConnectedScreen::Enter(UIScreen *pPrevScreen, float fTime) {
    mNumSlots = kNumSlots;
    mSlots.clear();
    mSlot = 0;
    mAdvance = 0;
    FreqScreen::Enter(pPrevScreen, fTime);
}

void CardsConnectedScreen::Poll(float fTime) {
    UIScreen::Poll(fTime);
    if (mAdvance != 0) {
        mAdvance = 0;
        CheckNextSlot();
    }
}

void CardsConnectedScreen::CheckNextSlot() {
    if (mSlot + 1 < mNumSlots) {
        ++mSlot;
        dynamic_cast<MCDialogPanel *>(TheUI.FindPanel(kDialogPanel, false))->ShowSlotText();
        TheMCManager.GetCardStatus(this, mSlot);
        return;
    }
    auto *pChoose = dynamic_cast<ChooseMemCardScreen *>(TheUI.FindScreen(kChooseScreen, false));
    pChoose->mSlots = mSlots;
    TheUI.GotoScreen(pChoose);
}

void CardsConnectedScreen::OnCardStatus(int nStatus) {
    if (nStatus != MemcardTask::kStatusNoCard) {
        mSlots.push_back(mSlot);
    }
    mAdvance = 1;
}

bool CardsConnectedScreen::DispatchPriv(Message *pMsg) {
    if (pMsg->Type() == g_nUITransitionCompleteMsgType) {
        return HandleTransitionComplete(static_cast<UITransitionCompleteMsg *>(pMsg));
    }
    return FreqScreen::DispatchPriv(pMsg);
}

bool CardsConnectedScreen::HandleTransitionComplete(UITransitionCompleteMsg *pMsg) {
    if (std::strcmp(pMsg->mScreen->mName, mName) == 0) {
        TheMCManager.GetCardStatus(this, mSlot);
    }
    return false;
}
