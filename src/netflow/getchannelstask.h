#pragma once

#include <librtmedius.h>

#include "netflow/nettask.h"

/**
 * Step that lists the chat rooms into LobbyRT::mChatrooms.
 *
 * The RTTI includes the name and records NetTask as the base. The vtable is at `0x003d5368`. The
 * server sends one room per response. The step always succeeds.
 */
class GetChannelsTask : public NetTask {
public:
    /**
     * Destroy the object.
     *
     * @ghidraAddress NTSC-U/C: 0x0039c2d8
     */
    ~GetChannelsTask() override = default;

    /**
     * Add a room, and finish the step the response is tagged with after the last room.
     *
     * The name is inferred.
     *
     * @param nConnection The connection.
     * @param nType The kind of response.
     * @param nSize The size of the response.
     * @param pResponse The response.
     * @ghidraAddress NTSC-U/C: 0x00264890
     */
    static void
    OnResponse(int nConnection, int nType, int nSize, const rtMediusChannelListResponse *pResponse);

protected:
    /**
     * Clear the rooms and send the request.
     *
     * @ghidraAddress NTSC-U/C: 0x002647d8
     * @ghidraAddress PAL: 0x0026e0e8
     */
    void OnStart() override;
};
