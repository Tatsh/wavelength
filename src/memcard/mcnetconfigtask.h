#pragma once

#include <list>

#include "app/msgsink.h"
#include "memcard/memcardtask.h"
#include "msg/message.h"
#include "netflow/inetconfig.h"
#include "netflow/inetconfigsresultmsg.h"

/**
 * Task that requests the network configurations of the memory cards from the network layer.
 *
 * The RTTI records the class as deriving from MemcardTask and from MsgSink at `+0x10`. The object
 * is 0x1c bytes. The task finishes once its sink receives the InetConfigsResultMsg, and succeeds
 * when the message lists a configuration.
 */
class MCNetConfigTask : public MemcardTask, public MsgSink {
public:
    /**
     * Prepare the task. The task ignores the slot.
     *
     * @param nPort The memory card slot.
     * @ghidraAddress NTSC-U/C: 0x001623f0
     * @ghidraAddress PAL: 0x00165110
     */
    void Set(int nPort);

    /**
     * Pass the result of the request to OnConfigsResult().
     *
     * @param pMsg The message.
     * @return Whether the message was the result.
     * @ghidraAddress NTSC-U/C: 0x00162518
     * @ghidraAddress PAL: 0x00165238
     */
    bool DispatchPriv(Message *pMsg) override;

    std::list<InetConfig> mConfigs; /*!< The configurations found. */

protected:
    /**
     * Clear mConfigs and send the request.
     *
     * @ghidraAddress NTSC-U/C: 0x001624d0
     * @ghidraAddress PAL: 0x001651f0
     */
    void OnStart() override;

private:
    /**
     * Record the configurations and finish.
     *
     * @param pMsg The result.
     * @return True.
     * @ghidraAddress NTSC-U/C: 0x001623f8
     * @ghidraAddress PAL: 0x00165118
     */
    bool OnConfigsResult(InetConfigsResultMsg *pMsg);
};
