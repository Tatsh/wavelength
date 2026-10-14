#include "game/songstats.h"

#include <string.h>
#include <vector>

#include "game/gamedb.h"
#include "game/songentry.h"

namespace {

template <typename T>
inline void RaiseTo(T &value, const T &other) {
    value = (value < other) ? other : value;
}

} // namespace

SongStats::SongStats() {
    memset(static_cast<void *>(this), 0, sizeof(*this));
}

bool SongStats::Improve(const SongStats &other) {
    // The share of the song played is raised too, but a higher share alone is not an improvement.
    if (mScore < other.mScore || mFullMixBars < other.mFullMixBars || mBlasted < other.mBlasted ||
        mBestStreak < other.mBestStreak) {
        RaiseTo(mScore, other.mScore);
        RaiseTo(mBestStreak, other.mBestStreak);
        RaiseTo(mBlasted, other.mBlasted);
        RaiseTo(mFullMixBars, other.mFullMixBars);
        RaiseTo(mPercentDone, other.mPercentDone);
        return true;
    }
    return false;
}

bool SongStats::operator==(const SongStats &other) const {
    return mSong == other.mSong && mSkillLevel == other.mSkillLevel;
}

BinStream &operator<<(BinStream &stream, const SongStats &record) {
    const SongEntry entry{TheGameDb->FindSong(record.mSong)};
    const signed char nId = static_cast<signed char>(entry.GetId());
    stream.Write(&nId, sizeof(nId));
    stream.WriteEndian(&record.mScore, sizeof(record.mScore));
    stream.Write(&record.mSkillLevel, sizeof(record.mSkillLevel));
    stream.Write(&record.mBestStreak, sizeof(record.mBestStreak));
    stream.Write(&record.mBlasted, sizeof(record.mBlasted));
    stream.Write(&record.mFullMixBars, sizeof(record.mFullMixBars));
    stream.Write(&record.mPercentDone, sizeof(record.mPercentDone));
    return stream;
}

BinStream &operator>>(BinStream &stream, SongStats &record) {
    signed char nId;
    stream.Read(&nId, sizeof(nId));
    stream.ReadEndian(&record.mScore, sizeof(record.mScore));
    stream.Read(&record.mSkillLevel, sizeof(record.mSkillLevel));
    stream.Read(&record.mBestStreak, sizeof(record.mBestStreak));
    stream.Read(&record.mBlasted, sizeof(record.mBlasted));
    stream.Read(&record.mFullMixBars, sizeof(record.mFullMixBars));
    stream.Read(&record.mPercentDone, sizeof(record.mPercentDone));

    std::vector<const char *> arenas;
    TheGameDb->GetArenaNames(&arenas, true);
    std::vector<SongEntry> songs;
    for (const char *pszArena : arenas) {
        TheGameDb->GetArenaSongs(&songs, pszArena, GameDb::kSkillAny, 0);
    }
    for (const SongEntry &song : songs) {
        if (song.GetId() == nId) {
            record.mSong = song.GetName();
            break;
        }
    }
    return stream;
}
