#pragma once

#include <list>

#include "met/freqscreen.h"
#include "msg/joypadinputmsg.h"
#include "os/string.h"
#include "script/dataarray.h"
#include "ui/uiscreen.h"

/**
 * Screen `redbook_play` that plays the songs of the jukebox playlist one after another, showing the
 * band of the song that plays on the panel `jbox_band`.
 *
 * The RTTI records the class as deriving from FreqScreen. Its vtable is at `0x003d0310`. The songs
 * play in order or in a random order, and the list starts over after the last song. The START
 * button stops the playback, and the screen returns to `jbox_redbook` once the menu music plays.
 */
class RedbookPlaybackScreen : public FreqScreen {
public:
    /**
     * Construct the screen from its script description, with no songs.
     *
     * @param pData The script description.
     * @ghidraAddress NTSC-U/C: 0x001a6d48
     * @ghidraAddress PAL: 0x001aea38
     */
    explicit RedbookPlaybackScreen(DataArray *pData);

    /**
     * Destroy the screen.
     *
     * @ghidraAddress NTSC-U/C: 0x00361ed8
     * @ghidraAddress PAL: 0x003d03f0
     */
    ~RedbookPlaybackScreen() override {
    }

    /**
     * Create a screen from its script description.
     *
     * @param pData The script description.
     * @return The new screen.
     * @ghidraAddress NTSC-U/C: 0x00361e98
     * @ghidraAddress PAL: 0x003d03b0
     */
    static UIScreen *New(DataArray *pData) {
        return new RedbookPlaybackScreen(pData);
    }

    /**
     * Route the controller to HandleJoypad().
     *
     * @param pMsg The message.
     * @return Whether the message was handled.
     * @ghidraAddress NTSC-U/C: 0x001a72f8
     * @ghidraAddress PAL: 0x001aefe8
     */
    bool DispatchPriv(Message *pMsg) override;

    /**
     * Pick and load the next song once the previous one has finished, or leave once the playback
     * has stopped.
     *
     * @param fTime The front-end time in milliseconds.
     * @ghidraAddress NTSC-U/C: 0x001a7150
     * @ghidraAddress PAL: 0x001aee40
     */
    void Poll(float fTime) override;

    /**
     * Enter, show the help of the playback, and fade out the menu music.
     *
     * @param pPrevScreen The screen this one replaces.
     * @param fTime The front-end time in milliseconds.
     * @ghidraAddress NTSC-U/C: 0x001a7248
     * @ghidraAddress PAL: 0x001aef38
     */
    void Enter(UIScreen *pPrevScreen, float fTime) override;

    /**
     * Forget the songs.
     *
     * The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x001a6e50
     * @ghidraAddress PAL: 0x001aeb40
     */
    void ClearSongs();

    /**
     * Add a song to the songs to play.
     *
     * The name is inferred.
     *
     * @param pszSong The song, a symbol.
     * @ghidraAddress NTSC-U/C: 0x001a6e80
     * @ghidraAddress PAL: 0x001aeb70
     */
    void AddSong(const char *pszSong);

    int mShuffle; /*!< Non-zero when the songs play in a random order. */

private:
    /**
     * Stop the playback and restore the menu music.
     *
     * The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x001a6f98
     * @ghidraAddress PAL: 0x001aec88
     */
    void StopPlayback();

    /**
     * Take the next song from the songs left, show its band, and make it the song to load.
     *
     * The list of songs left starts over once it is empty. The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x001a6fd8
     * @ghidraAddress PAL: 0x001aecc8
     */
    void PickSong();

    /**
     * Swallow a button during a transition, and stop the playback with the START button.
     *
     * The name is inferred.
     *
     * @param pMsg The message.
     * @return True during a transition, otherwise the result of FreqScreen::HandleJoypad().
     * @ghidraAddress NTSC-U/C: 0x001a7360
     * @ghidraAddress PAL: 0x001af050
     */
    bool HandleJoypad(JoypadInputMsg *pMsg);

    std::list<const char *> mSongsLeft; // The songs not yet played in this round.
    std::list<const char *> mSongs;     // The songs of the playlist.
    int mPlaying;                       // Whether songs are picked and played.
    int mStopping;                      // Whether the playback stops.
    String mNextSong;                   // The song to load next, or empty.
};
