#pragma once

#include "netflow/nettask.h"

/**
 * Step that stops the lobby client and the network library.
 *
 * The RTTI includes the name and records NetTask as the base. The vtable is at `0x003d57b8`.
 */
class CloseRTTask : public NetTask {
public:
    /**
     * Destroy the object.
     *
     * @ghidraAddress NTSC-U/C: 0x0039b8a8
     * @ghidraAddress PAL: 0x0040a4f0
     */
    ~CloseRTTask() override = default;

protected:
    /**
     * Stop the library, clear LobbyRT::mRtReady, and finish.
     *
     * @ghidraAddress NTSC-U/C: 0x00262f58
     * @ghidraAddress PAL: 0x0026c0b0
     */
    void OnStart() override;
};
