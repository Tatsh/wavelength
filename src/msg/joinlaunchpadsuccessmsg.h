#pragma once

#include "msg/message.h"
#include "netflow/netgameparams.h"

/**
 * Identity that JoinLaunchpadSuccessMsg::Type() reports.
 *
 * @ghidraAddress NTSC-U/C: 0x003b0b64
 */
extern int g_nJoinLaunchpadSuccessMsgType;

/**
 * Message that reports that this console joined an online session.
 *
 * The RTTI records the class as deriving from Message. Only the member its receivers here read is
 * declared.
 */
class JoinLaunchpadSuccessMsg : public Message {
public:
    NetGameParams mParams; /*!< The settings of the game the host published. */
};
