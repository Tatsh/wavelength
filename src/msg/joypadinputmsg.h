#pragma once

#include "msg/message.h"

/**
 * Message the controller poll sends when a button is pressed or released.
 *
 * The class name comes from the name the message reports. Only the members its callers here read
 * are declared.
 */
class JoypadInputMsg : public Message {
public:
    int mPad;     /*!< The controller. */
    int mButton;  /*!< The button. */
    int mPressed; /*!< 1 when the button went down, 0 when it went up. */
};

/**
 * Identity that JoypadInputMsg::Type() reports.
 *
 * @ghidraAddress NTSC-U/C: 0x003b210c
 */
extern int g_nJoypadInputMsgType;
