#include "msg/joypadinputmsg.h"

int g_nJoypadInputMsgType = 100;

void JoypadInputMsg::PrintExtra(PrnStream &stream) const {
    stream << "  Player#: " << mPad << "  Button: " << mButton << "  Pressed: " << (mPressed != 0);
}
