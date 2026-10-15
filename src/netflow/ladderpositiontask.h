#pragma once

#include <librtmedius.h>

#include "netflow/nettask.h"

class LobbyPlayer;

/**
 * Step that reads the position of a player in the ranking into LobbyPlayer::mRank.
 *
 * The RTTI includes the name and records NetTask as the base. The vtable is at `0x003d5668` and the
 * object is 0x10 bytes. The step always succeeds.
 */
class LadderPositionTask : public NetTask {
public:
    /**
     * Destroy the object.
     *
     * @ghidraAddress NTSC-U/C: 0x0039bbc0
     * @ghidraAddress PAL: 0x0040a808
     */
    ~LadderPositionTask() override = default;

    /**
     * Construct a step.
     *
     * Inline.
     *
     * @param pPlayer The player.
     */
    explicit LadderPositionTask(LobbyPlayer *pPlayer) : mPlayer(pPlayer) {
    }

    /**
     * Take the position, or 0 after an error, and finish the step the response is tagged with.
     *
     * The name is inferred.
     *
     * @param nConnection The connection.
     * @param nType The kind of response.
     * @param nSize The size of the response.
     * @param pResponse The response.
     * @ghidraAddress NTSC-U/C: 0x00265b38
     * @ghidraAddress PAL: 0x0026f5f8
     */
    static void OnResponse(int nConnection,
                           int nType,
                           int nSize,
                           const rtMediusLadderPositionResponse *pResponse);

    LobbyPlayer *mPlayer; /*!< The player. */

protected:
    /**
     * Send the request.
     *
     * @ghidraAddress NTSC-U/C: 0x00265a78
     * @ghidraAddress PAL: 0x0026f538
     */
    void OnStart() override;
};
