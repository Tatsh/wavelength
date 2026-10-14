#pragma once

#include <librtmedius.h>

#include "netflow/nettask.h"

class LobbyPlayer;

/**
 * Step that reads the position of a player in the ranking from the rating of the player.
 *
 * The RTTI includes the name and records NetTask as the base. The vtable is at `0x003d5608` and the
 * object is 0x10 bytes. The position goes to LobbyPlayer::mRank, which an error does not change.
 * The step always succeeds.
 */
class LadderPositionFastTask : public NetTask {
public:
    /**
     * Destroy the object.
     *
     * @ghidraAddress NTSC-U/C: 0x0039bca8
     */
    ~LadderPositionFastTask() override = default;

    /**
     * Construct a step.
     *
     * Inline.
     *
     * @param pPlayer The player.
     */
    explicit LadderPositionFastTask(LobbyPlayer *pPlayer) : mPlayer(pPlayer) {
    }

    /**
     * Take the position and finish the step the response is tagged with.
     *
     * The name is inferred.
     *
     * @param nConnection The connection.
     * @param nType The kind of response.
     * @param nSize The size of the response.
     * @param pResponse The response.
     * @ghidraAddress NTSC-U/C: 0x00265e68
     * @ghidraAddress PAL: 0x0026f928
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
     * @ghidraAddress NTSC-U/C: 0x00265db8
     * @ghidraAddress PAL: 0x0026f878
     */
    void OnStart() override;
};
