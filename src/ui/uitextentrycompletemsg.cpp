#include "ui/uitextentrycompletemsg.h"

int g_nUITextEntryCompleteMsgType = 206;

void UITextEntryCompleteMsg::PrintExtra(PrnStream &stream) const {
    (stream << "  entered text ").Print(mText.c_str());
}
