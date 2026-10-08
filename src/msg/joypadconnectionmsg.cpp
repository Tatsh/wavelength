#include "msg/joypadconnectionmsg.h"

int g_nJoypadConnectionMsgType = 101;

void JoypadConnectionMsg::PrintExtra(PrnStream &stream) const {
    stream << "  Player#: " << mPad << "  Connected?: " << (mConnected != 0);
}
