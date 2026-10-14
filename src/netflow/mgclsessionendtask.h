#pragma once

#include <librtmgcl.h>

#include "netflow/nettask.h"

/**
 * Step that reports the end of the game of the host and ends its session with the game server.
 *
 * The RTTI includes the name and records NetTask as the base. The vtable is at `0x003d4c08`, and
 * the object is 0x10 bytes. The step finishes once the server answers or mDeadline passes.
 */
class MGCLSessionEndTask : public NetTask {
public:
    /**
     * Clear HostpadRT::mSessionKey and finish the step.
     *
     * The name is inferred.
     *
     * @param nConnection The connection.
     * @param nType The kind of response.
     * @param nSize The size of the response.
     * @param pResponse The response.
     * @ghidraAddress NTSC-U/C: 0x0025b7a0
     * @ghidraAddress PAL: 0x002647f8
     */
    static void OnResponse(int nConnection, int nType, int nSize, const rtMgclResponse *pResponse);

    float mDeadline; /*!< When the step finishes without an answer, from SystemMs(). */

protected:
    /**
     * Report the end of the game and send the request.
     *
     * @ghidraAddress NTSC-U/C: 0x0025b578
     * @ghidraAddress PAL: 0x002645d0
     */
    void OnStart() override;

    /**
     * Finish once mDeadline passes.
     *
     * @ghidraAddress NTSC-U/C: 0x0025b710
     * @ghidraAddress PAL: 0x00264768
     */
    void OnPoll() override;
};
