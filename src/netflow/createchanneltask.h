#pragma once

#include <librtmedius.h>

#include "netflow/nettask.h"

/**
 * Step that creates the chat room LobbyRT::mNewChatroomName.
 *
 * The RTTI includes the name and records NetTask as the base. The vtable is at `0x003d52d8`. The
 * new room goes to LobbyRT::mJoinChatroomId, and the step fails with a
 * JoinChatroomResultMsg::Result.
 */
class CreateChannelTask : public NetTask {
public:
    /**
     * Take the new room and finish the step the response is tagged with.
     *
     * The name is inferred.
     *
     * @param nConnection The connection.
     * @param nType The kind of response.
     * @param nSize The size of the response.
     * @param pResponse The response.
     * @ghidraAddress NTSC-U/C: 0x002662b0
     * @ghidraAddress PAL: 0x0026fdc8
     */
    static void OnResponse(int nConnection,
                           int nType,
                           int nSize,
                           const rtMediusCreateChannelResponse *pResponse);

protected:
    /**
     * Send the request, with the capacity of the room of the local player.
     *
     * @ghidraAddress NTSC-U/C: 0x002661e8
     */
    void OnStart() override;
};
