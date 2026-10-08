#include "msg/joypadinputmsg.h"

int g_nJoypadInputMsgType = 100;

void JoypadInputMsg::PrintExtra(PrnStream &stream) const {
    // The word goes to the bool writer, which converts it at the call as retail does.
    PrnStream &(PrnStream::*const pfnWriteBool)(bool) = &PrnStream::operator<<;
    ((stream << "  Player#: " << mPad << "  Button: " << mButton << "  Pressed: ").*
     pfnWriteBool)(mPressed);
}
