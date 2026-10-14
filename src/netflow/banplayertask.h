#pragma once

#include <librtmedius.h>

#include "netflow/nettask.h"

/**
 * Step that bans a player from the game of the host on the lobby server.
 *
 * The RTTI includes the name and records NetTask as the base. The vtable is at `0x003d49c8`, and
 * the object is 0x14 bytes. HostpadRT::Kick() runs the step on the stack.
 */
class BanPlayerTask : public NetTask {
public:
    /** The length of a ban the request gives. */
    static constexpr int kBanDuration = 60;

    /**
     * Construct a step for a player.
     *
     * Inline.
     *
     * @param nWorldId The game.
     * @param nAccountId The account of the player.
     */
    BanPlayerTask(int nWorldId, int nAccountId) : mWorldId(nWorldId), mAccountId(nAccountId) {
    }

    /**
     * Ignore the response of the lobby server.
     *
     * @param nConnection The connection.
     * @param nType The kind of response.
     * @param nSize The size of the response.
     * @param pResponse The response.
     * @ghidraAddress NTSC-U/C: 0x0025c6e0
     * @ghidraAddress PAL: 0x00265740
     */
    static void
    OnResponse(int nConnection, int nType, int nSize, const rtMediusResponse *pResponse);

    int mWorldId;   /*!< The game. */
    int mAccountId; /*!< The account of the player. */

protected:
    /**
     * Send the request and finish.
     *
     * @ghidraAddress NTSC-U/C: 0x0025c660
     * @ghidraAddress PAL: 0x002656c0
     */
    void OnStart() override;
};
