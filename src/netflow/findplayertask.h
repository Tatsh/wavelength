#pragma once

#include <librtmedius.h>

#include "netflow/nettask.h"

class LobbyPlayer;
struct NetChatroomInfo;

/**
 * Step that finds the chat room a player is in.
 *
 * The RTTI includes the name and records NetTask as the base. The vtable is at `0x003d5218` and the
 * object is 0x14 bytes. The step always succeeds.
 */
class FindPlayerTask : public NetTask {
public:
    /**
     * Destroy the object.
     *
     * @ghidraAddress NTSC-U/C: 0x0039c5c8
     * @ghidraAddress PAL: 0x0040b280
     */
    ~FindPlayerTask() override = default;

    /**
     * Construct a step.
     *
     * Inline.
     *
     * @param pChatroom Receives the identifier of the room.
     * @param pPlayer The player.
     */
    FindPlayerTask(NetChatroomInfo *pChatroom, LobbyPlayer *pPlayer)
        : mChatroom(pChatroom), mPlayer(pPlayer) {
    }

    /**
     * Take the room of a location of the player, and finish the step the response is tagged with
     * after the last location.
     *
     * The name is inferred.
     *
     * @param nConnection The connection.
     * @param nType The kind of response.
     * @param nSize The size of the response.
     * @param pResponse The response.
     * @ghidraAddress NTSC-U/C: 0x00265fd8
     * @ghidraAddress PAL: 0x0026fa98
     */
    static void
    OnResponse(int nConnection, int nType, int nSize, const rtMediusFindPlayerResponse *pResponse);

    NetChatroomInfo *mChatroom; /*!< Receives the identifier of the room. */
    LobbyPlayer *mPlayer;       /*!< The player. */

protected:
    /**
     * Clear the room and send the request, or finish for an offline player.
     *
     * @ghidraAddress NTSC-U/C: 0x00265ef8
     * @ghidraAddress PAL: 0x0026f9b8
     */
    void OnStart() override;
};
