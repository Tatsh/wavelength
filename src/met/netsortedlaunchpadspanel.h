#pragma once

#include "met/freqpanel.h"
#include "met/netflashupdate.h"
#include "msg/joypadinputmsg.h"
#include "msg/lobbylaunchpadsmsg.h"
#include "os/string.h"
#include "rnd/text.h"
#include "script/dataarray.h"
#include "ui/uicomponentselectmsg.h"
#include "ui/uipanel.h"

/**
 * Panel with the list of launchpads sorted or found by the player, whose mesh flashes while the
 * list loads.
 *
 * The RTTI records the class as deriving from FreqPanel and from NetFlashUpdate.
 */
class NetSortedLaunchpadsPanel : public FreqPanel, public NetFlashUpdate {
public:
    /** The pages Request() takes, as NetLobby::RequestLaunchpads() takes them. */
    enum Page {
        kPageFirst = 0,    /*!< The start of the list. */
        kPageRefresh = 2,  /*!< The list again, from the L1 button. */
        kPagePrevious = 3, /*!< The page before the list. */
    };

    /**
     * Construct the panel from its script description, with an empty query.
     *
     * @param pData The script description.
     * @param pszDir The directory of the description file.
     * @ghidraAddress NTSC-U/C: 0x001762b0
     * @ghidraAddress PAL: 0x00179710
     */
    NetSortedLaunchpadsPanel(DataArray *pData, const char *pszDir);

    /**
     * Destroy the panel.
     *
     * @ghidraAddress NTSC-U/C: 0x003587e0
     * @ghidraAddress PAL: 0x003c5b90
     */
    ~NetSortedLaunchpadsPanel() override {
    }

    /**
     * Create a panel from its script description.
     *
     * @param pData The script description.
     * @param pszDir The directory of the description file.
     * @return The new panel.
     * @ghidraAddress NTSC-U/C: 0x003588a8
     * @ghidraAddress PAL: 0x003c5c58
     */
    static UIPanel *New(DataArray *pData, const char *pszDir) {
        return new NetSortedLaunchpadsPanel(pData, pszDir);
    }

    /**
     * Route the list of launchpads, the choice, and the controller messages.
     *
     * @param pMsg The message.
     * @return Whether the message was handled.
     * @ghidraAddress NTSC-U/C: 0x001765a8
     * @ghidraAddress PAL: 0x00179a08
     */
    bool DispatchPriv(Message *pMsg) override;

    /**
     * Enter, find the objects of the flash, and request the start of the list.
     *
     * @param bForce Whether the panel skips its animation.
     * @param fTime The front-end time in milliseconds.
     * @ghidraAddress NTSC-U/C: 0x00176358
     * @ghidraAddress PAL: 0x001797b8
     */
    void Enter(bool bForce, float fTime) override;

    /**
     * Set the search whose sessions the panel lists.
     *
     * @param pszArena The arena, or an empty string for any.
     * @param nRuleSet The game mode.
     * @param nSkillLevel The skill level, or -1 for any.
     * @ghidraAddress NTSC-U/C: 0x00176310
     * @ghidraAddress PAL: 0x00179770
     */
    void SetSearch(const char *pszArena, int nRuleSet, int nSkillLevel);

    /**
     * Clear the data panel and request a page, unless the previous page is requested and no
     * launchpads precede the list.
     *
     * The name is inferred.
     *
     * @param nPage One of Page.
     * @ghidraAddress NTSC-U/C: 0x00176498
     * @ghidraAddress PAL: 0x001798f8
     */
    void Request(int nPage);

private:
    /**
     * Request the list again with the L1 button.
     *
     * The name is inferred.
     *
     * @param pMsg The message.
     * @return True while a transition runs, otherwise false.
     * @ghidraAddress NTSC-U/C: 0x00176658
     * @ghidraAddress PAL: 0x00179ab8
     */
    bool HandleJoypad(JoypadInputMsg *pMsg);

    /**
     * Fill the list, show the arrows and the notice of an empty list, and stop the flash.
     *
     * The panel does nothing while it is not loaded. The name is inferred.
     *
     * @param pMsg The message.
     * @return False.
     * @ghidraAddress NTSC-U/C: 0x001766b0
     * @ghidraAddress PAL: 0x00179b10
     */
    bool HandleLobbyLaunchpads(LobbyLaunchpadsMsg *pMsg);

    /**
     * Go to join the chosen launchpad when `cursor` is chosen.
     *
     * The name is inferred.
     *
     * @param pMsg The message.
     * @return False.
     * @ghidraAddress NTSC-U/C: 0x00176880
     * @ghidraAddress PAL: 0x00179ce0
     */
    bool HandleSelect(UIComponentSelectMsg *pMsg);

    String mArena;        /*!< The arena of the search, or empty for any. */
    int mRuleSet;         /*!< The game mode of the search. */
    int mSkillLevel;      /*!< The skill level of the search, or -1 for any. */
    int mHasPrevious;     /*!< Non-zero when launchpads precede the last list received. */
    int mPage;            /*!< The page last requested, one of Page. */
    Rnd::Text *mNoneText; /*!< The text `fn_sorted_none.txt` an empty list shows. */
};
