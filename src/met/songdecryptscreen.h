#pragma once

#include "met/freqscreen.h"
#include "met/viewanimplayer.h"
#include "rnd/matanim.h"
#include "script/dataarray.h"

/**
 * Screen that decrypts the picture of an unlocked bonus, boss, or secret song.
 *
 * The RTTI records the class as deriving from FreqScreen. The object is 0x90 bytes and its vtable
 * is at `0x003cf308`. The view `s_g_bonus.view` plays while the material animation
 * `band_pic.mnm` reveals the picture, and the screen moves to `song_decrypt_done` once the reveal
 * ends. The destructor at `0x0035ed98` is compiler-generated and has no declaration here.
 */
class SongDecryptScreen : public FreqScreen {
public:
    /**
     * Construct the screen from its script description, with no song.
     *
     * @param pData The script description.
     * @ghidraAddress NTSC-U/C: 0x0035edf0
     * @ghidraAddress PAL: 0x003cced0
     */
    explicit SongDecryptScreen(DataArray *pData);

    /**
     * Create a screen from its script description.
     *
     * @param pData The script description.
     * @return The new screen.
     * @ghidraAddress NTSC-U/C: 0x0035ed58
     * @ghidraAddress PAL: 0x003cce38
     */
    static UIScreen *New(DataArray *pData) {
        return new SongDecryptScreen(pData);
    }

    /**
     * Play the decrypt sound once the reveal reaches its first key, and move to
     * `song_decrypt_done` once it reaches its last.
     *
     * @param fTime The front-end time in milliseconds.
     * @ghidraAddress NTSC-U/C: 0x00198818
     * @ghidraAddress PAL: 0x0019fd38
     * @stub
     */
    void Poll(float fTime) override;

    /**
     * Stop the view and start the exit.
     *
     * @param pNextScreen The screen to change to.
     * @param fTime The front-end time in milliseconds.
     * @ghidraAddress NTSC-U/C: 0x001988e0
     * @ghidraAddress PAL: 0x0019fe00
     */
    void Exit(UIScreen *pNextScreen, float fTime) override;

    /**
     * Label the picture with the type of the song, and start the view.
     *
     * @param pPrevScreen The screen this one replaces, or null.
     * @param fTime The front-end time in milliseconds.
     * @ghidraAddress NTSC-U/C: 0x00198518
     * @ghidraAddress PAL: 0x0019fa38
     */
    void Enter(UIScreen *pPrevScreen, float fTime) override;

    /**
     * Choose the song, and the picture the `s_g_bonus_pic` panel loads.
     *
     * @param pszSong The song, a symbol.
     * @ghidraAddress NTSC-U/C: 0x001984d8
     * @ghidraAddress PAL: 0x0019f9f8
     */
    void SetSong(const char *pszSong);

    const char *mSong;          /*!< The song, a symbol. */
    Rnd::MatAnim *mPicAnim;     /*!< The reveal, `band_pic.mnm`, or null. */
    ViewAnimPlayer mAnimPlayer; /*!< The player of `s_g_bonus.view`. +0x78 */
    int mDecryptPlayed;         /*!< Whether the decrypt sound has played. +0x88 */
};
