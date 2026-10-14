#include "game/songentry.h"

#include <cstring>

#include "os/locale.h"
#include "os/string.h"
#include "script/symbol.h"

namespace {

// The tunnel scale of an entry without `tunnel_scale`.
constexpr float kDefaultTunnelScale = 1.0f;

// Look up the localized text of a token the name of the song fills in.
inline const char *LocalizeSongText(const SongEntry *pSong, const char *pszFormat, bool bFail) {
    return TheLocale.Localize(FormatString(pszFormat, pSong->GetName()), bFail);
}

} // namespace

int SongEntry::GetId() const {
    int nId;
    mData->FindInt("id", &nId, true);
    return nId;
}

const char *SongEntry::GetArenaPath() const {
    const char *pszPath;
    mData->FindSymbol("arena_path", &pszPath, true);
    return pszPath;
}

float SongEntry::GetTunnelScale() const {
    float fScale = kDefaultTunnelScale;
    mData->FindFloat("tunnel_scale", &fScale, false);
    return fScale;
}

const char *SongEntry::GetName() const {
    return mData->Sym(0);
}

const char *SongEntry::GetTitle() const {
    return LocalizeSongText(this, "%s_TITLE", true);
}

const char *SongEntry::GetLabel() const {
    return LocalizeSongText(this, "%s_LABEL", true);
}

const char *SongEntry::GetTitleShort() const {
    const char *pszToken = FormatString("%s_TITLE_SHORT", GetName());
    const char *pszText = TheLocale.Localize(pszToken, false);
    if (strcmp(pszText, pszToken) == 0) {
        return GetTitle();
    }
    return pszText;
}

const char *SongEntry::GetRemixTitle() const {
    const char *pszToken = FormatString("%s_REMIX_TITLE", GetName());
    const char *pszText = TheLocale.Localize(pszToken, false);
    if (strcmp(pszText, pszToken) == 0) {
        return GetTitleShort();
    }
    return pszText;
}

const char *SongEntry::GetArtist() const {
    return LocalizeSongText(this, "%s_ARTIST", true);
}

const char *SongEntry::GetArtistShort() const {
    return LocalizeSongText(this, "%s_ARTIST_SHORT", true);
}

const char *SongEntry::GetArtistShortest() const {
    const char *pszToken = FormatString("%s_ARTIST_SHORTEST", GetName());
    const char *pszText = TheLocale.Localize(pszToken, false);
    if (strcmp(pszText, pszToken) == 0) {
        return GetArtistShort();
    }
    return pszText;
}

const char *SongEntry::GetBio() const {
    return LocalizeSongText(this, "%s_BIO", true);
}

const char *SongEntry::GetWww() const {
    return LocalizeSongText(this, "%s_WWW", true);
}

const char *SongEntry::GetGenre() const {
    const char *pszGenre = LocalizeSongText(this, "%s_GENRE", true);
    if (pszGenre[0] == '\0') {
        return pszGenre;
    }
    const char *pszFormat = TheLocale.Localize("GENRE_BPM", true);
    return FormatString(pszFormat, pszGenre, static_cast<int>(GetBpm()));
}

const char *SongEntry::GetGenreName() const {
    return LocalizeSongText(this, "%s_GENRE", true);
}

int SongEntry::GetType() const {
    int nType = kTypeNormal;
    mData->FindInt("type", &nType, false);
    return nType;
}

bool SongEntry::IsAvailable(int nSkillLevel) const {
    int nSkill = 0;
    mData->FindInt("skill", &nSkill, false);
    return nSkill <= nSkillLevel;
}

bool SongEntry::IsDuel() const {
    int nDuel = 0;
    mData->FindInt("duel", &nDuel, false);
    return nDuel != 0;
}

bool SongEntry::IsRemixable() const {
    int nNotRemix = 0;
    mData->FindInt("not_remix", &nNotRemix, false);
    return nNotRemix == 0;
}

bool SongEntry::IsGameable() const {
    int nNotGame = 0;
    mData->FindInt("not_game", &nNotGame, false);
    return nNotGame == 0;
}

int SongEntry::GetBars() const {
    int nBars;
    mData->FindInt("song_bars", &nBars, true);
    return nBars;
}

int SongEntry::GetRemixBars() const {
    int nBars;
    mData->FindInt("remix_song_bars", &nBars, true);
    return nBars;
}

int SongEntry::GetDuelBars() const {
    int nBars = 0;
    mData->FindInt("duel_song_bars", &nBars, false);
    if (nBars == 0) {
        mData->FindInt("remix_song_bars", &nBars, true);
    }
    return nBars;
}

float SongEntry::GetBpm() const {
    float fBpm;
    mData->FindFloat("bpm", &fBpm, true);
    return fBpm;
}

DataArray *SongEntry::Find(const std::vector<SongEntry> &songs, const char *pszName) {
    const char *pszSymbol = LookupSymbol(pszName);
    for (const SongEntry &song : songs) {
        if (song.GetName() == pszSymbol) {
            return song.mData;
        }
    }
    return nullptr;
}
