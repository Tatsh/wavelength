#pragma once

#include <librtmedius.h>

#include "netflow/nettask.h"

/**
 * Step that reads the licence agreement of the lobby server into LobbyRT::mPolicy.
 *
 * The RTTI includes the name and records NetTask as the base. The vtable is at `0x003d5398`. The
 * server sends the text in parts. The step always succeeds.
 */
class GetPolicyTask : public NetTask {
public:
    /**
     * Destroy the object.
     *
     * @ghidraAddress NTSC-U/C: 0x0039c268
     * @ghidraAddress PAL: 0x0040af20
     */
    ~GetPolicyTask() override = default;

    /**
     * Append a part of the text, and finish the step the response is tagged with after the last
     * part.
     *
     * The name is inferred.
     *
     * @param nConnection The connection.
     * @param nType The kind of response.
     * @param nSize The size of the response.
     * @param pResponse The response.
     * @ghidraAddress NTSC-U/C: 0x00264748
     * @ghidraAddress PAL: 0x0026dff0
     */
    static void
    OnResponse(int nConnection, int nType, int nSize, const rtMediusPolicyResponse *pResponse);

protected:
    /**
     * Clear the text and send the request.
     *
     * @ghidraAddress NTSC-U/C: 0x00264690
     * @ghidraAddress PAL: 0x0026df38
     */
    void OnStart() override;
};
