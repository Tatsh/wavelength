#include "ui/uitransitioncompletemsg.h"

int g_nUITransitionCompleteMsgType = 205;

void UITransitionCompleteMsg::PrintExtra(PrnStream &stream) const {
    stream << "  from " << (mPrevScreen != nullptr ? mPrevScreen->GetName() : "null") << "  to "
           << (mScreen != nullptr ? mScreen->GetName() : "null");
}
