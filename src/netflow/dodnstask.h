#pragma once

#include "app/msgsink.h"
#include "netflow/nettask.h"

class InetDNSLookupResultMsg;

/**
 * Step that looks up the address of the lobby server.
 *
 * The RTTI includes the name and records NetTask and MsgSink as the bases. The vtables are at
 * `0x003d5840` and, for the MsgSink part, `0x003d5818`. The address goes to
 * LobbyRT::mServerAddress, and the step fails with LobbyConnectResultMsg::kResultLookupFailed.
 */
class DoDNSTask : public NetTask, public MsgSink {
public:
    /**
     * Destroy the object.
     *
     * @ghidraAddress NTSC-U/C: 0x0039b778
     * @ghidraAddress PAL: 0x0040a3c0
     */
    ~DoDNSTask() override = default;

    /**
     * Pass an InetDNSLookupResultMsg to OnResult().
     *
     * @param pMsg The message.
     * @return False.
     * @ghidraAddress NTSC-U/C: 0x00262d08
     * @ghidraAddress PAL: 0x0026be60
     */
    bool DispatchPriv(Message *pMsg) override;

protected:
    /**
     * Request the lookup of LobbyRT::mServerName.
     *
     * @ghidraAddress NTSC-U/C: 0x00262c40
     * @ghidraAddress PAL: 0x0026bd98
     */
    void OnStart() override;

    /**
     * Clear the pending lookup.
     *
     * @ghidraAddress NTSC-U/C: 0x00262c80
     * @ghidraAddress PAL: 0x0026bdd8
     */
    void OnStop() override;

private:
    /**
     * Finish with the result of the lookup, and take the address after a success.
     *
     * @param pMsg The result.
     * @return False.
     * @ghidraAddress NTSC-U/C: 0x00262cb0
     * @ghidraAddress PAL: 0x0026be08
     */
    bool OnResult(InetDNSLookupResultMsg *pMsg);
};
