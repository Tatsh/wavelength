#pragma once

#include <vector>

#include "met/netparamsscreen.h"
#include "msg/joypadinputmsg.h"
#include "netflow/netgameparams.h"
#include "os/string.h"
#include "script/dataarray.h"
#include "ui/uicomponentselectmsg.h"
#include "ui/uicomponentselectstartmsg.h"
#include "ui/uiscreen.h"

/**
 * Screen where the host of an online game chooses the mode, the skill, the number of players, the
 * power-ups, and the song, or edits them from the launchpad.
 *
 * The RTTI records the class as deriving from NetParamsScreen, and the vtable is at `0x003cdca0`.
 * The destructor at `0x0035a308` (PAL `0x003c7e58`) is compiler-generated. The description may set
 * `is_edit` for the screen that edits the game of an open launchpad.
 */
class NetHostingScreen : public NetParamsScreen {
public:
    /**
     * Construct the screen from its script description.
     *
     * @param pData The script description.
     * @ghidraAddress NTSC-U/C: 0x00180528
     * @ghidraAddress PAL: 0x00184768
     */
    explicit NetHostingScreen(DataArray *pData);

    /**
     * Create a screen from its script description.
     *
     * @param pData The script description.
     * @return The new screen.
     * @ghidraAddress NTSC-U/C: 0x0035b5c8
     * @ghidraAddress PAL: 0x003c9120
     */
    static UIScreen *New(DataArray *pData) {
        return new NetHostingScreen(pData);
    }

    /**
     * Route the choices, the left and right buttons, and the controller to their handlers.
     *
     * @param pMsg The message.
     * @return Whether the message was handled.
     * @ghidraAddress NTSC-U/C: 0x00183690
     * @ghidraAddress PAL: 0x001878d0
     */
    bool DispatchPriv(Message *pMsg) override;

    /**
     * List the modes, the numbers of players, and the power-ups, and choose the settings of the
     * game database.
     *
     * Entered from `fn_h_lpad` to edit, the screen saves the settings for the triangle button to
     * restore. A duel is not offered when the session already takes more than two players.
     *
     * @param pPrevScreen The screen that is exiting.
     * @param fTime The time.
     * @ghidraAddress NTSC-U/C: 0x00180610
     * @ghidraAddress PAL: 0x00184850
     */
    void Enter(UIScreen *pPrevScreen, float fTime) override;

    /**
     * Label the choices, and disable the power-ups for a mode other than the game and the number
     * of players for a duel or while editing.
     *
     * @ghidraAddress NTSC-U/C: 0x001834e8
     * @ghidraAddress PAL: 0x00187728
     */
    void UpdateLabels() override;

    /**
     * List the skills of the chosen mode, keeping the chosen skill where the lists differ.
     *
     * @ghidraAddress NTSC-U/C: 0x00181ab8
     * @ghidraAddress PAL: 0x00185cf8
     */
    void OnModeChanged() override;

    /**
     * List `host_random`, the songs of the chosen mode and skill, and for a game or a remix
     * `host_custom`, keeping the song already selected or the song of the game database.
     *
     * @param bReset Non-zero to select the first entry.
     * @ghidraAddress NTSC-U/C: 0x00182900
     * @ghidraAddress PAL: 0x00186b40
     */
    void OnChoiceChanged(int bReset) override;

    /**
     * Write the choices to the game database and host the game, or choose a saved remix for
     * `host_custom`. Any other component moves the focus to `host`.
     *
     * @param pMsg The message.
     * @return The result of NetParamsScreen::HandleSelect().
     * @ghidraAddress NTSC-U/C: 0x00183740
     * @ghidraAddress PAL: 0x00187980
     */
    bool HandleSelect(UIComponentSelectMsg *pMsg);

    /**
     * Change the number of players or the power-ups with the left and right buttons.
     *
     * @param pMsg The message.
     * @return The result of NetParamsScreen::HandleSelectStart().
     * @ghidraAddress NTSC-U/C: 0x00183b68
     * @ghidraAddress PAL: 0x00187da8
     */
    bool HandleSelectStart(UIComponentSelectStartMsg *pMsg);

    /**
     * Restore the saved settings when the triangle button leaves the editing screen.
     *
     * @param pMsg The message.
     * @return True while a transition runs, otherwise the result of
     *         NetParamsScreen::HandleJoypad().
     * @ghidraAddress NTSC-U/C: 0x00183d10
     * @ghidraAddress PAL: 0x00187f50
     */
    bool HandleJoypad(JoypadInputMsg *pMsg);

    int mNumPlayers;                   /*!< The chosen entry of mPlayerCounts. */
    int mLastNumPlayers;               /*!< The last entry of mPlayerCounts. */
    int mDuelNumPlayers;               /*!< The entry of mPlayerCounts for a duel. */
    int mCustomChoice;                 /*!< The entry of `host_custom` in the songs, or -1. */
    std::vector<String> mPlayerCounts; /*!< The labels of two, three, and four players. */
    int mPowerup;                      /*!< The chosen entry of mPowerups. */
    std::vector<String> mPowerups;     /*!< The labels of the power-up levels. */
    int mIsEdit;                       /*!< `is_edit`, non-zero to edit an open launchpad. */
    NetGameParams mSavedParams;        /*!< The settings the triangle button restores. */
};
