#pragma once

#include "netflow/nettask.h"

/**
 * Step that logs in the local player without an account.
 *
 * The RTTI includes the name and records NetTask as the base. The vtable is at `0x003d5578`. The
 * `anon_login` setting of the `net` configuration adds the step in place of an AccountLoginTask,
 * and AccountLoginTask::OnResponse() receives the response.
 */
class AnonymousLoginTask : public NetTask {
public:
    /**
     * Destroy the object.
     *
     * @ghidraAddress NTSC-U/C: 0x0039bdf8
     * @ghidraAddress PAL: 0x0040aa40
     */
    ~AnonymousLoginTask() override = default;

protected:
    /**
     * Send the request with the name and the statistics of the local player.
     *
     * @ghidraAddress NTSC-U/C: 0x00263bb0
     * @ghidraAddress PAL: 0x0026cfe8
     */
    void OnStart() override;
};
