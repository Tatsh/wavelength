#include "msg/joypadconnectionmsg.h"

int g_nJoypadConnectionMsgType = 101;

void JoypadConnectionMsg::PrintExtra(PrnStream &stream) const {
    // The word goes to the bool writer, which converts it at the call as retail does.
    PrnStream &(PrnStream::*const pfnWriteBool)(bool) = &PrnStream::operator<<;
    ((stream << "  Player#: " << mPad << "  Connected?: ").*pfnWriteBool)(mConnected);
}
