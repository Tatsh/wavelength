#pragma once

#include <librtmedius.h>

#include "netflow/nettask.h"

/**
 * Step that begins the session on the lobby server.
 *
 * The RTTI includes the name and records NetTask as the base. The vtable is at `0x003d5728`. The
 * key of the session goes to LobbyRT::mSessionKey, and the step fails with
 * LobbyConnectResultMsg::kResultSessionFailed.
 */
class SessionBeginTask : public NetTask {
public:
    /**
     * Take the key of the session and finish the step the response is tagged with.
     *
     * The name is inferred.
     *
     * @param nConnection The connection.
     * @param nType The kind of response.
     * @param nSize The size of the response.
     * @param pResponse The response.
     * @ghidraAddress NTSC-U/C: 0x002633b0
     * @ghidraAddress PAL: 0x0026c608
     */
    static void OnResponse(int nConnection,
                           int nType,
                           int nSize,
                           const rtMediusSessionBeginResponse *pResponse);

protected:
    /**
     * Send the request, with the connection class of the console.
     *
     * @ghidraAddress NTSC-U/C: 0x00263300
     * @ghidraAddress PAL: 0x0026c558
     */
    void OnStart() override;
};
