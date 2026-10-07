#include "ui/uicomponentfocuschangemsg.h"

int g_nUIComponentFocusChangeMsgType = 203;

void UIComponentFocusChangeMsg::PrintExtra(PrnStream &stream) const {
    stream << "  from " << (mOldComponent != nullptr ? mOldComponent->mName : "null") << "  to "
           << (mComponent != nullptr ? mComponent->mName : "null")
           << "  on UIPanel: " << (mPanel != nullptr ? mPanel->mName : "null")
           << "  on UIScreen: " << (mScreen != nullptr ? mScreen->mName : "null");
}
