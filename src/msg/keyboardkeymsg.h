#pragma once

#include "msg/message.h"

/**
 * Message the keyboard poll sends when a key of a USB keyboard is pressed.
 *
 * The class name comes from the name the message reports. The object is 8 bytes. Only the members
 * its callers here read are declared.
 */
class KeyboardKeyMsg : public Message {
public:
    int mKey; /*!< The key. */
};

/**
 * Identity that KeyboardKeyMsg::Type() reports.
 *
 * @ghidraAddress NTSC-U/C: 0x003b2118
 */
extern int g_nKeyboardKeyMsgType;
