#include "met/neteulascreen.h"

#include "os/joypad.h"
#include "rnd/manager.h"
#include "rnd/text.h"

namespace {

constexpr char kEulaText[] = "d_eula_02.txt";

constexpr int kLinesPerPage = 15;
constexpr int kNoWordWrap = 0;
constexpr int kPreviousPage = -1;
constexpr int kNextPage = 1;
constexpr int kSamePage = 0;

// mPages ends with the end of the last page. The last page starts one entry before it.
constexpr int kPageEndEntries = 2;

Rnd::Text *FindEulaText() {
    return dynamic_cast<Rnd::Text *>(Rnd::TheManager.Find(kEulaText));
}

} // namespace

NetEULAScreen::NetEULAScreen(DataArray *pData) : FreqScreen(pData), mText(nullptr), mPage(0) {
}

void NetEULAScreen::SetText(const char *pszText) {
    mText = pszText;
}

void NetEULAScreen::TurnPage(int nDelta) {
    mPage += nDelta;
    if (mPage < 0) {
        mPage = 0;
    }
    const unsigned int nLastPage = mPages.size() - kPageEndEntries;
    if (nLastPage < static_cast<unsigned int>(mPage)) {
        mPage = nLastPage;
    }

    Rnd::Text *pText = FindEulaText();
    char *pPageEnd = mPages[mPage + 1];
    const char cSaved = *pPageEnd;
    *pPageEnd = '\0';
    pText->SetText(mPages[mPage]);
    *mPages[mPage + 1] = cSaved;
}

void NetEULAScreen::Enter(UIScreen *pPrevScreen, float fTime) {
    FreqScreen::Enter(pPrevScreen, fTime);
    Rnd::Text *pText = FindEulaText();
    pText->WrapText(mText, &mWrappedText);
    pText->SetWordWrap(kNoWordWrap);

    int nLines = 0;
    char *pChar = mWrappedText.mBuffer;
    mPages.clear();
    mPages.push_back(pChar);
    for (; *pChar != '\0'; ++pChar) {
        if (*pChar == '\n' && ++nLines == kLinesPerPage && pChar[1] != '\0') {
            mPages.push_back(pChar + 1);
            nLines = 0;
        }
    }
    mPages.push_back(pChar);
    mPage = 0;
    TurnPage(kSamePage);
}

bool NetEULAScreen::DispatchPriv(Message *pMsg) {
    if (pMsg->Type() == g_nJoypadInputMsgType) {
        return HandleJoypad(static_cast<JoypadInputMsg *>(pMsg));
    }
    return FreqScreen::DispatchPriv(pMsg);
}

bool NetEULAScreen::HandleJoypad(JoypadInputMsg *pMsg) {
    if (pMsg->mPressed != 0) {
        if (mNextScreen != nullptr || mPrevScreen != nullptr) {
            return true;
        }
        if (pMsg->mButton == kPadDUp) {
            TurnPage(kPreviousPage);
        } else if (pMsg->mButton == kPadDDown) {
            TurnPage(kNextPage);
        }
    }
    return FreqScreen::HandleJoypad(pMsg);
}
