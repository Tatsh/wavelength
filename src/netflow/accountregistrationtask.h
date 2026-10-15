#pragma once

#include <librtmedius.h>

#include "netflow/nettask.h"

/**
 * Step that creates the account of the local player.
 *
 * The RTTI includes the name and records NetTask as the base. The vtable is at `0x003d55d8`. The
 * step fails with a CreateAccountResultMsg::Result, or with LoginResultMsg::kResultServerError
 * when the request cannot be sent.
 */
class AccountRegistrationTask : public NetTask {
public:
    /**
     * Destroy the object.
     *
     * @ghidraAddress NTSC-U/C: 0x0039bd18
     * @ghidraAddress PAL: 0x0040a960
     */
    ~AccountRegistrationTask() override = default;

    /**
     * Finish the step the response is tagged with.
     *
     * The name is inferred.
     *
     * @param nConnection The connection.
     * @param nType The kind of response.
     * @param nSize The size of the response.
     * @param pResponse The response.
     * @ghidraAddress NTSC-U/C: 0x00263a30
     * @ghidraAddress PAL: 0x0026ce10
     */
    static void
    OnResponse(int nConnection, int nType, int nSize, const rtMediusResponse *pResponse);

protected:
    /**
     * Send the request, unless the name begins with the letters of the developer.
     *
     * @ghidraAddress NTSC-U/C: 0x00263918
     * @ghidraAddress PAL: 0x0026cc18
     */
    void OnStart() override;
};
