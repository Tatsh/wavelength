#pragma once

#include "msg/message.h"

/**
 * Identity that StartGameMsg::Type() reports.
 *
 * @ghidraAddress NTSC-U/C: 0x003b0b7c
 */
extern int g_nStartGameMsgType;

/**
 * Message that starts the song of an online session on every console.
 *
 * The class name comes from the name the message reports. Only the member its receivers here read
 * is declared.
 */
class StartGameMsg : public Message {
public:
    int mSeed; /*!< The seed of the random numbers every console's logic draws. */
};
