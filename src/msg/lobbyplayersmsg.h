#pragma once

#include <list>

#include "msg/message.h"
#include "netflow/netlaunchpadplayer.h"

/**
 * Identity that LobbyPlayersMsg::Type() reports.
 *
 * @ghidraAddress NTSC-U/C: 0x003b0b38
 */
extern int g_nLobbyPlayersMsgType;

/**
 * Message that delivers the players of an online session the lobby lists.
 *
 * The RTTI records the class as deriving from Message. Only the members its receivers here read
 * are declared.
 */
class LobbyPlayersMsg : public Message {
public:
    std::list<NetLaunchpadPlayer> *mPlayers; /*!< The players. */
    int mLaunchpad;                          /*!< The index of the session in the lobby list. */
};
