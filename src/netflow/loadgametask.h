#pragma once

#include "netflow/nettask.h"

/**
 * Step that tells every console of the session, the host included, to load the song.
 *
 * The RTTI includes the name and records NetTask as the base. The vtable is at `0x003d4968`.
 */
class LoadGameTask : public NetTask {
public:
    /**
     * Destroy the object.
     *
     * @ghidraAddress NTSC-U/C: 0x0039a590
     * @ghidraAddress PAL: 0x00408e18
     */
    ~LoadGameTask() override = default;

protected:
    /**
     * Send LoadGamePkt and pass it to the session of this console, and finish.
     *
     * @ghidraAddress NTSC-U/C: 0x0025cbb0
     * @ghidraAddress PAL: 0x00265c10
     */
    void OnStart() override;
};
