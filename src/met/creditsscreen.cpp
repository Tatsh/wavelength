#include "met/creditsscreen.h"

#include <cstring>

#include "os/joypad.h"
#include "os/locale.h"
#include "os/string.h"
#include "os/system.h"
#include "rnd/manager.h"
#include "rnd/text.h"
#include "ui/uimanager.h"

namespace {

constexpr char kTextTagKey[] = "text_tag";
constexpr char kNextScreenKey[] = "next_screen";
constexpr char kBetweenScreenKey[] = "between_screen";
constexpr char kHoldTimeKey[] = "hold_time";
constexpr char kExitScreen[] = "credits_out";
constexpr char kTextObjectFormat[] = "%s_%02i.txt";

// mEndTime while no hold runs.
constexpr float kNoHold = -1.0f;

constexpr float kDefaultHoldMs = 1000.0f;

// The size of the buffer a line is copied into, terminator included.
constexpr int kLineSize = 0x80;

// Each line ends with a carriage return and a line feed.
constexpr char kLineEnd = '\r';
constexpr int kLineEndLength = 2;

} // namespace

CreditsScreen::CreditsScreen(DataArray *pData) : UIScreen(pData) {
    mTextTag = nullptr;
    mCursor = nullptr;
    mEnd = nullptr;
    mBetweenScreen = nullptr;
    mFinalScreen = nullptr;
    mHoldMs = kDefaultHoldMs;
    mEndTime = kNoHold;
    pData->FindSymbol(kTextTagKey, &mTextTag, true);
    pData->FindSymbol(kNextScreenKey, &mFinalScreen, true);
    pData->FindSymbol(kBetweenScreenKey, &mBetweenScreen, true);
    pData->FindFloat(kHoldTimeKey, &mHoldMs, false);
}

void CreditsScreen::Enter(UIScreen *pPrevScreen, float fTime) {
    UIScreen::Enter(pPrevScreen, fTime);
    mEndTime = SystemMs() + mHoldMs;
    if (strcmp(pPrevScreen->mName, mBetweenScreen) != 0 || mCursor == nullptr || mEnd == nullptr) {
        mCursor = TheLocale.Localize(mTextTag, true);
        mEnd = mCursor;
        while (*mEnd++ != '\0') {
        }
    }
    ShowPage();
}

void CreditsScreen::Poll(float fTime) {
    UIScreen::Poll(fTime);
    if (0.0f < mEndTime && mEndTime < SystemMs()) {
        Advance();
    }
}

bool CreditsScreen::HandleJoypad(JoypadInputMsg *pMsg) {
    if (pMsg->mPressed != 0) {
        if (mNextScreen != nullptr || mPrevScreen != nullptr) {
            return true;
        }
        if (pMsg->mButton == kPadTriangle) {
            TheUI.GotoScreen(kExitScreen);
        } else {
            Advance();
        }
    }
    return UIScreen::HandleJoypad(pMsg);
}

void CreditsScreen::Advance() {
    mEndTime = kNoHold;
    if (mCursor < mEnd) {
        TheUI.GotoScreen(mBetweenScreen);
    } else {
        TheUI.GotoScreen(mFinalScreen);
    }
}

void CreditsScreen::ShowPage() {
    for (int i = 0;; ++i) {
        Rnd::Text *pText = dynamic_cast<Rnd::Text *>(
            Rnd::TheManager.Find(FormatString(kTextObjectFormat, mTextTag, i + 1)));
        if (pText == nullptr) {
            break;
        }
        if (mCursor < mEnd) {
            const char *pszLineEnd = strchr(mCursor, kLineEnd);
            if (pszLineEnd == nullptr) {
                pszLineEnd = mEnd;
            }
            char line[kLineSize];
            strncpy(line, mCursor, pszLineEnd - mCursor);
            line[pszLineEnd - mCursor] = '\0';
            pText->SetText(line);
            mCursor = &pszLineEnd[kLineEndLength];
        } else {
            pText->SetText("");
        }
    }
}

bool CreditsScreen::DispatchPriv(Message *pMsg) {
    if (pMsg->Type() == g_nJoypadInputMsgType) {
        return HandleJoypad(static_cast<JoypadInputMsg *>(pMsg));
    }
    return UIScreen::DispatchPriv(pMsg);
}
