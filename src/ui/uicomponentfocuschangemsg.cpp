#include "ui/uicomponentfocuschangemsg.h"

int g_nUIComponentFocusChangeMsgType = 203;

void UIComponentFocusChangeMsg::PrintExtra(PrnStream &stream) const {
    stream << "  from " << (mOldComponent != nullptr ? mOldComponent->GetName() : "null") << "  to "
           << (mComponent != nullptr ? mComponent->GetName() : "null")
           << "  on UIPanel: " << (mPanel != nullptr ? mPanel->GetName() : "null")
           << "  on UIScreen: " << (mScreen != nullptr ? mScreen->GetName() : "null");
}
