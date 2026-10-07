#pragma once

#include <list>

#include "met/freqscreen.h"
#include "msg/lobbyplayersmsg.h"
#include "netflow/netlaunchpadinfo.h"
#include "os/string.h"
#include "script/dataarray.h"

/**
 * The screen of the online sessions that match a search.
 *
 * The RTTI records the class as deriving from FreqScreen, and its vtable is at `0x003cdb68`. The
 * metagame registers the class for the screen type `net_sorted_screen`. Its panels are
 * `fn_sorted`, the list of sessions, and `fn_sorted_pic`, the chosen session.
 */
class NetSortedScreen : public FreqScreen {
public:
    /**
     * Construct the screen from its script description, with no search.
     *
     * @param pData The script description.
     * @ghidraAddress NTSC-U/C: 0x00186600
     * @ghidraAddress PAL: 0x0018a840
     */
    explicit NetSortedScreen(DataArray *pData);

    /**
     * Create a screen from its script description.
     *
     * The metagame registers the routine for the screen type `net_sorted_screen`.
     *
     * @param pData The script description.
     * @return The new screen.
     * @ghidraAddress NTSC-U/C: 0x0035b8c8
     * @ghidraAddress PAL: 0x003c9420
     */
    static UIScreen *New(DataArray *pData) {
        return new NetSortedScreen(pData);
    }

    /**
     * Route the players of a session, and pass on every other message.
     *
     * @param pMsg The message.
     * @return Whether the message was handled.
     * @ghidraAddress NTSC-U/C: 0x00186798
     * @ghidraAddress PAL: 0x0018a9d8
     */
    bool DispatchPriv(Message *pMsg) override;

    /**
     * Give the search to the list of sessions, start the entry, and clear the chosen session.
     *
     * @param pPrevScreen The screen this one replaces, or null.
     * @param fTime The front-end time in milliseconds.
     * @ghidraAddress NTSC-U/C: 0x001866b0
     * @ghidraAddress PAL: 0x0018a8f0
     */
    void Enter(UIScreen *pPrevScreen, float fTime) override;

    /**
     * Set the search the screen shows the sessions of.
     *
     * @param pszArena The arena, or an empty string for any.
     * @param nRuleSet The game mode.
     * @param nSkillLevel The skill level, or -1 for any.
     * @ghidraAddress NTSC-U/C: 0x00186750
     * @ghidraAddress PAL: 0x0018a990
     */
    void SetSearch(const char *pszArena, int nRuleSet, int nSkillLevel);

    /**
     * Show the players of a session while this screen is the current one.
     *
     * @param pMsg The message.
     * @return False.
     * @ghidraAddress NTSC-U/C: 0x00186800
     * @ghidraAddress PAL: 0x0018aa40
     */
    bool HandleLobbyPlayers(LobbyPlayersMsg *pMsg);

    std::list<NetLaunchpadInfo> mLaunchpads; /*!< Constructed and destroyed only. */
    String mArena;                           /*!< The arena of the search. */
    int mRuleSet;                            /*!< The game mode of the search. */
    int mSkillLevel;                         /*!< The skill level of the search. */
};
