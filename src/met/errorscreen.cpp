#include "met/errorscreen.h"

#include "memcard/mcmanager.h"

ErrorScreen::ErrorScreen(DataArray *pData) : FreqScreen(pData), mSlot(0) {
    pData->FindString("start_screen", &mStartScreen, false);
    pData->FindString("done_screen", &mDoneScreen, false);
}

ErrorScreen::~ErrorScreen() {
}

const char *ErrorScreen::GetSlotName() {
    return TheMCManager.GetSlotName(mSlot);
}
