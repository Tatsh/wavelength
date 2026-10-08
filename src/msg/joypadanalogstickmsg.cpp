#include "msg/joypadanalogstickmsg.h"

int g_nJoypadAnalogStickMsgType = 102;

void JoypadAnalogStickMsg::PrintExtra(PrnStream &stream) const {
    stream << "  Player#: " << mPad << "  Stick: " << mStick << "  Horizontal: " << mX
           << "  Vertical: " << mY << "\n";
}
