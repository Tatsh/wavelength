#include "ui/uicomponentselectmsg.h"

#include "ui/uicomponent.h"
#include "ui/uipanel.h"
#include "ui/uiscreen.h"

int g_nUIComponentSelectMsgType = 201;

void UIComponentSelectMsg::PrintExtra(PrnStream &stream) const {
    stream << "  UIComponent: " << (mComponent != nullptr ? mComponent->GetName() : "null")
           << "  on UIPanel: " << (mPanel != nullptr ? mPanel->GetName() : "null")
           << "  on UIScreen: " << (mScreen != nullptr ? mScreen->GetName() : "null");
}
