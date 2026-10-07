#pragma once

#include "msg/message.h"

/**
 * Identity that PlayerAbortedMsg::Type() reports.
 *
 * @ghidraAddress NTSC-U/C: 0x003b0b84
 */
extern int g_nPlayerAbortedMsgType;

/**
 * Message that reports a player leaving an online session.
 *
 * The RTTI records the class as deriving from Message. Only the member its receivers here read is
 * declared.
 */
class PlayerAbortedMsg : public Message {
public:
    int mNetOrder; /*!< The player's position in the order of the session. */
};
