#include "met/choosememcardscreen.h"

#include "memcard/mcmanager.h"
#include "met/remixorfreqscreen.h"
#include "os/joypad.h"
#include "rnd/manager.h"
#include "rnd/text.h"
#include "rnd/view.h"
#include "ui/lrbutton.h"
#include "ui/uicomponent.h"
#include "ui/uimanager.h"

namespace {

constexpr char kCardView[] = "o_memcard_card.view";
constexpr char kNoCardText[] = "o_memcard_no_card.txt";
constexpr char kSlotText[] = "o_memcard_slot.txt";
constexpr char kPanel[] = "o_memcard";
constexpr char kSlotComponent[] = "slot";
constexpr char kCardsConnectedScreen[] = "mem_cards_connected";
constexpr char kRemixOrFreqScreen[] = "o_rf_mem";
constexpr char kNoSlotName[] = "";

} // namespace

ChooseMemCardScreen::ChooseMemCardScreen(DataArray *pData) : FreqScreen(pData), mSlots(), mSlot(0) {
}

bool ChooseMemCardScreen::DispatchPriv(Message *pMsg) {
    const int nType = pMsg->Type();
    if (nType == g_nJoypadInputMsgType) {
        return HandleJoypad(static_cast<JoypadInputMsg *>(pMsg));
    }
    if (nType == g_nUIComponentSelectStartMsgType) {
        return HandleSelectStart(static_cast<UIComponentSelectStartMsg *>(pMsg));
    }
    return FreqScreen::DispatchPriv(pMsg);
}

void ChooseMemCardScreen::Enter(UIScreen *pPrevScreen, float fTime) {
    FreqScreen::Enter(pPrevScreen, fTime);
    Rnd::View *pCard = dynamic_cast<Rnd::View *>(Rnd::TheManager.Find(kCardView));
    Rnd::Text *pNoCard = dynamic_cast<Rnd::Text *>(Rnd::TheManager.Find(kNoCardText));
    mSlot = 0;
    if (!mSlots.empty()) {
        pCard->SetShowing(true);
        pNoCard->SetShowing(false);
        Rnd::Text *pSlot = dynamic_cast<Rnd::Text *>(Rnd::TheManager.Find(kSlotText));
        pSlot->SetText(TheMCManager.GetSlotName(mSlots[mSlot]));
    } else {
        pCard->SetShowing(false);
        pNoCard->SetShowing(true);
    }

    LRButton *pButton = static_cast<LRButton *>(TheUI.FindComponent(kPanel, kSlotComponent, false));
    pButton->SetState(UIComponent::kStateSelected, true);
    const bool bArrows = mSlots.size() >= 2;
    pButton->SetArrowShowing(LRButton::kArrowLeft, bArrows);
    pButton->SetArrowShowing(LRButton::kArrowRight, bArrows);
}

bool ChooseMemCardScreen::HandleSelectStart(UIComponentSelectStartMsg *pMsg) {
    const int nSlots = static_cast<int>(mSlots.size());
    if (nSlots == 0) {
        return false;
    }
    if (pMsg->mButton == kPadDLeft) {
        mSlot = mSlot - 1 > -1 ? mSlot - 1 : nSlots - 1;
    } else if (pMsg->mButton == kPadDRight) {
        mSlot = mSlot + 1 < nSlots ? mSlot + 1 : 0;
    }

    Rnd::Text *pSlot = dynamic_cast<Rnd::Text *>(Rnd::TheManager.Find(kSlotText));
    if (!mSlots.empty()) {
        pSlot->SetText(TheMCManager.GetSlotName(mSlots[mSlot]));
    } else {
        pSlot->SetText(kNoSlotName);
    }
    return false;
}

bool ChooseMemCardScreen::HandleJoypad(JoypadInputMsg *pMsg) {
    if (pMsg->mPressed) {
        if (mNextScreen != nullptr || mPrevScreen != nullptr) {
            return true;
        }
        if (pMsg->mButton == kPadCircle) {
            TheUI.GotoScreen(kCardsConnectedScreen);
        } else if (pMsg->mButton == kPadCross && !mSlots.empty()) {
            RemixOrFreqScreen *pNext =
                dynamic_cast<RemixOrFreqScreen *>(TheUI.FindScreen(kRemixOrFreqScreen, false));
            pNext->mSlot = mSlots[mSlot];
            TheUI.GotoScreen(pNext);
        }
    }
    return UIScreen::HandleJoypad(pMsg);
}
