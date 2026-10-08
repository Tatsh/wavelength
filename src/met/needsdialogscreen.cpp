#include "met/needsdialogscreen.h"

#include <cstring>

#include "ui/uipanel.h"

namespace {

constexpr char kHelpPanel[] = "help";
constexpr char kTitlePanel[] = "title";

} // namespace

void NeedsDialogScreen::SetPanelsShowing(bool bShowing) {
    for (const auto &entry : mPanels) {
        UIPanel *pPanel = entry.second;
        if (std::strcmp(pPanel->mName, kHelpPanel) != 0 &&
            std::strcmp(pPanel->mName, kTitlePanel) != 0) {
            pPanel->SetShowing(bShowing);
        }
    }
}
