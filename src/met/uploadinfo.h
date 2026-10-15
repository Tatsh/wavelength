#pragma once

#ifdef VIDEO_STANDARD_PAL

#include <cstring>

#include "os/string.h"

/**
 * A file of the player's upload directory on the server, as the upload tool lists it.
 *
 * The name is the type name the RTTI records for the list node. The file name is a two-character
 * prefix, a six-character date and a separator, the song, an underscore, the title, and an
 * extension.
 */
struct UploadInfo {
    String mName;     /*!< The name of the file. */
    int mInRepos = 0; /*!< Non-zero when the file is one of the remixes of the repository. */
};

/** The bytes of the buffer that receives the date of an upload file name. */
constexpr int kUploadDateSize = 7;

/** The bytes of the buffer that receives the song or the title of an upload file name. */
constexpr int kUploadPartSize = 30;

/**
 * Split an upload file name into its date, its song, and its title.
 *
 * Each part is copied without a terminator into a buffer that starts zeroed. A song part of
 * `SHADES` is taken as the song `SHADES_OF`, whose name has underscores, and the title then
 * begins 15 characters after the date.
 *
 * @param pszName The file name.
 * @param pszDate The buffer of kUploadDateSize bytes for the date.
 * @param pszSong The buffer of kUploadPartSize bytes for the song.
 * @param pszTitle The buffer of kUploadPartSize bytes for the title.
 */
inline void SplitUploadName(const char *pszName, char *pszDate, char *pszSong, char *pszTitle) {
    constexpr int kPrefixLength = 2;
    constexpr int kDateLength = 6;
    constexpr int kSeparatorLength = 1;
    constexpr int kShadesSongLength = 15;
    const char *p = pszName + kPrefixLength;
    std::strncpy(pszDate, p, kDateLength);
    p += kDateLength + kSeparatorLength;
    const size_t nSong = std::strcspn(p, "_");
    std::strncpy(pszSong, p, nSong);
    if (std::strcmp(pszSong, "SHADES") == 0) {
        p += kShadesSongLength;
        std::strcpy(pszSong, "SHADES_OF");
    } else {
        p += nSong + kSeparatorLength;
    }
    std::strncpy(pszTitle, p, std::strcspn(p, "."));
}

#endif
