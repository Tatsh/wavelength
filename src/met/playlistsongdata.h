#pragma once

/**
 * One entry of the jukebox playlist.
 *
 * The name comes from the type information the standard library vector of the type records. The
 * record is 8 bytes.
 */
struct PlaylistSongData {
    int mSelected; /*!< Non-zero when the entry plays. */
    const char
        *mSong; /*!< The song, a symbol, or the localised name of the entry for every song. */
};
