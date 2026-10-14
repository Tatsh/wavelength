#pragma once

#include <librtmedius.h>

#include "netflow/nettask.h"

/**
 * Step that joins the chat room LobbyRT::mJoinChatroomId.
 *
 * The RTTI includes the name and records NetTask as the base. The vtable is at `0x003d5308`. The
 * step fails with JoinChatroomResultMsg::kResultFailed.
 */
class JoinChannelTask : public NetTask {
public:
    /**
     * Destroy the object.
     *
     * @ghidraAddress NTSC-U/C: 0x0039c3b8
     */
    ~JoinChannelTask() override = default;

    /**
     * Take the connection to the room and finish the step the response is tagged with.
     *
     * The name is inferred.
     *
     * @param nConnection The connection.
     * @param nType The kind of response.
     * @param nSize The size of the response.
     * @param pResponse The response.
     * @ghidraAddress NTSC-U/C: 0x00266170
     * @ghidraAddress PAL: 0x0026fc30
     */
    static void
    OnResponse(int nConnection, int nType, int nSize, const rtMediusJoinChannelResponse *pResponse);

protected:
    /**
     * Send the request.
     *
     * @ghidraAddress NTSC-U/C: 0x002660c0
     * @ghidraAddress PAL: 0x0026fb80
     */
    void OnStart() override;
};
