#pragma once

#include <librtmedius.h>

#include "netflow/nettask.h"

class MsgSink;

/**
 * Step that reports the position of an account in the ranking to a sink as a PlayerRankMsg.
 *
 * The RTTI includes the name and records NetTask as the base. The vtable is at `0x003d5638` and the
 * object is 0x14 bytes. LobbyRT::AddLadderListener() adds the step to LobbyRT::mTasks. The step
 * always succeeds, and reports a position of 0 after an error.
 */
class LadderPosition2Task : public NetTask {
public:
    /**
     * Destroy the object.
     *
     * @ghidraAddress NTSC-U/C: 0x0039bc30
     * @ghidraAddress PAL: 0x0040a878
     */
    ~LadderPosition2Task() override = default;

    /**
     * Construct a step.
     *
     * Inline.
     *
     * @param pSink The receiver of the PlayerRankMsg, or null.
     * @param nAccount The account.
     */
    LadderPosition2Task(MsgSink *pSink, int nAccount) : mSink(pSink), mAccount(nAccount) {
    }

    /**
     * Finish the step the response is tagged with, and report the position.
     *
     * The name is inferred.
     *
     * @param nConnection The connection.
     * @param nType The kind of response.
     * @param nSize The size of the response.
     * @param pResponse The response.
     * @ghidraAddress NTSC-U/C: 0x00265cd0
     * @ghidraAddress PAL: 0x0026f790
     */
    static void OnResponse(int nConnection,
                           int nType,
                           int nSize,
                           const rtMediusLadderPositionResponse *pResponse);

protected:
    /**
     * Send the request, or finish and report a position of 0 when it cannot be sent.
     *
     * @ghidraAddress NTSC-U/C: 0x00265bd0
     * @ghidraAddress PAL: 0x0026f690
     */
    void OnStart() override;

private:
    MsgSink *mSink; /*!< The receiver of the PlayerRankMsg, or null. */
    int mAccount;   /*!< The account. */
};
