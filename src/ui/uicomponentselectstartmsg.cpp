#include "ui/uicomponentselectstartmsg.h"

int g_nUIComponentSelectStartMsgType = 202;

void UIComponentSelectStartMsg::PrintExtra(PrnStream &stream) const {
    stream << "  UIComponent: " << (mComponent != nullptr ? mComponent->GetName() : "null")
           << "  on UIPanel: " << (mPanel != nullptr ? mPanel->GetName() : "null")
           << "  on UIScreen: " << (mScreen != nullptr ? mScreen->GetName() : "null");
}
