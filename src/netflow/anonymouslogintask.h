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
protected:
    /**
     * Send the request with the name and the statistics of the local player.
     *
     * @ghidraAddress NTSC-U/C: 0x00263bb0
     */
    void OnStart() override;
};
