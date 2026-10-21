#pragma once

#include <librtmedius.h>

#include "netflow/nettask.h"

class LobbyPlayer;

/**
 * Step that looks up the account of a player by name.
 *
 * The RTTI includes the name and records NetTask as the base. The vtable is at `0x003d5488` and the
 * object is 0x10 bytes. The step fails with LoginResultMsg::kResultAccountNotFound.
 */
class AccountGetIDTask : public NetTask {
public:
    /**
     * Destroy the object.
     *
     * @ghidraAddress NTSC-U/C: 0x0039c030
     * @ghidraAddress PAL: 0x0040ac78
     */
    ~AccountGetIDTask() override = default;

    /**
     * Construct a step.
     *
     * Inline.
     *
     * @param pPlayer The player, whose name is looked up and whose account is set.
     */
    explicit AccountGetIDTask(LobbyPlayer *pPlayer) : mPlayer(pPlayer) {
    }

    /**
     * Take the account and finish the step the response is tagged with.
     *
     * The name is inferred.
     *
     * @param nConnection The connection.
     * @param nType The kind of response.
     * @param nSize The size of the response.
     * @param pResponse The response.
     * @ghidraAddress NTSC-U/C: 0x00264140
     * @ghidraAddress PAL: 0x0026d8c0
     */
    static void OnResponse(int nConnection,
                           int nType,
                           int nSize,
                           const rtMediusAccountGetIdResponse *pResponse);

protected:
    /**
     * Send the request.
     *
     * @ghidraAddress NTSC-U/C: 0x00264090
     * @ghidraAddress PAL: 0x0026d7b8
     */
    void OnStart() override;

private:
    LobbyPlayer *mPlayer; /*!< The player. */
};
