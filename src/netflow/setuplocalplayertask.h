#pragma once

#include "netflow/nettask.h"

class LobbyPlayer;

/**
 * Step that takes the record the server keeps for the local player.
 *
 * The RTTI includes the name and records NetTask as the base. The vtable is at `0x003d5518` and the
 * object is 0x14 bytes.
 */
class SetupLocalPlayerTask : public NetTask {
public:
    /**
     * Construct a step.
     *
     * Inline.
     *
     * @param pSource The player the server reported.
     * @param pTarget The local player.
     */
    SetupLocalPlayerTask(LobbyPlayer *pSource, LobbyPlayer *pTarget)
        : mSource(pSource), mTarget(pTarget) {
    }

    LobbyPlayer *mSource; /*!< The player the server reported. */
    LobbyPlayer *mTarget; /*!< The local player. */

protected:
    /**
     * Copy the rating, the games, and the record when the server reported a rating, and finish.
     *
     * @ghidraAddress NTSC-U/C: 0x00263e08
     * @ghidraAddress PAL: 0x0026d490
     */
    void OnStart() override;
};
