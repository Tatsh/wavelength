#pragma once

#include <list>
#include <memory>

#include <librtmedius.h>

#include "netflow/lobbyplayer.h"
#include "netflow/nettask.h"

class PlayerInfoTask;

/**
 * Step that lists a page of the ranking into LobbyRT::mRankedPlayers.
 *
 * The RTTI includes the name and records NetTask as the base. The vtable is at `0x003d5248` and the
 * object is 0x14 bytes. Unless LobbyRT::mRanksMode is set, the step then reads the details of each
 * player with a PlayerInfoTask, and drops the new players unless the `rank_newbies` setting of the
 * `net` configuration is set. The step always succeeds.
 */
class LadderListTask : public NetTask {
public:
    /**
     * Construct a step.
     *
     * Inline.
     */
    LadderListTask() : mInfoTask(nullptr) {
    }

    /**
     * Release the step and its PlayerInfoTask.
     *
     * @ghidraAddress NTSC-U/C: 0x00265468
     * @ghidraAddress PAL: 0x0026eed0
     */
    ~LadderListTask() override;

    /**
     * Add a player, and after the last player finish the step the response is tagged with or
     * start reading the details of the players.
     *
     * The name is inferred.
     *
     * @param nConnection The connection.
     * @param nType The kind of response.
     * @param nSize The size of the response.
     * @param pResponse The response.
     * @ghidraAddress NTSC-U/C: 0x002657c8
     * @ghidraAddress PAL: 0x0026f230
     */
    static void
    OnResponse(int nConnection, int nType, int nSize, const rtMediusLadderListResponse *pResponse);

    /** @return The step reading the player mNext, or null. */
    PlayerInfoTask *GetInfoTask() const {
        return mInfoTask.get();
    }

protected:
    /**
     * Clear the ranking and send the request for the page of LobbyRT::mRanksPage.
     *
     * @ghidraAddress NTSC-U/C: 0x002654e0
     * @ghidraAddress PAL: 0x0026ef48
     */
    void OnStart() override;

    /**
     * Poll mInfoTask, and once it ends go on to the next player or drop the new players.
     *
     * @ghidraAddress NTSC-U/C: 0x00265648
     * @ghidraAddress PAL: 0x0026f0b0
     */
    void OnPoll() override;

private:
    /**
     * Start reading the details of mNext, or finish after the last player.
     *
     * @ghidraAddress NTSC-U/C: 0x002655c0
     * @ghidraAddress PAL: 0x0026f028
     */
    void StartNextInfo();

    std::unique_ptr<PlayerInfoTask> mInfoTask; /*!< The step reading the player mNext, or null. */
    std::list<LobbyPlayer>::iterator mNext;    /*!< The player whose details are read next. */
};
