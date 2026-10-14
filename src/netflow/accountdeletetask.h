#pragma once

#include <librtmedius.h>

#include "netflow/nettask.h"

/**
 * Step that deletes the account of the local player.
 *
 * The RTTI includes the name and records NetTask as the base. The vtable is at `0x003d55a8`. The
 * step fails with DeleteAccountResultMsg::kResultDeleteFailed, or with
 * LoginResultMsg::kResultServerError when the request cannot be sent.
 */
class AccountDeleteTask : public NetTask {
public:
    /**
     * Destroy the object.
     *
     * @ghidraAddress NTSC-U/C: 0x0039bd88
     */
    ~AccountDeleteTask() override = default;

    /**
     * Finish the step the response is tagged with.
     *
     * The name is inferred.
     *
     * @param nConnection The connection.
     * @param nType The kind of response.
     * @param nSize The size of the response.
     * @param pResponse The response.
     * @ghidraAddress NTSC-U/C: 0x00263b70
     * @ghidraAddress PAL: 0x0026cfa8
     */
    static void
    OnResponse(int nConnection, int nType, int nSize, const rtMediusResponse *pResponse);

protected:
    /**
     * Send the request.
     *
     * @ghidraAddress NTSC-U/C: 0x00263ab8
     */
    void OnStart() override;
};
