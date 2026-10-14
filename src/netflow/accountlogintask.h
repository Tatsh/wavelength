#pragma once

#include <librtmedius.h>

#include "netflow/nettask.h"

/**
 * Step that logs in the account of the local player.
 *
 * The RTTI includes the name and records NetTask as the base. The vtable is at `0x003d5548`. The
 * step fails with a LoginResultMsg::Result.
 */
class AccountLoginTask : public NetTask {
public:
    /**
     * Take the account, the chat channel, and the connection of a login, and finish the step the
     * response is tagged with.
     *
     * AnonymousLoginTask shares the member. The name is inferred.
     *
     * @param nConnection The connection.
     * @param nType The kind of response.
     * @param nSize The size of the response.
     * @param pResponse The response.
     * @ghidraAddress NTSC-U/C: 0x00263d60
     * @ghidraAddress PAL: 0x0026d290
     */
    static void
    OnResponse(int nConnection, int nType, int nSize, const rtMediusLoginResponse *pResponse);

protected:
    /**
     * Send the request with the name and the password.
     *
     * @ghidraAddress NTSC-U/C: 0x00263c88
     */
    void OnStart() override;
};
