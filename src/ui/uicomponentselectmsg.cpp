#include "ui/uicomponentselectmsg.h"

#include "ui/uicomponent.h"
#include "ui/uipanel.h"
#include "ui/uiscreen.h"

int g_nUIComponentSelectMsgType = 201;

void UIComponentSelectMsg::PrintExtra(PrnStream &stream) const {
    stream << "  UIComponent: " << (mComponent != nullptr ? mComponent->mName : "null")
           << "  on UIPanel: " << (mPanel != nullptr ? mPanel->mName : "null")
           << "  on UIScreen: " << (mScreen != nullptr ? mScreen->mName : "null");
}
