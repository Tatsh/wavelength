#pragma once

#include "netflow/nettask.h"

/**
 * Step that connects a guest to the connection of the session on the session library.
 *
 * The RTTI includes the name and records NetTask as the base. The vtable is at `0x003d4b48`, and
 * the object is 0x10 bytes. LaunchpadRT::OnNetConnect() finishes the step, which fails with
 * LaunchpadAbortedMsg::kReasonConnectFailed, or with LaunchpadAbortedMsg::kReasonIncompatibleHost
 * once mDeadline passes.
 */
class NetConnectTask : public NetTask {
public:
    /**
     * Destroy the object.
     *
     * @ghidraAddress NTSC-U/C: 0x0039a0b8
     * @ghidraAddress PAL: 0x00408940
     */
    ~NetConnectTask() override = default;

protected:
    /**
     * Start connecting to LaunchpadRT::mConnectionInfo, with LaunchpadRT::sNetTask set to the
     * step.
     *
     * @ghidraAddress NTSC-U/C: 0x0025bac0
     * @ghidraAddress PAL: 0x00264b18
     */
    void OnStart() override;

    /**
     * Clear LaunchpadRT::sNetTask.
     *
     * @ghidraAddress NTSC-U/C: 0x0025bd58
     * @ghidraAddress PAL: 0x00264db0
     */
    void OnStop() override;

    /**
     * Fail once mDeadline passes.
     *
     * @ghidraAddress NTSC-U/C: 0x0025bcc8
     * @ghidraAddress PAL: 0x00264d20
     */
    void OnPoll() override;

private:
    float mDeadline; /*!< When the step fails, from SystemMs(). */
};
