#include "ui/uicomponentselectstartmsg.h"

int g_nUIComponentSelectStartMsgType = 202;

void UIComponentSelectStartMsg::PrintExtra(PrnStream &stream) const {
    stream << "  UIComponent: " << (mComponent != nullptr ? mComponent->mName : "null")
           << "  on UIPanel: " << (mPanel != nullptr ? mPanel->mName : "null")
           << "  on UIScreen: " << (mScreen != nullptr ? mScreen->mName : "null");
}
