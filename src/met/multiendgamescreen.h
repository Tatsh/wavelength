#pragma once

#include <vector>

#include "met/freqscreen.h"
#include "os/string.h"
#include "script/dataarray.h"
#include "ui/uicomponentselectmsg.h"

/**
 * Screen that ranks the players of a game of several players, one row panel per player, with
 * each Freq dancing to its outcome.
 *
 * The RTTI records the class as deriving from FreqScreen. The object is 0xa0 bytes and its vtable
 * is at `0x003cea80`. A row panel is `m_g_end_<row>_<layout>`, where the layout is `tie` when
 * several players won and the number of players otherwise. The destructor at `0x0035df00` is
 * compiler-generated and has no declaration here.
 */
class MultiEndGameScreen : public FreqScreen {
public:
    /**
     * Construct the screen from its script description.
     *
     * @param pData The script description.
     * @ghidraAddress NTSC-U/C: 0x001952b0
     * @ghidraAddress PAL: 0x0019c680
     */
    explicit MultiEndGameScreen(DataArray *pData);

    /**
     * Create a screen from its script description.
     *
     * @param pData The script description.
     * @return The new screen.
     * @ghidraAddress NTSC-U/C: 0x0035dfd0
     * @ghidraAddress PAL: 0x003cc0b0
     */
    static UIScreen *New(DataArray *pData) {
        return new MultiEndGameScreen(pData);
    }

    /**
     * Route a chosen button, and pass on every other message.
     *
     * @param pMsg The message.
     * @return Whether the message was handled.
     * @ghidraAddress NTSC-U/C: 0x00195b58
     * @ghidraAddress PAL: 0x0019cf28
     */
    bool DispatchPriv(Message *pMsg) override;

    /**
     * Show the winner movie at the position of the song.
     *
     * @param fTime The front-end time in milliseconds.
     * @ghidraAddress NTSC-U/C: 0x00195348
     * @ghidraAddress PAL: 0x0019c718
     */
    void Poll(float fTime) override;

    /**
     * Fill a row panel per player in the order of the ranks, the song, and the difficulty, and
     * start the dance of each Freq.
     *
     * @param pPrevScreen The screen this one replaces, or null.
     * @param fTime The front-end time in milliseconds.
     * @ghidraAddress NTSC-U/C: 0x001953d0
     * @ghidraAddress PAL: 0x0019c7a0
     */
    void Enter(UIScreen *pPrevScreen, float fTime) override;

    /**
     * Show the screens that follow the game for the choice of the cross button.
     *
     * @param pMsg The message.
     * @return The result of UIScreen::HandleSelect().
     * @ghidraAddress NTSC-U/C: 0x00195bc0
     * @ghidraAddress PAL: 0x0019cf90
     */
    bool HandleSelect(UIComponentSelectMsg *pMsg);

    const char *mStats;           /*!< `m_stats` of the description. The screen does not read it. */
    String mLayout;               /*!< The layout part of the names of the row panels. +0x74 */
    std::vector<int> mPlayerRows; /*!< The row of each player. +0x88 */
};
