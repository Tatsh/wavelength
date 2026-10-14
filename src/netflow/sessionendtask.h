#pragma once

#include <librtmedius.h>

#include "netflow/nettask.h"

/**
 * Step that ends the session on the lobby server.
 *
 * The RTTI includes the name and records NetTask as the base. The vtable is at `0x003d56f8` and the
 * object is 0x10 bytes. The step always succeeds, at the latest three seconds after it starts.
 */
class SessionEndTask : public NetTask {
public:
    /**
     * Destroy the object.
     *
     * @ghidraAddress NTSC-U/C: 0x0039ba68
     * @ghidraAddress PAL: 0x0040a6b0
     */
    ~SessionEndTask() override = default;

    /**
     * Forget the key of the session and finish the step the response is tagged with.
     *
     * The name is inferred.
     *
     * @param nConnection The connection.
     * @param nType The kind of response.
     * @param nSize The size of the response.
     * @param pResponse The response.
     * @ghidraAddress NTSC-U/C: 0x00263620
     * @ghidraAddress PAL: 0x0026c878
     */
    static void
    OnResponse(int nConnection, int nType, int nSize, const rtMediusResponse *pResponse);

    float mDeadline; /*!< The SystemMs() reading the step finishes at. */

protected:
    /**
     * Send the request, or finish when there is no session or the connection failed.
     *
     * @ghidraAddress NTSC-U/C: 0x00263440
     * @ghidraAddress PAL: 0x0026c698
     */
    void OnStart() override;

    /**
     * Finish once mDeadline passes.
     *
     * @ghidraAddress NTSC-U/C: 0x00263590
     * @ghidraAddress PAL: 0x0026c7e8
     */
    void OnPoll() override;
};
