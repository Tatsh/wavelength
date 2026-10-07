#include "game/gamelogic.h"

#include <map>
#include <utility>

#include "game/axetrack.h"
#include "game/controllerdisplay.h"
#include "game/forcefeedbackmgr.h"
#include "game/gameconfig.h"
#include "game/gamedb.h"
#include "game/gamefx.h"
#include "game/helptext.h"
#include "game/mixer.h"
#include "game/rateaverager.h"
#include "game/remoteplayer.h"
#include "game/scratchtrack.h"
#include "game/sessionlog.h"
#include "game/songentry.h"
#include "game/triggermgr.h"
#include "gfx/gfxmanager.h"
#include "math/statsaccumulator.h"
#include "met/metagame.h"
#include "os/debug.h"
#include "os/joypad.h"
#include "os/locale.h"
#include "os/mem.h"
#include "os/memfun1command.h"
#include "os/memfuncommand.h"
#include "os/system.h"
#include "script/scriptfunction.h"

namespace {

constexpr char kInitialTrackLoopKey[] = "initial_track_loop";
constexpr char kControllerKey[] = "controller";
constexpr char kGameKey[] = "game";
constexpr char kPowerupDistKey[] = "powerup_dist";
constexpr char kAutocatcherKey[] = "autocatcher";
constexpr char kMultiplierKey[] = "multiplier";
constexpr char kSlowdownKey[] = "slowdown";
constexpr char kFreestyleKey[] = "freestyle";

constexpr char kAutocatcherCommand[] = "autocatcher";
constexpr char kMultiplierCommand[] = "multiplier";
constexpr char kSlowCommand[] = "slow";
constexpr char kBumperCommand[] = "bumper";
constexpr char kCripplerCommand[] = "crippler";
constexpr char kFreestyleCommand[] = "freestyle";
constexpr char kFreestyleToggleCommand[] = "freestyle_toggle";
constexpr char kPowerupCommand[] = "powerup";

constexpr char kSlowdownText[] = "SLOWDOWN";
constexpr char kAutocatcherText[] = "AUTOCATCHER";
constexpr char kMultiplierText[] = "MULTIPLIER";
constexpr char kFreestyleStartText[] = "FREESTYLE_START";

constexpr char kCatchTrackTag[] = "CatchTrack";
constexpr char kAxeTrackTag[] = "AxeTrack";
constexpr char kScratchTrackTag[] = "ScratchTrack";

// Length of the first loop of a tutorial song when the configuration does not set one, in bars.
constexpr int kDefaultInitialTrackLoop = 2;

// Tick the metagame shows the blank screen at, before the first bar.
constexpr int kBlankScreenTick = -3840;

// Period of the force feedback beat, in ticks.
constexpr int kForceFeedbackBeatTicks = 480;

// Shortest fade of the ending, in milliseconds.
constexpr float kMinimumEndingFadeMs = 1000.0f;

// Time the song runs on after the fade of the ending, in milliseconds.
constexpr float kEndingTailMs = 100.0f;

// Time the power-up text stays on the screen, in milliseconds.
constexpr float kPowerupTextMs = 1600.0f;

// Time between two steps of the slowdown ramp, in ticks.
constexpr int kSlowdownStepTicks = 20;

// Scale of the checkpoints Start() places.
constexpr float kCheckpointScale = 1.0f;

// Time of the camera move of the ending when it starts at once.
constexpr float kOutroNow = -1.0f;

// Value of the bars and tracks of PlayerData while no phrase is known.
constexpr int kNone = -1;

// Track argument of LeaveFreestyle() that selects the track the camera of the player shows.
constexpr int kViewedTrack = -1;

// Bar the first section of a song starts receiving power-ups at.
constexpr int kFirstPowerupBar = 4;

// Skill level at which the solo game places power-ups from the start of each section.
constexpr int kHardestSkillLevel = 3;

} // namespace

int GameLogic::GetControllerArgument(DataArray *pCommand) {
    int nController = -1;
    pCommand->FindData(kControllerKey, &nController, true);
    return nController;
}

void GameLogic::OnAutocatcherCommand(DataArray *pCommand, void *pUserData) {
    GameLogic *pLogic = static_cast<GameLogic *>(pUserData);
    if (TheCommandScheduler.mTick >= 0) {
        pLogic->DeployAutocatcher(pLogic->mLocalPlayers[GetControllerArgument(pCommand)]);
    }
}

void GameLogic::OnMultiplierCommand(DataArray *pCommand, void *pUserData) {
    GameLogic *pLogic = static_cast<GameLogic *>(pUserData);
    if (TheCommandScheduler.mTick >= 0) {
        pLogic->DeployMultiplier(pLogic->mLocalPlayers[GetControllerArgument(pCommand)]);
    }
}

void GameLogic::OnSlowCommand([[maybe_unused]] DataArray *pCommand, void *pUserData) {
    GameLogic *pLogic = static_cast<GameLogic *>(pUserData);
    if (TheGameDb->mCommunity == GameDb::kCommunityOnline) {
        return;
    }
    GameFx::PlayCheat();
    if (pLogic->mSlowdown) {
        pLogic->StopSlowdown();
    } else {
        pLogic->StartSlowdown();
    }
}

void GameLogic::OnFreestyleCommand(DataArray *pCommand, void *pUserData) {
    GameLogic *pLogic = static_cast<GameLogic *>(pUserData);
    if (TheCommandScheduler.mTick >= 0) {
        pLogic->DeployFreestyle(pLogic->mLocalPlayers[GetControllerArgument(pCommand)]);
    }
}

void GameLogic::OnFreestyleToggleCommand(DataArray *pCommand, void *pUserData) {
    GameLogic *pLogic = static_cast<GameLogic *>(pUserData);
    LocalPlayer *pPlayer = pLogic->mLocalPlayers[GetControllerArgument(pCommand)];
    if (pPlayer->GetTrack() == pLogic->mFreestyleTrack) {
        pLogic->LeaveFreestyle(pPlayer, kViewedTrack);
    } else {
        pLogic->EnterFreestyle(pPlayer, false);
    }
}

void GameLogic::OnBumperCommand(DataArray *pCommand, void *pUserData) {
    GameLogic *pLogic = static_cast<GameLogic *>(pUserData);
    if (TheCommandScheduler.mTick >= 0) {
        pLogic->DeployBumper(pLogic->mLocalPlayers[GetControllerArgument(pCommand)]);
    }
}

void GameLogic::OnCripplerCommand(DataArray *pCommand, void *pUserData) {
    GameLogic *pLogic = static_cast<GameLogic *>(pUserData);
    if (TheCommandScheduler.mTick >= 0) {
        pLogic->DeployCrippler(pLogic->mLocalPlayers[GetControllerArgument(pCommand)]);
    }
}

void GameLogic::OnPowerupCommand(DataArray *pCommand, void *pUserData) {
    GameLogic *pLogic = static_cast<GameLogic *>(pUserData);
    if (TheGameDb->mCommunity == GameDb::kCommunityOnline || g_bPowerupCheat == 0) {
        return;
    }
    const int nController = GetControllerArgument(pCommand);
    const int nPowerup = pCommand->Int(1);
    // The solo game has no other player to bump or cripple.
    if ((nPowerup == GameLogic::kPowerupBumper || nPowerup == GameLogic::kPowerupCrippler) &&
        TheGameDb->mCommunity == GameDb::kCommunitySolo) {
        return;
    }
    GameFx::PlayCheat();
    pLogic->mPlayers[nController]->SetPowerup(nPowerup);
}

GameLogic::PlayerData::PlayerData(Command *pEndFreestyle) : mEndFreestyleCmd(pEndFreestyle) {
    mCapturedTrack = kNone;
    mNextPhraseBar = kNone;
    mLastNextPhraseBar = kNone;
    mMissedBar = kNone;
    mSavedStreak = 0;
}

GameLogic::GameLogic(Song *pSong, DataArray *pConfig, int nSeed)
    : mSong(pSong), mState(kStateIdle), mReserved0c(0), mTicksPerBar(pSong->mBuilder->mTicksPerBar),
      mNumBars(pSong->mNumBars), mPlayMap(pSong->GetPlayMap()), mFreestyleTrack(nullptr),
      mTrackSelector(nullptr), mSpeedRamp(new SpeedRamp(&TheCommandScheduler, mSong->GetSpeed())),
      mPhraseThisBar(false), mQuit(0), mReserved60(0), mSectionStartBar(0),
      mNextSectionBar(pSong->GetSections()->GetSectionEnd(0)), mSection(0),
      mWorldBeat(&TheCommandScheduler, pSong->GetWorldTrack(), mTicksPerBar * mNumBars),
      mSavedState(kStatePlaying), mSlowdown(false), mNextEnableStep(0),
      mBarCmd(NewMemFunCommand(this, &GameLogic::TickBar)),
      mStopSlowdownCmd(NewMemFunCommand(this, &GameLogic::StopSlowdown)), mRand(nSeed),
      mEndTick(0) {
    TheControllerDisplay->Init();
    if (TheGameDb->mTutorial == 0) {
        TheCommandScheduler.AddAtTick(
            NewMemFun1Command(&TheGfxManager, &GfxManager::ShowTrackLabels, true), 0, false);
    }
    if (TheGameDb->mTutorial != 0) {
        int nLoopBars = kDefaultInitialTrackLoop;
        pConfig->FindData(kInitialTrackLoopKey, &nLoopBars, true);
        mPlayMap->AddLoop(0, nLoopBars, 0);
    }

    const int nTracks = mSong->GetNumTracks();
    int nLocalPlayers = 0;
    for (int i = 0; i < TheGameDb->GetNumPlayers(); ++i) {
        if (TheGameDb->IsLocalPlayer(i)) {
            LocalPlayer *pLocal = new LocalPlayer(i, mTicksPerBar, nLocalPlayers++);
            mPlayers.push_back(pLocal);
            mLocalPlayers.push_back(pLocal);
        } else {
            mPlayers.push_back(new RemotePlayer(i, mTicksPerBar));
        }
        mPlayerData.push_back(
            PlayerData(NewMemFun1Command(this, &GameLogic::EndFreestyle, mPlayers[i])));
    }

    int nFreestyleType = 0;
    int nFreestyleInstrument = 0;
    CreateTracks(&nFreestyleType, &nFreestyleInstrument);
    TheMixer->Init(nTracks, nFreestyleType);
    for (int i = 0; i < Song::kNumMixerChannels; ++i) {
        if (mSong->HasLowVolumes(i)) {
            TheMixer->SetVolumes(i, *mSong->GetLowVolumes(i));
        }
    }
    if (mSong->HasFreestyleLowVolumes()) {
        TheMixer->SetInstrumentVolumes(*mSong->GetFreestyleLowVolumes());
    }
    TheSessionLog->Begin(nTracks, mNumBars);

    std::vector<Track *> tracks(mCatchTracks.begin(), mCatchTracks.end());
    mTrackSelector =
        new GameTrackSelector(tracks, mFreestyleTrack, nFreestyleType, nFreestyleInstrument);
    AssignTracks();
    PlacePowerups();

    ScriptFunction::Register(OnAutocatcherCommand, kAutocatcherCommand, this);
    ScriptFunction::Register(OnMultiplierCommand, kMultiplierCommand, this);
    ScriptFunction::Register(OnSlowCommand, kSlowCommand, this);
    ScriptFunction::Register(OnBumperCommand, kBumperCommand, this);
    ScriptFunction::Register(OnCripplerCommand, kCripplerCommand, this);
    ScriptFunction::Register(OnFreestyleCommand, kFreestyleCommand, this);
    ScriptFunction::Register(OnFreestyleToggleCommand, kFreestyleToggleCommand, this);
    ScriptFunction::Register(OnPowerupCommand, kPowerupCommand, this);
}

void GameLogic::AssignTracks() {
    if (TheGameDb->mCommunity == GameDb::kCommunitySolo) {
        mTrackSelector->AddPlayer(mPlayers[0], mSong->GetEnableOrder()[0][0]);
        return;
    }

    std::vector<int> order(mPlayers.size(), 0);
    if (TheGameDb->mCommunity == GameDb::kCommunityOnline) {
        // An online session seats the players in the order of the session.
        std::map<int, int> byNetOrder;
        for (int i = 0; i < TheGameDb->GetNumPlayers(); ++i) {
            byNetOrder[TheGameDb->GetPlayerNetOrder(i)] = i;
        }
        int nSeat = 0;
        for (const auto &entry : byNetOrder) {
            order[nSeat++] = entry.second;
        }
    } else {
        for (size_t i = 0; i < order.size(); ++i) {
            order[i] = static_cast<int>(i);
        }
    }

    std::vector<int> freeTracks(mCatchTracks.size(), 0);
    std::vector<int> playerTracks(mPlayers.size(), -1);
    for (size_t i = 0; i < freeTracks.size(); ++i) {
        freeTracks[i] = static_cast<int>(i);
    }
    for (size_t i = 0; i < order.size(); ++i) {
        const int nPick = mRand.Int(0, static_cast<int>(freeTracks.size()));
        playerTracks[order[i]] = freeTracks[nPick];
        freeTracks.erase(freeTracks.begin() + nPick);
    }
    for (size_t i = 0; i < mPlayers.size(); ++i) {
        mTrackSelector->AddPlayer(mPlayers[i], playerTracks[i]);
    }
}

void GameLogic::ApplyFreestyleEffect(int nSet) {
    mSong->GetFreestyleFx()->ApplySet(nSet);
}

GameLogic::~GameLogic() {
    // The destructor calls this class's Stop() directly, not through the vtable.
    GameLogic::Stop();
    TheControllerDisplay->Clear();
    ScriptFunction::Unregister(OnAutocatcherCommand);
    ScriptFunction::Unregister(OnMultiplierCommand);
    ScriptFunction::Unregister(OnSlowCommand);
    ScriptFunction::Unregister(OnBumperCommand);
    ScriptFunction::Unregister(OnCripplerCommand);
    ScriptFunction::Unregister(OnFreestyleCommand);
    ScriptFunction::Unregister(OnFreestyleToggleCommand);
    ScriptFunction::Unregister(OnPowerupCommand);
    delete mSpeedRamp;
    for (CatchTrack *pTrack : mCatchTracks) {
        delete pTrack;
    }
    delete mFreestyleTrack;
    for (Player *pPlayer : mPlayers) {
        delete pPlayer;
    }
    delete mTrackSelector;
    mPlayMap->ClearLoops();
}

float GameLogic::GetProgress() {
    const float fProgress =
        static_cast<float>(GetTick()) / static_cast<float>(mNumBars * mTicksPerBar);
    if (fProgress < 0.0f) {
        return 0.0f;
    }
    if (1.0f < fProgress) {
        return 1.0f;
    }
    return fProgress;
}

void GameLogic::Start() {
    mState = kStatePlaying;
    SectionList *pSections = mSong->GetSections();
    TheGfxManager.AddCheckpoint(0, 0.0f, 0.0f, kCheckpointScale);
    for (int i = 1; i < pSections->GetNumSections(); ++i) {
        TheGfxManager.AddCheckpoint(
            0, static_cast<float>(pSections->GetSectionStart(i) * mTicksPerBar), 0.0f,
            kCheckpointScale);
    }
    if (TheGameDb->mTutorial == 0) {
        TheGfxManager.AddCheckpoint(
            0, static_cast<float>(mNumBars * mTicksPerBar), 0.0f, kCheckpointScale);
    }

    for (int i = 0; i < mSong->GetNumBackMusic(); ++i) {
        const int nMaster = mSong->GetBackMusicMaster(i);
        if (nMaster == -1) {
            mSong->GetBackMusic(i)->Start(mPlayMap);
        } else {
            mCatchTracks[nMaster]->AddBackMusic(mSong->GetBackMusic(i));
        }
    }
    for (CatchTrack *pTrack : mCatchTracks) {
        pTrack->Start();
    }
    mFreestyleTrack->Start();
    for (Player *pPlayer : mPlayers) {
        pPlayer->HidePowerup();
    }
    mWorldBeat.Start();
    mSong->GetLyric()->Start();
    TheMixer->Activate();
    TheHelpText->Reset();
    mSong->GetBankTrack()->Start();
    for (int i = 0; i < mSong->GetNumIntroMuses(); ++i) {
        mSong->GetIntroMuse(i)->Play(&TheCommandScheduler);
    }

    if (TheGameDb->GetDemo() == 0) {
        const int nFirstBeat = mTicksPerBar - mTicksPerBar * mSong->mIntroBars;
        TheForceFeedbackMgr->Start(mSong->GetMsPerTick(), mTicksPerBar);
        TheForceFeedbackMgr->StartMetronome(kForceFeedbackBeatTicks, nFirstBeat);
    }
    TheCommandScheduler.AddAtTick(mBarCmd.Get(), 0, false);
    OnStart();
    TheCommandScheduler.AddAtTick(NewMemFunCommand(&TheMetagame, &Metagame::ShowBlankScreen),
                                  kBlankScreenTick, false);

    FreestyleFx *pFx = mSong->GetFreestyleFx();
    if (pFx != nullptr) {
        pFx->Activate();
        for (int i = 0; i < pFx->GetNumSets(); ++i) {
            const int nBar = pFx->GetSetBar(i);
            TheCommandScheduler.AddAtTick(
                NewMemFun1Command(this, &GameLogic::ApplyFreestyleEffect, i), nBar * mTicksPerBar,
                false);
        }
    }

    const int nCommunity = TheGameDb->mCommunity;
    if ((nCommunity == GameDb::kCommunityOnline || nCommunity == GameDb::kCommunitySolo) &&
        TheGameDb->mTutorial == 0) {
        bool bOption;
        if (nCommunity == GameDb::kCommunitySolo) {
            bOption = TheGameDb->GetProfile(mLocalPlayers[0]->GetIndex())->GetSoloOption() == 0;
        } else {
            bOption = TheGameDb->GetProfile(mLocalPlayers[0]->GetIndex())->GetOnlineOption() == 0;
        }
        TheGfxManager.SetOption(bOption);
        TheControllerDisplay->SetOption(bOption);
    }
    ScheduleControllerCheck();
}

void GameLogic::Stop() {
    mState = kStateFinished;
    FreestyleFx *pFx = mSong->GetFreestyleFx();
    if (pFx != nullptr) {
        pFx->Reset();
    }
    for (int i = 0; i < mSong->GetNumBackMusic(); ++i) {
        mSong->GetBackMusic(i)->Stop();
    }
    for (int i = 0; i < mSong->GetNumIntroMuses(); ++i) {
        mSong->GetIntroMuse(i)->Stop();
    }
    for (CatchTrack *pTrack : mCatchTracks) {
        pTrack->Stop();
    }
    mFreestyleTrack->Stop();
    for (Player *pPlayer : mPlayers) {
        pPlayer->CancelMultiplier();
    }
    mWorldBeat.Stop();
    mSong->GetLyric()->Stop();
    TheMixer->Deactivate();
    TheSessionLog->End();
    TheForceFeedbackMgr->StopAll();
    TheCommandScheduler.Remove(mBarCmd.Get());
}

void GameLogic::SetPaused(bool bPaused, int nPad, int nReason) {
    if (bPaused) {
        TheForceFeedbackMgr->Pause();
        const int nState = mState;
        mState = kStatePaused;
        mSavedState = nState;
        for (int i = 0; i < TheGameDb->GetNumPads(); ++i) {
            JoypadSetMenuControl(i, true);
        }
        SystemSetPadCheck(true);
    } else {
        TheForceFeedbackMgr->Resume();
        mState = mSavedState;
        if (TheGameDb->mCommunity == GameDb::kCommunitySolo) {
            TheGfxManager.SetFreqSize(0, TheGameDb->GetOptions()->mFreqSize);
        }
        for (int i = 0; i < TheGameDb->GetNumPads(); ++i) {
            JoypadSetMenuControl(i, false);
        }
        SystemSetPadCheck(false);
    }
    WorldLogic::SetPaused(bPaused, nPad, nReason);
}

void GameLogic::Poll() {
    if (mState != kStateEnding) {
        return;
    }
    if (!TheGfxManager.IsOutroDone()) {
        return;
    }
    if (TheCommandScheduler.IsRunning() && TheCommandScheduler.mTick < mEndTick) {
        return;
    }
    Stop();
}

void GameLogic::EndSong() {
    TheSessionLog->End();
    GameFx::StopLoop();
    float fOutroMs = 0.0f;
    if (mQuit == 0) {
        const int nCommunity = TheGameDb->mCommunity;
        int nSkipOutro = 0;
        if (nCommunity == GameDb::kCommunitySolo && TheGameDb->mReserved88 == 0) {
            const char *pszNextSong =
                TheGameDb->GetProfile(0)->FindFirstUnfinishedSong(TheGameDb->mSkillLevel);
            if (pszNextSong != nullptr) {
                SongEntry entry{TheGameDb->FindSong(pszNextSong)};
                const int nType = entry.GetType();
                if (nType == nCommunity) {
                    nSkipOutro = mReserved60 != 0 ? nType : 0;
                }
            }
        }
        if (nSkipOutro == 0) {
            fOutroMs = TheGfxManager.StartOutro(kOutroNow);
        }
    }

    if (mState != kStatePaused && mState != kStateSuspended) {
        const float *pfMsPerTick = mSong->GetMsPerTick();
        const float fFadeMs = kMinimumEndingFadeMs < fOutroMs ? fOutroMs : kMinimumEndingFadeMs;
        TheMixer->FadeOut(static_cast<int>(fFadeMs / *pfMsPerTick));
        mEndTick = static_cast<int>((TheCommandScheduler.mTime + fFadeMs + kEndingTailMs) /
                                    *pfMsPerTick);
        TheCommandScheduler.AddAtTick(
            NewMemFunCommand<GameLogic>(this, &WorldLogic::AllNotesOff), mEndTick - 1, false);
    }
    mState = kStateEnding;
}

void GameLogic::ShowPowerupText(const char *pszToken, [[maybe_unused]] int nPlayer) {
    if (TheGameDb->mCommunity != GameDb::kCommunitySolo) {
        return;
    }
    TheGfxManager.ShowMessage(
        TheLocale.Localize(pszToken, true), nullptr, -1, kPowerupTextMs, 1.0f, 0.0f, 0.0f);
}

bool GameLogic::IsFinished() const {
    return mState == kStateFinished;
}

int GameLogic::HasQuit() const {
    return mQuit;
}

int GameLogic::GetReserved60() const {
    return mReserved60;
}

bool GameLogic::IsFreestyling(int nPlayer) {
    return mPlayers[nPlayer]->GetTrack() == mFreestyleTrack;
}

bool GameLogic::IsFreestyleTrackActive() const {
    if (mFreestyleTrack->mPlayer == nullptr) {
        return false;
    }
    return mState == kStatePlaying || mState == kStateRunning;
}

bool GameLogic::HasPhraseThisBar() const {
    return mPhraseThisBar;
}

void GameLogic::CreateTracks(int *pFreestyleType, int *pFreestyleInstrument) {
    for (int i = 0; i < mSong->GetNumTracks(); ++i) {
        const int nType = mSong->GetTrackType(i);
        switch (nType) {
        case Song::kTrackTypeCatch: {
            void *pBlock = PoolMemAlloc(sizeof(CatchTrack), kCatchTrackTag, 0);
            CatchTrackData *pData = mSong->GetCatchTrackData(i);
            const float *pfMsPerTick = mSong->GetMsPerTick();
            SectionList *pSections = mSong->GetSections();
            const int nIndex = static_cast<int>(mCatchTracks.size());
            const int nIntroBars = mSong->mIntroBars;
            const int nFlags = mSong->GetTrackFlags(i);
            mCatchTracks.push_back(new (pBlock) CatchTrack(this,
                                                           pData,
                                                           pfMsPerTick,
                                                           mPlayMap,
                                                           pSections,
                                                           nIndex,
                                                           nIntroBars,
                                                           mNumBars,
                                                           mTicksPerBar,
                                                           nFlags));
            break;
        }
        case Song::kTrackTypeAxe: {
            *pFreestyleType = nType;
            *pFreestyleInstrument = mSong->GetTrackInstrument(i);
            void *pBlock = PoolMemAlloc(sizeof(AxeTrack), kAxeTrackTag, 0);
            AxeContour *pContour = mSong->GetAxeContour(i);
            SectionList *pSections = mSong->GetSections();
            PlayMap *pPlayMap = mSong->GetPlayMap();
            const float *pfMsPerTick = mSong->GetMsPerTick();
            mFreestyleTrack = new (pBlock) AxeTrack(pContour,
                                                    pSections,
                                                    pPlayMap,
                                                    pfMsPerTick,
                                                    i,
                                                    mSong->mIntroBars,
                                                    mNumBars,
                                                    mTicksPerBar);
            break;
        }
        case Song::kTrackTypeScratch: {
            *pFreestyleType = nType;
            *pFreestyleInstrument = mSong->GetTrackInstrument(i);
            void *pBlock = PoolMemAlloc(sizeof(ScratchTrack), kScratchTrackTag, 0);
            ScratchData *pData = mSong->GetScratchData(i);
            SectionList *pSections = mSong->GetSections();
            PlayMap *pPlayMap = mSong->GetPlayMap();
            const float *pfMsPerTick = mSong->GetMsPerTick();
            mFreestyleTrack = new (pBlock) ScratchTrack(pData,
                                                        pSections,
                                                        pPlayMap,
                                                        pfMsPerTick,
                                                        i,
                                                        mSong->mIntroBars,
                                                        mNumBars,
                                                        mTicksPerBar);
            break;
        }
        default:
            DebugWarn("unknown track type %i for track %i", nType, i);
            break;
        }
    }
}

void GameLogic::Finish(bool bWon) {
    TheHelpText->SetEnabled(false);
    for (int i = 0; i < TheGameDb->GetNumPads(); ++i) {
        JoypadSetMenuControl(i, true);
    }
    for (int i = 0; i < TheGameDb->GetNumPlayers(); ++i) {
        TheCommandScheduler.Remove(mPlayerData[i].mEndFreestyleCmd.Get());
        TheGfxManager.SetPendingPointsResult(i, GfxManager::kPendingPointsCleared);
        TheGfxManager.HidePendingPoints(i);
        if (mPlayers[i]->GetTrack() == mFreestyleTrack) {
            LeaveFreestyle(mPlayers[i], kViewedTrack);
        }
    }
    StopSlowdown();
    TheForceFeedbackMgr->StopAll();
    TheGameDb->SetWon(bWon);

    bool bUnlocked = false;
    if (TheGameDb->mCommunity == GameDb::kCommunitySolo) {
        const int nGroup = TheGameDb->FindSongGroup();
        std::vector<bool> cleared;
        TheGameDb->GetProfile(0)->GetClearedSongs(TheGameDb->mSkillLevel, nGroup, &cleared);
        const int nSong = TheGameDb->GetSongIndex();
        SongEntry entry{TheGameDb->FindSong(TheGameDb->mSong.c_str())};
        if (bWon && nSong >= 0 && !cleared[nSong] && TheGameDb->mPracticeMode == 0 &&
            TheGameDb->mLoadRemix == 0) {
            bUnlocked = entry.GetType() == 0;
        }
    }
    TheGfxManager.ShowResult(bWon, 0, bUnlocked);
    if (bWon) {
        EnterFreestyle(mPlayers[0], true);
    }
    for (size_t i = 0; i < mPlayers.size(); ++i) {
        TheGameDb->SetPlayerScore(static_cast<int>(i), mPlayers[i]->GetScore());
    }
    if (TheGameDb->GetDemo() != 0) {
        EndSong();
    } else {
        OnFinish(bWon);
    }
}

void GameLogic::TickBar() {
    const int nBar = TheCommandScheduler.mTick / mTicksPerBar;
    if (nBar == mNextSectionBar) {
        AdvanceSection();
    }
    TheTriggerMgr.NewBarEvent(nBar);
    if (mState == kStatePlaying || mState == kStatePaused) {
        TheCommandScheduler.AddAfterTicks(mBarCmd.Get(), mTicksPerBar, false);
    }
    mPhraseThisBar = false;
    if (nBar < mNumBars) {
        for (CatchTrack *pTrack : mCatchTracks) {
            if (pTrack->HasPhraseAt(nBar)) {
                mPhraseThisBar = true;
                break;
            }
        }
    }
    OnBar(nBar);
}

void GameLogic::AdvanceSection() {
    SectionList *pSections = mSong->GetSections();
    const int nTick = TheCommandScheduler.mTick;
    const int nBar = nTick / mTicksPerBar;
    ++mSection;
    mSectionStartBar = nBar;
    if (!pSections->IsPastEnd(nBar)) {
        mNextSectionBar = pSections->GetSectionEnd(mSection);
    }
    TheSessionLog->LogSectionEnd(nTick);
    OnSection();
}

void GameLogic::CapturePhrase(CatchTrack *pTrack, Player *pPlayer, int nBar, bool bStreak) {
    const int nTrack = pTrack->mIndex;
    int nEndTick = mCatchTracks[nTrack]->GetNextGemTick(nBar * mTicksPerBar, nullptr);
    if (nEndTick == -1) {
        nEndTick = mNumBars * mTicksPerBar;
    }
    TheMixer->HoldTrack(pTrack->mIndex, nEndTick);
    if (TheGameConfig->mStreaksEnabled) {
        const int nPlayer = pPlayer->GetIndex();
        for (LocalPlayer *pLocal : mLocalPlayers) {
            const int nOther = pLocal->GetIndex();
            if (nOther == nPlayer) {
                continue;
            }
            const int nNextBar = mPlayerData[nOther].mNextPhraseBar;
            if (nNextBar == kNone) {
                continue;
            }
            FindNextPhrase(nOther, pLocal->GetTrack()->mIndex, nNextBar);
        }

        if (bStreak) {
            PlayerData &data = mPlayerData[nPlayer];
            const int nNowBar = TheCommandScheduler.mTick / mTicksPerBar;
            if (data.mNextPhraseBar == kNone) {
                // A phrase missed in this bar is forgiven when the capture resumes the old streak.
                if (nNowBar == data.mMissedBar && data.mLastNextPhraseBar >= nNowBar) {
                    pPlayer->SetStreak(data.mSavedStreak);
                    if (nNowBar < data.mLastNextPhraseBar) {
                        FindNextPhrase(nPlayer, nTrack, data.mLastNextPhraseBar);
                    } else {
                        FindNextPhrase(nPlayer, nTrack, data.mLastNextPhraseBar + 1);
                    }
                }
            } else if (nNowBar < data.mNextPhraseBar - 1) {
                FindNextPhrase(nPlayer, nTrack, data.mNextPhraseBar);
            } else {
                FindNextPhrase(nPlayer, nTrack, data.mNextPhraseBar + 1);
            }
        }
    }
    OnPhraseCaptured(pTrack, pPlayer, nBar, bStreak);
}

void GameLogic::ContinueStreak(Track *pTrack, int nBar, int nFromBar) {
    if (!TheGameConfig->mStreaksEnabled) {
        return;
    }
    Player *pPlayer = pTrack->mPlayer;
    const int nPlayer = pPlayer->GetIndex();
    const int nTrack = pTrack->mIndex;
    if (mPlayerData[nPlayer].mNextPhraseBar < nBar) {
        pPlayer->ResetStreak();
    } else {
        pPlayer->IncrementStreak();
    }
    FindNextPhrase(nPlayer, nTrack, nFromBar);
}

void GameLogic::ClearNextPhrase(int nPlayer) {
    PlayerData &data = mPlayerData[nPlayer];
    const int nStreak = mPlayers[nPlayer]->GetStreak();
    const int nNextBar = data.mNextPhraseBar;
    data.mCapturedTrack = kNone;
    data.mSavedStreak = nStreak;
    data.mLastNextPhraseBar = nNextBar;
    data.mNextPhraseBar = kNone;
    if (TheGameDb->IsLocalPlayer(nPlayer)) {
        TheGfxManager.HideNextPhrase(0); // Yes, the binary passes player 0 for every player.
    }
}

void GameLogic::FindNextPhrase(int nPlayer, int nTrack, int nFromBar) {
    PlayerData &data = mPlayerData[nPlayer];
    const bool bSinglePad = TheGameDb->GetNumPads() == 1;
    ClearNextPhrase(nPlayer);
    const int nEndBar = mPlayMap->GetEndBar();
    bool bFound = false;
    for (int nBar = nFromBar; nBar < nEndBar && !bFound; ++nBar) {
        for (size_t i = 0; i < mCatchTracks.size(); ++i) {
            if (static_cast<int>(i) == nTrack) {
                continue;
            }
            if (mCatchTracks[i]->HasPhraseAt(nBar)) {
                data.mNextPhraseBar = nBar;
                bFound = true;
                data.mCapturedTrack = nTrack;
                break;
            }
        }
    }
    if (!bFound || !bSinglePad) {
        return;
    }
    for (size_t i = 0; i < mCatchTracks.size(); ++i) {
        if (static_cast<int>(i) == nTrack) {
            continue;
        }
        const int nBar = data.mNextPhraseBar;
        if (!mCatchTracks[i]->HasPhraseAt(nBar)) {
            continue;
        }
        int nGemType = 0;
        const int nTick = mCatchTracks[i]->GetNextGemTick(nBar * mTicksPerBar, &nGemType);
        TheGfxManager.ShowNextPhrase(static_cast<int>(i), static_cast<float>(nTick), nGemType);
    }
}

void GameLogic::PlaceRandomPowerups() {
    // Only the local multiplayer game draws bumpers and cripplers.
    const int nKinds = TheGameDb->mCommunity == GameDb::kCommunityLocal ? kPowerupCount :
                                                                          kPowerupBumper;
    for (CatchTrack *pTrack : mCatchTracks) {
        for (int nBar = 0; nBar < mNumBars; ++nBar) {
            pTrack->SetPowerup(nBar, RandomInt(kPowerupAutocatcher, nKinds));
        }
    }
}

void GameLogic::PlacePowerups() {
    if (TheGameDb->mTutorial != 0 || TheGameDb->mPracticeMode != 0) {
        return;
    }
    if (g_bPowerupsAPlenty != 0 && TheGameDb->mCommunity != GameDb::kCommunityOnline) {
        PlaceRandomPowerups();
        return;
    }

    float fDensity;
    if (TheGameDb->mCommunity == GameDb::kCommunitySolo) {
        fDensity = TheGameConfig->mPowerupProbSolo;
    } else {
        if (TheGameDb->mPowerupLevel == GameDb::kPowerupLevelNone) {
            return;
        }
        fDensity = TheGameConfig->mPowerupProbMulti[TheGameDb->mPowerupLevel];
    }

    std::vector<bool> enabled(kPowerupCount, true);
    if (TheGameDb->mCommunity == GameDb::kCommunitySolo && TheGameDb->GetDemo() == 0) {
        const int nSkillLevel = TheGameDb->mSkillLevel;
        PlayerProfile *pProfile = TheGameDb->GetProfile(0);
        DataArray *pDist =
            SystemConfig()->FindArray(kGameKey, true)->FindArray(kPowerupDistKey, true);
        enabled[kPowerupAutocatcher] =
            pProfile->IsUnlocked(pDist->FindArray(kAutocatcherKey, true)->Sym(0), nSkillLevel);
        enabled[kPowerupMultiplier] =
            pProfile->IsUnlocked(pDist->FindArray(kMultiplierKey, true)->Sym(0), nSkillLevel);
        enabled[kPowerupSlowdown] =
            pProfile->IsUnlocked(pDist->FindArray(kSlowdownKey, true)->Sym(0), nSkillLevel);
        enabled[kPowerupFreestyle] =
            pProfile->IsUnlocked(pDist->FindArray(kFreestyleKey, true)->Sym(0), nSkillLevel);
    }

    // A bar receives a power-up when the rate of its gems reaches the median rate of every bar
    // of every catch track.
    StatsAccumulator stats;
    for (size_t nTrack = 0; nTrack < mCatchTracks.size(); ++nTrack) {
        const CatchTrackData *pData = mSong->GetCatchTrackData(static_cast<int>(nTrack));
        const int nGems = pData->GetNumGems();
        RateAverager rate(mTicksPerBar);
        int nLastBar = -1;
        for (int i = 0; i < nGems; ++i) {
            const CatchTrackData::Gem *pGem = pData->GetGem(i);
            const int nBar = pGem->mTick / mTicksPerBar;
            if (nBar != nLastBar) {
                if (rate.mSum != 0.0f) {
                    stats.AddSample(rate.GetMean());
                }
                rate.Reset();
            }
            rate.Sample(pGem->mLane, pGem->mTick);
            nLastBar = nBar;
        }
    }
    const float fThreshold = stats.GetMedian();

    SectionList *pSections = mSong->GetSections();
    const int nSections = pSections->GetNumSections();
    for (int nSection = 0; nSection < nSections; ++nSection) {
        const int nTracks = static_cast<int>(mCatchTracks.size());
        const int nEndBar = pSections->GetSectionEnd(nSection);
        int nStartBar = pSections->GetSectionStart(nSection);
        if (nSection == 0) {
            nStartBar = kFirstPowerupBar;
        } else {
            int nLeadIn = 0;
            if (TheGameDb->mSkillLevel != kHardestSkillLevel ||
                TheGameDb->mCommunity != GameDb::kCommunitySolo) {
                nLeadIn = TheGameConfig->mCheckpointBars;
            }
            nStartBar += nLeadIn;
        }

        std::vector<std::pair<int, int>> candidates;
        int nRatedBars = 0;
        candidates.reserve(nTracks * (nEndBar - nStartBar) / 2);
        for (int nTrack = 0; nTrack < nTracks; ++nTrack) {
            int nLastBar = -1;
            const CatchTrackData *pData = mSong->GetCatchTrackData(nTrack);
            const int nGems = pData->GetNumGems();
            RateAverager rate(mTicksPerBar);
            for (int i = 0; i < nGems; ++i) {
                const CatchTrackData::Gem *pGem = pData->GetGem(i);
                const int nBar = pGem->mTick / mTicksPerBar;
                if (nBar < nStartBar) {
                    continue;
                }
                if (nBar >= nEndBar) {
                    break;
                }
                if (nBar != nLastBar) {
                    if (rate.mSum != 0.0f) {
                        ++nRatedBars;
                        if (fThreshold <= rate.GetMean()) {
                            candidates.push_back(std::pair<int, int>(nTrack, nLastBar));
                        }
                    }
                    rate.Reset();
                }
                rate.Sample(pGem->mLane, pGem->mTick);
                nLastBar = nBar;
            }
        }

        std::vector<float> weights(kPowerupCount, -1.0f);
        float fTotal = 0.0f;
        for (int nKind = 0; nKind < kPowerupCount; ++nKind) {
            if (nKind == kPowerupNone) {
                weights[kPowerupNone] = 0.0f;
            } else if (nKind == kPowerupSlowdown &&
                       TheGameDb->mCommunity == GameDb::kCommunityOnline) {
                weights[kPowerupSlowdown] = 0.0f;
            } else {
                weights[nKind] = TheGameConfig->GetPowerupWeight(
                    TheGameDb->GetNumPlayers(), nSection, nSections, nKind);
            }
            fTotal += weights[nKind];
        }
        const float fScale = 1.0 / fTotal;
        for (float &fWeight : weights) {
            fWeight *= fScale;
        }

        for (int nKind = kPowerupAutocatcher; nKind < kPowerupCount; ++nKind) {
            if (!enabled[nKind]) {
                continue;
            }
            if (candidates.empty()) {
                break;
            }
            const int nCount =
                static_cast<int>(weights[nKind] * static_cast<float>(nRatedBars) * fDensity);
            for (int i = 0; i < nCount; ++i) {
                auto it = candidates.begin() + mRand.Int(0, static_cast<int>(candidates.size()));
                mCatchTracks[it->first]->SetPowerup(it->second, nKind);
                // The bars on either side of a power-up on the same track receive none.
                if (it + 1 != candidates.end() && (it + 1)->first == it->first &&
                    (it + 1)->second == it->second + 1) {
                    candidates.erase(it + 1);
                }
                if (it != candidates.begin() && (it - 1)->first == it->first &&
                    (it - 1)->second == it->second - 1) {
                    it = candidates.erase(it - 1);
                }
                candidates.erase(it);
                if (candidates.empty()) {
                    break;
                }
            }
        }
    }
}

void GameLogic::EndPhrase(Track *pTrack) {
    TheTriggerMgr.PhraseEndEvent(pTrack->mIndex);
    OnPhraseEnded(pTrack);
}

void GameLogic::MissPhrase(Track *pTrack) {
    Player *pPlayer = pTrack->mPlayer;
    if (pPlayer != nullptr) {
        const int nPlayer = pPlayer->GetIndex();
        TheTriggerMgr.PhraseMissEvent(nPlayer);
        if (TheGameConfig->mStreaksEnabled) {
            PlayerData &data = mPlayerData[nPlayer];
            if (TheCommandScheduler.mTick / mTicksPerBar >= data.mNextPhraseBar ||
                data.mCapturedTrack == pTrack->mIndex) {
                ClearNextPhrase(nPlayer);
                data.mMissedBar = TheCommandScheduler.mTick / mTicksPerBar;
                pPlayer->ResetStreak();
            }
        }
    }
    OnPhraseMissed(pTrack);
}

void GameLogic::HandleInput(const RotateEvent &event) {
    if (mState != kStatePlaying) {
        return;
    }
    const int nPlayer = event.mPlayer;
    if (mPlayers[nPlayer]->GetTrack() == mFreestyleTrack) {
        return;
    }
    if (event.mDirection == RotateEvent::kDirectionNext) {
        mTrackSelector->RotateNext(nPlayer);
    } else {
        mTrackSelector->RotatePrevious(nPlayer);
    }
}

void GameLogic::HandleInput(const PlayNoteEvent &event) {
    if (mState == kStatePlaying || mState == kStateRunning) {
        mPlayers[event.mPlayer]->HandleInput(event);
    }
}

void GameLogic::HandleInput(const BtnEvent<10> &event) {
    if (mState == kStatePlaying || mState == kStateRunning) {
        mPlayers[event.mPlayer]->HandleInput(event);
    }
}

void GameLogic::HandleInput(const StickEvent<2> &event) {
    if (mState == kStatePlaying || mState == kStateRunning) {
        mPlayers[event.mPlayer]->HandleInput(event);
    }
}

void GameLogic::HandleInput(const StickEvent<6> &event) {
    if (mState == kStatePlaying || mState == kStateRunning) {
        mPlayers[event.mPlayer]->HandleInput(event);
    }
}

void GameLogic::HandleInput(const BtnEvent<3> &event) {
    if (mState == kStatePlaying) {
        SetPaused(true, TheGameDb->GetPlayerPad(event.mPlayer), 0);
    }
}

void GameLogic::HandleInput([[maybe_unused]] const BtnEvent<4> &event) {
    if (mState != kStatePlaying) {
        return;
    }
    if (TheGameDb->GetNumPads() != 1) {
        return;
    }
    const bool bOption = TheGfxManager.GetOption();
    TheGfxManager.SetOption(!bOption);
    if (TheGameDb->GetDemo() != 0) {
        return;
    }
    if (TheGameDb->mCommunity == GameDb::kCommunitySolo) {
        TheGameDb->GetProfile(0)->SetSoloOption(bOption);
    } else if (TheGameDb->mCommunity == GameDb::kCommunityOnline) {
        TheGameDb->GetProfile(0)->SetOnlineOption(bOption);
    }
    TheControllerDisplay->SetOption(!bOption);
}

void GameLogic::HandleInput(const BtnEvent<5> &event) {
    if (mState != kStatePlaying || TheCommandScheduler.mTick < 0) {
        return;
    }
    Player *pPlayer = mPlayers[event.mPlayer];
    const int nPowerup = pPlayer->GetPowerup();
    if (pPlayer->GetTrack() == mFreestyleTrack) {
        return;
    }
    const int nTick = TheCommandScheduler.mTick;
    bool bDeployed = false;
    switch (nPowerup) {
    case kPowerupNone:
        return;
    case kPowerupAutocatcher:
        bDeployed = DeployAutocatcher(pPlayer);
        break;
    case kPowerupMultiplier:
        bDeployed = DeployMultiplier(pPlayer);
        break;
    case kPowerupSlowdown:
        bDeployed = DeploySlowdown(pPlayer);
        break;
    case kPowerupFreestyle:
        bDeployed = DeployFreestyle(pPlayer);
        break;
    case kPowerupBumper:
        bDeployed = DeployBumper(pPlayer);
        break;
    case kPowerupCrippler:
        bDeployed = DeployCrippler(pPlayer);
        break;
    default:
        DebugWarn("unknown powerup");
        break;
    }
    if (!bDeployed) {
        return;
    }
    TheSessionLog->LogPowerup(pPlayer->GetIndex(), nTick, nPowerup);
    GameFx::PlayPowerup(nPowerup);
    pPlayer->SetPowerup(kPowerupNone);
}

void GameLogic::EnableNextTracks(int nBar) {
    const std::vector<std::vector<int>> &order = mSong->GetEnableOrder();
    if (static_cast<size_t>(mNextEnableStep) >= order.size()) {
        return;
    }
    for (int nTrack : order[mNextEnableStep]) {
        mCatchTracks[nTrack]->Enable(nBar);
        TheGfxManager.SetTrackEnabled(nTrack, true);
    }
    ++mNextEnableStep;
}

void GameLogic::EnterFreestyle(Player *pPlayer, bool bVictory) {
    mTrackSelector->EnterFreestyle(pPlayer->GetIndex(), bVictory);
}

void GameLogic::LeaveFreestyle(Player *pPlayer, int nTrack) {
    if (pPlayer->GetTrack() != mFreestyleTrack) {
        return;
    }
    const int nPlayer = pPlayer->GetIndex();
    if (nTrack == kViewedTrack) {
        nTrack = TheGfxManager.GetViewedTrack(nPlayer);
    }
    mTrackSelector->MovePlayer(nPlayer, nTrack);
    TheCommandScheduler.Remove(mPlayerData[nPlayer].mEndFreestyleCmd.Get());
    if (pPlayer->GetStreak() > 0) {
        FindNextPhrase(nPlayer, kNone, TheCommandScheduler.mTick / mTicksPerBar);
    }
}

void GameLogic::StartSlowdown() {
    const float fSlowdownSpeed = TheGameConfig->mSlowdownSpeed;
    mSpeedRamp->MoveTo(TheGameConfig->mSlowdownStartTicks,
                       kSlowdownStepTicks,
                       fSlowdownSpeed * mSong->GetSpeed());
    mSlowdown = true;
}

void GameLogic::StopSlowdown() {
    if (!mSlowdown) {
        return;
    }
    mSpeedRamp->MoveTo(TheGameConfig->mSlowdownStopTicks, kSlowdownStepTicks, mSong->GetSpeed());
    mSlowdown = false;
}

bool GameLogic::DeploySlowdown(Player *pPlayer) {
    ShowPowerupText(kSlowdownText, pPlayer->GetIndex());
    const int nNow = TheCommandScheduler.mTick;
    const int nEndTick = nNow + TheGameConfig->mSlowdownDurationBars * mTicksPerBar;
    TheGfxManager.ShowSlowdown(pPlayer->GetIndex(),
                               static_cast<float>(nNow + TheGameConfig->mSlowdownStartTicks),
                               static_cast<float>(nEndTick),
                               static_cast<float>(nEndTick + TheGameConfig->mSlowdownStopTicks));
    StartSlowdown();
    TheCommandScheduler.Remove(mStopSlowdownCmd.Get());
    TheCommandScheduler.AddAtTick(mStopSlowdownCmd.Get(), nEndTick, false);
    return true;
}

bool GameLogic::DeployAutocatcher(Player *pPlayer) {
    Track *pTrack = pPlayer->GetTrack();
    const int nBar = TheCommandScheduler.mTick / mTicksPerBar;
    if (!mCatchTracks[pTrack->mIndex]->Autocatch(nBar, pPlayer)) {
        TheHelpText->ShowAutocatcherFailed();
        return false;
    }
    ShowPowerupText(kAutocatcherText, pPlayer->GetIndex());
    const int nPad = TheGameDb->GetPlayerPad(pPlayer->GetIndex());
    if (nPad != -1) {
        TheForceFeedbackMgr->PlayAutocatchEffect(nPad);
    }
    Track *pPlayerTrack = pPlayer->GetTrack();
    for (Player *pOther : mPlayers) {
        if (pOther == pPlayer || pOther->GetTrack() != pPlayerTrack) {
            continue;
        }
        const int nOtherPad = TheGameDb->GetPlayerPad(pOther->GetIndex());
        if (nOtherPad != -1) {
            TheForceFeedbackMgr->PlayAutocatchEffect(nOtherPad);
        }
    }
    return true;
}

bool GameLogic::DeployMultiplier(Player *pPlayer) {
    ShowPowerupText(kMultiplierText, pPlayer->GetIndex());
    pPlayer->ActivateMultiplier();
    return true;
}

bool GameLogic::DeployFreestyle(Player *pPlayer) {
    const int nPlayer = pPlayer->GetIndex();
    ShowPowerupText(kFreestyleStartText, nPlayer);
    EnterFreestyle(pPlayer, false);
    const int nBars = TheGameDb->mCommunity == GameDb::kCommunitySolo ?
                          TheGameConfig->mFreestyleDurationBarsSolo :
                          TheGameConfig->mFreestyleDurationBarsMultiNet;
    TheCommandScheduler.Remove(mPlayerData[nPlayer].mEndFreestyleCmd.Get());
    TheCommandScheduler.AddAfterTicks(
        mPlayerData[nPlayer].mEndFreestyleCmd.Get(), mTicksPerBar * nBars, false);
    if (TheGameDb->mCommunity == GameDb::kCommunitySolo) {
        const int nBar = TheCommandScheduler.mTick / mTicksPerBar;
        for (CatchTrack *pTrack : mCatchTracks) {
            pTrack->Freestyle(nBar, nBars + 1, pPlayer);
        }
    }
    pPlayer->SetStreak(mPlayerData[nPlayer].mSavedStreak);
    TheGfxManager.HideNextPhrase(0);
    return true;
}

void GameLogic::EndFreestyle(Player *pPlayer) {
    LeaveFreestyle(pPlayer, kViewedTrack);
}
