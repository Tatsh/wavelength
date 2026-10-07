#pragma once

#include <vector>

#include "met/freqscreen.h"
#include "msg/joypadinputmsg.h"
#include "script/dataarray.h"
#include "ui/uicomponentselectmsg.h"
#include "ui/uicomponentselectstartmsg.h"

/**
 * The screen on which every player of a local game chooses a Freq at the same time.
 *
 * The RTTI records the class as deriving from FreqScreen, and its vtable is at `0x003ced80`. The
 * metagame registers the class for the screen type `f_m_sel_screen`. Player `n` of `p` players
 * uses the panel `m_g_s_f_<p>pl_0<n>`.
 */
class MultiFreqSelectScreen : public FreqScreen {
public:
    /**
     * Construct the screen from its script description, reading `num_players`.
     *
     * @param pData The script description.
     * @ghidraAddress NTSC-U/C: 0x0018eea8
     * @ghidraAddress PAL: 0x001961c0
     */
    explicit MultiFreqSelectScreen(DataArray *pData);

    /**
     * Create a screen from its script description.
     *
     * The metagame registers the routine for the screen type `f_m_sel_screen`.
     *
     * @param pData The script description.
     * @return The new screen.
     * @ghidraAddress NTSC-U/C: 0x0035d818
     * @ghidraAddress PAL: 0x003cb8f0
     */
    static UIScreen *New(DataArray *pData) {
        return new MultiFreqSelectScreen(pData);
    }

    /**
     * Route the choices and the controller buttons, and pass on every other message.
     *
     * @param pMsg The message.
     * @return Whether the message was handled.
     * @ghidraAddress NTSC-U/C: 0x0018fe58
     * @ghidraAddress PAL: 0x00197170
     */
    bool DispatchPriv(Message *pMsg) override;

    /**
     * Start the entry with no player having chosen.
     *
     * @param pPrevScreen The screen this one replaces, or null.
     * @param fTime The front-end time in milliseconds.
     * @ghidraAddress NTSC-U/C: 0x0018ef18
     * @ghidraAddress PAL: 0x00196230
     */
    void Enter(UIScreen *pPrevScreen, float fTime) override;

    /**
     * Pass the start of a choice to the panel of the controller that made it.
     *
     * @param pMsg The message.
     * @return The result of FreqSelPanel::HandleSelectStart(), or false for a controller without
     * a player.
     * @ghidraAddress NTSC-U/C: 0x0018ff08
     * @ghidraAddress PAL: 0x00197220
     */
    bool HandleSelectStart(UIComponentSelectStartMsg *pMsg);

    /**
     * Pass a choice made with the cross button to the panel of its player, mark the player as
     * done, and go on when every player is done.
     *
     * The player is the last digit of the chosen component's name.
     *
     * @param pMsg The message.
     * @return The result of FreqSelPanel::HandleSelect(), or false.
     * @ghidraAddress NTSC-U/C: 0x0018ffd0
     * @ghidraAddress PAL: 0x001972e8
     */
    bool HandleSelect(UIComponentSelectMsg *pMsg);

    /**
     * Respond to a controller button.
     *
     * @param pMsg The message of the button.
     * @return Whether the message was handled.
     * @ghidraAddress NTSC-U/C: 0x00190190
     * @ghidraAddress PAL: 0x001974a8
     */
    bool HandleJoypad(JoypadInputMsg *pMsg);

    /**
     * Go on to the next screen once every player has chosen.
     *
     * @ghidraAddress NTSC-U/C: 0x001906a0
     * @ghidraAddress PAL: 0x001979b8
     */
    void CheckDone();

    int mNumPlayers;              /*!< The number of players, from `num_players`. */
    std::vector<bool> mConfirmed; /*!< Whether each player has chosen. */
};
