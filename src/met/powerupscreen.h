#pragma once

#include <vector>

#include "met/freqscreen.h"
#include "os/string.h"
#include "script/dataarray.h"
#include "ui/uicomponentselectmsg.h"
#include "ui/uicomponentselectstartmsg.h"

/**
 * The power-up menu of a multiplayer game, such as `m_g_sel_pup`, whose `powerup` button cycles
 * through the power-up levels.
 *
 * The RTTI records the class as deriving from FreqScreen. The object is 0x90 bytes and its vtable
 * is at `0x003cec00`.
 */
class PowerupScreen : public FreqScreen {
public:
    /**
     * Construct the screen from its script description.
     *
     * @param pData The script description.
     * @ghidraAddress NTSC-U/C: 0x00190da0
     * @ghidraAddress PAL: 0x001980b8
     */
    explicit PowerupScreen(DataArray *pData);

    /**
     * Route a chosen button and a press on a button, and pass on every other message.
     *
     * @param pMsg The message.
     * @return Whether the message was handled.
     * @ghidraAddress NTSC-U/C: 0x001914a0
     * @ghidraAddress PAL: 0x001987b8
     */
    bool DispatchPriv(Message *pMsg) override;

    /**
     * Build the label of every power-up level, show the label of the level of the game, and start
     * the entry.
     *
     * @param pPrevScreen The screen this one replaces, or null.
     * @param fTime The front-end time in milliseconds.
     * @ghidraAddress NTSC-U/C: 0x00190de0
     * @ghidraAddress PAL: 0x001980f8
     */
    void Enter(UIScreen *pPrevScreen, float fTime) override;

    /**
     * Record the level shown as the power-up level of the game when the cross button chooses the
     * `powerup` button, and go to `multiskill2multiarena`.
     *
     * @param pMsg The message.
     * @return The result of UIScreen::HandleSelect().
     * @ghidraAddress NTSC-U/C: 0x00191658
     * @ghidraAddress PAL: 0x00198970
     */
    bool HandleSelect(UIComponentSelectMsg *pMsg);

    /**
     * Show the previous or the next level when left or right is pressed on the `powerup` button.
     *
     * @param pMsg The message.
     * @return The result of FreqScreen::HandleSelectStart().
     * @ghidraAddress NTSC-U/C: 0x00191530
     * @ghidraAddress PAL: 0x00198848
     */
    bool HandleSelectStart(UIComponentSelectStartMsg *pMsg);

    int mLevel;                  /*!< The power-up level shown, an index into mLabels. */
    std::vector<String> mLabels; /*!< The label of each power-up level. */
};
