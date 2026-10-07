#pragma once

#include "msg/message.h"
#include "netflow/netchatroominfo.h"

/**
 * Identity that JoinChatroomResultMsg::Type() reports.
 *
 * @ghidraAddress NTSC-U/C: 0x003b0b4c
 */
extern int g_nJoinChatroomResultMsgType;

/**
 * Message that reports the result of joining a chat room of the online lobby.
 *
 * The RTTI records the class as deriving from Message. Only the members its receivers here read
 * are declared.
 */
class JoinChatroomResultMsg : public Message {
public:
    int mResult;               /*!< Zero on success, otherwise an error code. */
    NetChatroomInfo mChatroom; /*!< The room joined. */
};
