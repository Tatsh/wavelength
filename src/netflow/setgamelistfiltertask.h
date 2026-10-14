#pragma once

#include <librtmedius.h>

#include "netflow/nettask.h"

/**
 * Step that sets each filter of LobbyRT::mFilters that LobbyRT::mActiveFilters lacks.
 *
 * The RTTI includes the name and records NetTask as the base. The vtable is at `0x003d51e8` and the
 * object is 0x10 bytes. The step fails with JoinChatroomResultMsg::kResultFailed.
 */
class SetGameListFilterTask : public NetTask {
public:
    /**
     * Count down the requests of the step the response is tagged with, and finish it after an
     * error or the last response.
     *
     * The name is inferred.
     *
     * @param nConnection The connection.
     * @param nType The kind of response.
     * @param nSize The size of the response.
     * @param pResponse The response.
     * @ghidraAddress NTSC-U/C: 0x002666c8
     * @ghidraAddress PAL: 0x002701e0
     */
    static void
    OnResponse(int nConnection, int nType, int nSize, const rtMediusResponse *pResponse);

    int mPending; /*!< The requests without a response, set by OnStart(). */

protected:
    /**
     * Send a request for each filter to set, or finish when there is none.
     *
     * @ghidraAddress NTSC-U/C: 0x00266580
     * @ghidraAddress PAL: 0x00270098
     */
    void OnStart() override;
};
