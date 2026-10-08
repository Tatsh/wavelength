#pragma once

#include <vector>

#include "met/netparamsscreen.h"
#include "msg/joypadinputmsg.h"
#include "netflow/netgameparams.h"
#include "os/string.h"
#include "script/dataarray.h"
#include "ui/uicomponentselectmsg.h"
#include "ui/uicomponentselectstartmsg.h"

/**
 * The screen where the host chooses the mode, the skill, the players, the power-ups, and the song
 * of an online game.
 *
 * The RTTI records the class as deriving from NetParamsScreen. The object is 0x144 bytes and its
 * vtable is at `0x003cdca0`. The metagame registers the class for the screen type
 * `net_hosting_screen`. The description's `is_edit` entry marks the screen that changes the
 * settings of a hosted game from its launchpad. The cross button on `host` starts hosting, or
 * applies the settings.
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
     * The metagame registers the routine for the screen type `net_hosting_screen`.
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
     * Route the choices and the controller buttons, and pass on every other message.
     *
     * @param pMsg The message.
     * @return Whether the message was handled.
     * @ghidraAddress NTSC-U/C: 0x00183690
     * @ghidraAddress PAL: 0x001878d0
     */
    bool DispatchPriv(Message *pMsg) override;

    /**
     * Fill the lists of modes, players, and power-ups, choose the entries of the game database,
     * and start the entry.
     *
     * The edit screen takes the settings of the hosted game first, and offers no duel to more than
     * two players.
     *
     * @param pPrevScreen The screen this one replaces, or null.
     * @param fTime The front-end time in milliseconds.
     * @ghidraAddress NTSC-U/C: 0x00180610
     * @ghidraAddress PAL: 0x00184850
     */
    void Enter(UIScreen *pPrevScreen, float fTime) override;

    /**
     * Show the choices, and enable the power-up and player choices the mode allows.
     *
     * @ghidraAddress NTSC-U/C: 0x001834e8
     * @ghidraAddress PAL: 0x00187728
     */
    void UpdateLabels() override;

    /**
     * Fill the list of skills of the chosen mode, and move the chosen skill along with the list.
     *
     * @ghidraAddress NTSC-U/C: 0x00181ab8
     * @ghidraAddress PAL: 0x00185cf8
     */
    void OnModeChanged() override;

    /**
     * Fill the song list with the random choice, the remix choice, and the songs of the chosen
     * mode and skill, and select the song chosen before.
     *
     * @param nReset Non-zero to select the first entry.
     * @ghidraAddress NTSC-U/C: 0x00182900
     * @ghidraAddress PAL: 0x00186b40
     */
    void OnChoiceChanged(int nReset) override;

    /**
     * Apply the choices to the game database and host, or go to the remix list for the remix
     * choice. Any other button chosen with the cross button moves the focus back to `host`.
     *
     * @param pMsg The message.
     * @return The result of NetParamsScreen::HandleSelect().
     * @ghidraAddress NTSC-U/C: 0x00183740
     * @ghidraAddress PAL: 0x00187980
     */
    bool HandleSelect(UIComponentSelectMsg *pMsg);

    /**
     * Step the number of players and the power-ups with the left and right buttons.
     *
     * @param pMsg The message.
     * @return The result of NetParamsScreen::HandleSelectStart().
     * @ghidraAddress NTSC-U/C: 0x00183b68
     * @ghidraAddress PAL: 0x00187da8
     */
    bool HandleSelectStart(UIComponentSelectStartMsg *pMsg);

    /**
     * Restore the settings of the hosted game when the edit screen is exited with the triangle
     * button.
     *
     * @param pMsg The message of the button.
     * @return True while the screen moves in or out, otherwise the result of
     * NetParamsScreen::HandleJoypad().
     * @ghidraAddress NTSC-U/C: 0x00183d10
     * @ghidraAddress PAL: 0x00187f50
     */
    bool HandleJoypad(JoypadInputMsg *pMsg);

    int mReservedB8[2];                  // +0xb8, not yet recovered.
    int mNetPlayersChoice;               /*!< The chosen entry of mPlayerChoices. */
    int mLastPlayersChoice;              /*!< The last entry of mPlayerChoices. */
    int mDuelPlayersChoice;              /*!< The entry of mPlayerChoices a duel uses. */
    int mCustomChoice;                   /*!< The entry of the remix choice, or -1 for none. */
    std::vector<String> mPlayerChoices;  /*!< The names of the numbers of players. */
    int mPowerupChoice;                  /*!< The chosen entry of mPowerupChoices. */
    std::vector<String> mPowerupChoices; /*!< The names of the power-up levels. */
    bool mIsEdit;                        /*!< The `is_edit` entry of the description. */
    NetGameParams mParams;               /*!< The settings of the hosted game, for the edit. */
};
