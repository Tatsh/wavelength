#include "game/gamedb.h"

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <functional>

#include "game/avatar.h"
#include "game/avatarpartset.h"
#include "os/debug.h"
#include "os/locale.h"
#include "os/mem.h"
#include "os/system.h"
#include "script/symbol.h"

namespace {

constexpr char kDbSection[] = "db";
constexpr char kTutorialArena[] = "Tutorial";
constexpr char kRemixBufferTag[] = "RemixBuf";
constexpr char kSongIdTag[] = "id";
constexpr char kEmptyName[] = "";

// The power-up levels below GameDb::kPowerupLevelNone.
constexpr int kPowerupLevelLow = 0;
constexpr int kPowerupLevelHigh = 1;

// The defaults of the constructor.
constexpr int kDefaultPowerupLevel = kPowerupLevelHigh;
constexpr int kDefaultMaxPlayers = 4;
constexpr float kNoResult = -1.0f;
constexpr int kNoStreak = -1;

// The players' colours, in slot order.
enum Slot {
    kSlotGreen = 0,
    kSlotPurple = 1,
    kSlotRed = 2,
    kSlotYellow = 3,
};

// The songs GetAllSongs() reserves room for.
constexpr int kSongsReserved = 30;

// The songs of an arena FindSongGroup() reserves room for.
constexpr int kArenaSongsReserved = 6;

// The GetArenaSongs() filters.
enum ArenaSongFilter {
    kArenaSongFilterAll = 0,
    kArenaSongFilterGame = 1,
    kArenaSongFilterRemix = 2,
    kArenaSongFilterDuel = 3,
};

// The value of mRank before RankPlayers() runs, and of mNetOrder for a player of a local game.
constexpr int kUnranked = -1;
constexpr int kNoPad = -1;
constexpr int kOnlinePad = 0;

// The shortest arena name GetSongIndex() reads a digit from.
constexpr int kMinArenaNameLength = 2;
constexpr int kNoSongIndex = -1;

} // namespace

// The unit's static initialiser at NTSC-U/C: 0x0026f568, PAL: 0x00279108, and its global
// constructor at NTSC-U/C: 0x0026f5a0, PAL: 0x00279140, store the instance.
// NTSC-U/C: 0x00440d44
GameDb *TheGameDb = GameDb::shared();

int GameDb::FindInt(const char *pszName, DataArray *pData, bool bFail) {
    int nValue = 0;
    pData->FindInt(pszName, &nValue, bFail);
    return nValue;
}

const char *GameDb::FindSymbol(const char *pszName, DataArray *pData, bool bFail) {
    const char *pszValue = nullptr;
    pData->FindSymbol(pszName, &pszValue, bFail);
    return pszValue;
}

const char *GameDb::GetSlotColor(int nSlot) {
    switch (nSlot) {
    case kSlotGreen:
        return "green";
    case kSlotPurple:
        return "purple";
    case kSlotRed:
        return "red";
    case kSlotYellow:
        return "yellow";
    default:
        DebugWarn("bad color");
        return nullptr;
    }
}

void GameDb::GetSkillRange(int nSkillLevel, bool bExact, int *pnFirst, int *pnEnd) {
    if (nSkillLevel != kSkillAny && bExact) {
        *pnFirst = nSkillLevel;
        *pnEnd = nSkillLevel + 1;
    } else {
        *pnFirst = kSkillNovice;
        *pnEnd = kSkillAny;
    }
}

GameDb::GameDb()
    : mLoadRemix(0), mRemixReadOnly(0), mPracticeMode(0), mTutorial(0), mSkillLevel(kSkillNovice),
      mPowerupLevel(kDefaultPowerupLevel), mRuleSet(kRuleSetGame), mCommunity(kCommunitySolo),
      mMaxPlayers(kDefaultMaxPlayers), mNumPads(0), mDemo(nullptr), mWinSequence(0), mUnlockAll(0),
      mWon(0), mEnergized(kNoResult), mFullMixBars(0), mBestStreak(kNoStreak), mProgress(kNoResult),
      mReservedA4(0), mSongs(nullptr), mRemixBuffer(nullptr), mRemixActive(1), mNetRemixEnded(0),
      mSongTick(0.0f), mSongTime(0.0f) {
}

void GameDb::GetArenaNames(std::vector<const char *> *pArenas, bool bIncludeTutorial) {
    const int nSize = mSongs->Size();
    pArenas->clear();
    pArenas->reserve(nSize - 1);
    for (int i = 1; i < nSize; ++i) {
        const char *pszArena = mSongs->Array(i)->Sym(0);
        if (!bIncludeTutorial && strcmp(pszArena, kTutorialArena) == 0) {
            continue;
        }
        pArenas->push_back(pszArena);
    }
}

void GameDb::GetArenaSongs(std::vector<SongEntry> *pSongs,
                           const char *pszArena,
                           int nSkillLevel,
                           int nFilter) {
    DataArray *pArena = mSongs->FindArray(pszArena, true);
    const int nSize = pArena->Size();
    pSongs->reserve(pSongs->size() + nSize - 1);
    for (int i = 1; i < nSize; ++i) {
        const SongEntry song{pArena->Array(i)};
        if (nSkillLevel != kSkillAny && !song.IsAvailable(nSkillLevel)) {
            continue;
        }
        bool bListed;
        switch (nFilter) {
        case kArenaSongFilterAll:
            bListed = true;
            break;
        case kArenaSongFilterGame:
            bListed = song.IsGameable();
            break;
        case kArenaSongFilterRemix:
            bListed = song.IsRemixable();
            break;
        case kArenaSongFilterDuel:
            bListed = song.IsDuel();
            break;
        default:
            bListed = false;
            break;
        }
        if (bListed) {
            pSongs->push_back(song);
        }
    }
}

void GameDb::GetAllSongs(std::vector<SongEntry> *pSongs, bool bDuelOnly, int nSkillLevel) {
    pSongs->clear();
    pSongs->reserve(kSongsReserved);
    const char *pszTutorial = LookupSymbol(kTutorialArena);
    for (int i = 1; i < mSongs->Size(); ++i) {
        DataArray *pArena = mSongs->Array(i);
        if (pArena->Sym(0) == pszTutorial) {
            continue;
        }
        for (int j = 1; j < pArena->Size(); ++j) {
            const SongEntry song{pArena->Array(j)};
            if (bDuelOnly && !song.IsDuel()) {
                continue;
            }
            if (nSkillLevel != kSkillAny && !song.IsAvailable(nSkillLevel)) {
                continue;
            }
            pSongs->push_back(song);
        }
    }
}

void GameDb::GetHostSongs(std::vector<SongEntry> *pSongs, int nSkillLevel, bool bDuelOnly) {
    GetAllSongs(pSongs, bDuelOnly, nSkillLevel);
    auto it = pSongs->begin();
    while (it != pSongs->end()) {
        if (it->GetType() == SongEntry::kTypeSecret &&
            !mPlayers[0]->mProfile.IsUnlocked(it->GetName(), nSkillLevel)) {
            it = pSongs->erase(it);
        } else {
            ++it;
        }
    }
}

void GameDb::GetUnlockedSongs(std::vector<SongEntry> *pSongs,
                              const char *pszArena,
                              int nSkillLevel,
                              bool bFirstPlayerOnly) {
    std::vector<const char *> arenas;
    if (pszArena == nullptr || *pszArena == '\0') {
        GetUnlockedArenas(&arenas, nSkillLevel, bFirstPlayerOnly, false);
    } else {
        arenas.push_back(pszArena);
    }
    pSongs->clear();
    pSongs->reserve(kSongsReserved);
    for (const char *pszName : arenas) {
        GetArenaSongs(pSongs, pszName, nSkillLevel, kArenaSongFilterAll);
    }
    if (mUnlockAll != 0) {
        return;
    }

    const int nPlayers = bFirstPlayerOnly ? 1 : static_cast<int>(mPlayers.size());
    int nFirst;
    int nEnd;
    GetSkillRange(nSkillLevel, bFirstPlayerOnly, &nFirst, &nEnd);
    auto it = pSongs->begin();
    while (it != pSongs->end()) {
        int i = 0;
        while (i < nPlayers) {
            const int nType = it->GetType();
            int nLevel = nFirst;
            while (nLevel < nEnd && nType != SongEntry::kTypeNormal &&
                   !mPlayers[i]->mProfile.IsUnlocked(it->GetName(), nLevel)) {
                ++nLevel;
            }
            if (nLevel != nEnd) {
                break;
            }
            ++i;
        }
        if (i != nPlayers) {
            ++it;
        } else {
            it = pSongs->erase(it);
        }
    }
}

void GameDb::GetUnlockedArenas(std::vector<const char *> *pArenas,
                               int nSkillLevel,
                               bool bFirstPlayerOnly,
                               bool bIncludeTutorial) {
    GetArenaNames(pArenas, bIncludeTutorial);
    if (mUnlockAll != 0) {
        return;
    }

    const int nPlayers = bFirstPlayerOnly ? 1 : static_cast<int>(mPlayers.size());
    int nFirst;
    int nEnd;
    GetSkillRange(nSkillLevel, bFirstPlayerOnly, &nFirst, &nEnd);
    auto it = pArenas->begin();
    while (it != pArenas->end()) {
        int i = 0;
        while (i < nPlayers) {
            int nLevel = nFirst;
            while (nLevel < nEnd && !mPlayers[i]->mProfile.IsUnlocked(*it, nLevel)) {
                ++nLevel;
            }
            if (nLevel != nEnd) {
                break;
            }
            ++i;
        }
        if (i != nPlayers) {
            ++it;
        } else {
            it = pArenas->erase(it);
        }
    }
}

DataArray *GameDb::FindSong(const char *pszSong) {
    DataArray *pSong = nullptr;
    for (int i = 1; i < mSongs->Size(); ++i) {
        pSong = mSongs->Array(i)->FindArray(pszSong, false);
        if (pSong != nullptr) {
            break;
        }
    }
    return pSong;
}

bool GameDb::HasSong(const char *pszSong) {
    for (int i = 1; i < mSongs->Size(); ++i) {
        if (mSongs->Array(i)->FindArray(pszSong, false) != nullptr) {
            return true;
        }
    }
    return false;
}

bool GameDb::FindSongById(DataArray **ppSong, int nId) {
    DataArray *pFound = nullptr;
    for (int i = 1; i < mSongs->Size(); ++i) {
        DataArray *pArena = mSongs->Array(i);
        for (int j = 1; j < pArena->Size(); ++j) {
            int nSongId = -1;
            pArena->Array(j)->FindInt(kSongIdTag, &nSongId, false);
            if (nSongId == nId) {
                pFound = pArena->Array(j);
                break;
            }
        }
    }
    if (pFound == nullptr) {
        return false;
    }
    *ppSong = pFound;
    return true;
}

void GameDb::UnlockAllSongs() {
    mUnlockAll = 1;
    Campaign::UnlockAllSongs();
}

int GameDb::IsWinSequence() const {
    return mWinSequence;
}

void GameDb::SetWinSequence(bool bWinSequence) {
    mWinSequence = bWinSequence;
}

RemixInfo *GameDb::GetRemixInfo() {
    return &mRemixInfo;
}

void GameDb::SetRemixBuffer(int nSize) {
    PoolMemFree(mRemixBuffer);
    if (nSize == 0) {
        mRemixBuffer = nullptr;
    } else {
        mRemixBuffer = static_cast<char *>(PoolMemAlloc(nSize, kRemixBufferTag, 0));
    }
}

void GameDb::SetRemix(const RemixInfo *pInfo) {
    mRemixInfo = *pInfo;
}

int GameDb::GetRemixActive() const {
    return mRemixActive;
}

void GameDb::SetRemixActive(int nActive) {
    mRemixActive = nActive;
}

int GameDb::GetNetRemixEnded() const {
    return mNetRemixEnded;
}

void GameDb::SetNetRemixEnded(int nEnded) {
    mNetRemixEnded = nEnded;
}

void GameDb::Init() {
    DataArray *pDb = SystemConfig()->FindArray(kDbSection, true);
    mPracticeMode = FindInt("practice_mode", pDb, true) != 0;
    mSkillLevel = FindInt("skill_level", pDb, true);
    mLoadRemix = FindInt("load_remix", pDb, false) != 0;
    mCommunity = FindInt("community", pDb, true);
    mRuleSet = FindInt("rule_set", pDb, true);
    mSong = FindSymbol("song", pDb, true);
    mSongs = pDb->FindArray("songs", true);
    AvatarPartSet::Init();
    ClearPlayers();

    const int nPlayers = FindInt("num_players", pDb, true);
    int nMaxPlayers;
    if (pDb->FindInt("max_players", &nMaxPlayers, false)) {
        SetMaxPlayers(nMaxPlayers);
    }
    int bNet = 0;
    pDb->FindBool("net", &bNet, false);
    DataArray *pPlayers = pDb->FindArray("players", true);
    for (int i = 0; i < nPlayers; ++i) {
        const char *pszName = pPlayers->Sym(i + 1);
        Campaign profile;
        profile.mName = pszName;
        if (bNet != 0) {
            AddPlayer(i == 0, &profile, i, i);
        } else {
            AddPlayer(&profile);
        }
    }
    InitSfx();
    mOptions.ApplyOutputMode();
}

void GameDb::Terminate() {
    ClearPlayers();
    TerminateSfx();
    AvatarPartSet::Terminate();
}

GameDb *GameDb::shared() {
    // The registered destructor at NTSC-U/C: 0x0026e428, PAL: 0x00277fc8 destroys the instance.
    static GameDb instance;
    return &instance;
}

void GameDb::Poll() {
    mSfxScheduler.Poll();
    AvatarPartSet::Poll(mSongTick);
}

void GameDb::ClearPlayers() {
    for (PlayerInfo *pPlayer : mPlayers) {
        delete pPlayer;
    }
    mReservedA4 = 0;
    mPlayers.clear();
    mNumPads = 0;
    mWon = 0;
    mEnergized = 0.0f;
    mFullMixBars = 0;
    mBestStreak = 0;
    mProgress = 0.0f;
}

void GameDb::AddPlayer(const Campaign *pProfile) {
    const int nSlot = static_cast<int>(mPlayers.size());
    mPlayers.push_back(new PlayerInfo(1, nSlot, *pProfile, kUnranked));
    ++mNumPads;
}

void GameDb::AddPlayer(int bLocal, const Campaign *pProfile, int nSlot, int nNetOrder) {
    mPlayers.push_back(new PlayerInfo(bLocal, nSlot, *pProfile, nNetOrder));
    if (bLocal != 0) {
        ++mNumPads;
    }
}

int GameDb::GetNumPlayers() const {
    return static_cast<int>(mPlayers.size());
}

int GameDb::GetNumPads() const {
    return mNumPads;
}

bool GameDb::IsLocalPlayer(int nPlayer) const {
    return mPlayers[nPlayer]->mLocal != 0;
}

int GameDb::GetPlayerSlot(int nPlayer) const {
    return mPlayers[nPlayer]->mSlot;
}

const char *GameDb::GetPlayerColor(int nPlayer) const {
    return GetSlotColor(mPlayers[nPlayer]->mSlot);
}

const char *GameDb::GetPlayerName(int nPlayer) const {
    return mPlayers[nPlayer]->mProfile.mName.c_str();
}

int GameDb::GetPlayerNetOrder(int nPlayer) const {
    return mPlayers[nPlayer]->mNetOrder;
}

int GameDb::GetPlayerPad(int nPlayer) const {
    if (mCommunity != kCommunityOnline) {
        return nPlayer;
    }
    return IsLocalPlayer(nPlayer) ? kOnlinePad : kNoPad;
}

Campaign *GameDb::GetProfile(int nPlayer) {
    return &mPlayers[nPlayer]->mProfile;
}

AvatarPartSet *GameDb::GetAvatar(int nPlayer) {
    return &mPlayers[nPlayer]->mProfile.mAvatar;
}

int GameDb::GetPlayerScore(int nPlayer) const {
    return mPlayers[nPlayer]->mScore;
}

int GameDb::GetPlayerRank(int nPlayer) const {
    return mPlayers[nPlayer]->mRank;
}

InputMap *GameDb::GetInputMap(int nPlayer) {
    return mPlayers[nPlayer]->mProfile.GetInputMap();
}

void GameDb::SetPlayerScore(int nPlayer, int nScore) {
    mPlayers[nPlayer]->mScore = nScore;
    RankPlayers();
}

void GameDb::SetGameParams(const NetGameParams *pParams) {
    mSong = pParams->mSong;
    mLoadRemix = pParams->mLoadRemix;
    mRemixReadOnly = pParams->mRemixReadOnly;
    mRemixName = pParams->mRemixName;
    mPracticeMode = pParams->mPracticeMode;
    mTutorial = pParams->mTutorial;
    mSkillLevel = pParams->mSkillLevel;
    mPowerupLevel = pParams->mPowerupLevel;
    mRuleSet = pParams->mRuleSet;
    mCommunity = pParams->mCommunity;
    mMaxPlayers = pParams->mMaxPlayers;
    const SongEntry song{FindSong(pParams->mSong.c_str())};
    SetArena(song.GetArenaPath());
}

void GameDb::SetSkillLevel(int nSkillLevel) {
    mSkillLevel = nSkillLevel;
}

void GameDb::SetPowerupLevel(int nPowerupLevel) {
    mPowerupLevel = nPowerupLevel;
}

void GameDb::SetPracticeMode(bool bPracticeMode) {
    mPracticeMode = bPracticeMode;
}

void GameDb::SetTutorial(int nTutorial) {
    mTutorial = nTutorial;
}

void GameDb::SetSong(const char *pszSong) {
    mSong = pszSong;
    const SongEntry song{FindSong(pszSong)};
    SetArena(song.GetArenaPath());
}

void GameDb::SetRemixName(const char *pszName) {
    mRemixName = pszName;
}

void GameDb::SetRemixReadOnly(int nReadOnly) {
    mRemixReadOnly = nReadOnly;
}

void GameDb::SetArena(const char *pszArena) {
    mArena = pszArena;
}

void GameDb::SetLoadRemix(bool bLoadRemix) {
    mLoadRemix = bLoadRemix;
}

void GameDb::SetRuleSet(int nRuleSet) {
    mRuleSet = nRuleSet;
}

void GameDb::SetCommunity(int nCommunity) {
    mCommunity = nCommunity;
}

void GameDb::SetMaxPlayers(int nMaxPlayers) {
    mMaxPlayers = nMaxPlayers;
}

const char *GameDb::GetDemo() const {
    return mDemo.mLength != 0 ? mDemo.c_str() : nullptr;
}

void GameDb::SetDemo(const char *pszDemo) {
    mDemo = pszDemo;
}

void GameDb::SetEnergized(float fEnergized) {
    mEnergized = fEnergized;
}

float GameDb::GetEnergized() const {
    return mEnergized;
}

void GameDb::SetFullMixBars(int nBars) {
    mFullMixBars = nBars;
}

int GameDb::GetFullMixBars() const {
    return mFullMixBars;
}

void GameDb::SetBestStreak(int nStreak) {
    mBestStreak = nStreak;
}

int GameDb::GetBestStreak() const {
    return mBestStreak;
}

void GameDb::SetProgress(float fProgress) {
    mProgress = fProgress;
}

float GameDb::GetProgress() const {
    return mProgress;
}

void GameDb::SetWon(bool bWon) {
    mWon = bWon;
}

void GameDb::RankPlayers() {
    std::vector<int> scores;
    for (size_t i = 0; i < mPlayers.size(); ++i) {
        scores.push_back(mPlayers[i]->mScore);
    }
    // The binary discards the end std::unique() reports and sorts every element.
    (void)std::unique(scores.begin(), scores.end());
    std::sort(scores.begin(), scores.end(), std::greater<int>());
    for (size_t i = 0; i < mPlayers.size(); ++i) {
        PlayerInfo *pPlayer = mPlayers[i];
        pPlayer->mRank = static_cast<int>(std::find(scores.begin(), scores.end(), pPlayer->mScore) -
                                          scores.begin());
    }
}

const char *GameDb::FindSongGroup() {
    std::vector<const char *> arenas;
    GetArenaNames(&arenas, true);
    const char *pszSong = mSong.c_str();
    std::vector<SongEntry> songs;
    songs.reserve(kArenaSongsReserved);
    for (size_t i = 0; i < arenas.size(); ++i) {
        songs.clear();
        GetArenaSongs(&songs, arenas[i], kSkillAny, kArenaSongFilterAll);
        if (SongEntry::Find(songs, pszSong) != nullptr) {
            return arenas[i];
        }
    }
    return nullptr;
}

int GameDb::GetSongIndex() {
    const SongEntry song{FindSong(mSong.c_str())};
    if (song.GetType() != SongEntry::kTypeNormal) {
        return kNoSongIndex;
    }
    const char *pszArena = mArena.c_str();
    const int nLength = static_cast<int>(strlen(pszArena));
    if (nLength < kMinArenaNameLength) {
        return kNoSongIndex;
    }
    return atoi(&pszArena[nLength - 1]) - 1;
}

const char *GameDb::GetDifficultyName() const {
    int nSkillLevel;
    if (mLoadRemix != 0) {
        if (mRemixInfo.mPlayable == 0) {
            return TheLocale.Localize("skill_unplayable", true);
        }
        nSkillLevel = mRemixInfo.mSkillLevel;
    } else {
        nSkillLevel = mSkillLevel;
    }
    return GetDifficultyName(nSkillLevel, mRuleSet);
}

const char *GameDb::GetDifficultyName(int nSkillLevel, int nRuleSet) const {
    const char *pszToken;
    switch (nRuleSet) {
    case kRuleSetRemix:
        return kEmptyName;
    case kRuleSetGame:
        switch (nSkillLevel) {
        case kSkillNovice:
            pszToken = "skill_easy";
            break;
        case kSkillIntermediate:
            pszToken = "skill_norm";
            break;
        case kSkillAdvanced:
            pszToken = "skill_exp";
            break;
        case kSkillInsane:
            pszToken = "skill_insane";
            break;
        default:
            return kEmptyName;
        }
        break;
    case kRuleSetDuel:
        switch (nSkillLevel) {
        case kSkillIntermediate:
            pszToken = "duel_easy";
            break;
        case kSkillAdvanced:
            pszToken = "duel_medium";
            break;
        case kSkillInsane:
            pszToken = "duel_hard";
            break;
        default:
            return kEmptyName;
        }
        break;
    default:
        DebugWarn("no rule set!\n");
        return kEmptyName;
    }
    return TheLocale.Localize(pszToken, false);
}

const char *GameDb::GetModeName() const {
    return GetModeName(mRuleSet);
}

const char *GameDb::GetModeName(int nRuleSet) const {
    switch (nRuleSet) {
    case kRuleSetDuel:
        return TheLocale.Localize("mode_duel", false);
    case kRuleSetGame:
        return TheLocale.Localize("mode_game", false);
    case kRuleSetRemix:
        return TheLocale.Localize("mode_remix", false);
    default:
        return kEmptyName;
    }
}

const char *GameDb::GetPowerupName(int nPowerupLevel) const {
    switch (nPowerupLevel) {
    case kPowerupLevelLow:
        return TheLocale.Localize("pup_low", false);
    case kPowerupLevelHigh:
        return TheLocale.Localize("pup_hi", false);
    case kPowerupLevelNone:
        return TheLocale.Localize("pup_off", false);
    default:
        return kEmptyName;
    }
}

void GameDb::SetAvatarCameraParams(float fNear, float fFar) {
    Avatar::SetDepthRange(fNear, fFar);
}

GameOptions *GameDb::GetOptions() {
    return &mOptions;
}

void GameDb::SetOptions(const GameOptions *pOptions) {
    mOptions = *pOptions;
    mOptions.ApplyOutputMode();
}
