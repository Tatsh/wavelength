#pragma once

#include "app/msgsink.h"
#include "netflow/nettask.h"

class InetCheckConnectionResultMsg;

/**
 * Step that checks that the network connection works.
 *
 * The RTTI includes the name and records NetTask and MsgSink as the bases. The vtables are at
 * `0x003d5898` and, for the MsgSink part, `0x003d5870`. The step fails with
 * LobbyConnectResultMsg::kResultLostInternet.
 */
class CheckInetTask : public NetTask, public MsgSink {
public:
    /**
     * Pass an InetCheckConnectionResultMsg to OnResult().
     *
     * @param pMsg The message.
     * @return False.
     * @ghidraAddress NTSC-U/C: 0x00262bd0
     * @ghidraAddress PAL: 0x0026bd28
     */
    bool DispatchPriv(Message *pMsg) override;

protected:
    /**
     * Request the check of the connection.
     *
     * @ghidraAddress NTSC-U/C: 0x00262b70
     * @ghidraAddress PAL: 0x0026bcc8
     */
    void OnStart() override;

private:
    /**
     * Finish with the result of the check.
     *
     * @param pMsg The result.
     * @return False.
     * @ghidraAddress NTSC-U/C: 0x00262ba8
     * @ghidraAddress PAL: 0x0026bd00
     */
    bool OnResult(InetCheckConnectionResultMsg *pMsg);
};
