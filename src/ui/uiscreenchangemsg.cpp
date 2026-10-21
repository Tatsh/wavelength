#include "ui/uiscreenchangemsg.h"

int g_nUIScreenChangeMsgType = 204;

void UIScreenChangeMsg::PrintExtra(PrnStream &stream) const {
    stream << "  from " << (mOldScreen != nullptr ? mOldScreen->GetName() : "null") << "  to "
           << (mScreen != nullptr ? mScreen->GetName() : "null");
}
