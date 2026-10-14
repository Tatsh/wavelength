#pragma once

#include <librtmedius.h>

#include "netflow/nettask.h"

/**
 * Step that lists the players of the launchpad LobbyRT::mLaunchpadId into
 * LobbyRT::mLaunchpadPlayers.
 *
 * The RTTI includes the name and records NetTask as the base. The vtable is at `0x003d5278`. The
 * server sends one player per response. The step always succeeds.
 */
class GetGamePlayersTask : public NetTask {
public:
    /**
     * Destroy the object.
     *
     * @ghidraAddress NTSC-U/C: 0x0039c508
     */
    ~GetGamePlayersTask() override = default;

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
     * @ghidraAddress NTSC-U/C: 0x002651e0
     */
    static void
    OnResponse(int nConnection, int nType, int nSize, const rtMediusGamePlayersResponse *pResponse);

protected:
    /**
     * Clear the players and send the request.
     *
     * @ghidraAddress NTSC-U/C: 0x00265128
     */
    void OnStart() override;
};
