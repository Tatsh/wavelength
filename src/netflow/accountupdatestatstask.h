#pragma once

#include <librtmedius.h>

#include "netflow/nettask.h"

/**
 * Step that stores the statistics of the local player on the lobby server.
 *
 * The RTTI includes the name and records NetTask as the base. The vtable is at `0x003d54e8`. The
 * step fails with LoginResultMsg::kResultStatsFailed.
 */
class AccountUpdateStatsTask : public NetTask {
public:
    /**
     * Destroy the object.
     *
     * @ghidraAddress NTSC-U/C: 0x0039bf50
     */
    ~AccountUpdateStatsTask() override = default;

    /**
     * Finish the step the response is tagged with.
     *
     * The name is inferred.
     *
     * @param nConnection The connection.
     * @param nType The kind of response.
     * @param nSize The size of the response.
     * @param pResponse The response.
     * @ghidraAddress NTSC-U/C: 0x00263f38
     * @ghidraAddress PAL: 0x0026d5c0
     */
    static void
    OnResponse(int nConnection, int nType, int nSize, const rtMediusResponse *pResponse);

protected:
    /**
     * Send the request with LobbyRT::GetPlayerStats().
     *
     * @ghidraAddress NTSC-U/C: 0x00263e80
     * @ghidraAddress PAL: 0x0026d508
     */
    void OnStart() override;
};
