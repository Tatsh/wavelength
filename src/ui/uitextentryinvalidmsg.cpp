#include "ui/uitextentryinvalidmsg.h"

int g_nUITextEntryInvalidMsgType = 207;

void UITextEntryInvalidMsg::PrintExtra(PrnStream &stream) const {
    stream << " invalid text Entry with char: " << mChar << " and hit end of field:" << mEndOfField;
}
