#pragma once

#include <librtmedius.h>

#include "netflow/nettask.h"

class LobbyPlayer;

/**
 * Step that reads the name, the connection, and the statistics of a player.
 *
 * The RTTI includes the name and records NetTask as the base. The vtable is at `0x003d5458` and the
 * object is 0x14 bytes. The step fails with LoginResultMsg::kResultAccountNotFound.
 */
class PlayerInfoTask : public NetTask {
public:
    /**
     * Destroy the object.
     *
     * @ghidraAddress NTSC-U/C: 0x0039c0a0
     * @ghidraAddress PAL: 0x0040ace8
     */
    ~PlayerInfoTask() override = default;

    /**
     * Construct a step.
     *
     * Inline.
     *
     * @param pPlayer The player, whose account is looked up.
     * @param nFlag The value of mFlag.
     */
    PlayerInfoTask(LobbyPlayer *pPlayer, int nFlag) : mPlayer(pPlayer), mFlag(nFlag) {
    }

    /**
     * Take the name, the connection, and the statistics, and finish the step the response is
     * tagged with.
     *
     * The name is inferred.
     *
     * @param nConnection The connection.
     * @param nType The kind of response.
     * @param nSize The size of the response.
     * @param pResponse The response.
     * @ghidraAddress NTSC-U/C: 0x00264270
     * @ghidraAddress PAL: 0x0026d9f0
     */
    static void
    OnResponse(int nConnection, int nType, int nSize, const rtMediusPlayerInfoResponse *pResponse);

protected:
    /**
     * Send the request.
     *
     * @ghidraAddress NTSC-U/C: 0x002641c8
     * @ghidraAddress PAL: 0x0026d948
     */
    void OnStart() override;

private:
    LobbyPlayer *mPlayer; /*!< The player. */
    int mFlag;            /*!< 1 in the chains of LobbyRT, 0 in LadderListTask. Never read. */
};
