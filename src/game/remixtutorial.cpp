#include "game/remixtutorial.h"

#include <cstdlib>
#include <cstring>

#include "game/gamecallback.h"
#include "game/gamedb.h"
#include "game/inputmap.h"
#include "game/scripttask.h"
#include "game/waitfortasktask.h"
#include "game/waitinteractiveremixtask.h"
#include "game/waittimetask.h"
#include "os/string.h"
#include "os/system.h"
#include "script/scriptfunction.h"
#include "synth/fxmidi.h"

namespace {

constexpr char kDisableRotCommand[] = "disable_rot";
constexpr char kDisablePitchingCommand[] = "disable_pitching";
constexpr char kDisableHudCommand[] = "disable_hud";
constexpr char kStreamCommand[] = "stream";
constexpr char kPrintCommand[] = "print";
constexpr char kEnableLoopingCommand[] = "enable_looping";
constexpr char kDisableSectionChangeCommand[] = "disable_section_change";
constexpr char kDisableErasingCommand[] = "disable_erasing";
constexpr char kClearGemsCommand[] = "clear_gems";
constexpr char kEnableMenuCommand[] = "enable_menu";
constexpr char kShowPatternsCommand[] = "show_patterns";
constexpr char kBurnPatternsCommand[] = "burn_patterns";
constexpr char kAllowTempoChangeCommand[] = "allow_tempo_change";
constexpr char kTempoOpenCommand[] = "tempo_open";
constexpr char kAdjustTempoCommand[] = "adjust_tempo";
constexpr char kAllowLoopingCommand[] = "allow_looping";
constexpr char kAllowPatternCancelCommand[] = "allow_pattern_cancel";
constexpr char kAllowRotationLeftCommand[] = "allow_rotation_left";

constexpr char kNoQueueOption[] = "no_queue";
constexpr char kStreamDirectoryFormat[] = "Songs/%s/";
constexpr char kDbSection[] = "db";
constexpr char kSongsEntry[] = "songs";
constexpr char kTutorialEntry[] = "Tutorial";
constexpr char kScriptEntry[] = "script";
constexpr char kWaitStreamStep[] = "wait_stream";
constexpr char kWaitTimeStep[] = "wait_time";
constexpr char kWaitInteractiveStep[] = "wait_interactive";

// A "stream" command with a third node has an option there.
constexpr int kStreamCommandWithOptionSize = 3;

// Nodes of a command. The command name is node 0.
enum CommandNode {
    kCommandNodeArgument = 1,
    kCommandNodeOption = 2,
};

// Nodes of a script step.
enum StepNode {
    kStepNodeType = 0,
    kStepNodeArgument = 1,
};

// The value of mLastRotation before NotifyRotate() records one.
constexpr int kNoRotation = -1;

// The first player, whose controller bindings the tutorial replaces.
constexpr int kTutorialPlayer = 0;

// The lane "enable_looping" acts on.
constexpr int kLoopingLane = 0;

// A "show_patterns" style command acts only on this exact argument.
constexpr int kCommandOn = 1;

// NTSC-U/C: 0x00436148
InputMap gSavedInputMap;

} // namespace

void RemixTutorial::OnStream(DataArray *pCommand, void *pUserData) {
    bool bSkipIfBusy = false;
    if (pCommand->mSize == kStreamCommandWithOptionSize) {
        bSkipIfBusy = strcmp(pCommand->Sym(kCommandNodeOption), kNoQueueOption) == 0;
    }
    static_cast<RemixTutorial *>(pUserData)->QueueStream(pCommand->Sym(kCommandNodeArgument),
                                                         bSkipIfBusy);
}

void RemixTutorial::OnEnableLooping(DataArray *pCommand, void *pUserData) {
    static_cast<RemixTutorial *>(pUserData)->mLogic->SetLooping(
        kLoopingLane, pCommand->Int(kCommandNodeArgument) != 0);
}

void RemixTutorial::OnPrint([[maybe_unused]] DataArray *pCommand,
                            [[maybe_unused]] void *pUserData) {
}

void RemixTutorial::OnDisableRot(DataArray *pCommand, void *pUserData) {
    static_cast<RemixTutorial *>(pUserData)->mRotationEnabled =
        pCommand->Int(kCommandNodeArgument) == 0;
}

void RemixTutorial::OnDisablePitching(DataArray *pCommand, void *pUserData) {
    static_cast<RemixTutorial *>(pUserData)->mPitchingEnabled =
        pCommand->Int(kCommandNodeArgument) == 0;
}

void RemixTutorial::OnDisableSectionChange(DataArray *pCommand, void *pUserData) {
    static_cast<RemixTutorial *>(pUserData)->mSectionChangeEnabled =
        pCommand->Int(kCommandNodeArgument) == 0;
}

void RemixTutorial::OnDisableHud(DataArray *pCommand, void *pUserData) {
    static_cast<RemixTutorial *>(pUserData)->mHudEnabled = pCommand->Int(kCommandNodeArgument) == 0;
}

void RemixTutorial::OnDisableErasing(DataArray *pCommand, void *pUserData) {
    static_cast<RemixTutorial *>(pUserData)->mErasingEnabled =
        pCommand->Int(kCommandNodeArgument) == 0;
}

void RemixTutorial::OnAllowLooping(DataArray *pCommand, void *pUserData) {
    static_cast<RemixTutorial *>(pUserData)->mLoopingAllowed =
        pCommand->Int(kCommandNodeArgument) == kCommandOn;
}

void RemixTutorial::OnClearGems([[maybe_unused]] DataArray *pCommand, void *pUserData) {
    static_cast<RemixTutorial *>(pUserData)->ClearGems();
}

void RemixTutorial::OnShowPatterns(DataArray *pCommand, void *pUserData) {
    static_cast<RemixTutorial *>(pUserData)->mShowPatterns =
        pCommand->Int(kCommandNodeArgument) == kCommandOn;
}

void RemixTutorial::OnBurnPatterns(DataArray *pCommand, void *pUserData) {
    static_cast<RemixTutorial *>(pUserData)->mBurnPatterns =
        pCommand->Int(kCommandNodeArgument) == kCommandOn;
}

void RemixTutorial::OnEnableMenu(DataArray *pCommand, void *pUserData) {
    static_cast<RemixTutorial *>(pUserData)->EnableMenu(pCommand->Int(kCommandNodeArgument) ==
                                                        kCommandOn);
}

void RemixTutorial::OnAllowTempoChange(DataArray *pCommand, void *pUserData) {
    static_cast<RemixTutorial *>(pUserData)->mTempoChangeAllowed =
        pCommand->Int(kCommandNodeArgument) == kCommandOn;
}

void RemixTutorial::OnTempoOpen(DataArray *pCommand, void *pUserData) {
    static_cast<RemixTutorial *>(pUserData)->OpenTempo(pCommand->Int(kCommandNodeArgument) ==
                                                       kCommandOn);
}

void RemixTutorial::OnAdjustTempo(DataArray *pCommand, void *pUserData) {
    static_cast<RemixTutorial *>(pUserData)->AdjustTempo(pCommand->Int(kCommandNodeArgument));
}

void RemixTutorial::OnAllowPatternCancel(DataArray *pCommand, void *pUserData) {
    static_cast<RemixTutorial *>(pUserData)->AllowPatternCancel(
        pCommand->Int(kCommandNodeArgument) == kCommandOn);
}

void RemixTutorial::OnAllowRotationLeft(DataArray *pCommand, void *pUserData) {
    static_cast<RemixTutorial *>(pUserData)->mRotationLeftAllowed =
        pCommand->Int(kCommandNodeArgument) == kCommandOn;
}

RemixTutorial::RemixTutorial(RemixLogic *pLogic) {
    mLastRotation = kNoRotation;
    mRotationLeftAllowed = true;
    mPitchingEnabled = false;
    mRotationEnabled = false;
    mSectionChangeEnabled = false;
    mHudEnabled = false;
    mErasingEnabled = false;
    mReserved5C = 0;
    mReserved60 = 0;
    mReserved64 = 0;
    mReserved68 = 0;
    mBurnPatterns = false;
    mShowPatterns = false;
    mTempoChangeAllowed = false;
    mLoopingAllowed = false;
    mPaused = false;
    mLogic = pLogic;

    ScriptFunction::Register(OnDisableRot, kDisableRotCommand, this);
    ScriptFunction::Register(OnDisablePitching, kDisablePitchingCommand, this);
    ScriptFunction::Register(OnDisableHud, kDisableHudCommand, this);
    ScriptFunction::Register(OnStream, kStreamCommand, this);
    ScriptFunction::Register(OnPrint, kPrintCommand, this);
    ScriptFunction::Register(OnEnableLooping, kEnableLoopingCommand, this);
    ScriptFunction::Register(OnDisableSectionChange, kDisableSectionChangeCommand, this);
    ScriptFunction::Register(OnDisableErasing, kDisableErasingCommand, this);
    ScriptFunction::Register(OnClearGems, kClearGemsCommand, this);
    ScriptFunction::Register(OnEnableMenu, kEnableMenuCommand, this);
    ScriptFunction::Register(OnShowPatterns, kShowPatternsCommand, this);
    ScriptFunction::Register(OnBurnPatterns, kBurnPatternsCommand, this);
    ScriptFunction::Register(OnAllowTempoChange, kAllowTempoChangeCommand, this);
    ScriptFunction::Register(OnTempoOpen, kTempoOpenCommand, this);
    ScriptFunction::Register(OnAdjustTempo, kAdjustTempoCommand, this);
    ScriptFunction::Register(OnAllowLooping, kAllowLoopingCommand, this);
    ScriptFunction::Register(OnAllowPatternCancel, kAllowPatternCancelCommand, this);
    ScriptFunction::Register(OnAllowRotationLeft, kAllowRotationLeftCommand, this);

    mStreams.mDirectory = FormatString(kStreamDirectoryFormat, TheGameDb->mSong.c_str());

    DataArray *pScript = SystemConfig()
                             ->FindArray(kDbSection, false)
                             ->FindArray(kSongsEntry, false)
                             ->FindArray(kTutorialEntry, false)
                             ->FindArray(TheGameDb->mSong.c_str(), false)
                             ->FindArray(kScriptEntry, false);
    const int nSize = pScript->mSize;
    for (int i = 1; i < nSize; ++i) {
        mScript.Add(CreateTask(pScript->Array(i)));
    }

    gSavedInputMap = *TheGameDb->GetProfile(kTutorialPlayer)->GetInputMap();
    TheGameDb->GetProfile(kTutorialPlayer)->GetInputMap()->LoadDefaults();
}

RemixTutorial::~RemixTutorial() {
    ScriptFunction::Unregister(OnDisableRot);
    ScriptFunction::Unregister(OnDisablePitching);
    ScriptFunction::Unregister(OnDisableHud);
    ScriptFunction::Unregister(OnStream);
    ScriptFunction::Unregister(OnPrint);
    ScriptFunction::Unregister(OnEnableLooping);
    ScriptFunction::Unregister(OnDisableSectionChange);
    ScriptFunction::Unregister(OnDisableErasing);
    ScriptFunction::Unregister(OnClearGems);
    ScriptFunction::Unregister(OnEnableMenu);
    ScriptFunction::Unregister(OnShowPatterns);
    ScriptFunction::Unregister(OnBurnPatterns);
    ScriptFunction::Unregister(OnAllowTempoChange);
    ScriptFunction::Unregister(OnTempoOpen);
    ScriptFunction::Unregister(OnAdjustTempo);
    ScriptFunction::Unregister(OnAllowLooping);
    ScriptFunction::Unregister(OnAllowPatternCancel);
    ScriptFunction::Unregister(OnAllowRotationLeft);
    mScript.Stop();
    mStreams.Stop();
    *TheGameDb->GetProfile(kTutorialPlayer)->GetInputMap() = gSavedInputMap;
}

void RemixTutorial::Start() {
    mScript.Start();
}

bool RemixTutorial::Poll() {
    if (mPaused) {
        return true;
    }

    bool bRunning = true;
    if (mScript.Poll() == Task::kStateDone) {
        mScript.Stop();
        bRunning = false;
    }
    mStreams.Poll();
    return bRunning;
}

void RemixTutorial::SetPaused(bool bPaused) {
    mPaused = bPaused;
    mStreams.SetPaused(bPaused);
}

void RemixTutorial::NotifyPitch() {
    if (TheGameCallback != nullptr) {
        TheGameCallback->OnPitch();
    }
}

void RemixTutorial::NotifyLoop() {
    if (TheGameCallback != nullptr) {
        TheGameCallback->OnLoop();
    }
}

void RemixTutorial::NotifySectionChange(int nSection) {
    if (TheGameCallback != nullptr) {
        TheGameCallback->OnSectionChange(nSection);
    }
}

void RemixTutorial::NotifyRotate(bool bRight, int nRotation) {
    if ((mLastRotation != nRotation) && (TheGameCallback != nullptr)) {
        TheGameCallback->OnRotate(bRight);
    }
    mLastRotation = nRotation;
}

Task *RemixTutorial::CreateTask(DataArray *pStep) {
    const char *pszType = pStep->Sym(kStepNodeType);
    if (strcmp(pszType, kWaitStreamStep) == 0) {
        return new WaitForTaskTask(pStep->Sym(kStepNodeArgument), &mStreams);
    }
    if (strcmp(pszType, kWaitTimeStep) == 0) {
        return new WaitTimeTask(pStep->Float(kStepNodeArgument));
    }
    if (strcmp(pszType, kWaitInteractiveStep) == 0) {
        return new WaitInteractiveRemixTask(pStep, this);
    }
    return new ScriptTask(pStep);
}

void RemixTutorial::QueueStream(const char *pszName, bool bSkipIfBusy) {
    mStreams.Enqueue(pszName, bSkipIfBusy);
}

void RemixTutorial::ClearGems() {
    mLogic->ClearGems();
}

void RemixTutorial::EnableMenu([[maybe_unused]] bool bEnable) {
    mLogic->OpenMenu(); // Yes, the binary opens the menu whatever the argument.
}

void RemixTutorial::OpenTempo(bool bOpen) {
    if (bOpen) {
        mLogic->OpenTempo();
    } else {
        mLogic->CloseTempo();
    }
}

void RemixTutorial::AdjustTempo(int nSteps) {
    for (int i = std::abs(nSteps); i > 0; --i) {
        if (nSteps < 0) {
            mLogic->LowerTempo();
        } else {
            mLogic->RaiseTempo();
        }
    }
}

void RemixTutorial::PlaySound() {
    FxMidi::PlaySound1();
}

void RemixTutorial::AllowPatternCancel([[maybe_unused]] bool bAllow) {
}
