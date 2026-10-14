#pragma once

#include <librtmedius.h>

#include "netflow/nettask.h"

/**
 * Step that prints the version of the lobby server.
 *
 * The RTTI includes the name and records NetTask as the base. The vtable is at `0x003d56c8`. The
 * `print_version` setting of the `net` configuration adds the step. It always succeeds.
 */
class VersionServerTask : public NetTask {
public:
    /**
     * Print the version and finish the step the response is tagged with.
     *
     * The name is inferred.
     *
     * @param nConnection The connection.
     * @param nType The kind of response.
     * @param nSize The size of the response.
     * @param pResponse The response.
     * @ghidraAddress NTSC-U/C: 0x00263718
     * @ghidraAddress PAL: 0x0026c970
     */
    static void
    OnResponse(int nConnection, int nType, int nSize, const rtMediusVersionResponse *pResponse);

protected:
    /**
     * Send the request.
     *
     * @ghidraAddress NTSC-U/C: 0x00263678
     * @ghidraAddress PAL: 0x0026c8d0
     */
    void OnStart() override;
};
