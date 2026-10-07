#pragma once

#include "met/freqscreen.h"

/**
 * Screen that reveals a song a result unlocked, `song_decrypt` of the front-end description.
 *
 * The RTTI records the class as deriving from FreqScreen. Only the members the metagame uses are
 * declared, and the routines of the class are not reconstructed.
 */
class SongDecryptScreen : public FreqScreen {
public:
    /**
     * Choose the song to reveal, and show its picture on the `s_g_bonus_pic` panel.
     *
     * @param pszSong The song, a symbol.
     * @ghidraAddress NTSC-U/C: 0x001984d8
     * @ghidraAddress PAL: 0x0019f9f8
     */
    void SetSong(const char *pszSong);
};
