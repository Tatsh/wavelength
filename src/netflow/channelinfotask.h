#pragma once

#include <librtmedius.h>

#include "netflow/nettask.h"

struct NetChatroomInfo;

/**
 * Step that reads the name and the counts of a chat room.
 *
 * The RTTI includes the name and records NetTask as the base. The vtable is at `0x003d5428` and the
 * object is 0x10 bytes. The step fails with JoinChatroomResultMsg::kResultFailed.
 */
class ChannelInfoTask : public NetTask {
public:
    /**
     * Construct a step.
     *
     * Inline.
     *
     * @param pChatroom The room, whose identifier is looked up.
     */
    explicit ChannelInfoTask(NetChatroomInfo *pChatroom) : mChatroom(pChatroom) {
    }

    /**
     * Take the name and the counts, and finish the step the response is tagged with.
     *
     * The name is inferred.
     *
     * @param nConnection The connection.
     * @param nType The kind of response.
     * @param nSize The size of the response.
     * @param pResponse The response.
     * @ghidraAddress NTSC-U/C: 0x00264420
     * @ghidraAddress PAL: 0x0026dc00
     */
    static void
    OnResponse(int nConnection, int nType, int nSize, const rtMediusChannelInfoResponse *pResponse);

protected:
    /**
     * Send the request, or clear the room and finish when it has no identifier.
     *
     * @ghidraAddress NTSC-U/C: 0x00264338
     * @ghidraAddress PAL: 0x0026db18
     */
    void OnStart() override;

private:
    NetChatroomInfo *mChatroom; /*!< The room. */
};
