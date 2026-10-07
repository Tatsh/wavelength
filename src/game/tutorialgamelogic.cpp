#include "game/tutorialgamelogic.h"

#include <algorithm>
#include <climits>
#include <cstring>

#include "game/gameconfig.h"
#include "game/gamedb.h"
#include "game/gametrackselector.h"
#include "game/inputmap.h"
#include "game/playerprofile.h"
#include "game/playmap.h"
#include "game/scripttask.h"
#include "game/waitfortasktask.h"
#include "game/waitinteractivetask.h"
#include "game/waittimetask.h"
#include "gfx/gfxmanager.h"
#include "math/rand.h"
#include "os/debug.h"
#include "os/scheduler.h"
#include "os/string.h"
#include "script/scriptfunction.h"
#include "synth/songspeed.h"

namespace {

constexpr char kMidiCommand[] = "midi";
constexpr char kStreamCommand[] = "stream";
constexpr char kLoopTrackCommand[] = "loop_track";
constexpr char kDisableRotCommand[] = "disable_rot";
constexpr char kDisableInputCommand[] = "disable_input";
constexpr char kTutorialPrintCommand[] = "tutorial_print";
constexpr char kDisablePowerupsCommand[] = "disable_powerups";
constexpr char kActivateBarsCommand[] = "activate_bars";
constexpr char kSetSongPosParamsCommand[] = "set_songpos_params";
constexpr char kCaptureTrackCommand[] = "capture_track";
constexpr char kCaptureAllTracksCommand[] = "capture_all_tracks";
constexpr char kSetPowerupCommand[] = "set_powerup";
constexpr char kMoveTrackCommand[] = "move_track";
constexpr char kSetNoCaptureCommand[] = "set_no_capture";
constexpr char kSetNoDeactivateCommand[] = "set_no_deactivate";
constexpr char kSetNoSeekerCommand[] = "set_no_seeker";
constexpr char kSetNoStreaksCommand[] = "set_no_streaks";
constexpr char kSetStrandBarsCommand[] = "set_strand_bars";
constexpr char kSchedDeactivateTrackCommand[] = "sched_deactivate_track";
constexpr char kShowStreakArrowCommand[] = "show_streak_arrow";
constexpr char kSchedSetJuiceCommand[] = "sched_set_juice";
constexpr char kSetAutopilotCommand[] = "set_autopilot";
constexpr char kSetManualEnableNextCommand[] = "set_manual_enable_next";
constexpr char kEnableNextCommand[] = "enable_next";
constexpr char kSchedStageCompleteCommand[] = "sched_stage_complete";

constexpr char kNoQueueOption[] = "no_queue";
constexpr char kStreamDirectoryFormat[] = "Songs/%s/";
constexpr char kScriptEntry[] = "script";
constexpr char kBadRotateStateFormat[] = "Bad rotate state. state = %i.\n";
constexpr char kIllegalDialogResponse[] = "illegal dialog response";
constexpr char kMissingScriptTrackFormat[] = "Cannot find a midi SCRIPT track named:%s";
constexpr char kWaitStreamStep[] = "wait_stream";
constexpr char kWaitMidiStep[] = "wait_midi";
constexpr char kWaitTimeStep[] = "wait_time";
constexpr char kWaitInteractiveStep[] = "wait_interactive";

// A "stream" command with a third node has an option there.
constexpr int kStreamCommandWithOptionSize = 3;

// Nodes of a command. The command name is node 0.
enum CommandNode {
    kCommandNodeFirst = 1,
    kCommandNodeSecond = 2,
    kCommandNodeThird = 3,
};

// Nodes of a script step.
enum StepNode {
    kStepNodeType = 0,
    kStepNodeArgument = 1,
};

// The script numbers the tracks and the bars from 1.
constexpr int kScriptNumberBase = 1;

// The first player, whose tracks the commands act on and whose bindings the tutorial replaces.
constexpr int kTutorialPlayer = 0;

// The controller the end-of-tutorial menu listens to.
constexpr int kTutorialPad = 0;

constexpr int kNoPad = -1;
constexpr int kNoReason = 0;
constexpr int kFirstStep = 1;
constexpr int kFirstBar = 0;
constexpr float kNormalSpeed = 1.0f;
constexpr float kInitialProgressLength = 50.0f;
constexpr float kNoProgress = 0.0f;
constexpr float kFullProgress = 1.0f;

// The last random seed GameLogic draws from.
constexpr int kMaxSeed = INT_MAX;

// The bindings of the first player before the tutorial replaced them.
// NTSC-U/C: 0x004361e0
InputMap gSavedInputMap;

// Takes a run of bars of a catch track away from every player when it runs.
class TrackDeactivator : public Command {
public:
    TrackDeactivator(int nTrack, int nBar, int nBars, CatchTrack *pTrack)
        : mTrackIndex(nTrack), mBar(nBar), mBars(nBars), mTrack(pTrack) {
    }

    // NTSC-U/C: 0x00343b00, PAL: 0x003b1038
    ~TrackDeactivator() override {
    }

    // NTSC-U/C: 0x00343b78, PAL: 0x003b10b0
    void Execute() override {
        mTrack->AssignBars(mBar, mBars, nullptr);
    }

    int mTrackIndex;
    int mBar;
    int mBars;
    CatchTrack *mTrack;
};

// Sets the first player's energy meter when it runs.
class SetJuiceCmd : public Command {
public:
    explicit SetJuiceCmd(float fJuice) : mJuice(fJuice) {
    }

    // NTSC-U/C: 0x00343ba8, PAL: 0x003b10e0
    ~SetJuiceCmd() override {
    }

    // NTSC-U/C: 0x00343c20, PAL: 0x003b1158
    void Execute() override {
        TheGfxManager.SetEnergy(kTutorialPlayer, mJuice);
    }

    float mJuice;
};

// Shows the stage completion of the first player when it runs.
class StageCompleteCmd : public Command {
public:
    // NTSC-U/C: 0x00343c48, PAL: 0x003b1180
    ~StageCompleteCmd() override {
    }

    // NTSC-U/C: 0x00343cc0, PAL: 0x003b11f8
    void Execute() override {
        TheGfxManager.CompleteStage(kTutorialPlayer, 0);
    }
};

// The tick at the start of the bar a number of bars after the current one.
inline int BarTickAhead(int nBarsAhead, int nTicksPerBar) {
    return (nBarsAhead + TheSongScheduler.mTick / nTicksPerBar) * nTicksPerBar;
}

} // namespace

void TutorialGameLogic::OnMidiCommand(DataArray *pCommand, void *pUserData) {
    static_cast<TutorialGameLogic *>(pUserData)->PlayScriptTrack(pCommand->Sym(kCommandNodeFirst));
}

void TutorialGameLogic::OnSchedDeactivateTrackCommand(DataArray *pCommand, void *pUserData) {
    const int nBarsAhead = pCommand->Int(kCommandNodeFirst);
    const int nBar = pCommand->Int(kCommandNodeSecond) - kScriptNumberBase;
    const int nBars = pCommand->Int(kCommandNodeThird);
    static_cast<TutorialGameLogic *>(pUserData)->ScheduleDeactivateTrack(nBarsAhead, nBar, nBars);
}

void TutorialGameLogic::OnStreamCommand(DataArray *pCommand, void *pUserData) {
    bool bSkipIfBusy = false;
    if (pCommand->mSize == kStreamCommandWithOptionSize) {
        bSkipIfBusy = strcmp(pCommand->Sym(kCommandNodeSecond), kNoQueueOption) == 0;
    }
    static_cast<TutorialGameLogic *>(pUserData)->QueueStream(pCommand->Sym(kCommandNodeFirst),
                                                             bSkipIfBusy);
}

void TutorialGameLogic::OnLoopTrackCommand(DataArray *pCommand, void *pUserData) {
    const int nStartBar = pCommand->Int(kCommandNodeFirst) - kScriptNumberBase;
    const int nNumBars = pCommand->Int(kCommandNodeSecond);
    const int nBarsAhead = pCommand->Int(kCommandNodeThird);
    static_cast<TutorialGameLogic *>(pUserData)->LoopTrack(nStartBar, nNumBars, nBarsAhead);
}

void TutorialGameLogic::OnDisableRotCommand(DataArray *pCommand, void *pUserData) {
    static_cast<TutorialGameLogic *>(pUserData)->SetRotateMode(pCommand->Int(kCommandNodeFirst));
}

void TutorialGameLogic::OnDisableInputCommand(DataArray *pCommand, void *pUserData) {
    static_cast<TutorialGameLogic *>(pUserData)->SetInputDisabled(
        pCommand->Int(kCommandNodeFirst) != 0);
}

void TutorialGameLogic::OnTutorialPrintCommand(DataArray *pCommand,
                                               [[maybe_unused]] void *pUserData) {
    (void)pCommand->Sym(kCommandNodeFirst); // Yes, the binary discards the text.
}

void TutorialGameLogic::OnDisablePowerupsCommand(DataArray *pCommand, void *pUserData) {
    static_cast<TutorialGameLogic *>(pUserData)->SetPowerupsDisabled(
        pCommand->Int(kCommandNodeFirst) != 0);
}

void TutorialGameLogic::OnActivateBarsCommand(DataArray *pCommand, void *pUserData) {
    const int nBar = pCommand->Int(kCommandNodeFirst) - kScriptNumberBase;
    const int nBars = pCommand->Int(kCommandNodeSecond);
    const int nTrack = pCommand->Int(kCommandNodeThird) - kScriptNumberBase;
    static_cast<TutorialGameLogic *>(pUserData)->ActivateBars(nBar, nBars, nTrack);
}

void TutorialGameLogic::OnSetSongPosParamsCommand(DataArray *pCommand, void *pUserData) {
    const int nPlayedBars = pCommand->Int(kCommandNodeFirst);
    const int nTotalBars = pCommand->Int(kCommandNodeSecond);
    static_cast<TutorialGameLogic *>(pUserData)->SetSongPosParams(nPlayedBars, nTotalBars);
}

void TutorialGameLogic::OnCaptureTrackCommand([[maybe_unused]] DataArray *pCommand,
                                              void *pUserData) {
    static_cast<TutorialGameLogic *>(pUserData)->CaptureTrack();
}

void TutorialGameLogic::OnCaptureAllTracksCommand([[maybe_unused]] DataArray *pCommand,
                                                  void *pUserData) {
    static_cast<TutorialGameLogic *>(pUserData)->CaptureAllTracks();
}

void TutorialGameLogic::OnSetPowerupCommand(DataArray *pCommand, void *pUserData) {
    const int nPowerup = pCommand->Int(kCommandNodeFirst);
    const int nTrack = pCommand->Int(kCommandNodeSecond) - kScriptNumberBase;
    const int nBar = pCommand->Int(kCommandNodeThird) - kScriptNumberBase;
    static_cast<TutorialGameLogic *>(pUserData)->PlacePowerup(nPowerup, nTrack, nBar);
}

void TutorialGameLogic::OnMoveTrackCommand(DataArray *pCommand, void *pUserData) {
    static_cast<TutorialGameLogic *>(pUserData)->MoveTrack(pCommand->Int(kCommandNodeFirst) -
                                                           kScriptNumberBase);
}

void TutorialGameLogic::OnSetNoCaptureCommand(DataArray *pCommand,
                                              [[maybe_unused]] void *pUserData) {
    TheGameConfig->mNoCapture = pCommand->Int(kCommandNodeFirst) != 0;
}

void TutorialGameLogic::OnSetNoDeactivateCommand(DataArray *pCommand,
                                                 [[maybe_unused]] void *pUserData) {
    TheGameConfig->mNoDeactivate = pCommand->Int(kCommandNodeFirst) != 0;
}

void TutorialGameLogic::OnSetNoSeekerCommand(DataArray *pCommand, void *pUserData) {
    static_cast<TutorialGameLogic *>(pUserData)->SetNoSeeker(pCommand->Int(kCommandNodeFirst) != 0);
}

void TutorialGameLogic::OnSetNoStreaksCommand(DataArray *pCommand,
                                              [[maybe_unused]] void *pUserData) {
    TheGameConfig->mStreaksEnabled = pCommand->Int(kCommandNodeFirst) == 0;
}

void TutorialGameLogic::OnSetStrandBarsCommand(DataArray *pCommand,
                                               [[maybe_unused]] void *pUserData) {
    const int nBars = pCommand->Int(kCommandNodeFirst);
    TheGameConfig->SetStrandBars(TheGameDb->GetNumPlayers(), nBars);
}

void TutorialGameLogic::OnShowStreakArrowCommand(DataArray *pCommand, void *pUserData) {
    const int nTrack = pCommand->Int(kCommandNodeFirst) - kScriptNumberBase;
    const int nBarsAhead = pCommand->Int(kCommandNodeSecond);
    const int nUnused = pCommand->Int(kCommandNodeThird);
    static_cast<TutorialGameLogic *>(pUserData)->ShowStreakArrow(nTrack, nBarsAhead, nUnused);
}

void TutorialGameLogic::OnSchedSetJuiceCommand(DataArray *pCommand, void *pUserData) {
    const int nBarsAhead = pCommand->Int(kCommandNodeFirst);
    const float fJuice = pCommand->Float(kCommandNodeSecond);
    static_cast<TutorialGameLogic *>(pUserData)->ScheduleSetJuice(nBarsAhead, fJuice);
}

void TutorialGameLogic::OnSetAutopilotCommand(DataArray *pCommand, void *pUserData) {
    static_cast<TutorialGameLogic *>(pUserData)->SetAutopilot(pCommand->Int(kCommandNodeFirst) !=
                                                              0);
}

void TutorialGameLogic::OnSetManualEnableNextCommand(DataArray *pCommand, void *pUserData) {
    static_cast<TutorialGameLogic *>(pUserData)->SetManualEnableNext(
        pCommand->Int(kCommandNodeFirst) != 0);
}

void TutorialGameLogic::OnEnableNextCommand([[maybe_unused]] DataArray *pCommand, void *pUserData) {
    static_cast<TutorialGameLogic *>(pUserData)->EnableNext();
}

void TutorialGameLogic::OnSchedStageCompleteCommand(DataArray *pCommand, void *pUserData) {
    static_cast<TutorialGameLogic *>(pUserData)->ScheduleStageComplete(
        pCommand->Int(kCommandNodeFirst));
}

TutorialGameLogic::TutorialGameLogic(Song *pSong, DataArray *pSongConfig)
    : GameLogic(pSong, pSongConfig, RandomInt(0, kMaxSeed)) {
    mBar = 0;
    mInputDisabled = false;
    mProgressOffset = 0.0f;
    mManualEnableNext = false;
    mRotateMode = kRotateModeNone;
    mProgressLength = kInitialProgressLength;
    mPowerupsDisabled = true;

    ScriptFunction::Register(OnMidiCommand, kMidiCommand, this);
    ScriptFunction::Register(OnStreamCommand, kStreamCommand, this);
    ScriptFunction::Register(OnLoopTrackCommand, kLoopTrackCommand, this);
    ScriptFunction::Register(OnDisableRotCommand, kDisableRotCommand, this);
    ScriptFunction::Register(OnDisableInputCommand, kDisableInputCommand, this);
    ScriptFunction::Register(OnTutorialPrintCommand, kTutorialPrintCommand, this);
    ScriptFunction::Register(OnDisablePowerupsCommand, kDisablePowerupsCommand, this);
    ScriptFunction::Register(OnActivateBarsCommand, kActivateBarsCommand, this);
    ScriptFunction::Register(OnSetSongPosParamsCommand, kSetSongPosParamsCommand, this);
    ScriptFunction::Register(OnCaptureTrackCommand, kCaptureTrackCommand, this);
    ScriptFunction::Register(OnCaptureAllTracksCommand, kCaptureAllTracksCommand, this);
    ScriptFunction::Register(OnSetPowerupCommand, kSetPowerupCommand, this);
    ScriptFunction::Register(OnMoveTrackCommand, kMoveTrackCommand, this);
    ScriptFunction::Register(OnSetNoCaptureCommand, kSetNoCaptureCommand, nullptr);
    ScriptFunction::Register(OnSetNoDeactivateCommand, kSetNoDeactivateCommand, nullptr);
    ScriptFunction::Register(OnSetNoSeekerCommand, kSetNoSeekerCommand, this);
    ScriptFunction::Register(OnSetNoStreaksCommand, kSetNoStreaksCommand, nullptr);
    ScriptFunction::Register(OnSetStrandBarsCommand, kSetStrandBarsCommand, nullptr);
    ScriptFunction::Register(OnSchedDeactivateTrackCommand, kSchedDeactivateTrackCommand, this);
    ScriptFunction::Register(OnShowStreakArrowCommand, kShowStreakArrowCommand, this);
    ScriptFunction::Register(OnSchedSetJuiceCommand, kSchedSetJuiceCommand, this);
    ScriptFunction::Register(OnSetAutopilotCommand, kSetAutopilotCommand, this);
    ScriptFunction::Register(OnSetManualEnableNextCommand, kSetManualEnableNextCommand, this);
    ScriptFunction::Register(OnEnableNextCommand, kEnableNextCommand, this);
    ScriptFunction::Register(OnSchedStageCompleteCommand, kSchedStageCompleteCommand, this);

    mStreams.mDirectory = FormatString(kStreamDirectoryFormat, TheGameDb->mSong.c_str());

    DataArray *pScript = pSongConfig->FindArray(kScriptEntry, false);
    const int nSize = pScript->mSize;
    for (int i = kFirstStep; i < nSize; ++i) {
        mScript.Add(CreateTask(pScript->Array(i)));
    }

    mSavedNoCapture = TheGameConfig->mNoCapture;
    mSavedNoDeactivate = TheGameConfig->mNoDeactivate;
    mSavedStreaksEnabled = TheGameConfig->mStreaksEnabled;
    mSavedGuideTicks = TheGameConfig->mGuideTicks;
    mSavedStrandBars = TheGameConfig->mStrandBars[TheGameDb->GetNumPlayers() - 1];
    TheGameConfig->mGuideTicks = false;

    gSavedInputMap = *TheGameDb->GetProfile(kTutorialPlayer)->GetInputMap();
    TheGameDb->GetProfile(kTutorialPlayer)->GetInputMap()->LoadDefaults();
}

TutorialGameLogic::~TutorialGameLogic() {
    const int nDeactivators = mDeactivators.size();
    for (int i = 0; i < nDeactivators; ++i) {
        TheSongScheduler.Cancel(mDeactivators[i].Get());
    }
    SetSongSpeed(kNormalSpeed);

    ScriptFunction::Unregister(OnMidiCommand);
    ScriptFunction::Unregister(OnStreamCommand);
    ScriptFunction::Unregister(OnLoopTrackCommand);
    ScriptFunction::Unregister(OnDisableRotCommand);
    ScriptFunction::Unregister(OnDisableInputCommand);
    ScriptFunction::Unregister(OnTutorialPrintCommand);
    ScriptFunction::Unregister(OnDisablePowerupsCommand);
    ScriptFunction::Unregister(OnActivateBarsCommand);
    ScriptFunction::Unregister(OnSetSongPosParamsCommand);
    ScriptFunction::Unregister(OnCaptureTrackCommand);
    ScriptFunction::Unregister(OnCaptureAllTracksCommand);
    ScriptFunction::Unregister(OnSetPowerupCommand);
    ScriptFunction::Unregister(OnMoveTrackCommand);
    ScriptFunction::Unregister(OnSetNoCaptureCommand);
    ScriptFunction::Unregister(OnSetNoDeactivateCommand);
    ScriptFunction::Unregister(OnSetNoSeekerCommand);
    ScriptFunction::Unregister(OnSetNoStreaksCommand);
    ScriptFunction::Unregister(OnSetStrandBarsCommand);
    ScriptFunction::Unregister(OnSchedDeactivateTrackCommand);
    ScriptFunction::Unregister(OnShowStreakArrowCommand);
    ScriptFunction::Unregister(OnSchedSetJuiceCommand);
    ScriptFunction::Unregister(OnSetAutopilotCommand);
    ScriptFunction::Unregister(OnSetManualEnableNextCommand);
    ScriptFunction::Unregister(OnEnableNextCommand);
    ScriptFunction::Unregister(OnSchedStageCompleteCommand);

    mScript.Stop();
    mStreams.Stop();

    TheGameConfig->mNoCapture = mSavedNoCapture;
    TheGameConfig->mNoDeactivate = mSavedNoDeactivate;
    TheGameConfig->mStreaksEnabled = mSavedStreaksEnabled;
    TheGameConfig->mGuideTicks = mSavedGuideTicks;
    TheGameConfig->SetStrandBars(TheGameDb->GetNumPlayers(), mSavedStrandBars);

    *TheGameDb->GetProfile(kTutorialPlayer)->GetInputMap() = gSavedInputMap;
}

void TutorialGameLogic::HandleInput(const RotateEvent &event) {
    if (mInputDisabled) {
        return;
    }
    switch (mRotateMode) {
    case kRotateModeNextOnly:
        if (event.mDirection != RotateEvent::kDirectionNext) {
            return;
        }
        break;
    case kRotateModePreviousOnly:
        if (event.mDirection != RotateEvent::kDirectionPrevious) {
            return;
        }
        break;
    case kRotateModeNone:
        return;
    case kRotateModeAll:
        break;
    default:
        DebugPrint(kBadRotateStateFormat, mRotateMode);
        return;
    }
    GameLogic::HandleInput(event);
}

void TutorialGameLogic::HandleInput(const PlayNoteEvent &event) {
    if (!mInputDisabled) {
        GameLogic::HandleInput(event);
    }
}

void TutorialGameLogic::HandleInput([[maybe_unused]] const BtnEvent<4> &event) {
}

void TutorialGameLogic::HandleInput(const BtnEvent<5> &event) {
    if (!mInputDisabled && !mPowerupsDisabled) {
        GameLogic::HandleInput(event);
    }
}

void TutorialGameLogic::SetSongPosParams(int nPlayedBars, int nTotalBars) {
    mProgressLength = static_cast<float>(nTotalBars * mTicksPerBar);
    mProgressOffset = static_cast<float>(nPlayedBars * mTicksPerBar - GetTick());
}

float TutorialGameLogic::GetProgress() {
    const float fProgress = (static_cast<float>(GetTick()) + mProgressOffset) / mProgressLength;
    if (fProgress < kNoProgress) {
        return kNoProgress;
    }
    if (kFullProgress < fProgress) {
        return kFullProgress;
    }
    return fProgress;
}

int TutorialGameLogic::GetTick() {
    return TheSongScheduler.GetClockTick();
}

float TutorialGameLogic::GetTime() {
    return TheSongScheduler.GetClockTime();
}

void TutorialGameLogic::Poll() {
    GameLogic::Poll();
    if (mState != kStatePlaying) {
        return;
    }
    if (mScript.Poll() == Task::kStateDone) {
        mScript.Stop();
        TheMetagame.ShowDialog(Metagame::kDialogTutorialEnd, OnDialog, this, kTutorialPad);
    }
    mStreams.Poll();
}

void TutorialGameLogic::OnStart() {
    EnableNextTracks(kFirstBar);
    mScript.Start();
}

void TutorialGameLogic::OnBar(int nBar) {
    if (nBar >= kFirstBar) {
        mBar = nBar;
    }
}

void TutorialGameLogic::OnPhraseCaptured([[maybe_unused]] CatchTrack *pTrack,
                                         [[maybe_unused]] Player *pPlayer,
                                         [[maybe_unused]] int nBar,
                                         [[maybe_unused]] bool bStreak) {
    if (!mManualEnableNext) {
        EnableNext();
    }
}

void TutorialGameLogic::EnableNext() {
    EnableNextTracks(std::max(TheSongScheduler.mTick / mTicksPerBar + 1, kFirstBar));
}

void TutorialGameLogic::OnPhraseEnded([[maybe_unused]] Track *pTrack) {
}

void TutorialGameLogic::SetPaused(bool bPaused, int nPad, int nReason) {
    GameLogic::SetPaused(bPaused, nPad, nReason);
    mStreams.SetPaused(bPaused);
    if (bPaused) {
        TheSongScheduler.Pause();
        AllNotesOff();
        TheMetagame.ShowDialog(nReason != 0 ? Metagame::kDialogNoController :
                                              Metagame::kDialogPause,
                               OnDialog,
                               this,
                               nPad);
    } else {
        TheSongScheduler.Resume();
    }
}

void TutorialGameLogic::OnDialog(Metagame::DialogAction action, void *pUserData) {
    auto *pLogic = static_cast<TutorialGameLogic *>(pUserData);
    switch (action) {
    case Metagame::kDialogActionResume:
        pLogic->SetPaused(false, kNoPad, kNoReason);
        break;
    case Metagame::kDialogActionQuit:
        pLogic->mQuit = action;
        pLogic->EndSong();
        break;
    case Metagame::kDialogActionEnd:
        pLogic->EndSong();
        break;
    default:
        DebugWarn(kIllegalDialogResponse);
        break;
    }
}

void TutorialGameLogic::PlayScriptTrack(const char *pszName) {
    ScriptTrackData *pTrack = mSong->GetScriptTrack(pszName);
    if (pTrack == nullptr) {
        DebugFail(kMissingScriptTrackFormat, pszName);
        return;
    }
    Ptr<PlayScriptCmd> cmd(new PlayScriptCmd(pTrack, mSong->GetMsPerTick()));
    mScriptCmds.push_back(cmd);
    cmd->mNextCommand = 0;
    cmd->mStartTick = TheSongScheduler.mTick;
    cmd->ScheduleNext();
}

void TutorialGameLogic::NotifyWhenDone(Task *pTask, const char *pszName) {
    for (const auto &cmd : mScriptCmds) {
        if (strcmp(cmd->mTrack->mName.c_str(), pszName) == 0) {
            cmd->mWaitingTask = pTask;
            return;
        }
    }
    ReportDone(pTask);
}

void TutorialGameLogic::QueueStream(const char *pszName, bool bSkipIfBusy) {
    mStreams.Enqueue(pszName, bSkipIfBusy);
}

void TutorialGameLogic::LoopTrack(int nStartBar, int nNumBars, int nBarsAhead) {
    mPlayMap->AddLoop(nStartBar, nNumBars, mBar + nBarsAhead);
    for (CatchTrack *pTrack : mCatchTracks) {
        pTrack->Loop(mBar + nBarsAhead);
    }
}

void TutorialGameLogic::SetInputDisabled(bool bDisabled) {
    mInputDisabled = bDisabled;
}

void TutorialGameLogic::SetRotateMode(int nMode) {
    mRotateMode = nMode;
}

void TutorialGameLogic::SetPowerupsDisabled(bool bDisabled) {
    mPowerupsDisabled = bDisabled;
}

void TutorialGameLogic::ActivateBars(int nBar, int nBars, int nTrack) {
    mCatchTracks[nTrack]->AssignBars(nBar, nBars, mPlayers[kTutorialPlayer]);
}

void TutorialGameLogic::ScheduleDeactivateTrack(int nBarsAhead, int nBar, int nBars) {
    const int nTick = TheSongScheduler.mTick - TheSongScheduler.mTick % mTicksPerBar +
                      (nBarsAhead * mTicksPerBar + mTicksPerBar);
    const int nTrack = mPlayers[kTutorialPlayer]->GetTrack()->mIndex;
    auto *pDeactivator = new TrackDeactivator(nTrack, nBar, nBars, mCatchTracks[nTrack]);
    mDeactivators.push_back(Ptr<Command>(pDeactivator));
    TheSongScheduler.PostAt(pDeactivator, nTick, false);
}

void TutorialGameLogic::CaptureTrack() {
    const int nTrack = mPlayers[kTutorialPlayer]->GetTrack()->mIndex;
    mCatchTracks[nTrack]->Capture(mBar, mPlayers[kTutorialPlayer]);
}

void TutorialGameLogic::CaptureAllTracks() {
    const int nBar = TheSongScheduler.mTick / mTicksPerBar;
    const int nTracks = mCatchTracks.size();
    for (int i = 0; i < nTracks; ++i) {
        mCatchTracks[i]->Capture(nBar, mPlayers[kTutorialPlayer]);
    }
}

void TutorialGameLogic::PlacePowerup(int nPowerup, int nTrack, int nBar) {
    mCatchTracks[nTrack]->SetBarPowerup(nBar, nPowerup);
}

void TutorialGameLogic::MoveTrack(int nTrack) {
    mTrackSelector->MovePlayer(kTutorialPlayer, nTrack);
}

void TutorialGameLogic::SetAutopilot(bool bAutopilot) {
    const int nTrack = mPlayers[kTutorialPlayer]->GetTrack()->mIndex;
    mCatchTracks[nTrack]->SetAutopilot(bAutopilot);
}

void TutorialGameLogic::SetNoSeeker(bool bNoSeeker) {
    const int nTrack = mPlayers[kTutorialPlayer]->GetTrack()->mIndex;
    mCatchTracks[nTrack]->SetNoSeeker(bNoSeeker);
}

void TutorialGameLogic::SetManualEnableNext(bool bManual) {
    mManualEnableNext = bManual;
}

Task *TutorialGameLogic::CreateTask(DataArray *pStep) {
    const char *pszType = pStep->Sym(kStepNodeType);
    if (strcmp(pszType, kWaitStreamStep) == 0) {
        return new WaitForTaskTask(pStep->Sym(kStepNodeArgument), &mStreams);
    }
    if (strcmp(pszType, kWaitMidiStep) == 0) {
        return new WaitForTaskTask(pStep->Sym(kStepNodeArgument), this);
    }
    if (strcmp(pszType, kWaitTimeStep) == 0) {
        return new WaitTimeTask(pStep->Float(kStepNodeArgument));
    }
    if (strcmp(pszType, kWaitInteractiveStep) == 0) {
        return new WaitInteractiveTask(pStep, this);
    }
    return new ScriptTask(pStep);
}

bool TutorialGameLogic::IsPhraseAbsent() {
    const int nBar = TheSongScheduler.mTick / mTicksPerBar;
    const int nTrack = mPlayers[kTutorialPlayer]->GetTrack()->mIndex;
    int nStartTick;
    int nEndTick;
    return !mCatchTracks[nTrack]->FindPhrase(nBar, &nStartTick, &nEndTick);
}

void TutorialGameLogic::ShowStreakArrow(int nTrack, int nBarsAhead, [[maybe_unused]] int nUnused) {
    const int nBar = TheSongScheduler.mFrameTick / mTicksPerBar + nBarsAhead;
    int nStartTick;
    int nEndTick;
    // Yes, the binary discards the result and reads the ticks either way.
    (void)mCatchTracks[nTrack]->FindPhrase(nBar, &nStartTick, &nEndTick);
    TheGfxManager.ShowNextPhrase(nTrack, static_cast<float>(nStartTick), nEndTick);
}

void TutorialGameLogic::ScheduleSetJuice(int nBarsAhead, float fJuice) {
    // The binary passes the tick through a float.
    const float fTick = static_cast<float>(BarTickAhead(nBarsAhead, mTicksPerBar));
    Ptr<Command> cmd(new SetJuiceCmd(fJuice));
    TheSongScheduler.PostAt(cmd.Get(), static_cast<int>(fTick), false);
}

void TutorialGameLogic::ScheduleStageComplete(int nBarsAhead) {
    // The binary passes the tick through a float.
    const float fTick = static_cast<float>(BarTickAhead(nBarsAhead, mTicksPerBar));
    Ptr<Command> cmd(new StageCompleteCmd());
    TheSongScheduler.PostAt(cmd.Get(), static_cast<int>(fTick), false);
}

void TutorialGameLogic::DispatchPriv(Message *pMsg) {
    (void)pMsg->Type(); // Yes, the binary discards this call's result.
    WorldLogic::DispatchPriv(pMsg);
}
