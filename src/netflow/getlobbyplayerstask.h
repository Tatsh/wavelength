#pragma once

#include <librtmedius.h>

#include "netflow/nettask.h"

/**
 * Step that lists the players of the chat room of the local player into LobbyRT::mLobbyPlayers.
 *
 * The RTTI includes the name and records NetTask as the base. The vtable is at `0x003d52a8`. The
 * server sends one player per response. The step always succeeds.
 */
class GetLobbyPlayersTask : public NetTask {
public:
    /**
     * Destroy the object.
     *
     * @ghidraAddress NTSC-U/C: 0x0039c498
     * @ghidraAddress PAL: 0x0040b150
     */
    ~GetLobbyPlayersTask() override = default;

    /**
     * Decode and add a player, and finish the step the response is tagged with after the last
     * player.
     *
     * The name is inferred.
     *
     * @param nConnection The connection.
     * @param nType The kind of response.
     * @param nSize The size of the response.
     * @param pResponse The response.
     * @ghidraAddress NTSC-U/C: 0x00264ea0
     * @ghidraAddress PAL: 0x0026e858
     */
    static void OnResponse(int nConnection,
                           int nType,
                           int nSize,
                           const rtMediusChannelPlayersResponse *pResponse);

protected:
    /**
     * Clear the players and send the request.
     *
     * @ghidraAddress NTSC-U/C: 0x00264de8
     * @ghidraAddress PAL: 0x0026e7a0
     */
    void OnStart() override;
};
