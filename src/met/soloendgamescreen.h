#pragma once

#include "met/freqscreen.h"
#include "script/dataarray.h"
#include "ui/uicomponentselectmsg.h"
#include "ui/uitransitioncompletemsg.h"

/**
 * Screen that shows the result of a solo game, with the statistics and the Freq dancing to the
 * outcome.
 *
 * The RTTI records the class as deriving from FreqScreen. The object is 0x80 bytes and its vtable
 * is at `0x003ceae0`. The description's `stats_panel` and `av_panel` name the panel of the
 * statistics and the panel of the Freq. The won screen uses `s_g_end_win_stats`, and any other
 * statistics panel means a lost game. The destructor at `0x0035de18` is compiler-generated and has
 * no declaration here.
 */
class SoloEndGameScreen : public FreqScreen {
public:
    /**
     * Construct the screen from its script description.
     *
     * @param pData The script description.
     * @ghidraAddress NTSC-U/C: 0x00194b60
     * @ghidraAddress PAL: 0x0019bf30
     */
    explicit SoloEndGameScreen(DataArray *pData);

    /**
     * Create a screen from its script description.
     *
     * @param pData The script description.
     * @return The new screen.
     * @ghidraAddress NTSC-U/C: 0x0035deb0
     * @ghidraAddress PAL: 0x003cbf90
     */
    static UIScreen *New(DataArray *pData) {
        return new SoloEndGameScreen(pData);
    }

    /**
     * Route a chosen button and the end of a transition, and pass on every other message.
     *
     * @param pMsg The message.
     * @return Whether the message was handled.
     * @ghidraAddress NTSC-U/C: 0x00194fe8
     * @ghidraAddress PAL: 0x0019c3b8
     */
    bool DispatchPriv(Message *pMsg) override;

    /**
     * Show the winner movie at the position of the song, and advance the avatar players.
     *
     * @param fTime The front-end time in milliseconds.
     * @ghidraAddress NTSC-U/C: 0x00195218
     * @ghidraAddress PAL: 0x0019c5e8
     */
    void Poll(float fTime) override;

    /**
     * Record the campaign, choose the focused button, fill the statistics, and start the dance
     * of the Freq.
     *
     * A won game hides `continue` behind `exit` for a remix, the win sequence, a song already
     * finished, a beaten game, or a bonus song.
     *
     * @param pPrevScreen The screen this one replaces, or null.
     * @param fTime The front-end time in milliseconds.
     * @ghidraAddress NTSC-U/C: 0x00194be0
     * @ghidraAddress PAL: 0x0019bfb0
     */
    void Enter(UIScreen *pPrevScreen, float fTime) override;

    /**
     * Report no title.
     *
     * @return An empty string.
     * @ghidraAddress NTSC-U/C: 0x0035def0
     * @ghidraAddress PAL: 0x003cbfd0
     */
    const char *Title() override;

    /**
     * Record the choice of the cross button for the metagame and show the next screen.
     *
     * @param pMsg The message.
     * @return The result of UIScreen::HandleSelect().
     * @ghidraAddress NTSC-U/C: 0x00195078
     * @ghidraAddress PAL: 0x0019c448
     */
    bool HandleSelect(UIComponentSelectMsg *pMsg);

    /**
     * Start the animation of the won statistics.
     *
     * @param pMsg The message.
     * @return False.
     * @ghidraAddress NTSC-U/C: 0x00195178
     * @ghidraAddress PAL: 0x0019c548
     */
    bool HandleTransitionComplete(UITransitionCompleteMsg *pMsg);

    const char *mStatsPanel;  /*!< `stats_panel`, the panel of the statistics, or empty. */
    const char *mAvatarPanel; /*!< `av_panel`, the panel of the Freq, or empty. */
};
