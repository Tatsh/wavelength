#include "met/setupremixsavescreen.h"

#include <cstring>

#include "game/gamedb.h"
#include "game/remixinfo.h"
#include "game/songentry.h"
#include "memcard/mcmanager.h"
#include "met/keyboardpanel.h"
#include "met/keyboardrequest.h"
#include "met/metagameutil.h"
#include "met/remixdiscardscreen.h"
#include "met/saveremixscreen.h"
#include "met/songpicpanel.h"
#include "os/joypad.h"
#include "os/locale.h"
#include "synth/fxmidi.h"
#include "ui/uilabel.h"
#include "ui/uimanager.h"
#include "ui/uitextentry.h"

namespace {

constexpr char kTitle[] = "title";
constexpr char kDiscardFormat[] = "%s_discard";
constexpr char kLocalDiscardFormat[] = "m_r_end_remix_%d_discard";
constexpr char kKeyboardScreen[] = "kb_screen";
constexpr char kSaveScreen[] = "save_remix";

// The name entry and the keyboard accept this many characters, no wider than this.
constexpr int kMaxTitleChars = 29;
constexpr float kMaxTitleWidth = 158.0f;
constexpr int kMaxTitleWidthInt = 158;

constexpr int kNumCreators = 4;

// The keyboard opened for the name refuses the `invalid_chars` of the metagame configuration.
constexpr int kNoFunctionKeys = 0;
constexpr int kInvalidChars = 1;
constexpr int kNoPassword = 0;
constexpr int kOneLine = 0;

const char *DiscardScreenName(const char *pszScreen) {
    return FormatString(kDiscardFormat, pszScreen);
}

} // namespace

SetupRemixSaveScreen::SetupRemixSaveScreen(DataArray *pData) : NeedsDialogScreen(pData) {
    pData->FindSymbol("band_pic_panel", &mPanel, true);
    mPlayer = 1;
}

int SetupRemixSaveScreen::ReceiveKeyboardText(const char *pszText) {
    mTypedTitle = pszText;
    return 1;
}

void SetupRemixSaveScreen::Enter(UIScreen *pPrevScreen, float fTime) {
    FreqScreen::Enter(pPrevScreen, fTime);
    auto *pPicture = static_cast<SongPicPanel *>(TheUI.FindPanel(mPanel, false));
    const char *pszSong = TheGameDb->mSong.c_str();
    pPicture->SetBandPicture(pszSong, true, false, false);

    auto *pTitle =
        static_cast<UITextEntry *>(TheUI.FindComponent(mFocusPanel->mName, kTitle, false));
    pTitle->SetEditing(true);
    pTitle->mMaxNumChars = kMaxTitleChars;
    pTitle->mMaxEntryWidth = kMaxTitleWidth;

    RemixInfo record = *TheGameDb->GetRemixInfo();
    String discard(DiscardScreenName(mName));
    String localDiscard(FormatString(kLocalDiscardFormat, mPlayer - 1));
    const char *pszPrev = pPrevScreen->mName;
    if (strcmp(pszPrev, discard.c_str()) != 0 && strcmp(pszPrev, localDiscard.c_str()) != 0) {
        if (strcmp(pszPrev, kKeyboardScreen) == 0 || strcmp(mTypedTitle.c_str(), "") != 0) {
            pTitle->SetText(mTypedTitle.c_str());
            mTypedTitle = "";
        } else if (record.mSource != RemixInfo::kSourceNone) {
            pTitle->SetText(record.mName);
        } else {
            SongEntry song;
            song.mData = TheGameDb->FindSong(pszSong);
            pTitle->SetText(FormatString("%s 01", song.GetRemixTitle()));
        }
    }

    SongEntry song;
    song.mData = TheGameDb->FindSong(pszSong);
    dynamic_cast<UILabel *>(TheUI.FindComponent(mPanel, "genre", false))
        ->SetText(FormatGenreTempo(song, &record));
    UIComponent *pReadOnly = TheUI.FindComponent(mPanel, "read_only", true);
    if (pReadOnly != nullptr) {
        pReadOnly->SetShowing(record.mReadOnly);
    }

    record.mDate.ReadClock();
    TheGameDb->SetRemix(&record);
    String date;
    record.mDate.FormatDate(date);
    dynamic_cast<UILabel *>(TheUI.FindComponent(mPanel, "date", false))->SetText(date.c_str());

    for (int i = 0; i < kNumCreators; ++i) {
        UIComponent *pCreator =
            TheUI.FindComponent(mPanel, FormatString("creator_%d", i + 1), true);
        if (pCreator != nullptr) {
            pCreator->SetText(record.mCreators[i]);
        }
    }

    auto *pRating = dynamic_cast<UILabel *>(TheUI.FindComponent(mPanel, "rating", false));
    String ratingFormat(TheLocale.Localize("remix_rating", true));
    if (record.mPlayable) {
        pRating->SetText(
            FormatString(ratingFormat.c_str(),
                         TheGameDb->GetDifficultyName(record.mSkillLevel, GameDb::kRuleSetGame)));
    } else {
        pRating->SetText(
            FormatString(ratingFormat.c_str(), TheLocale.Localize("skill_unplayable", true)));
    }

    UIComponent *pPlayer = TheUI.FindComponent(mFocusPanel->mName, "player", false);
    String player(FormatString(TheLocale.Localize("save_remix_player", true),
                               TheGameDb->GetPlayerName(mPlayer - 1)));
    pPlayer->SetText(player.c_str());

    UIComponent *pMemcard = TheUI.FindComponent(mFocusPanel->mName, "memcard", false);
    String slot(TheMCManager.GetSlotName(mPlayer - 1));
    String memcard(FormatString(TheLocale.Localize("save_remix_memcard", true), slot.c_str()));
    pMemcard->SetText(memcard.c_str());
}

void SetupRemixSaveScreen::Proceed() {
    FxMidi::PlayMenuSelect();
    String title(TheUI.FindComponent("s_r_end_player", kTitle, false)->Text());
    const bool bTrimmed = TrimSpaces(&title);
    if (title.mLength == 0 || bTrimmed) {
        const char *pszError =
            title.mLength == 0 ? "no_empty_filename_screen" : "no_lead_trail_spaces_screen";
        UIScreen *pError = TheUI.FindScreen(pszError, false);
        pError->ClearTransitions();
        pError->AddTransition("ok", kPadNone, "s_r_end_remix");
        TheUI.GotoScreen(pError);
        return;
    }
    auto *pSave = dynamic_cast<SaveRemixScreen *>(TheUI.FindScreen(kSaveScreen, false));
    pSave->SetSlot(0);
    pSave->SetStartScreen("s_r_end_remix");
    pSave->SetDoneScreen("s_r_mode");
    pSave->mReservedA0 = 0;
    pSave->mRemixName = title.c_str();
    TheUI.GotoScreen(pSave);
}

bool SetupRemixSaveScreen::DispatchPriv(Message *pMsg) {
    const int nType = pMsg->Type();
    if (nType == g_nJoypadInputMsgType) {
        return HandleJoypad(static_cast<JoypadInputMsg *>(pMsg));
    }
    if (nType == g_nUITextEntryCompleteMsgType) {
        return HandleTextEntryComplete(static_cast<UITextEntryCompleteMsg *>(pMsg));
    }
    if (nType == g_nUITransitionCompleteMsgType) {
        return HandleTransitionComplete(static_cast<UITransitionCompleteMsg *>(pMsg));
    }
    if (nType == g_nUITextEntryInvalidMsgType) {
        return HandleTextEntryInvalid(static_cast<UITextEntryInvalidMsg *>(pMsg));
    }
    return FreqScreen::DispatchPriv(pMsg);
}

bool SetupRemixSaveScreen::HandleJoypad(JoypadInputMsg *pMsg) {
    if (pMsg->mPressed != 0 && (mNextScreen != nullptr || mPrevScreen != nullptr)) {
        return true;
    }
    if (strcmp(mFocusPanel->mFocus->mName, kTitle) == 0 && pMsg->mPressed != 0 &&
        pMsg->mPad == mPlayer - 1) {
        if (pMsg->mButton == kPadCross) {
            Proceed();
        } else if (pMsg->mButton == kPadCircle) {
            auto *pKeyboard = dynamic_cast<KeyboardPanel *>(TheUI.FindPanel("keyboard", false));
            KeyboardRequest request(this,
                                    TheUI.mCurrentScreen,
                                    "",
                                    mPlayer - 1,
                                    kMaxTitleChars,
                                    kMaxTitleWidthInt,
                                    kNoFunctionKeys,
                                    kInvalidChars,
                                    kNoPassword,
                                    kOneLine);
            pKeyboard->SetRequest(request);
            TheUI.GotoScreen(kKeyboardScreen);
        } else if (pMsg->mButton == kPadSquare) {
            FxMidi::PlaySquare();
            String discard(DiscardScreenName(mName));
            auto *pDiscard =
                dynamic_cast<RemixDiscardScreen *>(TheUI.FindScreen(discard.c_str(), false));
            pDiscard->mPlayer = mPlayer;
            TheUI.GotoScreen(pDiscard);
            SetPanelsShowing(false);
        }
    }
    return FreqScreen::HandleJoypad(pMsg);
}

bool SetupRemixSaveScreen::HandleTransitionComplete(UITransitionCompleteMsg *pMsg) {
    String discard(DiscardScreenName(mName));
    String localDiscard(FormatString(kLocalDiscardFormat, mPlayer - 1));
    const char *pszPrev = pMsg->mPrevScreen->mName;
    if (strcmp(pszPrev, discard.c_str()) == 0 || strcmp(pszPrev, localDiscard.c_str()) == 0) {
        SetPanelsShowing(true);
    }
    return FreqScreen::HandleTransitionComplete(pMsg);
}

bool SetupRemixSaveScreen::HandleTextEntryComplete(UITextEntryCompleteMsg *pMsg) {
    if (pMsg->mText.mLength != 0) {
        Proceed();
    }
    return true;
}

bool SetupRemixSaveScreen::HandleTextEntryInvalid(UITextEntryInvalidMsg *pMsg) {
    if (strcmp(pMsg->mEntry->mName, kTitle) == 0) {
        FxMidi::PlayWrong();
    }
    return false;
}
