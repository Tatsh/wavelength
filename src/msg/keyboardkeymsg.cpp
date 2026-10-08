#include "msg/keyboardkeymsg.h"

int g_nKeyboardKeyMsgType = 103;

void KeyboardKeyMsg::PrintExtra(PrnStream &stream) const {
    stream << mKey;
}
