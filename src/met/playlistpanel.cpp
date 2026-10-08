#include "met/playlistpanel.h"

#include "met/metagame.h"
#include "met/playlist.h"
#include "os/joypad.h"
#include "os/locale.h"
#include "synth/fxmidi.h"
#include "ui/uimanager.h"
#include "ui/uiscreen.h"

namespace {

constexpr char kListComponent[] = "list";
constexpr char kButtonPanel[] = "jbox";
constexpr char kHelpToken[] = "jbox_create_HELP";
constexpr char kActionToken[] = "default_ACTION";

PlayList *FindPlayList(UIPanel *pPanel) {
    return static_cast<PlayList *>(pPanel->FindComponent(kListComponent, false));
}

} // namespace

void PlaylistPanel::Focus() {
    FocusChangePanel::Focus();
    FindPlayList(this)->UpdateCursor();
}

void PlaylistPanel::Unfocus() {
    FocusChangePanel::Unfocus();
    FindPlayList(this)->SetCursorSelected(false);
}

bool PlaylistPanel::DispatchPriv(Message *pMsg) {
    if (pMsg->Type() == g_nJoypadInputMsgType) {
        return HandleJoypad(static_cast<JoypadInputMsg *>(pMsg));
    }
    return UIPanel::DispatchPriv(pMsg);
}

bool PlaylistPanel::HandleJoypad(JoypadInputMsg *pMsg) {
    if (pMsg->mPressed == 0) {
        return false;
    }
    switch (pMsg->mButton) {
    case kPadTriangle:
    case kPadDLeft:
        TheMetagame.SetHelpText(TheLocale.Localize(kHelpToken, true));
        TheMetagame.SetActionText(TheLocale.Localize(kActionToken, true));
        TheUI.mCurrentScreen->SetFocus(TheUI.FindPanel(kButtonPanel, false));
        return false;
    case kPadSquare:
        FxMidi::PlaySquare();
        FindPlayList(this)->ToggleSelected();
        return false;
    case kPadCross:
        return true;
    default:
        return false;
    }
}
