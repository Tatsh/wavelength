#include "game/sologamelogic.h"

#include <algorithm>

#include "game/campaign.h"
#include "game/controllerdisplay.h"
#include "game/gameconfig.h"
#include "game/gamedb.h"
#include "game/helptext.h"
#include "game/mixer.h"
#include "game/sectionboundaries.h"
#include "game/stats.h"
#include "gfx/gfxmanager.h"
#include "os/debug.h"
#include "os/joypad.h"
#include "os/locale.h"
#include "os/memfun1command.h"
#include "os/memfun3command.h"
#include "os/memfuncommand.h"
#include "os/scheduler.h"
#include "os/string.h"
#include "os/system.h"
#include "script/scriptfunction.h"
#include "synth/fxmidi.h"
#include "synth/songspeed.h"
#include "synth/synth.h"

namespace {

constexpr char kGameSection[] = "game";
constexpr char kBankSlotEntry[] = "game_fx_bank_slot";
constexpr char kWinBankFileEntry[] = "win_fx_bank_file";
constexpr char kSoloBankFileEntry[] = "solo_game_fx_bank_file";
constexpr char kJuiceCommand[] = "juice";
constexpr char kWinCheatCommand[] = "win_cheat";
constexpr char kWinSequenceCommand[] = "win_sequence";
constexpr char kEmptyText[] = "";

constexpr char kPracticeModeToken[] = "PRACTICE_MODE";
constexpr char kWonCampaign11Token[] = "WON_CAMPAIGN_1_1";
constexpr char kWonCampaign12Token[] = "WON_CAMPAIGN_1_2";
constexpr char kWonCampaign2Token[] = "WON_CAMPAIGN_2";
constexpr char kWonCampaign3Token[] = "WON_CAMPAIGN_3";
constexpr char kWonCampaign4Token[] = "WON_CAMPAIGN_4";
constexpr char kPressStartToExitToken[] = "PRESS_START_TO_EXIT";
constexpr char kEnergyBonusToken[] = "ENERGY_BONUS";
constexpr char kWonSoloGameToken[] = "WON_SOLO_GAME";
constexpr char kLostSoloGameToken[] = "LOST_SOLO_GAME";
constexpr char kStageCompletedToken[] = "STAGE_COMPLETED";

constexpr int kNoTrack = -1;
constexpr int kNoPad = -1;
constexpr int kAllPlayers = -1;
constexpr int kNoWinner = -1;
constexpr int kTrackOfCamera = -1;
constexpr int kLocalPlayer = 0;
constexpr int kSkillLevelBrutal = 2;
constexpr int kSkillLevelInsane = 3;

constexpr float kMessageOffset = 0.0f;
constexpr float kMessageDurationMs = 3200.0f;
constexpr float kResultScale = 1.5f;
constexpr float kEnergyBonusScale = 1.2f;
constexpr float kClearDurationMs = 1600.0f;
constexpr float kClearScale = 1.0f;
constexpr float kCheckpointTextDurationMs = 2600.0f;
constexpr float kCheckpointTextScale = 1.15f;
constexpr int kCheckpointTextDelayTicks = 1920;

constexpr float kWinSequenceStartDelayMs = 2000.0f;
constexpr float kWinSequenceGapMs = 1000.0f;
constexpr float kWinMessageDurationMs = 6000.0f;
constexpr float kFinalWinMessageDurationMs = 8000.0f;
constexpr float kCampaignScale = 1.0f;
constexpr float kNextDifficultyScale = 0.75f;
constexpr float kFinalCampaignScale = 0.62f;
constexpr float kEndSongDelayMs = 5000.0f;
constexpr float kRetryDelayMs = 100.0f;

constexpr int kLossSlowdownTicks = 960;
constexpr int kLossSlowdownStepTicks = 20;
constexpr float kLossSpeed = 0.1f;
constexpr float kMaxLostProgress = 99.0f;
constexpr float kFullProgress = 1.0f;

constexpr float kNoJuice = 0.0f;
constexpr float kMinJuiceAfterCapture = 1.0f;
constexpr float kJuiceDrainPerBar = 1.0f;
constexpr float kMinEnergyBonus = 1.0f;
constexpr double kLowJuiceFraction = 0.25;
constexpr float kInitialCaptureJuice = -1.0f;
constexpr float kEnableTracksDelayMs = 1.0f;
constexpr int kFreestyleExtraBars = 2;
constexpr int kWinCheatScore = 1500;

constexpr float kIntroArgument = -1.0f;
constexpr float kNormalScrollSpeed = 1.0f;
constexpr float kLoopEndMarginMs = 4000.0f;
constexpr float kRestartDelayMs = 200.0f;
constexpr float kIntroLeadMs = 100.0f;
constexpr float kRestartPollTime = 100.0f;

// Bars at the start of each section that do not count against the player. An insane solo game
// has none.
inline int CheckpointBars() {
    if (TheGameDb->mSkillLevel == kSkillLevelInsane &&
        TheGameDb->mCommunity == GameDb::kCommunitySolo) {
        return 0;
    }
    return TheGameConfig->mCheckpointBars;
}

// Shows the winner movie on alternate periods of the victory lap.
class SwapMovieCmd : public Command {
public:
    explicit SwapMovieCmd(int nPeriod) : mPeriod(nPeriod), mOn(0) {
    }

    // NTSC-U/C: 0x00340df0, PAL: 0x003ae328
    ~SwapMovieCmd() override {
    }

    // NTSC-U/C: 0x00340e68, PAL: 0x003ae3a0
    void Execute() override {
        TheGfxManager.SetWinner(mOn ? kLocalPlayer : kNoWinner);
        mOn ^= 1;
        TheSongScheduler.PostIn(this, mPeriod, false);
    }

    int mPeriod;
    int mOn;
};

} // namespace

void SoloGameLogic::CheckpointTextCmd::Execute() {
    TheGfxManager.ShowMessage(
        FormatString(TheLocale.Localize(kStageCompletedToken, true), mLogic->mSection + 1),
        nullptr,
        kAllPlayers,
        kCheckpointTextDurationMs,
        kCheckpointTextScale,
        kMessageOffset,
        kMessageOffset);
}

SoloGameLogic::SoloGameLogic(Song *pSong, DataArray *pConfig, int nSeed)
    : GameLogic(pSong, pConfig, nSeed),
      mJuice(TheGameConfig->mInitialJuice[TheGameDb->mSkillLevel]), mPlayer(mLocalPlayers[0]),
      mTickOffset(0), mTimeOffset(0.0f), mBankSlot(-1),
      mFreezeCmd(NewMemFunCommand(this, &SoloGameLogic::FreezeAfterLoss)),
      mRestartCmd(NewMemFun1Command(this, &SoloGameLogic::Restart, true)),
      mIntroCmd(NewMemFunCommand(&TheGfxManager, &GfxManager::StartIntro)),
      mSaveOrRetryCmd(NewMemFunCommand(this, &SoloGameLogic::SaveOrRetry)),
      mCheckpointTextCmd(new CheckpointTextCmd(this)),
      mSwapMovieCmd(new SwapMovieCmd(pSong->mBuilder->mTicksPerBar)), mIntroDuration(0.0f),
      mPlayBars(0), mCapturedTracks(0), mFailingTrack(kNoTrack), mPendingJuice(0.0f),
      mCaptureJuice(kInitialCaptureJuice), mDialogOpen(0), mForceWinSequence(0),
      mWinSequenceStep(kWinStepCampaign), mFullMixBars(0), mBestStreak(0), mPossibleCaptureBars(0) {
    (void)TheGameDb->GetNumPlayers(); // Yes, the binary discards this call's result.
    if (TheGameDb->mPracticeMode) {
        TheGfxManager.SetLyricText(TheLocale.Localize(kPracticeModeToken, true), true);
    }

    DataArray *pGame = SystemConfig()->FindArray(kGameSection, false);
    pGame->FindInt(kBankSlotEntry, &mBankSlot, true);
    const char *pszFile = nullptr;
    pGame->FindSymbol(kWinBankFileEntry, &pszFile, true);
    mWinBankFile = pszFile;
    pGame->FindSymbol(kSoloBankFileEntry, &pszFile, true);
    mSoloBankFile = pszFile;

    const SectionBoundaries *pSections = mSong->GetSections();
    for (int i = 0; i < pSections->NumSections(); ++i) {
        int nStart;
        int nEnd;
        pSections->GetSectionRange(i, &nStart, &nEnd);
        mPlayBars += nEnd - nStart;
        if (i > 0) {
            mPlayBars -= CheckpointBars();
        }
    }

    mCaptureJuice = TheGameConfig->mCaptureJuice[TheGameDb->mSkillLevel];
    ScriptFunction::Register(OnJuice, kJuiceCommand, this);
    ScriptFunction::Register(OnWinCheat, kWinCheatCommand, this);
    ScriptFunction::Register(OnWinSequence, kWinSequenceCommand, this);
}

SoloGameLogic::~SoloGameLogic() {
    ScriptFunction::Unregister(OnJuice);
    ScriptFunction::Unregister(OnWinCheat);
    ScriptFunction::Unregister(OnWinSequence);
    SetSongSpeed(mSong->GetSpeed());
    TheGfxManager.SetLyricText(kEmptyText, true);
}

void SoloGameLogic::OnStart() {
    mJuice.Set(TheGameConfig->mInitialJuice[TheGameDb->mSkillLevel]);
    mPendingJuice = 0.0f;
    if (TheGfxManager.GetOption()) {
        TheControllerDisplay->SetOption(true);
    }
    if (TheGameDb->mSkillLevel < kSkillLevelBrutal) {
        TheControllerDisplay->Show();
    }
    EnableNextTracks(0);
    if (TheGameDb->IsWinSequence()) {
        ResumeWithCue(false);
        TheSongScheduler.PostAfter(NewMemFunCommand(this, &SoloGameLogic::AdvanceWinSequence),
                                   kWinSequenceStartDelayMs,
                                   false);
    }
}

void SoloGameLogic::AdvanceWinSequence() {
    const char *pszLine = nullptr;
    const char *pszSecondLine = nullptr;
    float fScale = kCampaignScale;
    float fDurationMs = kWinMessageDurationMs;
    switch (mWinSequenceStep) {
    case kWinStepCampaign: {
        pszLine = TheLocale.Localize(kWonCampaign11Token, true);
        const char *pszFormat = TheLocale.Localize(kWonCampaign12Token, true);
        const char *pszDifficulty = TheGameDb->GetDifficultyName();
        pszSecondLine = FormatString(pszFormat, pszDifficulty);
        if (TheGameDb->mSkillLevel == kSkillLevelInsane) {
            ++mWinSequenceStep;
        }
        break;
    }
    case kWinStepNextDifficulty: {
        fScale = kNextDifficultyScale;
        const char *pszFormat = TheLocale.Localize(kWonCampaign2Token, true);
        const char *pszDifficulty =
            TheGameDb->GetDifficultyName(TheGameDb->mSkillLevel + 1, GameDb::kRuleSetGame);
        pszLine = FormatString(pszFormat, pszDifficulty);
        break;
    }
    case kWinStepNextCampaign:
        fScale = kNextDifficultyScale;
        pszLine = TheLocale.Localize(kWonCampaign3Token, true);
        break;
    case kWinStepFinalCampaign:
        fScale = kFinalCampaignScale;
        pszLine = TheLocale.Localize(kWonCampaign4Token, true);
        fDurationMs = kFinalWinMessageDurationMs;
        break;
    case kWinStepPrompt:
        TheGfxManager.SetLyricText(TheLocale.Localize(kPressStartToExitToken, true), true);
        break;
    default:
        DebugWarn("illegal win msg");
        break;
    }
    if (pszLine) {
        TheGfxManager.ShowMessage(pszLine,
                                  pszSecondLine,
                                  kAllPlayers,
                                  fDurationMs,
                                  fScale,
                                  kMessageOffset,
                                  kMessageOffset);
    }
    if (mWinSequenceStep < kWinStepPrompt) {
        TheSongScheduler.PostAfter(NewMemFunCommand(this, &SoloGameLogic::AdvanceWinSequence),
                                   fDurationMs + kWinSequenceGapMs,
                                   false);
    }
    ++mWinSequenceStep;
}

void SoloGameLogic::SetPaused(bool bPaused, int nPad, int nReason) {
    if (bPaused && mState != kStatePlaying) {
        return;
    }
    if (mPaused == bPaused) {
        return;
    }
    mPaused = bPaused;
    GameLogic::SetPaused(bPaused, nPad, nReason);
    if (bPaused) {
        TheSongScheduler.Pause();
        AllNotesOff();
        TheMetagame.ShowDialog(nReason ? Metagame::kDialogNoController : Metagame::kDialogPause,
                               OnPauseDialog,
                               this,
                               nPad);
    } else {
        TheSongScheduler.Resume();
        TheControllerDisplay->RefreshLabels();
    }
}

void SoloGameLogic::OnPauseDialog(Metagame::DialogAction action, void *pUserData) {
    auto *pLogic = static_cast<SoloGameLogic *>(pUserData);
    switch (action) {
    case Metagame::kDialogActionResume:
        pLogic->SetPaused(false, kNoPad, 0);
        break;
    case Metagame::kDialogActionQuit:
        pLogic->mQuit = action;
        pLogic->EndSong();
        break;
    case Metagame::kDialogActionEnd:
        pLogic->EndSong();
        break;
    default:
        DebugWarn("illegal dialog response");
        break;
    }
}

int SoloGameLogic::GetTick() {
    return TheSongScheduler.GetClockTick() + mTickOffset;
}

float SoloGameLogic::GetTime() {
    return TheSongScheduler.GetClockTime() + mTimeOffset;
}

void SoloGameLogic::HandleInput(const BtnEvent<3> &event) {
    GameLogic::HandleInput(event);
    if (mState != kStateRunning) {
        return;
    }
    if (!TheGameDb->IsWinSequence()) {
        EndSong();
        return;
    }
    if (mWinSequenceStep < kWinStepDone || mDialogOpen) {
        return;
    }
    TheGfxManager.SetLyricText(kEmptyText, true);
    TheGfxManager.ShowMessage(kEmptyText,
                              nullptr,
                              kAllPlayers,
                              kClearDurationMs,
                              kClearScale,
                              kMessageOffset,
                              kMessageOffset);
    TheMetagame.ShowDialog(Metagame::kDialogSoloWon, OnWinDialog, this, kNoPad);
    mDialogOpen = 1;
}

void SoloGameLogic::HandleInput(const PlayNoteEvent &event) {
    if (!mDialogOpen || !event.mState) {
        GameLogic::HandleInput(event);
    }
}

void SoloGameLogic::HandleInput(const StickEvent<2> &event) {
    if (!mDialogOpen) {
        GameLogic::HandleInput(event);
    }
}

void SoloGameLogic::HandleInput(const StickEvent<6> &event) {
    if (!mDialogOpen) {
        GameLogic::HandleInput(event);
    }
}

void SoloGameLogic::OnPhraseCaptured([[maybe_unused]] CatchTrack *pTrack,
                                     Player *pPlayer,
                                     int nBar,
                                     [[maybe_unused]] bool bStreak) {
    if (++mCapturedTracks == static_cast<int>(mCatchTracks.size())) {
        ++mFullMixBars;
    }
    int nCurrentBar = TheSongScheduler.mTick / mTicksPerBar;
    mBestStreak = std::max(pPlayer->GetStreak(), mBestStreak);
    mPossibleCaptureBars += std::min(nBar, mNumBars) - nCurrentBar + 1;
    TheSongScheduler.PostAfter(
        NewMemFun1Command(this,
                          &GameLogic::EnableNextTracks,
                          std::max(TheSongScheduler.mTick / mTicksPerBar + 1, 0)),
        kEnableTracksDelayMs,
        false);
    if (TheGameDb->mPracticeMode) {
        return;
    }
    mPendingJuice += mCaptureJuice;
    CommitPendingJuice();
    mFailingTrack = kNoTrack;
    if (mJuice.Get() < kMinJuiceAfterCapture) {
        mJuice.Set(kMinJuiceAfterCapture);
    }
}

void SoloGameLogic::OnPhraseEnded([[maybe_unused]] Track *pTrack) {
    --mCapturedTracks;
}

void SoloGameLogic::OnPhraseMissed(Track *pTrack) {
    if (TheGameDb->mPracticeMode) {
        return;
    }
    int nTrack = pTrack->mIndex;
    if (mFailingTrack == nTrack) {
        Finish(false);
    }
    if (mPlayer->GetTrack()->mIndex == nTrack) {
        CommitPendingJuice();
    }
}

void SoloGameLogic::CommitPendingJuice() {
    if (mPendingJuice == kNoJuice || IsFreestyleTrackActive()) {
        return;
    }
    mJuice.Add(mPendingJuice);
    mPendingJuice = kNoJuice;
}

void SoloGameLogic::DrainPendingJuice() {
    int nBar = TheSongScheduler.mTick / mTicksPerBar;
    if (nBar < 0 || nBar >= mNumBars) {
        return;
    }
    if (mPlayer->GetTrack() == mFreestyleTrack || !mPhraseThisBar) {
        return;
    }
    mPendingJuice -= kJuiceDrainPerBar;
}

void SoloGameLogic::OnBar(int nBar) {
    int nCheckpointBars = CheckpointBars();
    if (nBar == mSectionStartBar && nBar != 0) {
        TheHelpText->SetEnabled(false);
    } else if (nBar == mSectionStartBar + nCheckpointBars && nBar < mNumBars) {
        TheHelpText->SetEnabled(true);
    }

    bool bFullMix = mCapturedTracks == static_cast<int>(mCatchTracks.size());
    if (bFullMix) {
        ++mFullMixBars;
    }
    if (nBar < mNumBars) {
        TheGfxManager.SetWinner(bFullMix ? kLocalPlayer : kNoWinner);
    }
    if (TheGameDb->mPracticeMode) {
        return;
    }

    if (!mPlayer->GetCatching()) {
        CommitPendingJuice();
    }
    DrainPendingJuice();
    if (mJuice.Get() <= kNoJuice && !TheGameDb->mPracticeMode && nBar != mNextSectionBar &&
        (mSectionStartBar == 0 || mSectionStartBar + nCheckpointBars < nBar) &&
        mFailingTrack == kNoTrack) {
        unsigned int nTrack = mPlayer->GetTrack()->mIndex;
        int nStartTick;
        int nEndTick;
        if (nTrack < mCatchTracks.size() &&
            mCatchTracks[nTrack]->FindPhrase(nBar, &nStartTick, &nEndTick) &&
            nStartTick / mTicksPerBar == nBar) {
            mFailingTrack = nTrack;
        } else if (mPhraseThisBar) {
            Finish(false);
        }
    }
    if (mJuice.Get() / mJuice.GetMax() < kLowJuiceFraction &&
        mPlayer->GetPowerup() == kPowerupAutocatcher) {
        TheHelpText->ShowDeployAutocatcher();
    }
}

void SoloGameLogic::OnSection() {
    int nSkillLevel = TheGameDb->mSkillLevel;
    if (nSkillLevel < kSkillLevelBrutal) {
        TheControllerDisplay->Hide();
    }
    if (mSong->GetSections()->IsPastEnd(TheSongScheduler.mTick / mTicksPerBar)) {
        if (mState == kStatePlaying) {
            Finish(true);
        }
        return;
    }
    if (mState != kStatePlaying) {
        return;
    }

    TheSongScheduler.PostIn(mCheckpointTextCmd.Get(), kCheckpointTextDelayTicks, false);
    if (TheGameDb->mPracticeMode) {
        return;
    }
    TheGfxManager.CompleteStage(kLocalPlayer, mSection);
    if (nSkillLevel == kSkillLevelInsane) {
        FxMidi::PlaySound3();
        return;
    }
    FxMidi::PlaySound2();

    const GameConfig *pConfig = TheGameConfig;
    float fCaptureShare =
        pConfig->mCaptureJuice[nSkillLevel] / pConfig->mBarsPerCapture[nSkillLevel];
    int nBars = mNextSectionBar - mSectionStartBar - pConfig->mCheckpointBars;
    float fBonus = std::max(kMinEnergyBonus, static_cast<float>(nBars) * (1.0f - fCaptureShare));
    mPendingJuice += fBonus;
    CommitPendingJuice();
    TheGfxManager.ShowMessage(TheLocale.Localize(kEnergyBonusToken, true),
                              nullptr,
                              kAllPlayers,
                              kMessageDurationMs,
                              kEnergyBonusScale,
                              kMessageOffset,
                              kMessageOffset);
}

void SoloGameLogic::OnWinDialog(Metagame::DialogAction action, void *pUserData) {
    auto *pLogic = static_cast<SoloGameLogic *>(pUserData);
    switch (action) {
    case Metagame::kDialogActionResume:
        pLogic->mReserved60 = 1;
        pLogic->EndSong();
        break;
    case Metagame::kDialogActionEnd:
        pLogic->EndSong();
        break;
    case Metagame::kDialogActionQuit:
        if (TheGameDb->IsWinSequence()) {
            pLogic->mReserved60 = action;
        } else {
            auto nSlot = static_cast<unsigned short>(pLogic->mBankSlot);
            TheSynth->UnloadBank(nSlot);
            TheSynth->LoadBank(nSlot, pLogic->mSoloBankFile, true);
            TheGameDb->SetPracticeMode(false);
            pLogic->mQuit = action;
        }
        pLogic->EndSong();
        break;
    case Metagame::kDialogActionContinue:
        if (TheGameDb->IsWinSequence()) {
            TheGfxManager.SetLyricText(TheLocale.Localize(kPressStartToExitToken, true), true);
        } else {
            pLogic->ResumeWithCue(true);
        }
        break;
    default:
        DebugWarn("illegal choice");
        break;
    }
    pLogic->mDialogOpen = 0;
    TheGameDb->SetWinSequence(false);
}

void SoloGameLogic::OnResultDialog(Metagame::DialogAction action, void *pUserData) {
    auto *pLogic = static_cast<SoloGameLogic *>(pUserData);
    switch (action) {
    case Metagame::kDialogActionPractice:
        TheGameDb->SetPracticeMode(true);
        pLogic->mQuit = 1;
        pLogic->EndSong();
        break;
    case Metagame::kDialogActionQuit:
        TheGameDb->SetPracticeMode(false);
        pLogic->mQuit = action;
        pLogic->EndSong();
        break;
    case Metagame::kDialogActionEnd:
        pLogic->EndSong();
        break;
    default:
        DebugWarn("illegal choice");
        break;
    }
}

bool SoloGameLogic::DeployFreestyle(Player *pPlayer) {
    bool bDeployed = GameLogic::DeployFreestyle(pPlayer);
    if (bDeployed) {
        int nBar = TheSongScheduler.mTick / mTicksPerBar;
        int nDurationBars = TheGameDb->mCommunity == GameDb::kCommunitySolo ?
                                TheGameConfig->mFreestyleDurationBarsSolo :
                                TheGameConfig->mFreestyleDurationBarsMultiNet;
        int nEndBar = std::min(nBar + nDurationBars + kFreestyleExtraBars, mNumBars);
        mPossibleCaptureBars += static_cast<int>(mCatchTracks.size()) * (nEndBar - nBar);
    }
    return bDeployed;
}

void SoloGameLogic::OnJuice(DataArray *pCommand, void *pUserData) {
    static_cast<SoloGameLogic *>(pUserData)->mJuice.Add(static_cast<float>(pCommand->Int(1)));
}

void SoloGameLogic::OnWinCheat([[maybe_unused]] DataArray *pCommand, void *pUserData) {
    static_cast<SoloGameLogic *>(pUserData)->WinNow();
}

void SoloGameLogic::OnWinSequence([[maybe_unused]] DataArray *pCommand, void *pUserData) {
    DebugPrint("CHEAT: setting win sequence flag\n");
    static_cast<SoloGameLogic *>(pUserData)->mForceWinSequence = 1;
}

void SoloGameLogic::WinNow() {
    mPlayer->SetScore(kWinCheatScore);
    Finish(true);
    TheGameDb->SetProgress(kFullProgress);
}

void SoloGameLogic::SaveOrRetry() {
    if (TheGfxManager.IsIdle() && TheSynth->IsBankLoaded(static_cast<unsigned short>(mBankSlot))) {
        TheMetagame.ShowDialog(Metagame::kDialogSoloWon, OnWinDialog, this, kNoPad);
        mDialogOpen = 1;
    } else {
        TheSongScheduler.PostAfter(mSaveOrRetryCmd.Get(), kRetryDelayMs, false);
    }
}

void SoloGameLogic::OnFinish(bool bWon) {
    int nBar = TheSongScheduler.mTick / mTicksPerBar;
    float fProgress =
        std::min(kFullProgress, static_cast<float>(nBar) / static_cast<float>(mNumBars));
    // Yes, the binary limits the fraction of a lost song to 99.
    if (!bWon && kMaxLostProgress < fProgress) {
        fProgress = kMaxLostProgress;
    }
    int nTracks = static_cast<int>(mCatchTracks.size());
    float fEnergized = std::min(kFullProgress,
                                static_cast<float>(mPossibleCaptureBars) /
                                    static_cast<float>(nTracks * (mNumBars - nTracks) + nTracks));
    TheGameDb->SetProgress(fProgress);
    TheGameDb->SetEnergized(fEnergized);
    TheGameDb->SetFullMixBars(mFullMixBars);
    TheGameDb->SetBestStreak(mBestStreak);

    if (bWon) {
        TheStats->WinSoloGame(mPlayer->GetScore(), mFullMixBars, mBestStreak, fEnergized);
        mState = kStateWon;
        TheMixer->PlayFullMix();
        if (!TheGameDb->mPracticeMode) {
            TheGfxManager.ShowMessage(TheLocale.Localize(kWonSoloGameToken, true),
                                      nullptr,
                                      kAllPlayers,
                                      kMessageDurationMs,
                                      kResultScale,
                                      kMessageOffset,
                                      kMessageOffset);
        }
        TheGfxManager.BeginLoad(0, 0);
        TheGfxManager.SetWinner(kLocalPlayer);
        TheSongScheduler.PostAt(mSwapMovieCmd.Get(), (nBar + 1) * mTicksPerBar, false);
        if (TheGameDb->mPracticeMode) {
            TheMetagame.ShowDialog(Metagame::kDialogSoloWon, OnResultDialog, this, kNoPad);
            mDialogOpen = 1;
        } else {
            FxMidi::PlayWinSound();
            Campaign *pProfile = TheGameDb->GetProfile(kLocalPlayer);
            if (pProfile->CompletesTier(TheGameDb->mSong.c_str(), TheGameDb->mSkillLevel) ||
                mForceWinSequence) {
                TheGameDb->SetWinSequence(true);
                mReserved60 = 1;
                TheSongScheduler.PostAfter(
                    NewMemFunCommand(this, &GameLogic::EndSong), kEndSongDelayMs, false);
            } else {
                auto nSlot = static_cast<unsigned short>(mBankSlot);
                TheSynth->UnloadBank(nSlot);
                TheSynth->LoadBank(nSlot, mWinBankFile, true);
                SaveOrRetry();
            }
        }
    } else {
        TheStats->LoseSoloGame(TheSongScheduler.mTick, mPlayer->GetScore(), fProgress);
        mState = kStateSuspended;
        mSpeedRamp->MoveTo(kLossSlowdownTicks, kLossSlowdownStepTicks, kLossSpeed);
        TheSongScheduler.PostIn(mFreezeCmd.Get(), kLossSlowdownTicks + 1, false);
        TheGfxManager.ShowMessage(TheLocale.Localize(kLostSoloGameToken, true),
                                  nullptr,
                                  kAllPlayers,
                                  kMessageDurationMs,
                                  kResultScale,
                                  kMessageOffset,
                                  kMessageOffset);
        TheSongScheduler.PostIn(NewMemFun3Command(&TheMetagame,
                                                  &Metagame::ShowDialog,
                                                  Metagame::kDialogSoloLost,
                                                  &SoloGameLogic::OnResultDialog,
                                                  this),
                                kLossSlowdownTicks,
                                false);
    }
    TheStats->End();
}

void SoloGameLogic::FreezeAfterLoss() {
    TheSongScheduler.Pause();
    TheGfxManager.ShowMessage(kEmptyText,
                              nullptr,
                              kAllPlayers,
                              kClearDurationMs,
                              kClearScale,
                              kMessageOffset,
                              kMessageOffset);
    AllNotesOff();
    TheSynth->Poll();
    SetSongSpeed(mSong->GetSpeed());
}

void SoloGameLogic::ResumeWithCue(bool bIntro) {
    TheGfxManager.SetLyricText(kEmptyText, true);
    TheMixer->StartVictoryLap();
    for (int i = 0; i < TheGameDb->GetNumPads(); ++i) {
        JoypadSetMenuControl(i, false);
    }
    if (bIntro) {
        mIntroDuration = TheGfxManager.StartIntro(kIntroArgument);
    }
    if (mPlayer->GetTrack() == mFreestyleTrack) {
        LeaveFreestyle(mPlayer, kTrackOfCamera);
    }
    TheGfxManager.SetActive(true);
    TheGfxManager.ClearLanes();
    TheGfxManager.SetScrollSpeed(kNormalScrollSpeed);
    if (!bIntro) {
        Restart(false);
        return;
    }

    const float *pfMsPerTick = mSong->GetMsPerTick();
    int nSongTicks = mNumBars * mTicksPerBar;
    float fDelay = mIntroDuration;
    float fLoopEnd =
        static_cast<float>((TheSongScheduler.mTick / nSongTicks + 1) * nSongTicks) * *pfMsPerTick;
    if (fLoopEnd - kLoopEndMarginMs < TheSongScheduler.mTime + (mIntroDuration + mIntroDuration)) {
        fDelay = (fLoopEnd - TheSongScheduler.mTime) + fDelay;
    }
    TheSongScheduler.PostAfter(mRestartCmd.Get(), fDelay + kRestartDelayMs, false);
}

void SoloGameLogic::Restart(bool bRestartIntro) {
    TheGfxManager.ClearAll();
    if (!TheGameDb->IsWinSequence()) {
        TheGfxManager.SetLyricText(TheLocale.Localize(kPressStartToExitToken, true), true);
    }
    int nSongTicks = mNumBars * mTicksPerBar;
    int nTick = TheSongScheduler.mTick;
    mTickOffset = nTick % nSongTicks - nTick;
    mFreestyleTrack->mTickOffset = mTickOffset;
    const float *pfOffsetMsPerTick = mSong->GetMsPerTick();
    mState = kStateRunning;
    mTimeOffset = static_cast<float>(mTickOffset) * *pfOffsetMsPerTick;
    const float *pfMsPerTick = mSong->GetMsPerTick();
    TheGameDb->mSongTick = static_cast<float>(GetTick());
    TheGameDb->mSongTime = static_cast<float>(nTick) * *pfMsPerTick;
    (void)TheGfxManager.Poll(kRestartPollTime); // Yes, the binary discards this call's result.
    for (unsigned int i = 0; i < mCatchTracks.size(); ++i) {
        mCatchTracks[i]->Restart(mPlayer, GetTick());
    }
    EnterFreestyle(mPlayer, false);

    int nLoopEnd = nTick + nSongTicks - nTick % nSongTicks;
    TheSongScheduler.PostAtTime(mIntroCmd.Get(),
                                static_cast<float>(nLoopEnd) * *pfMsPerTick - mIntroDuration -
                                    kIntroLeadMs,
                                false);
    TheSongScheduler.PostAt(mRestartCmd.Get(), nLoopEnd, false);
    if (bRestartIntro) {
        TheGfxManager.RestartIntro();
    }
}

bool SoloGameLogic::DispatchPriv(Message *pMsg) {
    (void)pMsg->Type(); // Yes, the binary discards this call's result.
    return WorldLogic::DispatchPriv(pMsg);
}
