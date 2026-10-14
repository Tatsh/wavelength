#pragma once

#include <librtmedius.h>

#include "netflow/nettask.h"

/**
 * Step that joins the game of the session on the lobby server, which gives the connection to it.
 *
 * The RTTI includes the name and records NetTask as the base. The vtable is at `0x003d4ae8`.
 */
class MediusJoinGameTask : public NetTask {
public:
    /**
     * Destroy the object.
     *
     * @ghidraAddress NTSC-U/C: 0x0039a198
     * @ghidraAddress PAL: 0x00408a20
     */
    ~MediusJoinGameTask() override = default;

    /** The status of the response for a game that refuses this console. */
    static constexpr int kStatusDenied = -972;

    /**
     * Take the connection into LaunchpadRT::mConnectionInfo and finish the step.
     *
     * A refusal fails the step with LaunchpadAbortedMsg::kReasonDenied, and another error with
     * LaunchpadAbortedMsg::kReasonConnectFailed. The name is inferred.
     *
     * @param nConnection The connection.
     * @param nType The kind of response.
     * @param nSize The size of the response.
     * @param pResponse The response.
     * @ghidraAddress NTSC-U/C: 0x0025c140
     * @ghidraAddress PAL: 0x002651a0
     */
    static void
    OnResponse(int nConnection, int nType, int nSize, const rtMediusJoinGameResponse *pResponse);

protected:
    /**
     * Send the request, which fails the step with LaunchpadAbortedMsg::kReasonInit.
     *
     * @ghidraAddress NTSC-U/C: 0x0025c0b0
     * @ghidraAddress PAL: 0x00265110
     */
    void OnStart() override;
};
