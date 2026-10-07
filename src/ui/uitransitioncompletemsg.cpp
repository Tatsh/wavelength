#include "ui/uitransitioncompletemsg.h"

int g_nUITransitionCompleteMsgType = 205;

void UITransitionCompleteMsg::PrintExtra(PrnStream &stream) const {
    stream << "  from " << (mPrevScreen != nullptr ? mPrevScreen->mName : "null") << "  to "
           << (mScreen != nullptr ? mScreen->mName : "null");
}
