#pragma once

#include "met/freqscreen.h"
#include "script/dataarray.h"
#include "ui/uicomponentselectmsg.h"

/**
 * The menu that chooses two, three, or four players for a local game. Three and four players need
 * a multitap.
 *
 * The RTTI records the class as deriving from FreqScreen. The object is 0x74 bytes and its vtable
 * is at `0x003ced20`. The metagame registers the class for the screen type `num_players_screen`.
 * Its panel is `m_player` with the buttons `2play`, `3play`, and `4play`.
 */
class NumPlayersScreen : public FreqScreen {
public:
    /**
     * Construct the screen from its script description.
     *
     * @param pData The script description.
     * @ghidraAddress NTSC-U/C: 0x0018d980
     * @ghidraAddress PAL: 0x00194878
     */
    explicit NumPlayersScreen(DataArray *pData);

    /**
     * Create a screen from its script description.
     *
     * The metagame registers the routine for the screen type `num_players_screen`.
     *
     * @param pData The script description.
     * @return The new screen.
     * @ghidraAddress NTSC-U/C: 0x0035d8f0
     * @ghidraAddress PAL: 0x003cb9c8
     */
    static UIScreen *New(DataArray *pData) {
        return new NumPlayersScreen(pData);
    }

    /**
     * Route a chosen button, and pass on every other message.
     *
     * @param pMsg The message.
     * @return Whether the message was handled.
     * @ghidraAddress NTSC-U/C: 0x0018dbc8
     * @ghidraAddress PAL: 0x00194ac0
     */
    bool DispatchPriv(Message *pMsg) override;

    /**
     * Advance the screen, and update the buttons when a multitap is plugged in or out.
     *
     * @param fTime The front-end time.
     * @ghidraAddress NTSC-U/C: 0x0018db80
     * @ghidraAddress PAL: 0x00194a78
     */
    void Poll(float fTime) override;

    /**
     * Start the entry and enable the buttons the controllers allow.
     *
     * @param pPrevScreen The screen this one replaces, or null.
     * @param fTime The front-end time in milliseconds.
     * @ghidraAddress NTSC-U/C: 0x0018d9c0
     * @ghidraAddress PAL: 0x001948b8
     */
    void Enter(UIScreen *pPrevScreen, float fTime) override;

    /**
     * Enable the three and four player buttons when a multitap is connected, disable them
     * otherwise, and focus the two player button.
     *
     * @param nMultitap Non-zero when a multitap is connected.
     * @ghidraAddress NTSC-U/C: 0x0018d9f8
     * @ghidraAddress PAL: 0x001948f0
     */
    void UpdateButtons(int nMultitap);

    /**
     * Set up the players of the button chosen with the cross button.
     *
     * The first player retains a custom Freq, and every other player gets a default name.
     *
     * @param pMsg The message.
     * @return The result of UIScreen::HandleSelect().
     * @ghidraAddress NTSC-U/C: 0x0018dc30
     * @ghidraAddress PAL: 0x00194b28
     */
    bool HandleSelect(UIComponentSelectMsg *pMsg);

    int mMultitap; /*!< Whether a multitap was connected when the buttons were last updated. */
};
