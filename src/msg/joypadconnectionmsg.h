#pragma once

#include "msg/message.h"

/**
 * Message the controller poll sends when a controller is connected or disconnected.
 *
 * The class name comes from the name the message reports. Only the members its callers here read
 * are declared.
 */
class JoypadConnectionMsg : public Message {
public:
    int mPad;       /*!< The controller. */
    int mConnected; /*!< 1 when the controller was connected, 0 when it was disconnected. */
};

/**
 * Identity that JoypadConnectionMsg::Type() reports.
 *
 * @ghidraAddress NTSC-U/C: 0x003b2110
 */
extern int g_nJoypadConnectionMsgType;
