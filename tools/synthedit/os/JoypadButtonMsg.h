#pragma once

#include "utl/Message.h"

/**
 * The type of a JoypadButtonMsg.
 *
 * @ghidraAddress 0x100390a8
 */
extern int gJoypadButtonMsgType;

/**
 * A change of a controller button.
 *
 * The control never constructs one; CheatsManager reads these members. The class name is inferred.
 */
class JoypadButtonMsg : public Message {
public:
    virtual int Type() {
        return gJoypadButtonMsgType;
    }

    int mPad;      /*!< The controller. */
    int mButton;   /*!< The button. */
    bool mPressed; /*!< Whether the button went down. */
};
