#pragma once

#include <librtmedius.h>

#include "netflow/nettask.h"

/**
 * Step that changes the password of the local player to LobbyRT::mNewPassword.
 *
 * The RTTI includes the name and records NetTask as the base. The vtable is at `0x003d54b8`. The
 * step fails with ChangePasswordResultMsg::kResultUpdateFailed.
 */
class AccountUpdatePasswordTask : public NetTask {
public:
    /**
     * Finish the step the response is tagged with.
     *
     * The name is inferred.
     *
     * @param nConnection The connection.
     * @param nType The kind of response.
     * @param nSize The size of the response.
     * @param pResponse The response.
     * @ghidraAddress NTSC-U/C: 0x00264050
     * @ghidraAddress PAL: 0x0026d778
     */
    static void
    OnResponse(int nConnection, int nType, int nSize, const rtMediusResponse *pResponse);

protected:
    /**
     * Send the request.
     *
     * @ghidraAddress NTSC-U/C: 0x00263f78
     */
    void OnStart() override;
};
