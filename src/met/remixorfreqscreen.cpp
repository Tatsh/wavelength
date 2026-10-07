#include "met/remixorfreqscreen.h"

#include <cstring>

#include "met/errorscreen.h"
#include "met/freqcopydelscreen.h"
#include "met/remixcopydelscreen.h"
#include "os/joypad.h"
#include "ui/uicomponent.h"
#include "ui/uimanager.h"

namespace {

constexpr char kRemixComponent[] = "remix";
constexpr char kFreqComponent[] = "freq";
constexpr char kRemixListScreen[] = "mem_remix";
constexpr char kRemixLoadScreen[] = "load_remix_list_mc";
constexpr char kFreqListScreen[] = "mem_freqs";
constexpr char kFreqLoadScreen[] = "mc_load_freq";

} // namespace

RemixOrFreqScreen::RemixOrFreqScreen(DataArray *pData) : FreqScreen(pData) {
}

bool RemixOrFreqScreen::DispatchPriv(Message *pMsg) {
    if (pMsg->Type() == g_nUIComponentSelectMsgType) {
        return HandleSelect(static_cast<UIComponentSelectMsg *>(pMsg));
    }
    return FreqScreen::DispatchPriv(pMsg);
}

void RemixOrFreqScreen::Enter(UIScreen *pPrevScreen, float fTime) {
    FreqScreen::Enter(pPrevScreen, fTime);
}

bool RemixOrFreqScreen::HandleSelect(UIComponentSelectMsg *pMsg) {
    if (pMsg->mButton == kPadCross) {
        ErrorScreen *pLoad = nullptr;
        if (strcmp(pMsg->mComponent->mName, kRemixComponent) == 0) {
            dynamic_cast<RemixCopyDelScreen *>(TheUI.FindScreen(kRemixListScreen, false))->mSlot =
                mSlot;
            pLoad = dynamic_cast<ErrorScreen *>(TheUI.FindScreen(kRemixLoadScreen, false));
        } else if (strcmp(pMsg->mComponent->mName, kFreqComponent) == 0) {
            dynamic_cast<FreqCopyDelScreen *>(TheUI.FindScreen(kFreqListScreen, false))->mSlot =
                mSlot;
            pLoad = dynamic_cast<ErrorScreen *>(TheUI.FindScreen(kFreqLoadScreen, false));
        }
        // Yes, the binary calls through a null screen when another component was chosen.
        pLoad->SetSlot(mSlot);
        TheUI.GotoScreen(pLoad);
    }
    return UIScreen::HandleSelect(pMsg);
}
