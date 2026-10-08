#include "met/errorscreen.h"

#include "memcard/mcmanager.h"
#include "memcard/memcardtask.h"
#include "met/msgerrorscreen.h"
#include "met/spaceerrorscreen.h"
#include "os/joypad.h"
#include "os/locale.h"
#include "ui/uimanager.h"

namespace {

constexpr char kThreeOptionNoCardScreen[] = "3opt_no_card_error";
constexpr char kThreeOptionDiffCardScreen[] = "3opt_diff_card_error";
constexpr char kTwoOptionNoCardScreen[] = "2opt_no_card_error";
constexpr char kTwoOptionDiffCardScreen[] = "2opt_diff_card_error";
constexpr char kFormatScreen[] = "mc_format";
constexpr char kUnformattedScreen[] = "unformatted_card_error";
constexpr char kNoFormatContinueScreen[] = "no_format_warning_continue";
constexpr char kNoFormatCancelScreen[] = "no_format_warning_cancel";
constexpr char kNoSpaceScreen[] = "no_space_error";
constexpr char kRetryComponent[] = "retry";
constexpr char kCancelComponent[] = "cancel";
constexpr char kContinueComponent[] = "continue";
constexpr char kYesComponent[] = "yes";
constexpr char kNoComponent[] = "no";
constexpr char kNoFormatSaveToken[] = "no_format_save_warning_cancel_dlg";
constexpr char kNoFormatCopyToken[] = "no_format_copy_warning_cancel_dlg";
constexpr char kNoFormatDeleteToken[] = "no_format_del_warning_cancel_dlg";
constexpr char kNoFormatLoadToken[] = "no_format_load_warning_cancel_dlg";

// The operations ShowCardErrorTwoOption() takes.
enum Operation {
    kOperationSave = 1,
    kOperationCopy = 2,
    kOperationDelete = 3,
};

// The slot whose unformatted memory card may be skipped.
constexpr int kFirstSlot = 0;

inline ErrorScreen *FindErrorScreen(const char *pszName) {
    return dynamic_cast<ErrorScreen *>(TheUI.FindScreen(pszName, false));
}

// The locale token of the warning that a memory card stays unformatted.
inline const char *NoFormatToken(int nOperation) {
    switch (nOperation) {
    case kOperationSave:
        return kNoFormatSaveToken;
    case kOperationCopy:
        return kNoFormatCopyToken;
    case kOperationDelete:
        return kNoFormatDeleteToken;
    default:
        return kNoFormatLoadToken;
    }
}

} // namespace

ErrorScreen::ErrorScreen(DataArray *pData) : FreqScreen(pData), mSlot(0) {
    pData->FindString("start_screen", &mStartScreen, false);
    pData->FindString("done_screen", &mDoneScreen, false);
}

ErrorScreen::~ErrorScreen() {
}

const char *ErrorScreen::GetSlotName() {
    return TheMCManager.GetSlotName(mSlot);
}

void ErrorScreen::ShowCardError(int nStatus) {
    ErrorScreen *pError;
    switch (nStatus) {
    case MemcardTask::kStatusNoCard:
        pError = FindErrorScreen(kThreeOptionNoCardScreen);
        TheUI.GotoScreen(pError);
        pError->SetSlot(mSlot);
        pError->ClearTransitions();
        break;
    case MemcardTask::kStatusChangedCard:
        pError = FindErrorScreen(kThreeOptionDiffCardScreen);
        pError->ClearTransitions();
        pError->SetSlot(mSlot);
        break;
    case MemcardTask::kStatusUnformatted: {
        ErrorScreen *pFormat = FindErrorScreen(kFormatScreen);
        pFormat->SetSlot(mSlot);
        pFormat->SetDoneScreen(mName);
        pFormat->SetStartScreen(mName);
        pFormat->SetSlot(mSlot);
        pError = FindErrorScreen(kUnformattedScreen);
        pError->ClearTransitions();
        pError->SetSlot(mSlot);
        pError->AddTransition(kYesComponent, kPadNone, kFormatScreen);
        if (mSlot == kFirstSlot) {
            pError->AddTransition(kNoComponent, kPadNone, kNoFormatContinueScreen);
            ErrorScreen *pWarning = FindErrorScreen(kNoFormatContinueScreen);
            pWarning->ClearTransitions();
            pWarning->AddTransition(kRetryComponent, kPadNone, mName);
            pWarning->AddTransition(kContinueComponent, kPadNone, mDoneScreen.c_str());
        } else {
            pError->AddTransition(kNoComponent, kPadNone, mDoneScreen.c_str());
        }
        TheUI.GotoScreen(kUnformattedScreen);
        return;
    }
    case MemcardTask::kStatusFull:
        pError = FindErrorScreen(kNoSpaceScreen);
        pError->SetSlot(mSlot);
        pError->ClearTransitions();
        pError->AddTransition(kRetryComponent, kPadNone, mName);
        pError->AddTransition(kCancelComponent, kPadNone, mDoneScreen.c_str());
        TheUI.GotoScreen(pError);
        return;
    default:
        return;
    }
    pError->AddTransition(kRetryComponent, kPadNone, mName);
    pError->AddTransition(kCancelComponent, kPadNone, mStartScreen.c_str());
    pError->AddTransition(kContinueComponent, kPadNone, mDoneScreen.c_str());
    TheUI.GotoScreen(pError);
}

void ErrorScreen::ShowCardErrorTwoOption(int nStatus, int nOperation) {
    ErrorScreen *pError;
    switch (nStatus) {
    case MemcardTask::kStatusNoCard:
        pError = FindErrorScreen(kTwoOptionNoCardScreen);
        TheUI.GotoScreen(pError);
        pError->SetSlot(mSlot);
        pError->ClearTransitions();
        break;
    case MemcardTask::kStatusChangedCard:
        pError = FindErrorScreen(kTwoOptionDiffCardScreen);
        pError->ClearTransitions();
        pError->SetSlot(mSlot);
        break;
    case MemcardTask::kStatusUnformatted: {
        ErrorScreen *pFormat = FindErrorScreen(kFormatScreen);
        pFormat->SetSlot(mSlot);
        pFormat->SetDoneScreen(mName);
        pFormat->SetStartScreen(mName);
        pFormat->SetSlot(mSlot);
        pError = FindErrorScreen(kUnformattedScreen);
        pError->ClearTransitions();
        pError->SetSlot(mSlot);
        pError->AddTransition(kYesComponent, kPadNone, kFormatScreen);
        pError->AddTransition(kNoComponent, kPadNone, kNoFormatCancelScreen);
        auto *pWarning =
            dynamic_cast<MsgErrorScreen *>(TheUI.FindScreen(kNoFormatCancelScreen, false));
        pWarning->SetSlot(mSlot);
        pWarning->mMessage = TheLocale.Localize(NoFormatToken(nOperation), true);
        pWarning->ClearTransitions();
        pWarning->AddTransition(kRetryComponent, kPadNone, mName);
        pWarning->AddTransition(kCancelComponent, kPadNone, mStartScreen.c_str());
        TheUI.GotoScreen(kUnformattedScreen);
        return;
    }
    case MemcardTask::kStatusFull:
        pError = FindErrorScreen(kNoSpaceScreen);
        pError->SetSlot(mSlot);
        pError->ClearTransitions();
        pError->AddTransition(kRetryComponent, kPadNone, mName);
        pError->AddTransition(kCancelComponent, kPadNone, mDoneScreen.c_str());
        TheUI.GotoScreen(pError);
        return;
    default:
        return;
    }
    pError->AddTransition(kRetryComponent, kPadNone, mName);
    pError->AddTransition(kCancelComponent, kPadNone, mStartScreen.c_str());
    TheUI.GotoScreen(pError);
}

void ErrorScreen::ShowNoSpaceError(int nSpace) {
    auto *pError = dynamic_cast<SpaceErrorScreen *>(TheUI.FindScreen(kNoSpaceScreen, false));
    pError->SetSlot(mSlot);
    pError->mSpace = nSpace;
    pError->ClearTransitions();
    pError->AddTransition(kRetryComponent, kPadNone, mName);
    pError->AddTransition(kContinueComponent, kPadNone, mDoneScreen.c_str());
    TheUI.GotoScreen(pError);
}
