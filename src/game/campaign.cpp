#include "game/campaign.h"

#include <algorithm>
#include <string.h>

#include "game/gamedb.h"
#include "game/songentry.h"
#include "os/debug.h"
#include "os/system.h"
#include "script/symbol.h"

unsigned int g_dwUnlockAllSongs = 0;
unsigned int g_dwUnlockAllParts = 0;

namespace {

// The version Save() writes. Load() warns about anything older than kOldestVersion.
constexpr unsigned char kVersion = 6;
constexpr unsigned char kOldestVersion = 3;
constexpr unsigned char kInputMapVersion = 4;
constexpr unsigned char kFlagsVersion = 5;
constexpr unsigned char kLastUnlocksVersion = 6;

constexpr int kNumSkillLevels = 4;
constexpr int kNumUnlockKinds = 16;
constexpr int kPercentFinished = 100;
constexpr int kOptionDefault = 1;

// Every song type, as the `filter` and `all_except` clauses write it.
constexpr int kAnyType = 5;
constexpr int kFilteredReserve = 5;

// The `campaign` array and each entry start with a tag, the entry with its skill level.
constexpr int kFirstEntry = 1;
constexpr int kEntrySkillLevel = 0;
constexpr int kEntryCondition = 1;
constexpr int kFirstEntryUnlock = 2;

// Indices of the clauses.
constexpr int kClauseTag = 0;
constexpr int kClauseFirstArgument = 1;
constexpr int kClauseSecondArgument = 2;
constexpr int kClauseThirdArgument = 3;
constexpr int kAllExceptOneTypeSize = 3;
constexpr int kAllExceptTwoTypesSize = 4;

// An `unlock` clause is the tag, the kind, and the items.
constexpr int kUnlockKind = 1;
constexpr int kFirstUnlockItem = 2;

// The prefab lists of the "metagame" section, each starting with its tag.
constexpr int kNumPrefabLists = 2;
constexpr int kFirstPrefab = 1;
constexpr int kPrefabName = 0;
constexpr int kFirstPrefabPart = 1;
constexpr int kPartIndex = 0;
constexpr int kPartValue = 1;

// Indices of the goals in a row of a `score_goal`.
constexpr int kGoalFirst = 0;
constexpr int kGoalSecond = 1;
constexpr int kGoalThird = 2;

const char *const kDbTag = "db";
const char *const kCampaignTag = "campaign";
const char *const kInputMapTag = "input_map";
const char *const kAllArenas = "all";

// The `arena_scores` of an arena and the `score_goal` of a song have a row for each skill level
// after the tag.
constexpr int kFirstSkillRow = 1;

constexpr int kNoSkillLevel = -1;

} // namespace

Campaign::Campaign()
    : mNameLocked(0), mModified(0), mCustom(0), mFlags(0), mSoloOption(kOptionDefault),
      mOnlineOption(kOptionDefault) {
    memset(static_cast<void *>(&mBorn), 0, sizeof(mBorn));
    (void)mBorn.ReadClock(); // Yes, the binary discards the result.

    const DataArray *pDb = SystemConfig()->FindArray(kDbTag, false);
    mSkillUnlocks.resize(kNumSkillLevels, std::vector<const char *>());
    mPassedEntries.resize(pDb->FindArray(kCampaignTag, false)->Size() - kFirstEntry, 0);
    mLastUnlocks.resize(kNumUnlockKinds, "");

    std::vector<UnlockableItem> unlocks;
    for (int nSkillLevel = 0; nSkillLevel < kNumSkillLevels; ++nSkillLevel) {
        UpdateUnlocks(&unlocks, nSkillLevel);
    }
    mInputMap.Load(pDb->FindArray(kInputMapTag, false));
}

Campaign::Campaign(const Campaign &other) {
    memset(static_cast<void *>(&mBorn), 0, sizeof(mBorn));
    *this = other;
}

Campaign::~Campaign() {
}

Campaign &Campaign::operator=(const Campaign &other) {
    if (this != &other) {
        mUnlocks = other.mUnlocks;
        mSkillUnlocks = other.mSkillUnlocks;
        mPassedEntries = other.mPassedEntries;
        mName = other.mName;
        mBorn = other.mBorn;
        mPassword = other.mPassword;
        mNameLocked = other.mNameLocked;
        mRecords = other.mRecords;
        mInputMap = other.mInputMap;
        mSoloOption = other.mSoloOption;
        mOnlineOption = other.mOnlineOption;
        mPendingPowerup = other.mPendingPowerup;
        mFlags = other.mFlags;
        mCustom = other.mCustom;
        mLastUnlocks = other.mLastUnlocks;
        mAvatar = other.mAvatar;
        mModified = other.mModified;
    }
    return *this;
}

void Campaign::AddRecord(const SongStats *pRecord) {
    auto it = std::find(mRecords.begin(), mRecords.end(), *pRecord);
    if (it != mRecords.end()) {
        const bool bImproved = it->Improve(*pRecord);
        mModified = (mModified != 0) || bImproved;
    } else if (pRecord->mPercentDone == kPercentFinished) {
        mRecords.push_back(*pRecord);
        mModified = 1;
    }
}

void Campaign::UpdateUnlocks(std::vector<UnlockableItem> *pUnlocks, int nSkillLevel) {
    pUnlocks->clear();
    DataArray *pCampaign = SystemConfig()->FindArray(kDbTag, false)->FindArray(kCampaignTag, false);
    for (int i = kFirstEntry; i < pCampaign->Size(); ++i) {
        ProcessEntry(pCampaign->Array(i), nSkillLevel, i - kFirstEntry, pUnlocks);
    }
}

void Campaign::RecordResult(const SongStats *pRecord, std::vector<UnlockableItem> *pUnlocks) {
    if (g_dwUnlockAllSongs != 0) {
        pUnlocks->clear();
        return;
    }
    AddRecord(pRecord);
    UpdateUnlocks(pUnlocks, pRecord->mSkillLevel);
}

void Campaign::ProcessEntry(DataArray *pEntry,
                            int nSkillLevel,
                            int nEntry,
                            std::vector<UnlockableItem> *pUnlocks) {
    const int nEntrySkillLevel = pEntry->Int(kEntrySkillLevel);
    if (nEntrySkillLevel != nSkillLevel && nEntrySkillLevel != GameDb::kSkillAny) {
        return;
    }
    if (((mPassedEntries[nEntry] >> nSkillLevel) & 1) != 0) {
        return;
    }
    if (!CheckCondition(pEntry->Array(kEntryCondition), nSkillLevel)) {
        return;
    }
    mPassedEntries[nEntry] |= 1 << nSkillLevel;
    for (int i = kFirstEntryUnlock; i < pEntry->Size(); ++i) {
        if (strcmp(pEntry->Array(i)->Sym(kClauseTag), "prefab_unlock") == 0) {
            PrefabUnlock(pEntry->Array(i), pUnlocks);
        } else {
            ApplyUnlockNode(pEntry->Array(i), nSkillLevel, pUnlocks);
        }
    }
}

bool Campaign::CheckCondition(DataArray *pCondition, int nSkillLevel) {
    const char *pszTag = pCondition->Sym(kClauseTag);
    if (strcmp(pszTag, "beat_num") == 0) {
        return CheckBeatNum(pCondition->Int(kClauseFirstArgument), nSkillLevel);
    }
    if (strcmp(pszTag, "beat_songs") == 0) {
        std::vector<const char *> songs;
        ParseFilter(pCondition->Array(kClauseFirstArgument), nSkillLevel, &songs);
        return CheckAllBeaten(&songs, nSkillLevel);
    }
    if (strcmp(pszTag, "beat_score") == 0) {
        std::vector<const char *> songs;
        ParseFilter(pCondition->Array(kClauseFirstArgument), nSkillLevel, &songs);
        const int nTotal = GetArenaScoreToBeat(pCondition->Sym(kClauseSecondArgument), nSkillLevel);
        return CheckScoreTotal(&songs, nSkillLevel, nTotal);
    }
    if (strcmp(pszTag, "always") == 0) {
        return true;
    }
    DebugWarn("Unrecognized token:%s (%s:%d)\n", pszTag, pCondition->mFile, pCondition->mLine);
    return false;
}

bool Campaign::CheckBeatNum(int nCount, int nSkillLevel) {
    if (nCount <= 0) {
        return true;
    }
    for (const SongStats &record : mRecords) {
        if (record.mSkillLevel == nSkillLevel && record.mPercentDone == kPercentFinished) {
            const SongEntry entry{TheGameDb->FindSong(record.mSong)};
            if (entry.GetType() != SongEntry::kTypeTutorial) {
                --nCount;
            }
        }
        if (nCount <= 0) {
            return true;
        }
    }
    return false;
}

void Campaign::ParseFilter(DataArray *pFilter, int nSkillLevel, std::vector<const char *> *pSongs) {
    const char *pszTag = pFilter->Sym(kClauseTag);
    if (strcmp(pszTag, "filter") == 0) {
        const int nType = pFilter->Int(kClauseFirstArgument);
        BuildFiltered(nType, pFilter->Sym(kClauseSecondArgument), nSkillLevel, pSongs);
    } else if (strcmp(pszTag, "all_except") == 0) {
        const int nSize = pFilter->Size();
        if (nSize == kAllExceptOneTypeSize) {
            const int nType = pFilter->Int(kClauseFirstArgument);
            ExceptType(nType, pFilter->Sym(kClauseSecondArgument), nSkillLevel, pSongs);
        } else if (nSize == kAllExceptTwoTypesSize) {
            const int nType = pFilter->Int(kClauseFirstArgument);
            const int nOtherType = pFilter->Int(kClauseSecondArgument);
            ExceptTwoTypes(
                nType, nOtherType, pFilter->Sym(kClauseThirdArgument), nSkillLevel, pSongs);
        } else {
            DebugWarn("AllExcept can't accept %d args", nSize - 1);
        }
    } else if (strcmp(pszTag, "songs") == 0) {
        FromList(pFilter, nSkillLevel, pSongs);
    } else {
        DebugWarn("Unrecognized token:%s (%s:%d)\n", pszTag, pFilter->mFile, pFilter->mLine);
    }
}

void Campaign::BuildFiltered(int nType,
                             const char *pszArena,
                             int nSkillLevel,
                             std::vector<const char *> *pSongs) {
    pSongs->clear();
    pSongs->reserve(kFilteredReserve);
    std::vector<SongEntry> songs;
    if (strcmp(pszArena, kAllArenas) == 0) {
        TheGameDb->GetAllSongs(&songs, false, GameDb::kSkillAny);
    } else {
        TheGameDb->GetArenaSongs(&songs, pszArena, nSkillLevel, 0);
    }
    for (const SongEntry &song : songs) {
        if (song.IsAvailable(nSkillLevel) && (song.GetType() == nType || nType == kAnyType)) {
            pSongs->push_back(song.GetName());
        }
    }
}

void Campaign::ExceptType(int nType,
                          const char *pszArena,
                          int nSkillLevel,
                          std::vector<const char *> *pSongs) {
    pSongs->clear();
    if (nType == kAnyType) {
        return;
    }
    pSongs->reserve(kFilteredReserve);
    std::vector<SongEntry> songs;
    if (strcmp(pszArena, kAllArenas) == 0) {
        TheGameDb->GetAllSongs(&songs, false, GameDb::kSkillAny);
    } else {
        TheGameDb->GetArenaSongs(&songs, pszArena, nSkillLevel, 0);
    }
    for (const SongEntry &song : songs) {
        if (song.IsAvailable(nSkillLevel) && song.GetType() != nType) {
            pSongs->push_back(song.GetName());
        }
    }
}

void Campaign::ExceptTwoTypes(int nType,
                              int nOtherType,
                              const char *pszArena,
                              int nSkillLevel,
                              std::vector<const char *> *pSongs) {
    pSongs->clear();
    if (nType == kAnyType || nOtherType == kAnyType) {
        return;
    }
    pSongs->reserve(kFilteredReserve);
    std::vector<SongEntry> songs;
    if (strcmp(pszArena, kAllArenas) == 0) {
        TheGameDb->GetAllSongs(&songs, false, GameDb::kSkillAny);
    } else {
        TheGameDb->GetArenaSongs(&songs, pszArena, nSkillLevel, 0);
    }
    for (const SongEntry &song : songs) {
        if (song.IsAvailable(nSkillLevel) && song.GetType() != nType &&
            song.GetType() != nOtherType) {
            pSongs->push_back(song.GetName());
        }
    }
}

void Campaign::FromList(DataArray *pList, int nSkillLevel, std::vector<const char *> *pSongs) {
    pSongs->clear();
    const int nSize = pList->Size();
    pSongs->reserve(nSize - 1);
    std::vector<SongEntry> songs; // Yes, the binary builds this list and never fills it.
    for (int i = kClauseFirstArgument; i < nSize; ++i) {
        const SongEntry song{TheGameDb->FindSong(pList->Sym(i))};
        if (song.IsAvailable(nSkillLevel)) {
            pSongs->push_back(song.GetName());
        }
    }
}

bool Campaign::CheckAllBeaten(const std::vector<const char *> *pSongs, int nSkillLevel) {
    for (const char *pszSong : *pSongs) {
        auto it = mRecords.begin();
        while (it != mRecords.end() && (it->mSkillLevel != nSkillLevel || it->mSong != pszSong)) {
            ++it;
        }
        if (it == mRecords.end() || it->mPercentDone != kPercentFinished) {
            return false;
        }
    }
    return true;
}

bool Campaign::CheckScoreTotal(const std::vector<const char *> *pSongs,
                               int nSkillLevel,
                               int nTotal) {
    if (nTotal <= 0) {
        return true;
    }
    int nSum = 0;
    for (const char *pszSong : *pSongs) {
        auto it = mRecords.begin();
        while (it != mRecords.end() &&
               (it->mSkillLevel != nSkillLevel || it->mSong != pszSong || it->mScore == 0)) {
            ++it;
        }
        if (it == mRecords.end()) {
            return false;
        }
        nSum += it->mScore;
    }
    return nSum >= nTotal;
}

void Campaign::RecordUnlock(int nKind,
                            const char *pszItem,
                            int nSkillLevel,
                            std::vector<UnlockableItem> *pUnlocks) {
    std::vector<const char *> &unlocks =
        (nSkillLevel == GameDb::kSkillAny) ? mUnlocks : mSkillUnlocks[nSkillLevel];
    if (std::find(unlocks.begin(), unlocks.end(), pszItem) != unlocks.end()) {
        return;
    }
    UnlockableItem item;
    item.mName = pszItem;
    item.mKind = static_cast<unsigned char>(nKind);
    pUnlocks->push_back(item);
    unlocks.push_back(pszItem);
    mLastUnlocks[nKind] = pszItem;
    if (nKind == UnlockableItem::kKindPowerup && GetFlagBit(GetPowerupBit(pszItem)) == 0) {
        mPendingPowerup = pszItem;
    }
}

void Campaign::ApplyUnlockNode(DataArray *pNode,
                               int nSkillLevel,
                               std::vector<UnlockableItem> *pUnlocks) {
    const char *pszTag = pNode->Sym(kClauseTag);
    if (strcmp(pszTag, "unlock") == 0) {
        nSkillLevel = GameDb::kSkillAny;
    } else if (strcmp(pszTag, "skill_unlock") != 0) {
        DebugWarn("Unrecognized token:%s (%s:%d)\n", pszTag, pNode->mFile, pNode->mLine);
    }
    const int nSize = pNode->Size();
    pUnlocks->reserve(pUnlocks->size() + nSize - 1);
    const unsigned char nKind = static_cast<unsigned char>(pNode->Int(kUnlockKind));
    for (int i = kFirstUnlockItem; i < nSize; ++i) {
        (void)pNode->Sym(i); // Yes, the binary looks the item up twice.
        RecordUnlock(nKind, pNode->Sym(i), nSkillLevel, pUnlocks);
    }
}

void Campaign::PrefabUnlock(DataArray *pNode, std::vector<UnlockableItem> *pUnlocks) {
    const int nSize = pNode->Size();
    pUnlocks->reserve(pUnlocks->size() + nSize - 1);
    mUnlocks.reserve(mUnlocks.size() + nSize - 1);
    const unsigned char nKind = static_cast<unsigned char>(pNode->Int(kUnlockKind));
    for (int i = kFirstUnlockItem; i < nSize; ++i) {
        const char *pszItem = pNode->Sym(i);
        if (nKind == UnlockableItem::kKindPrefab) {
            ExpandPrefab(pszItem, pUnlocks);
        }
        RecordUnlock(nKind, pszItem, GameDb::kSkillAny, pUnlocks);
    }
}

void Campaign::ExpandPrefab(const char *pszPrefab, std::vector<UnlockableItem> *pUnlocks) {
    DataArray *pMetagame = SystemConfig()->FindArray("metagame", true);
    DataArray *lists[] = {pMetagame->FindArray("locked_prefabs", true),
                          pMetagame->FindArray("prefabs", true)};
    for (int nList = 0; nList < kNumPrefabLists; ++nList) {
        DataArray *pList = lists[nList];
        for (int i = kFirstPrefab; i < pList->Size(); ++i) {
            // The names are symbols and compare as pointers.
            if (pList->Array(i)->Sym(kPrefabName) != pszPrefab) {
                continue;
            }
            DataArray *pPrefab = pList->Array(i);
            pUnlocks->reserve(pUnlocks->size() + pPrefab->Size() - 1);
            for (int j = kFirstPrefabPart; j < pPrefab->Size(); ++j) {
                RecordPrefabPart(pPrefab->Array(j), pUnlocks);
            }
            break;
        }
    }
}

void Campaign::RecordPrefabPart(DataArray *pPart, std::vector<UnlockableItem> *pUnlocks) {
    int nKind;
    switch (pPart->Int(kPartIndex)) {
    case AvatarPartSet::kPartLeftArm:
        nKind = UnlockableItem::kKindArms;
        break;
    case AvatarPartSet::kPartHead:
        nKind = UnlockableItem::kKindHead;
        break;
    case AvatarPartSet::kPartTorso:
        nKind = UnlockableItem::kKindTorso;
        break;
    case AvatarPartSet::kPartLowerBody:
        nKind = UnlockableItem::kKindLowerBody;
        break;
    case AvatarPartSet::kPartHeadGear:
        nKind = UnlockableItem::kKindHeadGear;
        break;
    case AvatarPartSet::kPartFaceGear:
        nKind = UnlockableItem::kKindFaceGear;
        break;
    default:
        return;
    }
    RecordUnlock(
        nKind, pPart->FindArray("type", true)->Sym(kPartValue), GameDb::kSkillAny, pUnlocks);
    if (nKind == UnlockableItem::kKindTorso) {
        const DataArray *pEmblem = pPart->FindArray("emblem", false);
        if (pEmblem != nullptr) {
            RecordUnlock(
                UnlockableItem::kKindEmblem, pEmblem->Sym(kPartValue), GameDb::kSkillAny, pUnlocks);
        }
    }
}

const char *Campaign::FindFirstUnfinishedSong(int nSkillLevel) {
    std::vector<const char *> arenas;
    TheGameDb->GetUnlockedArenas(&arenas, nSkillLevel, true, false);
    for (const char *pszArena : arenas) {
        std::vector<const char *> songs;
        ExceptType(SongEntry::kTypeBonus, pszArena, nSkillLevel, &songs);
        for (const char *pszSong : songs) {
            if (IsSongFinished(pszSong, nSkillLevel)) {
                continue;
            }
            const SongEntry song{TheGameDb->FindSong(pszSong)};
            if (song.GetType() != SongEntry::kTypeSecret) {
                return pszSong;
            }
        }
    }
    return nullptr;
}

bool Campaign::CompletesTier(const char *pszSong, int nSkillLevel) {
    if (IsSongFinished(pszSong, nSkillLevel)) {
        return false;
    }
    std::vector<const char *> arenas;
    TheGameDb->GetArenaNames(&arenas, false);
    int nUnfinished = 0;
    for (const char *pszArena : arenas) {
        std::vector<const char *> songs;
        ExceptType(SongEntry::kTypeBonus, pszArena, nSkillLevel, &songs);
        for (const char *pszOther : songs) {
            if (!IsSongFinished(pszOther, nSkillLevel)) {
                const SongEntry song{TheGameDb->FindSong(pszOther)};
                if (song.GetType() != SongEntry::kTypeSecret) {
                    ++nUnfinished;
                }
            }
            if (nUnfinished >= 2) {
                return false;
            }
        }
    }
    return nUnfinished == 1;
}

bool Campaign::HasBeatenGame(int nSkillLevel) {
    if (g_dwUnlockAllSongs != 0) {
        return true;
    }
    const char *pszBeatGame = LookupSymbol("beat_game");
    const std::vector<const char *> &unlocks = mSkillUnlocks[nSkillLevel];
    return std::find(unlocks.begin(), unlocks.end(), pszBeatGame) != unlocks.end();
}

int Campaign::GetHighestBeatenSkillLevel() {
    for (int nSkillLevel = kNumSkillLevels - 1; nSkillLevel >= 0; --nSkillLevel) {
        if (HasBeatenGame(nSkillLevel)) {
            return nSkillLevel;
        }
    }
    return kNoSkillLevel;
}

bool Campaign::IsSongFinished(const char *pszSong, int nSkillLevel) {
    if (g_dwUnlockAllSongs != 0) {
        return true;
    }
    if (nSkillLevel == GameDb::kSkillAny) {
        bool bFinished = false;
        SongStats record;
        for (int nLevel = 0; nLevel < kNumSkillLevels; ++nLevel) {
            GetSongStats(pszSong, nLevel, &record);
            bFinished = bFinished || record.mPercentDone == kPercentFinished;
        }
        return bFinished;
    }
    SongStats record;
    GetSongStats(pszSong, nSkillLevel, &record);
    return record.mPercentDone == kPercentFinished;
}

void Campaign::GetSongStats(const char *pszSong, int nSkillLevel, SongStats *pRecord) {
    SongStats key;
    key.mSong = LookupSymbol(pszSong);
    key.mSkillLevel = static_cast<unsigned char>(nSkillLevel);
    auto it = std::find(mRecords.begin(), mRecords.end(), key);
    *pRecord = (it == mRecords.end()) ? key : *it;
}

int Campaign::GetArenaScore(const char *pszArena, int nSkillLevel) {
    std::vector<SongEntry> songs;
    TheGameDb->GetArenaSongs(&songs, pszArena, nSkillLevel, 0);
    SongStats record;
    int nScore = 0;
    for (const SongEntry &song : songs) {
        GetSongStats(song.GetName(), nSkillLevel, &record);
        nScore += record.mScore;
    }
    return nScore;
}

int Campaign::GetArenaScoreToBeat(const char *pszArena, int nSkillLevel) {
    return SystemConfig()
        ->FindArray(kDbTag, false)
        ->FindArray("arena_scores", false)
        ->FindArray(pszArena, false)
        ->Int(nSkillLevel + kFirstSkillRow);
}

void Campaign::GetClearedSongs(int nSkillLevel, const char *pszGroup, std::vector<bool> *pCleared) {
    std::vector<const char *> songs;
    BuildFiltered(SongEntry::kTypeNormal, pszGroup, nSkillLevel, &songs);
    pCleared->resize(songs.size(), false);
    SongStats record; // Yes, the binary constructs this record and never uses it.
    for (unsigned int i = 0; i < songs.size(); ++i) {
        (*pCleared)[i] = IsSongFinished(songs[i], nSkillLevel);
    }
}

void Campaign::GetUnlockedParts(int nPart, std::vector<const char *> *pTypes) {
    AvatarPartSet::GetPartTypes(nPart, pTypes);
    if (g_dwUnlockAllParts == 0) {
        RemoveLockedItems(pTypes);
    }
}

void Campaign::GetUnlockedEmblems(std::vector<const char *> *pTypes) {
    AvatarPartSet::GetEmblemTypes(pTypes);
    if (g_dwUnlockAllParts == 0) {
        RemoveLockedItems(pTypes);
    }
}

int Campaign::GetMedal(const char *pszSong, int nSkillLevel) {
    SongStats record;
    GetSongStats(pszSong, nSkillLevel, &record);
    return GetMedalForScore(pszSong, nSkillLevel, record.mScore, record.mPercentDone);
}

int Campaign::GetMedalForScore(const char *pszSong, int nSkillLevel, int nScore, int nPercent) {
    const DataArray *pGoals = TheGameDb->FindSong(pszSong)->FindArray("score_goal", false);
    if (pGoals == nullptr || nPercent != kPercentFinished) {
        return kMedalNone;
    }
    const DataArray *pGoal = pGoals->Array(nSkillLevel + kFirstSkillRow);
    if (pGoal == nullptr) {
        return kMedalNone;
    }
    if (nScore >= pGoal->Int(kGoalThird)) {
        return kMedalThird;
    }
    if (nScore >= pGoal->Int(kGoalSecond)) {
        return kMedalSecond;
    }
    return (nScore < pGoal->Int(kGoalFirst)) ? kMedalFinished : kMedalFirst;
}

void Campaign::RemoveLockedItems(std::vector<const char *> *pItems) {
    auto it = pItems->begin();
    while (it != pItems->end()) {
        if (IsUnlocked(*it, GameDb::kSkillAny)) {
            ++it;
        } else {
            it = pItems->erase(it);
        }
    }
}

int Campaign::GetFlagBit(int nFlag) {
    return (mFlags >> nFlag) & 1;
}

void Campaign::SetFlagBit(int nFlag) {
    mFlags |= 1 << nFlag;
}

int Campaign::GetPowerupBit(const char *pszPowerup) {
    if (pszPowerup == nullptr) {
        return kFlagNone;
    }
    if (strcmp(pszPowerup, "autocatcher") == 0) {
        return kFlagAutocatcher;
    }
    if (strcmp(pszPowerup, "multiplier") == 0) {
        return kFlagMultiplier;
    }
    if (strcmp(pszPowerup, "slowdown") == 0) {
        return kFlagSlowdown;
    }
    return (strcmp(pszPowerup, "freestyle") == 0) ? kFlagFreestyle : kFlagNone;
}

const char *Campaign::TakePendingPowerup(int nSkillLevel) {
    const char *pszPowerup = nullptr;
    if (mPendingPowerup != nullptr && IsUnlocked(mPendingPowerup, nSkillLevel)) {
        SetFlagBit(GetPowerupBit(mPendingPowerup));
        pszPowerup = mPendingPowerup;
        mPendingPowerup = nullptr;
    }
    return pszPowerup;
}

int Campaign::HasSeenFreestyleTip() {
    return GetFlagBit(kFlagFreestyleTip);
}

void Campaign::SetSeenFreestyleTip() {
    SetFlagBit(kFlagFreestyleTip);
}

bool Campaign::IsUnlocked(const char *pszItem, int nSkillLevel) {
    const char *pszSymbol = LookupSymbol(pszItem);
    if (std::find(mUnlocks.begin(), mUnlocks.end(), pszSymbol) != mUnlocks.end()) {
        return true;
    }
    const int nFirst = (nSkillLevel == GameDb::kSkillAny) ? 0 : nSkillLevel;
    const int nEnd = (nSkillLevel == GameDb::kSkillAny) ? kNumSkillLevels : nSkillLevel + 1;
    for (int nLevel = nFirst; nLevel < nEnd; ++nLevel) {
        const std::vector<const char *> &unlocks = mSkillUnlocks[nLevel];
        if (std::find(unlocks.begin(), unlocks.end(), pszSymbol) != unlocks.end()) {
            return true;
        }
    }
    return false;
}

int Campaign::GetSoloOption() const {
    return mSoloOption;
}

InputMap *Campaign::GetInputMap() {
    return &mInputMap;
}

int Campaign::GetOnlineOption() const {
    return mOnlineOption;
}

void Campaign::SetInputMap(const InputMap *pMap) {
    mInputMap = *pMap;
    mModified = 1;
}

void Campaign::SetSoloOption(int nOption) {
    mSoloOption = nOption;
    mModified = 1;
}

void Campaign::SetOnlineOption(int nOption) {
    mOnlineOption = nOption;
}

int Campaign::IsModified() const {
    return mModified;
}

void Campaign::SetModified(int nModified) {
    mModified = nModified;
}

void Campaign::UnlockAllSongs() {
    g_dwUnlockAllSongs = 1;
}

void Campaign::UnlockAllParts() {
    g_dwUnlockAllParts = 1;
}

void Campaign::Save(BinStream &stream) const {
    const unsigned char nVersion = kVersion;
    stream.Write(&nVersion, sizeof(nVersion));
    BinStream &body = stream.WriteString(mName.c_str()) << mBorn;
    body.WriteString(mPassword.c_str());
    const unsigned char nNameLocked = static_cast<unsigned char>(mNameLocked);
    body.Write(&nNameLocked, sizeof(nNameLocked));
    BinStream &tail = body << mRecords;
    mAvatar.Save(tail);
    BinStream &options = tail << mInputMap;
    const unsigned char nSoloOption = static_cast<unsigned char>(mSoloOption);
    options.Write(&nSoloOption, sizeof(nSoloOption));
    options.Write(&mFlags, sizeof(mFlags));

    const unsigned int nCount = static_cast<unsigned int>(mLastUnlocks.size());
    stream.WriteEndian(&nCount, sizeof(nCount));
    for (unsigned int i = 0; i < nCount; ++i) {
        stream.WriteEndian(&i, sizeof(i));
        stream.WriteString(mLastUnlocks[i]);
    }
}

void Campaign::Load(BinStream &stream) {
    unsigned char nVersion;
    stream.Read(&nVersion, sizeof(nVersion));
    if (nVersion < kOldestVersion) {
        // Yes, the binary goes on to read the profile.
        DebugWarn("Error: cannot read in Campaign of version: %d\n", nVersion);
    }
    BinStream &body = stream >> mName >> mBorn >> mPassword;
    unsigned char nByte;
    body.Read(&nByte, sizeof(nByte));
    mNameLocked = nByte != 0;
    mAvatar.Load(body >> mRecords);

    if (nVersion >= kInputMapVersion) {
        BinStream &options = stream >> mInputMap;
        options.Read(&nByte, sizeof(nByte));
        mSoloOption = nByte != 0;
    } else {
        mSoloOption = kOptionDefault;
        mInputMap.Load(SystemConfig()->FindArray(kDbTag, false)->FindArray(kInputMapTag, false));
    }
    mPendingPowerup = nullptr;
    if (nVersion >= kFlagsVersion) {
        stream.Read(&mFlags, sizeof(mFlags));
    } else {
        mFlags = 0;
    }

    std::fill(mPassedEntries.begin(), mPassedEntries.end(), 0);
    mUnlocks.clear();
    for (std::vector<const char *> &unlocks : mSkillUnlocks) {
        unlocks.clear();
    }
    std::vector<UnlockableItem> unlocks;
    for (int nSkillLevel = 0; nSkillLevel < kNumSkillLevels; ++nSkillLevel) {
        UpdateUnlocks(&unlocks, nSkillLevel);
    }

    if (nVersion >= kLastUnlocksVersion) {
        unsigned int nCount;
        stream.ReadEndian(&nCount, sizeof(nCount));
        mLastUnlocks.resize(nCount, nullptr);
        String item;
        for (unsigned int i = 0; i < mLastUnlocks.size(); ++i) {
            unsigned int nIndex;
            stream.ReadEndian(&nIndex, sizeof(nIndex));
            stream >> item; // Yes, the binary discards the index and the item.
        }
    } else {
        mLastUnlocks.resize(kNumUnlockKinds, nullptr);
        for (const char *&pszItem : mLastUnlocks) {
            pszItem = "";
        }
    }
}

BinStream &operator<<(BinStream &stream, const Campaign &profile) {
    profile.Save(stream);
    return stream;
}

BinStream &operator>>(BinStream &stream, Campaign &profile) {
    profile.Load(stream);
    return stream;
}
