#pragma once

#include "netflow/nettask.h"

/**
 * Step that tells every console of the session, the host included, to load the song.
 *
 * The RTTI includes the name and records NetTask as the base. The vtable is at `0x003d4968`.
 */
class LoadGameTask : public NetTask {
protected:
    /**
     * Send LoadGamePkt and pass it to the session of this console, and finish.
     *
     * @ghidraAddress NTSC-U/C: 0x0025cbb0
     * @ghidraAddress PAL: 0x00265c10
     */
    void OnStart() override;
};
