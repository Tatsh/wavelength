#include "game/game.h"

#include <algorithm>
#include <vector>

#include "game/banktrack.h"
#include "game/gameconfig.h"
#include "game/multigamelogic.h"
#include "game/playerprofile.h"
#include "game/sologamelogic.h"
#include "game/song.h"
#include "game/songentry.h"
#include "game/tutorialgamelogic.h"
#include "gfx/gfxmanager.h"
#include "os/debug.h"
#include "os/locale.h"
#include "os/string.h"
#include "os/system.h"
#include "script/scriptfunction.h"
#include "synth/synth.h"

namespace {

constexpr char kPlayAllGemsCommand[] = "play_all_gems";
constexpr char kPracticeModeCommand[] = "practice_mode";
constexpr char kNoCaptureCommand[] = "no_capture";

constexpr char kGameSection[] = "game";
constexpr char kSoloFxBankFileKey[] = "solo_game_fx_bank_file";
constexpr char kMultiFxBankFileKey[] = "multi_game_fx_bank_file";
constexpr char kCheerFxBankFileKey[] = "cheer_fx_bank_file";
constexpr char kFxBankSlotKey[] = "game_fx_bank_slot";

constexpr char kOnText[] = "";
constexpr char kNotText[] = "not ";

constexpr char kDemoModeKey[] = "DEMO_MODE";

constexpr int kNoBankSlot = -1;

// The number of synthesiser bank slots the sound memory is divided among.
constexpr int kBankSlotCount = 7;

// The size Synth::Partition() leaves unchanged.
constexpr int kKeepBankSize = -1;

constexpr int kFreeBankSize = 0;

// The number of songs in a group, one cleared flag each.
constexpr int kGroupSongCount = 4;

// The song type that does not show the cleared songs of its group. The meaning is not yet
// recovered.
constexpr int kSongTypeWithoutClearedSongs = 1;

constexpr int kDefaultTrackOption = 0;

} // namespace

Game::Game() {
    mLogic = nullptr;
    mCheerBankSlot = kNoBankSlot;
    mFxBankSlot = kNoBankSlot;
    mDemo = TheGameDb->GetDemo() != 0 ? 1 : 0;
    mReserved58 = 0;
}

Game::~Game() {
    (void)TheSynth->DisableSoftFx(); // Yes, the result is discarded.
    ReleaseBanks();
    ScriptFunction::Unregister(TogglePlayAllGems);
    ScriptFunction::Unregister(TogglePracticeMode);
    ScriptFunction::Unregister(ToggleNoCapture);
}

void Game::OnStart() {
    ScriptFunction::Register(TogglePlayAllGems, kPlayAllGemsCommand, nullptr);
    ScriptFunction::Register(TogglePracticeMode, kPracticeModeCommand, nullptr);
    ScriptFunction::Register(ToggleNoCapture, kNoCaptureCommand, nullptr);
}

void Game::CreateLogic() {
    if (TheGameDb->GetDemo() != 0 || TheGameConfig->mZeroRandSeed) {
        mSeed = 0;
    }
    if (TheGameDb->mTutorial != 0) {
        mLogic = new TutorialGameLogic(mSong, mSongConfig);
    } else if (TheGameDb->mCommunity == GameDb::kCommunitySolo) {
        mLogic = new SoloGameLogic(mSong, mSongConfig, mSeed);
    } else {
        mLogic = new MultiGameLogic(mSong, mSongConfig, mSeed);
    }
    World::SetLogic(mLogic);
}

bool Game::IsFinished() {
    return mLogic->IsFinished() && mLogic->HasQuit() == 0;
}

void Game::Handle(const RotateEvent &event) {
    Record(event);
}

void Game::Handle(const PlayNoteEvent &event) {
    Record(event);
}

void Game::Handle(const StickEvent<2> &event) {
    if (mLogic->IsFreestyleTrackActive()) {
        Record(event);
    }
}

void Game::Handle(const StickEvent<6> &event) {
    if (mLogic->IsFreestyleTrackActive()) {
        Record(event);
    }
}

void Game::Handle(const BtnEvent<3> &event) {
    Record(event);
}

void Game::Handle(const BtnEvent<4> &event) {
    Record(event);
}

void Game::Handle(const BtnEvent<5> &event) {
    Record(event);
}

void Game::Handle(const BtnEvent<10> &event) {
    Record(event);
}

void Game::BeginAssetLoad() {
    Synth *pSynth = TheSynth;
    DataArray *pGame = SystemConfig()->FindArray(kGameSection, false);
    const char *pszFxBankFile = nullptr;
    const char *pszCheerBankFile = nullptr;
    const char *pszFxBankKey =
        TheGameDb->mCommunity == GameDb::kCommunitySolo ? kSoloFxBankFileKey : kMultiFxBankFileKey;
    (void)pGame->FindSymbol(pszFxBankKey, &pszFxBankFile, true); // Yes, the result is discarded.
    (void)pGame->FindSymbol(kCheerFxBankFileKey, &pszCheerBankFile, true);
    (void)pGame->FindInt(kFxBankSlotKey, &mFxBankSlot, true);
    mCheerBankSlot = mSong->GetBankTrack()->GetEndBankSlot();

    std::vector<int> blocks(kBankSlotCount, kKeepBankSize);
    std::fill(blocks.begin() + mCheerBankSlot, blocks.end(), kFreeBankSize);
    blocks[mCheerBankSlot] = pSynth->GetBankBlockCount(String(pszCheerBankFile));
    (void)pSynth->Partition(blocks);

    pSynth->UnloadBank(static_cast<unsigned short>(mFxBankSlot));
    pSynth->UnloadBank(static_cast<unsigned short>(mCheerBankSlot));
    pSynth->LoadBank(static_cast<unsigned short>(mFxBankSlot), String(pszFxBankFile), true);
    pSynth->LoadBank(static_cast<unsigned short>(mCheerBankSlot), String(pszCheerBankFile), true);
}

bool Game::IsAssetLoadDone() {
    return TheSynth->IsBankLoaded(static_cast<unsigned short>(mFxBankSlot));
}

void Game::BeginSongLoad() {
    mSong->GetBankTrack()->Load();
}

bool Game::IsSongLoadDone() {
    return mSong->GetBankTrack()->PollLoad();
}

void Game::ReleaseBanks() {
    TheSynth->UnloadBank(static_cast<unsigned short>(mFxBankSlot));
    TheSynth->UnloadBank(static_cast<unsigned short>(mCheerBankSlot));
    mCheerBankSlot = kNoBankSlot;
    mFxBankSlot = kNoBankSlot;
}

void Game::BeginEnding() {
    BankTrack *pBankTrack = mSong->GetBankTrack();
    pBankTrack->Stop();
    pBankTrack->Load();
}

void Game::PollEnding() {
}

bool Game::IsEndingDone() {
    return mSong->GetBankTrack()->PollLoad();
}

void Game::BuildTracks(int nStartTick) {
    std::vector<int> instruments;
    std::vector<int> trackTypes;
    int nOption = kDefaultTrackOption;
    for (int i = 0; i < mSong->GetNumTracks(); ++i) {
        const int nType = mSong->GetTrackType(i);
        if (nType == Song::kTrackTypeCatch) {
            instruments.push_back(mSong->GetTrackInstrument(i));
            trackTypes.push_back(nType);
        } else if (nType == Song::kTrackTypeScratch || nType == Song::kTrackTypeAxe) {
            nOption = nType;
        } else {
            DebugWarn("illegal music mode\n");
        }
    }

    const int nEndTick =
        TheGameDb->mTutorial != 0 ? 0 : mSong->mNumBars * mSong->mBuilder->mTicksPerBar;
    std::vector<bool> clearedSongs;
    const SongEntry entry{TheGameDb->FindSong(TheGameDb->mSong.mBuffer)};
    const int nSongType = entry.GetType();
    if (TheGameDb->mCommunity == GameDb::kCommunitySolo && TheGameDb->mTutorial == 0 &&
        nSongType != kSongTypeWithoutClearedSongs) {
        PlayerProfile *pProfile = TheGameDb->GetProfile(0);
        const int nSkillLevel = TheGameDb->mSkillLevel;
        pProfile->GetClearedSongs(nSkillLevel, TheGameDb->FindSongGroup(), &clearedSongs);
    } else {
        clearedSongs.assign(kGroupSongCount, false);
    }

    TheGfxManager.BuildTracks(static_cast<float>(nStartTick),
                              static_cast<float>(nEndTick),
                              mSong->GetMsPerTick(),
                              instruments,
                              trackTypes,
                              nOption,
                              clearedSongs);
    if (TheGameDb->GetDemo() != 0) {
        TheGfxManager.SetLyricText(TheLocale.Localize(kDemoModeKey, true), true);
    }
}

void Game::TogglePlayAllGems([[maybe_unused]] DataArray *pCommand,
                             [[maybe_unused]] void *pUserData) {
    TheGameConfig->mPlayAllGems = !TheGameConfig->mPlayAllGems;
    DebugPrint("CHEAT: %splaying all gems\n", TheGameConfig->mPlayAllGems ? kOnText : kNotText);
}

void Game::TogglePracticeMode([[maybe_unused]] DataArray *pCommand,
                              [[maybe_unused]] void *pUserData) {
    TheGameDb->SetPracticeMode(TheGameDb->mPracticeMode == 0);
    DebugPrint("CHEAT: %sin practice mode\n", TheGameDb->mPracticeMode != 0 ? kOnText : kNotText);
}

void Game::ToggleNoCapture([[maybe_unused]] DataArray *pCommand, [[maybe_unused]] void *pUserData) {
    TheGameConfig->mNoCapture = !TheGameConfig->mNoCapture;
    DebugPrint("CHEAT: %sin no capture mode\n", TheGameConfig->mNoCapture ? kOnText : kNotText);
}
