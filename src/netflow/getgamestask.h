#pragma once

#include <librtmedius.h>

#include "netflow/nettask.h"

/**
 * Step that lists a page of the launchpads into LobbyRT::mLaunchpads.
 *
 * The RTTI includes the name and records NetTask as the base. The vtable is at `0x003d5338`. The
 * server applies LobbyRT::mFilters and sends one game per response. The step always succeeds.
 */
class GetGamesTask : public NetTask {
public:
    /**
     * Destroy the object.
     *
     * @ghidraAddress NTSC-U/C: 0x0039c348
     */
    ~GetGamesTask() override = default;

    /**
     * Decode and add a game, and finish the step the response is tagged with after the last game.
     *
     * The name is inferred.
     *
     * @param nConnection The connection.
     * @param nType The kind of response.
     * @param nSize The size of the response.
     * @param pResponse The response.
     * @ghidraAddress NTSC-U/C: 0x00264b18
     */
    static void
    OnResponse(int nConnection, int nType, int nSize, const rtMediusGameListResponse *pResponse);

protected:
    /**
     * Clear the launchpads and send the request for the page of LobbyRT::mLaunchpadsPage.
     *
     * @ghidraAddress NTSC-U/C: 0x00264a50
     * @ghidraAddress PAL: 0x0026e3b8
     */
    void OnStart() override;
};
