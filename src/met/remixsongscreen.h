#pragma once

#include <vector>

#include "met/freqscreen.h"
#include "os/string.h"
#include "script/dataarray.h"
#include "ui/uicomponentselectmsg.h"

/**
 * The menu that chooses the song of a remix or a duel.
 *
 * The RTTI records the class as deriving from FreqScreen. The object is 0x94 bytes and its vtable
 * is at `0x003ce780`. The metagame registers the class for the screen type `remix_song_screen`.
 * Its list is `list` of the panel `s_r_sel_song`. A remix lists only the songs with a
 * `remix_midi_file`.
 */
class RemixSongScreen : public FreqScreen {
public:
    /**
     * Construct the screen from its script description, with no songs.
     *
     * @param pData The script description.
     * @ghidraAddress NTSC-U/C: 0x00188dc8
     * @ghidraAddress PAL: 0x0018f360
     */
    explicit RemixSongScreen(DataArray *pData);

    /**
     * Create a screen from its script description.
     *
     * The metagame registers the routine for the screen type `remix_song_screen`.
     *
     * @param pData The script description.
     * @return The new screen.
     * @ghidraAddress NTSC-U/C: 0x0035c3d8
     * @ghidraAddress PAL: 0x003ca4b0
     */
    static UIScreen *New(DataArray *pData) {
        return new RemixSongScreen(pData);
    }

    /**
     * Route a chosen song, and pass on every other message.
     *
     * @param pMsg The message.
     * @return Whether the message was handled.
     * @ghidraAddress NTSC-U/C: 0x00189688
     * @ghidraAddress PAL: 0x0018fc20
     */
    bool DispatchPriv(Message *pMsg) override;

    /**
     * Fill the list with the unlocked songs, and start the entry.
     *
     * Without unlocked songs, the songs of the first arena whose type is 0 are listed.
     *
     * @param pPrevScreen The screen this one replaces, or null.
     * @param fTime The front-end time in milliseconds.
     * @ghidraAddress NTSC-U/C: 0x00188e18
     * @ghidraAddress PAL: 0x0018f3b0
     */
    void Enter(UIScreen *pPrevScreen, float fTime) override;

    /**
     * Set up the song chosen with the cross button, and go on to the remix creation mode or to
     * the tutorial sequence.
     *
     * @param pMsg The message.
     * @return The result of UIScreen::HandleSelect().
     * @ghidraAddress NTSC-U/C: 0x001896f0
     * @ghidraAddress PAL: 0x0018fc88
     */
    bool HandleSelect(UIComponentSelectMsg *pMsg);

    String mSong;               /*!< The song chosen last. */
    std::vector<String> mSongs; /*!< The listed songs. */
};
