#pragma once

#include <vector>

#include <librtmedius.h>

#include "netflow/nettask.h"
#include "netflow/rtfilter.h"

/**
 * Step that clears each filter of LobbyRT::mActiveFilters whose field LobbyRT::mFilters does not
 * filter.
 *
 * The RTTI includes the name and records NetTask as the base. The vtable is at `0x003d51b8` and the
 * object is 0x10 bytes. The step fails with JoinChatroomResultMsg::kResultFailed.
 */
class ClearGameListFilterTask : public NetTask {
public:
    /**
     * Destroy the object.
     *
     * @ghidraAddress NTSC-U/C: 0x0039c6b0
     * @ghidraAddress PAL: 0x0040b368
     */
    ~ClearGameListFilterTask() override = default;

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
     * @ghidraAddress NTSC-U/C: 0x002664d0
     * @ghidraAddress PAL: 0x0026ffe8
     */
    static void
    OnResponse(int nConnection, int nType, int nSize, const rtMediusResponse *pResponse);

    /**
     * Report whether a list of filters filters a field.
     *
     * The name is inferred.
     *
     * @param nField One of RTFilter::Field.
     * @param pFilters The filters.
     * @return Whether a filter has the field.
     * @ghidraAddress NTSC-U/C: 0x00266360
     * @ghidraAddress PAL: 0x0026fe78
     */
    static bool HasField(int nField, const std::vector<RTFilter> *pFilters);

    int mPending; /*!< The requests without a response, set by OnStart(). */

protected:
    /**
     * Send a request for each filter to clear, or finish when there is none.
     *
     * @ghidraAddress NTSC-U/C: 0x002663a0
     * @ghidraAddress PAL: 0x0026feb8
     */
    void OnStart() override;
};
