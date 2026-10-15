#pragma once

#include <librtmedius.h>

#include "netflow/nettask.h"

/**
 * Step that tells the lobby server the client sends UTF-8 text in the language of the console.
 *
 * The European release has the step. The RTTI includes the name and records NetTask as the base.
 * The vtable is at `0x0046be00` and the object is 0xc bytes. The step succeeds when the request
 * cannot be sent, and otherwise once the response arrives.
 */
class SetLocParamsTask : public NetTask {
public:
    /**
     * Destroy the object.
     *
     * @ghidraAddress PAL: 0x0040ae40
     */
    ~SetLocParamsTask() override = default;

    /**
     * Finish the step the response is tagged with.
     *
     * The name is inferred.
     *
     * @param nConnection The connection.
     * @param nType The kind of response.
     * @param nSize The size of the response.
     * @param pResponse The response.
     * @ghidraAddress PAL: 0x0026d460
     */
    static void
    OnResponse(int nConnection, int nType, int nSize, const rtMediusResponse *pResponse);

protected:
    /**
     * Send the request and finish.
     *
     * @ghidraAddress PAL: 0x0026d338
     */
    void OnStart() override;
};
