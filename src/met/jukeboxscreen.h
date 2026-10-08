#pragma once

#include <vector>

#include "met/freqscreen.h"
#include "met/playlistsongdata.h"
#include "msg/joypadinputmsg.h"
#include "script/dataarray.h"
#include "ui/uicomponentselectmsg.h"
#include "ui/uicomponentselectstartmsg.h"
#include "ui/uiscreen.h"

/**
 * The jukebox screen `jbox_redbook`, which lists the songs the first player has finished and plays
 * the chosen ones on `redbook_play`.
 *
 * The RTTI records the class as deriving from FreqScreen. The object is 0x90 bytes and its vtable
 * is at `0x003d02b0`. The metagame registers the class for the screen type `jbox_screen`. The
 * first entry of the playlist stands for every song.
 */
class JukeboxScreen : public FreqScreen {
public:
    /**
     * Construct the screen from its script description, with an empty playlist.
     *
     * @param pData The script description.
     * @ghidraAddress NTSC-U/C: 0x001a73f0
     * @ghidraAddress PAL: 0x001af0e0
     */
    explicit JukeboxScreen(DataArray *pData) : FreqScreen(pData), mShowAllSongs(0) {
    }

    /**
     * Destroy the screen.
     *
     * @ghidraAddress NTSC-U/C: 0x00362010
     * @ghidraAddress PAL: 0x003d0528
     */
    ~JukeboxScreen() override {
    }

    /**
     * Create a screen from its script description.
     *
     * @param pData The script description.
     * @return The new screen.
     * @ghidraAddress NTSC-U/C: 0x00361fd0
     * @ghidraAddress PAL: 0x003d04e8
     */
    static UIScreen *New(DataArray *pData) {
        return new JukeboxScreen(pData);
    }

    /**
     * Route the choices and the controller to their handlers.
     *
     * @param pMsg The message.
     * @return Whether the message was handled.
     * @ghidraAddress NTSC-U/C: 0x001a7ab0
     * @ghidraAddress PAL: 0x001af7a0
     */
    bool DispatchPriv(Message *pMsg) override;

    /**
     * Exit.
     *
     * @param pNextScreen The screen that replaces this one.
     * @param fTime The front-end time in milliseconds.
     * @ghidraAddress NTSC-U/C: 0x001a7a90
     * @ghidraAddress PAL: 0x001af780
     */
    void Exit(UIScreen *pNextScreen, float fTime) override;

    /**
     * Enter and list the songs, or keep the playlist when the playback returns.
     *
     * @param pPrevScreen The screen this one replaces, or null.
     * @param fTime The front-end time in milliseconds.
     * @ghidraAddress NTSC-U/C: 0x001a7438
     * @ghidraAddress PAL: 0x001af128
     */
    void Enter(UIScreen *pPrevScreen, float fTime) override;

    std::vector<PlaylistSongData> mPlaylist; /*!< The playlist. */
    int mShowAllSongs; /*!< Non-zero when every song is listed, finished or not. */

private:
    /**
     * Focus the playlist with `create`, or play the chosen songs of the playlist with a playback
     * button.
     *
     * The name is inferred.
     *
     * @param pMsg The message.
     * @return The result of UIScreen::HandleSelect().
     * @ghidraAddress NTSC-U/C: 0x001a7b60
     * @ghidraAddress PAL: 0x001af850
     */
    bool HandleSelect(UIComponentSelectMsg *pMsg);

    /**
     * Focus the playlist when the right button chooses `create`.
     *
     * The name is inferred.
     *
     * @param pMsg The message.
     * @return The result of FreqScreen::HandleSelectStart().
     * @ghidraAddress NTSC-U/C: 0x001a7dd0
     * @ghidraAddress PAL: 0x001afac0
     */
    bool HandleSelectStart(UIComponentSelectStartMsg *pMsg);

    /**
     * Return to `s_mode` with the triangle button from `jbox`, or from the playlist to `jbox`.
     *
     * The name is inferred.
     *
     * @param pMsg The message.
     * @return True during a transition, otherwise the result of FreqScreen::HandleJoypad().
     * @ghidraAddress NTSC-U/C: 0x001a7ed0
     * @ghidraAddress PAL: 0x001afbc0
     */
    bool HandleJoypad(JoypadInputMsg *pMsg);
};
