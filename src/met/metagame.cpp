#include "met/metagame.h"

#include <algorithm>
#include <cstring>
#include <vector>

#include "app/cutscene.h"
#include "game/gamedb.h"
#include "game/gamefx.h"
#include "game/playerprofile.h"
#include "game/triggermgr.h"
#include "math/color.h"
#include "math/rand.h"
#include "memcard/mcmanager.h"
#include "met/freqconfirmscreen.h"
#include "met/freqpanel.h"
#include "met/freqscreen.h"
#include "met/introscreen.h"
#include "met/metaarenascreen.h"
#include "met/metagameutil.h"
#include "met/metamainscreen.h"
#include "met/metaskillscreen.h"
#include "met/metasongscreen.h"
#include "met/metastartscreen.h"
#include "met/modescreen.h"
#include "met/sharedmusic.h"
#include "met/songpicpanel.h"
#include "met/songpreview.h"
#include "met/transitionscreen.h"
#include "netflow/lobbymsgtypes.h"
#include "netflow/netlobby.h"
#include "os/debug.h"
#include "os/file.h"
#include "os/joypad.h"
#include "os/keyboard.h"
#include "os/locale.h"
#include "os/scheduler.h"
#include "os/system.h"
#include "os/timer.h"
#include "rnd/manager.h"
#include "rnd/mat.h"
#include "rnd/ps.h"
#include "rnd/rndrenderer.h"
#include "rnd/text.h"
#include "rnd/view.h"
#include "script/dataarray.h"
#include "script/scriptfunction.h"
#include "synth/synth.h"
#include "ui/uimanager.h"

Metagame TheMetagame;

namespace {

// Configuration sections and keys.
constexpr char kMetagameKey[] = "metagame";
constexpr char kGameKey[] = "game";
constexpr char kGameFxBankSlotKey[] = "game_fx_bank_slot";
constexpr char kMetaFxBankSlotKey[] = "meta_fx_bank_slot";
constexpr char kMusicSharedBankSlotKey[] = "music_shared_bank_slot";
constexpr char kMusicSwapBankSlotKey[] = "music_swap_bank_slot";
constexpr char kEffectsKey[] = "effects";
constexpr char kStartFileKey[] = "start_file";
constexpr char kFreqScreenKey[] = "freq_screen";
constexpr char kUiCameraKey[] = "ui_camera";
constexpr char kUiEnvironKey[] = "ui_environ";
constexpr char kPanelGizmoAnimEndKey[] = "panel_gizmo_anim_end";
constexpr char kSpeedUpKey[] = "speed_up";
constexpr char kStopSpeedUpKey[] = "stop_speed_up";
constexpr char kTransitionFxBankFileKey[] = "transition_fx_bank_file";
constexpr char kSoloGameFxBankFileKey[] = "solo_game_fx_bank_file";
constexpr char kMultiGameFxBankFileKey[] = "multi_game_fx_bank_file";
constexpr char kRemixFxBankFileKey[] = "remix_fx_bank_file";
constexpr char kMusicSharedBankFileKey[] = "music_shared_bank_file";
constexpr char kMetaFxBankFileKey[] = "meta_fx_bank_file";
constexpr char kSongsKey[] = "songs";

// Script commands.
constexpr char kMetaStartGameCommand[] = "meta_start_game";
constexpr char kMetaChangeScreenCommand[] = "meta_change_screen";
constexpr char kMetaChangeMixCommand[] = "meta_change_mix";
constexpr char kMetaStopMixCommand[] = "meta_stop_mix";
constexpr char kEnterTubeCommand[] = "enter_tube";
constexpr char kExitTubeCommand[] = "exit_tube";
constexpr char kUnlockAllCommand[] = "unlock_all";
constexpr char kSoloSongScreenCommand[] = "solo_song_screen";

// Screen groups, panels, and screens.
constexpr char kAlwaysGroup[] = "always";
constexpr char kIntroGroup[] = "intro";
constexpr char kFrontEndGroup[] = "front_end";
constexpr char kHelpPanelName[] = "help";
constexpr char kTitlePanelName[] = "title";
constexpr char kPreLogoScreen[] = "pre_logo";
constexpr char kBlankScreen[] = "blank";
constexpr char kTrainingLeaveGizmoScreen[] = "training_leave_gizmo";
constexpr char kJustMtvLeaveGizmoScreen[] = "just_mtv_leave_gizmo";
constexpr char kJustMtvScreen[] = "just_mtv";
constexpr char kPreGame2MetaScreen[] = "pre_game2meta";
constexpr char kInitialMcCheckScreen[] = "initial_mc_check";
constexpr char kBackToShellScreen[] = "back_to_shell";
constexpr char kLaunchScreen[] = "d_launch";
constexpr char kLaunchTrainingScreen[] = "d_launch_training";
constexpr char kStartScreen[] = "start";
constexpr char kHostLaunchpadScreen[] = "fn_h_lpad";
constexpr char kGuestLaunchpadScreen[] = "fn_g_lpad";
constexpr char kLobbyErrorScreen[] = "lobby_error";
constexpr char kLaunchpadErrorScreen[] = "lpad_error";
constexpr char kUnlockArenaAnimScreen[] = "unlockarena_anim";

// The screens after a song that show the player's character on the gizmo again.
constexpr const char *kReturnScreens[] = {
    "game2solometa",
    "game2soloremix",
    "game2soloremix_mode",
    "game2soloremix_mode_save",
    "game2soloarena",
    "game2soloarena_save",
    "game2solocustom",
    "game2netlaunch_host",
    "game2netlaunch_guest",
    "game2net_end_remix",
    "game2net_end_save_read_only",
    "game2lost_launchpad",
    "game2lost_lobby",
    "game2unlockarena",
};

// Renderer objects.
constexpr char kLoadingScreenFormat[] = "metagame\\loading%d.rnd";
constexpr char kLoadingViewName[] = "loading.view";
constexpr char kLoadingMatName[] = "loading.mat";
constexpr char kTitleTextName[] = "title_title.txt";
constexpr char kTitleViewName[] = "title.view";
constexpr char kIntroMovie[] = "amp.pss";
constexpr char kUnknownModeMessage[] = "unknown mode";

constexpr int kLoadingScreenCount = 4;

// The loading screen darkens by this much each frame while the screen groups load.
constexpr float kLoadingFadeStep = 0.032f;

// The title text measures between these widths, and the title view runs through 100 frames to
// fit it.
constexpr float kTitleMinWidth = 119.0f;
constexpr float kTitleWidthRange = 314.0f;
constexpr float kTitleFrames = 100.0f;

// Controllers the front end gives the menus to.
constexpr int kMenuPads = 4;

// The milliseconds the loading screen of a song shows for.
constexpr float kLeaveDelayMs = 400.0f;

// The front end clock runs at 960 ticks a second.
constexpr float kTickMs = 1000.0f / 960.0f;

// The sound effects of the game and the menu music share seven bank slots.
constexpr int kBankSlotCount = 7;

// Before the intro movie every MIDI channel is set to volume 0.
constexpr int kMidiChannelCount = 15;
constexpr unsigned char kMidiControlChange = 0xb0;
constexpr unsigned char kMidiControlVolume = 7;
constexpr int kStreamSilent = 1;

// The menu music mixes of the song screen, while no clip plays and before a clip starts.
constexpr int kIdleMix = 7;
constexpr int kIdleMixTick = 1920;
constexpr int kClipMix = 9;
constexpr int kClipMixTick = 500;

// NTSC-U/C: 0x001657c0, PAL: 0x001686d0
void MetaStartGame([[maybe_unused]] DataArray *pCommand, [[maybe_unused]] void *pUserData) {
    if (TheMetagame.mState == Metagame::kStateFrontEnd) {
        TheMetagame.mEvent = Metagame::kEventStartGame;
    }
}

// NTSC-U/C: 0x001657e0, PAL: 0x001686f0
void MetaChangeScreen(DataArray *pCommand, [[maybe_unused]] void *pUserData) {
    if (TheMetagame.mState == Metagame::kStateFrontEnd) {
        TheUI.GotoScreen(pCommand->Sym(1));
    }
}

// NTSC-U/C: 0x00165828, PAL: 0x00168738 (stub)
void SoloSongScreen([[maybe_unused]] DataArray *pCommand, [[maybe_unused]] void *pUserData) {
}

// NTSC-U/C: 0x00168bb0, PAL: 0x0016bd38
void MetaChangeMix(DataArray *pCommand, [[maybe_unused]] void *pUserData) {
    constexpr int kTimedCommandSize = 3;
    const int nMix = pCommand->Int(1);
    FadeOutSharedMusic();
    if (pCommand->mSize < kTimedCommandSize) {
        TheMetagame.mMusic->ChangeMix(nMix);
    } else {
        TheMetagame.mMusic->SwitchMix(nMix,
                                      static_cast<int>(pCommand->Float(2) / *TheMetagame.mTickMs));
    }
}

// NTSC-U/C: 0x00168c40, PAL: 0x0016bdc8
void MetaStopMix([[maybe_unused]] DataArray *pCommand, [[maybe_unused]] void *pUserData) {
    TheMetagame.mMusic->Stop();
    FadeInSharedMusic();
}

// NTSC-U/C: 0x00168c70, PAL: 0x0016bdf8
void EnterTube([[maybe_unused]] DataArray *pCommand, [[maybe_unused]] void *pUserData) {
    TheMetagame.mMusic->EnterTube();
}

// NTSC-U/C: 0x00168c98, PAL: 0x0016be20
void ExitTube([[maybe_unused]] DataArray *pCommand, [[maybe_unused]] void *pUserData) {
    TheMetagame.mMusic->ExitTube(1.0f < TheMetagame.mSpeed);
}

// NTSC-U/C: 0x00168ce0, PAL: 0x0016be68 (stub)
void UnlockAll([[maybe_unused]] DataArray *pCommand, [[maybe_unused]] void *pUserData) {
}

// Report whether a pair of screen names is listed in an entry of the metagame configuration.
inline bool IsTransitionListed(const char *pszKey, const char *pszFrom, const char *pszTo) {
    DataArray *pList = SystemConfig()->FindArray(kMetagameKey, true)->FindArray(pszKey, true);
    bool bListed = false;
    for (int i = 1; i < pList->mSize; ++i) {
        DataArray *pPair = pList->Array(i);
        if (strcmp(pPair->Sym(0), pszFrom) == 0 && strcmp(pPair->Sym(1), pszTo) == 0) {
            bListed = true;
            break;
        }
    }
    return bListed;
}

} // namespace

Metagame::Metagame() {
    mState = kStateFrontEnd;
    mLoadStage = kLoadStageAlways;
    mLoadingScreenLoader = nullptr;
    mEvent = kEventNone;
    mTime = 0.0f;
    mLastSchedulerTime = 0.0f;
    mLeaveTime = 0.0f;
    mDialogShowing = 0;
    mDialogCallback = nullptr;
    mMusic = nullptr;
    mMusicSong = nullptr;
    mLastSelectButton = kPadNone;
    mFirstBoot = 1;
    mReloadFrontEnd = 1;
    mLeaveScreenShown = 1;
    mReservedDC = -1;
    mGameFxBankSlot = -1;
    mMetaFxBankSlot = -1;
    mMusicSharedBankSlot = -1;
    mMusicSwapBankSlot = -1;
    mTickMs = nullptr;
    mSharedMusicFadedIn = 0;
    mFreqsOnCard = 0;
    mReserved144 = 1;
    mSpeed = 1.0f;
    mGizmo = nullptr;
    mUiCamera = nullptr;
    mUiEnviron = nullptr;
    mSpeedingUp = 0;
    mNetScreenPending = 0;
}

Metagame::~Metagame() {
}

void Metagame::RegisterScreenClasses() {
    DataArray *pMetagame = SystemConfig()->FindArray(kMetagameKey, true);
    FreqPanel::Init(pMetagame);
    // Only the classes ported so far are registered. The binary registers 121 screen classes,
    // 31 panel classes, and 12 component classes here, in the order of the screen file types.
    TheUI.RegisterScreenType(FreqScreen::New, "freq_screen");
    TheUI.RegisterScreenType(TransitionScreen::New, "transition_screen");
    TheUI.RegisterScreenType(IntroScreen::New, "intro_screen");
    TheUI.RegisterScreenType(MetaStartScreen::New, "meta_start_screen");
    TheUI.RegisterScreenType(MetaMainScreen::New, "meta_main_screen");
    TheUI.RegisterScreenType(ModeScreen::New, "mode_screen");
    TheUI.RegisterScreenType(MetaSkillScreen::New, "meta_skill_screen");
    TheUI.RegisterScreenType(MetaArenaScreen::New, "meta_arena_screen");
    TheUI.RegisterScreenType(MetaSongScreen::New, "meta_song_screen");
    TheUI.RegisterScreenType(FreqConfirmScreen::New, "confirm_screen");
    TheUI.RegisterPanelType(FreqPanel::New, "freq_panel");
    TheUI.RegisterPanelType(SongPicPanel::New, "song_pic_panel");
    Gizmo::Init(pMetagame);
    TheMetagame.CreateGizmo();
    LoadSharedMusic();
    TheMetagame.mSelectedArena = "Constructo";
}

void Metagame::CreateGizmo() {
    mGizmo = new Gizmo(0);
}

void Metagame::ShowLoadingScreen() {
    const int nScreen = RandomInt(0, kLoadingScreenCount) + 1;
    mLoadingScreenLoader = Rnd::TheManager.AddLoader(
        FormatString(kLoadingScreenFormat, nScreen), RndLoader::kDeleteObjects, nullptr, nullptr);
    Rnd::View *pView = dynamic_cast<Rnd::View *>(Rnd::TheManager.Find(kLoadingViewName));

    // The screen is drawn into both frame buffers.
    TheRnd->BeginFrame();
    pView->Draw();
    TheRnd->EndFrame();
    TheRnd->BeginFrame();
    pView->Draw();
    TheRnd->EndFrame();
}

void Metagame::ShowUnlockedArenas() {
    mArena->ShowUnlocks();
}

void Metagame::Init() {
    DataArray *pMetagame = SystemConfig()->FindArray(kMetagameKey, true);
    DataArray *pGame = SystemConfig()->FindArray(kGameKey, false);
    pGame->FindInt(kGameFxBankSlotKey, &mGameFxBankSlot, true);
    pMetagame->FindInt(kMetaFxBankSlotKey, &mMetaFxBankSlot, true);
    pMetagame->FindInt(kMusicSharedBankSlotKey, &mMusicSharedBankSlot, true);
    pMetagame->FindInt(kMusicSwapBankSlotKey, &mMusicSwapBankSlot, true);
    mEffects.Load(SystemConfig()->FindArray(kMetagameKey, false)->FindArray(kEffectsKey, true));

    TheUI.Init();
    RegisterScreenClasses();
    FindWinnerMaterial();
    TheNetLobby->SetSink(this);

    const char *pszStartFile = nullptr;
    pMetagame->FindSymbol(kStartFileKey, &pszStartFile, true);
    TheUI.LoadFile(pszStartFile, false);
    TheUI.LoadGroup(kAlwaysGroup);

    DataArray *pFreqScreen = pMetagame->FindArray(kFreqScreenKey, true);
    const char *pszObject;
    if (pFreqScreen->FindSymbol(kUiCameraKey, &pszObject, false)) {
        mUiCamera = dynamic_cast<Rnd::Cam *>(Rnd::TheManager.Find(pszObject));
    }
    if (pFreqScreen->FindSymbol(kUiEnvironKey, &pszObject, false)) {
        mUiEnviron = dynamic_cast<Rnd::Environ *>(Rnd::TheManager.Find(pszObject));
    }

    TheUI.AddSink(this);
    JoypadAddSink(this);
    KeyboardAddSink(&TheUI);
    for (int i = 0; i < kMenuPads; ++i) {
        JoypadSetMenuControl(i, true);
    }

    mArena = new MetagameArena;
    ScriptFunction::Register(MetaStartGame, kMetaStartGameCommand, nullptr);
    ScriptFunction::Register(MetaChangeScreen, kMetaChangeScreenCommand, nullptr);
    ScriptFunction::Register(MetaChangeMix, kMetaChangeMixCommand, nullptr);
    ScriptFunction::Register(MetaStopMix, kMetaStopMixCommand, nullptr);
    ScriptFunction::Register(EnterTube, kEnterTubeCommand, nullptr);
    ScriptFunction::Register(ExitTube, kExitTubeCommand, nullptr);
    ScriptFunction::Register(UnlockAll, kUnlockAllCommand, nullptr);
    ScriptFunction::Register(SoloSongScreen, kSoloSongScreenCommand, nullptr);
    mNextScreen = kPreLogoScreen;
    TheMCManager.Init();
}

void Metagame::Terminate() {
    ScriptFunction::Unregister(MetaStartGame);
    ScriptFunction::Unregister(MetaChangeScreen);
    ScriptFunction::Unregister(MetaChangeMix);
    ScriptFunction::Unregister(MetaStopMix);
    ScriptFunction::Unregister(EnterTube);
    ScriptFunction::Unregister(ExitTube);
    ScriptFunction::Unregister(UnlockAll);
    ScriptFunction::Unregister(SoloSongScreen);
    TheMCManager.Terminate();
    delete mArena;
    delete mGizmo;
    delete mMusicSong;
    delete mTickMs;
    delete mMusic;
    UnloadSharedMusic();
    KeyboardRemoveSink(&TheUI);
    JoypadRemoveSink(this);
    TheUI.RemoveSink(this);
    TheUI.Terminate();
}

int Metagame::GetState() const {
    return mState;
}

const char *Metagame::SelectedArenaName() {
    return ArenaName(mSelectedArena.c_str());
}

const char *Metagame::ArenaName(const char *pszArena) {
    return TheLocale.Localize(pszArena, false);
}

void Metagame::EnterFrontEnd() {
    ExitState(kStateFrontEnd);
    mState = kStateFrontEnd;
    TheTriggerMgr.MetagameEvent(mTime, mTime, 0.0f);
    TheGameDb->SetWinSequence(false);
    for (int i = 0; i < kMenuPads; ++i) {
        JoypadSetMenuControl(i, true);
    }
    ShowFrontEnd();
}

void Metagame::EnterLoading() {
    // Yes, the binary times the change of state with a timer it never reads.
    Timer timer{};
    timer.Start();
    ExitState(kStateLoading);
    mState = kStateLoading;
    StartLoading();
    timer.Stop();
}

void Metagame::EnterPlaying() {
    ExitState(kStatePlaying);
    StartPlaying();
    mState = kStatePlaying;
}

void Metagame::EnterLeaving() {
    ExitState(kStateLeaving);
    StartLeaving();
    mReloadFrontEnd = 1;
    mState = kStateLeaving;
}

void Metagame::EnterRestarting() {
    ExitState(kStatePlaying); // Yes, the binary passes kStatePlaying rather than kStateRestarting.
    StartRestarting();
    mState = kStateRestarting;
}

int Metagame::Update() {
    TheMetaScheduler.Poll();
    const float flNow = TheMetaScheduler.mTime;
    const float flTick = static_cast<float>(TheMetaScheduler.mTick);
    const float flElapsed = (flNow - mLastSchedulerTime) * mSpeed;
    mLastSchedulerTime = flNow;
    mTick = flTick;
    mTime += flElapsed;
    if (mState != kStatePlaying) {
        TheGameDb->mSongTime = TheMetaScheduler.mTime;
        TheGameDb->mSongTick = flTick;
    }

    mArena->Poll(mState, mLoadStage, mTime, mTick);
    TheUI.Poll(mTime);
    if (mGizmo != nullptr) {
        mGizmo->Poll(mTime);
    }
    TheMCManager.Poll();
    SongPreview::Poll();

    if (mState == kStateLoading || mState == kStateRestarting) {
        float flUnused;
        // Yes, the binary reads the entry and never uses it.
        SystemConfig()
            ->FindArray(kMetagameKey, false)
            ->FindArray(kFreqScreenKey, true)
            ->FindFloat(kPanelGizmoAnimEndKey, &flUnused, false);
        const float flSystemNow = SystemMs();
        if (mLeaveTime != 0.0f && mLeaveTime <= flSystemNow) {
            mLeaveTime = 0.0f;
            if (TheGameDb->IsWinSequence()) {
                TheUI.GotoScreen(kBlankScreen);
            } else if (TheGameDb->mTutorial != 0) {
                TheUI.GotoScreen(kTrainingLeaveGizmoScreen);
            } else if (mState == kStateLoading) {
                TheUI.GotoScreen(kJustMtvLeaveGizmoScreen);
            } else {
                TheUI.GotoScreen(kJustMtvScreen);
            }
            mLeaveScreenShown = 0;
        }
    } else if (mState == kStateLeaving) {
        if (mLeaveScreenShown == 0) {
            mLeaveScreenShown = 1;
            TheUI.GotoScreen(kPreGame2MetaScreen);
        }
        if (mLoadStage == kLoadStageDone && mReloadFrontEnd != 0) {
            mLoadStage = kLoadStageStartFxBanks;
            mReloadFrontEnd = 0;
        }
    }

    if (mState == kStateRestarting && mSharedMusicFadedIn == 0 && IsGameFxBankLoaded()) {
        mSharedMusicFadedIn = 1;
        FadeInSharedMusic();
    }

    if (mLoadStage != kLoadStageDone || mState == kStateLeaving) {
        PollLoad();
    }

    const int nEvent = mEvent;
    mEvent = kEventNone;
    return nEvent;
}

void Metagame::ShowBlankScreen() {
    if (mDialogShowing == 0) {
        TheUI.GotoScreen(kBlankScreen);
    }
}

void Metagame::SetHelpText(const char *pszText) {
    mHelpPanel->SetHelpText(pszText);
}

void Metagame::SetActionText(const char *pszText) {
    mHelpPanel->SetActionText(pszText);
}

void Metagame::StartIntro() {
    TheUI.GotoScreen(kInitialMcCheckScreen);
    float *pTickMs = new float;
    *pTickMs = kTickMs;
    mTickMs = pTickMs;
    TheMetaScheduler.Reset(mTickMs, 0);
    TheMetaScheduler.Resume();
    delete mLoadingScreenLoader;
    mLoadingScreenLoader = nullptr;
}

void Metagame::PollLoad() {
    switch (mLoadStage) {
    case kLoadStageAlways:
        if (TheUI.IsGroupLoaded(kAlwaysGroup)) {
            mHelpPanel = static_cast<HelpPanel *>(TheUI.FindPanel(kHelpPanelName, false));
            if (TheUI.mEditMode) {
                mLoadStage = kLoadStageIntroAndFrontEnd;
                TheUI.LoadGroup(kIntroGroup);
                TheUI.LoadGroup(kFrontEndGroup);
            } else {
                mLoadStage = kLoadStageIntro;
                TheUI.LoadGroup(kIntroGroup);
            }
        }
        break;
    case kLoadStageIntro:
        if (TheUI.IsGroupLoaded(kIntroGroup)) {
            mLoadStage = kLoadStageStartFxBanks;
        }
        break;
    case kLoadStageIntroAndFrontEnd:
        if (TheUI.IsGroupLoaded(kFrontEndGroup) && TheUI.IsGroupLoaded(kIntroGroup)) {
            if (mState == kStateFrontEnd && TheUI.mEditMode) {
                TheUI.ReportLocalizeErrors();
            }
            mLoadStage = kLoadStageStartFxBanks;
        }
        break;
    case kLoadStageStartFxBanks:
        mLoadStage = kLoadStageFxBanks;
        mEffects.Apply();
        LoadFxBanks();
        break;
    case kLoadStageFxBanks:
        if (IsGameFxBankLoaded()) {
            if (mFirstBoot != 0) {
                StartIntro();
            }
            mLoadStage = kLoadStageArena;
            CreateMusic();
            if (mFirstBoot == 0) {
                FadeInSharedMusic();
            }
            mArena->StartLoad();
        }
        break;
    case kLoadStageArena:
        if (mArena->IsLoaded()) {
            mLoadStage = kLoadStageMusic;
            mMusicSong->StartLoad();
        }
        break;
    case kLoadStageMusic:
        if (mMusicSong->IsLoaded()) {
            mLoadStage = kLoadStageMusicBanks;
            LoadMusicBanks();
        } else {
            mMusicSong->Poll();
        }
        break;
    case kLoadStageMusicBanks: {
        if (!AreBanksLoaded()) {
            break;
        }
        const char *pszScreen = TheUI.mCurrentScreen->mName;
        if (strcmp(pszScreen, kBlankScreen) != 0 && strcmp(pszScreen, kPreGame2MetaScreen) != 0 &&
            strcmp(pszScreen, kJustMtvScreen) != 0) {
            break;
        }
        mLoadStage = kLoadStageDone;
        mTick = 0.0f;
        mTime = 0.0f;
        if (mFirstBoot != 0) {
            TheMetaScheduler.Pause();
            ThePs.ReleaseForMovie();
            String path;
            MakeDevicePath(path, kIntroMovie);
            TheSynth->VirtualSlot32(kStreamSilent);
            for (int i = 0; i < kMidiChannelCount; ++i) {
                TheSynth->SendMessage(kMidiControlChange | i, kMidiControlVolume, 0, 0);
            }
            TheSynth->Poll();
            if (PlayMovieFile(path.c_str())) {
                (void)TheSynth->EnableSoftFx();
                (void)TheSynth->DisableSoftFx();
            }
            ThePs.RestoreAfterMovie();
            TheMetaScheduler.Resume();
            TheUI.UnloadGroup(kIntroGroup);
            TheUI.Poll(0.0f);
            TheTriggerMgr.MetagameEvent(mTime, mTime, 0.0f);
            ShowFrontEnd();
        } else {
            mLastSchedulerTime = 0.0f;
        }
        FadeOutSharedMusic();
        mMusic->Start(0);
        delete mMusicSong;
        mMusicSong = nullptr;
        break;
    }
    default:
        if (mState != kStateFrontEnd) {
            mEvent = kEventReturnToFrontEnd;
        }
        break;
    }
}

void Metagame::Draw() {
    if (mLoadStage == kLoadStageDone) {
        mArena->Draw(mState);
    }
}

void Metagame::DrawOverlay() {
    if (mLoadStage < kLoadStageStartFxBanks) {
        Rnd::Mat *pMat = dynamic_cast<Rnd::Mat *>(Rnd::TheManager.Find(kLoadingMatName));
        const float flLevel = std::max(0.0f, pMat->mAmbient.r - kLoadingFadeStep);
        const Color ambient{flLevel, flLevel, flLevel, 1.0f};
        pMat->SetAmbient(ambient);
        dynamic_cast<Rnd::View *>(Rnd::TheManager.Find(kLoadingViewName))->Draw();
        return;
    }

    if (mUiCamera != nullptr) {
        mUiCamera->Draw();
    }
    if (mGizmo != nullptr) {
        mGizmo->Draw();
    }
    if (mUiEnviron != nullptr) {
        mUiEnviron->Draw();
    }
    TheUI.Draw();
}

void Metagame::ShowFrontEnd() {
    constexpr float kAvatarCameraFirst = 0.05f;
    constexpr float kAvatarCameraSecond = 1.0f;
    mLeaveScreenShown = 1;
    TheGameDb->SetAvatarCameraParams(kAvatarCameraFirst, kAvatarCameraSecond);
    if (mState == kStateFrontEnd) {
        TheUI.GotoScreen(mNextScreen);
    }
}

void Metagame::ExitFrontEnd() {
}

void Metagame::StartLoading() {
    (void)TheSynth->DisableSoftFx();
    mLeaveTime = SystemMs() + kLeaveDelayMs;
}

void Metagame::ExitLoading() {
}

void Metagame::StartPlaying() {
}

void Metagame::ExitPlaying() {
}

void Metagame::StartLeaving() {
}

void Metagame::ExitLeaving() {
}

void Metagame::StartRestarting() {
}

void Metagame::ExitRestarting() {
}

void Metagame::ExitState([[maybe_unused]] int nNextState) {
    switch (mState) {
    case kStateFrontEnd:
        ExitFrontEnd();
        break;
    case kStateLoading:
        ExitLoading();
        break;
    case kStatePlaying:
        ExitPlaying();
        break;
    case kStateLeaving:
        ExitLeaving();
        break;
    case kStateRestarting:
        ExitRestarting();
        break;
    default:
        DebugWarn(kUnknownModeMessage);
        break;
    }
}

void Metagame::RecordCampaignResult() {
}

void Metagame::QueueUnlocks() {
}

void Metagame::ShowDialog([[maybe_unused]] DialogType type,
                          [[maybe_unused]] DialogCallback pfnCallback,
                          [[maybe_unused]] void *pUserData,
                          [[maybe_unused]] int nPad) {
}

void Metagame::RecordPracticeResult() {
}

void Metagame::ShowEndGameScreens([[maybe_unused]] DialogAction action) {
}

void Metagame::FinishDialog() {
    mDialogShowing = 0;
    if (strcmp(TheUI.mCurrentScreen->mName, kBlankScreen) != 0) {
        TheUI.GotoScreen(kBlankScreen);
    } else if (mDialogCallback != nullptr) {
        mDialogCallback(mDialogAction, mDialogUserData);
        mDialogCallback = nullptr;
    }
}

void Metagame::AdvanceUnlocks() {
}

bool Metagame::IsSpeedUp(const char *pszFrom, const char *pszTo) {
    return IsTransitionListed(kSpeedUpKey, pszFrom, pszTo);
}

bool Metagame::IsStopSpeedUp(const char *pszFrom, const char *pszTo) {
    return IsTransitionListed(kStopSpeedUpKey, pszFrom, pszTo);
}

bool Metagame::OnJoypadInput(JoypadInputMsg *pMsg) {
    if (mState == kStateLoading || mState == kStateLeaving) {
        return false;
    }
    if (mLoadStage != kLoadStageDone && mState != kStateFrontEnd) {
        return false;
    }
    if (mState == kStatePlaying && mDialogShowing == 0) {
        return false;
    }

    if (pMsg->mPressed != 0 && mState == kStateFrontEnd) {
        const int nPad = pMsg->mPad;
        if (nPad >= TheGameDb->GetNumPads()) {
            return true;
        }
        TheTriggerMgr.ButtonEvent(nPad, JoypadGetState(nPad)->mButtons);
        if (pMsg->mButton == kPadTriangle || pMsg->mButton == kPadCross) {
            mLastSelectButton = pMsg->mButton;
        }
    }
    return TheUI.Dispatch(pMsg);
}

bool Metagame::OnTransitionComplete(UITransitionCompleteMsg *pMsg) {
    if (mHelpPanel != nullptr) {
        mHelpPanel->Refresh();
        mHelpPanel->ShowActions(pMsg->mScreen);
    }

    const char *pszScreen = pMsg->mScreen->mName;
    bool bShowAvatar = false;
    if (strcmp(pszScreen, kBlankScreen) == 0) {
        if (mDialogCallback != nullptr) {
            mDialogCallback(mDialogAction, mDialogUserData);
            mDialogCallback = nullptr;
        }
    } else if (strcmp(pszScreen, kBackToShellScreen) == 0) {
        mEvent = kEventQuit;
    } else if (strcmp(pszScreen, kJustMtvLeaveGizmoScreen) == 0) {
        mGizmo->SetShowAvatar(false);
        TheUI.GotoScreen(kJustMtvScreen);
    } else if (strcmp(pszScreen, kLaunchScreen) == 0 ||
               strcmp(pszScreen, kLaunchTrainingScreen) == 0) {
        mGizmo->SetShowAvatar(false);
    } else {
        for (const char *pszReturnScreen : kReturnScreens) {
            if (strcmp(pszScreen, pszReturnScreen) == 0) {
                bShowAvatar = true;
                break;
            }
        }
        if (!bShowAvatar && strcmp(pszScreen, kStartScreen) == 0 &&
            strcmp(pMsg->mPrevScreen->mName, kPreGame2MetaScreen) == 0) {
            bShowAvatar = true;
        }
    }
    if (bShowAvatar && TheGameDb->GetProfile(0)->mCustom != 0) {
        mGizmo->SetShowAvatar(true);
    }

    if (IsStopSpeedUp(pMsg->mPrevScreen->mName, pMsg->mScreen->mName)) {
        mSpeedingUp = 0;
        mSpeed = 1.0f;
    }
    return false;
}

bool Metagame::OnComponentSelect(UIComponentSelectMsg *pMsg) {
    if (pMsg->mButton == kPadCross) {
        TheTriggerMgr.ComponentSelectEvent(pMsg);
    }
    return false;
}

bool Metagame::OnComponentSelectStart(UIComponentSelectStartMsg *pMsg) {
    TheTriggerMgr.ComponentSelectStartEvent(pMsg);
    return false;
}

bool Metagame::OnComponentFocusChange(UIComponentFocusChangeMsg *pMsg) {
    TheTriggerMgr.ComponentFocusEvent(pMsg);
    if (pMsg->mComponent != nullptr && TheUI.mCurrentScreen != nullptr &&
        TheUI.mCurrentScreen->mFocusPanel == pMsg->mPanel) {
        mHelpPanel->ShowHelp(TheUI.mCurrentScreen->mFocusPanel, pMsg->mComponent);
    }
    return false;
}

bool Metagame::OnScreenChange(UIScreenChangeMsg *pMsg) {
    TheTriggerMgr.ScreenChangeEvent(pMsg);

    FreqScreen *pScreen = dynamic_cast<FreqScreen *>(pMsg->mScreen);
    Rnd::Text *pTitle = dynamic_cast<Rnd::Text *>(Rnd::TheManager.Find(kTitleTextName));
    if (pTitle != nullptr && pScreen != nullptr) {
        Rnd::View *pView = dynamic_cast<Rnd::View *>(Rnd::TheManager.Find(kTitleViewName));
        const String title(pScreen->Title());
        pTitle->SetText(title.c_str());
        if (title.mLength != 0) {
            pView->SetShowing(1);
            const float flWidth =
                pTitle->MeasureWidth(title.c_str(), static_cast<int>(strlen(title.c_str())));
            const float flFraction = (flWidth - kTitleMinWidth) / kTitleWidthRange;
            dynamic_cast<Rnd::View *>(Rnd::TheManager.Find(kTitleViewName))
                ->SetFrame((1.0f - flFraction) * kTitleFrames);
            dynamic_cast<FreqPanel *>(TheUI.FindPanel(kTitlePanelName, false))->UpdateBgBox();
        } else {
            pView->SetShowing(0);
        }
    }

    if (mState != kStateFrontEnd || mLoadStage != kLoadStageDone) {
        return false;
    }

    const char *pszScreen = pMsg->mScreen->mName;
    if (strcmp(pszScreen, kHostLaunchpadScreen) == 0 ||
        strcmp(pszScreen, kGuestLaunchpadScreen) == 0) {
        if (mNetScreenPending != 0) {
            TheUI.GotoScreen(mNetScreen.c_str());
        }
        mReserved144 = 1;
    } else if (strcmp(pszScreen, kLobbyErrorScreen) == 0 ||
               strcmp(pszScreen, kLaunchpadErrorScreen) == 0) {
        mNetScreenPending = 0;
        mReserved144 = 1;
    } else if (strcmp(pszScreen, kPreLogoScreen) == 0) {
        if (TheGameDb->GetProfile(0)->mCustom != 0) {
            mGizmo->SetShowAvatar(true);
        }
    } else if (strcmp(pszScreen, kUnlockArenaAnimScreen) == 0) {
        mArena->StartReveal(mTime);
    }

    if (IsSpeedUp(pMsg->mOldScreen->mName, pMsg->mScreen->mName)) {
        mSpeedingUp = 1;
    }
    if (mLastSelectButton == kPadTriangle &&
        dynamic_cast<TransitionScreen *>(pMsg->mOldScreen) == nullptr) {
        GameFx::PlayBack();
    }
    mLastSelectButton = kPadNone;
    return false;
}

bool Metagame::OnLaunchpadAborted([[maybe_unused]] Message *pMsg) {
    return false;
}

bool Metagame::OnLostInternet([[maybe_unused]] Message *pMsg) {
    return false;
}

bool Metagame::OnGameParamsUpdate([[maybe_unused]] Message *pMsg) {
    return false;
}

bool Metagame::OnLoadGame([[maybe_unused]] Message *pMsg) {
    return false;
}

bool Metagame::OnShareRemixBegin([[maybe_unused]] Message *pMsg) {
    return false;
}

bool Metagame::OnShareRemixProgress([[maybe_unused]] Message *pMsg) {
    return false;
}

bool Metagame::DispatchPriv(Message *pMsg) {
    const int nType = pMsg->Type();
    if (nType == g_nJoypadInputMsgType) {
        return OnJoypadInput(static_cast<JoypadInputMsg *>(pMsg));
    }
    if (nType == g_nUIComponentSelectMsgType) {
        return OnComponentSelect(static_cast<UIComponentSelectMsg *>(pMsg));
    }
    if (nType == g_nUIScreenChangeMsgType) {
        return OnScreenChange(static_cast<UIScreenChangeMsg *>(pMsg));
    }
    if (nType == g_nUIComponentSelectStartMsgType) {
        return OnComponentSelectStart(static_cast<UIComponentSelectStartMsg *>(pMsg));
    }
    if (nType == g_nUIComponentFocusChangeMsgType) {
        return OnComponentFocusChange(static_cast<UIComponentFocusChangeMsg *>(pMsg));
    }
    if (nType == g_nUITransitionCompleteMsgType) {
        return OnTransitionComplete(static_cast<UITransitionCompleteMsg *>(pMsg));
    }
    if (nType == g_nLaunchpadAbortedMsgType) {
        return OnLaunchpadAborted(pMsg);
    }
    if (nType == g_nLobbyConnectionLostMsgType) {
        return OnLostInternet(pMsg);
    }
    if (nType == g_nGameParamsUpdateMsgType) {
        return OnGameParamsUpdate(pMsg);
    }
    if (nType == g_nShareRemixBeginMsgType) {
        return OnShareRemixBegin(pMsg);
    }
    if (nType == g_nShareRemixProgressMsgType) {
        return OnShareRemixProgress(pMsg);
    }
    if (nType == g_nLoadGameMsgType) {
        return OnLoadGame(pMsg);
    }
    return false;
}

void Metagame::LoadFxBanks() {
    (void)IsSharedMusicFading(); // Yes, the binary discards the result.
    Synth *pSynth = TheSynth;
    const char *pszTransition = nullptr;
    const char *pszSolo = nullptr;
    const char *pszMulti = nullptr;
    const char *pszRemix = nullptr;
    DataArray *pMetagame = SystemConfig()->FindArray(kMetagameKey, false);
    DataArray *pGame = SystemConfig()->FindArray(kGameKey, false);
    pMetagame->FindSymbol(kTransitionFxBankFileKey, &pszTransition, true);
    pGame->FindSymbol(kSoloGameFxBankFileKey, &pszSolo, true);
    pGame->FindSymbol(kMultiGameFxBankFileKey, &pszMulti, true);
    pGame->FindSymbol(kRemixFxBankFileKey, &pszRemix, true);

    // The slot of the game effects is sized for the largest of the four effect banks.
    std::vector<int> blocks(kBankSlotCount, 0);
    {
        const String solo(pszSolo);
        const int nSolo = pSynth->GetBankBlockCount(solo);
        const String multi(pszMulti);
        const int nMulti = pSynth->GetBankBlockCount(multi);
        const String transition(pszTransition);
        const int nTransition = pSynth->GetBankBlockCount(transition);
        const String remix(pszRemix);
        const int nRemix = pSynth->GetBankBlockCount(remix);
        blocks[mGameFxBankSlot] = std::max(std::max(nSolo, nMulti), std::max(nTransition, nRemix));
    }
    (void)pSynth->Partition(blocks);
    pSynth->UnloadBank(mGameFxBankSlot);
    pSynth->LoadBank(mGameFxBankSlot, String(pszTransition), true);
}

bool Metagame::IsGameFxBankLoaded() {
    return TheSynth->IsBankLoaded(mGameFxBankSlot);
}

void Metagame::LoadMusicBanks() {
    Synth *pSynth = TheSynth;
    const char *pszShared = nullptr;
    const char *pszMetaFx = nullptr;
    DataArray *pMetagame = SystemConfig()->FindArray(kMetagameKey, false);
    pMetagame->FindSymbol(kMusicSharedBankFileKey, &pszShared, true);
    pMetagame->FindSymbol(kMetaFxBankFileKey, &pszMetaFx, true);
    const char *pszSong = mMusicSong->BankFile();

    // The slots after the swap slot are emptied, and the slots before keep their sizes.
    constexpr int kKeepSize = -1;
    std::vector<int> blocks(kBankSlotCount, kKeepSize);
    blocks[mMetaFxBankSlot] = pSynth->GetBankBlockCount(String(pszMetaFx));
    blocks[mMusicSharedBankSlot] = pSynth->GetBankBlockCount(String(pszShared));
    blocks[mMusicSwapBankSlot] = pSynth->GetBankBlockCount(String(pszSong));
    std::fill(blocks.begin() + mMusicSwapBankSlot + 1, blocks.end(), 0);
    (void)pSynth->Partition(blocks);
    pSynth->UnloadBank(mMetaFxBankSlot);
    pSynth->UnloadBank(mMusicSharedBankSlot);
    pSynth->UnloadBank(mMusicSwapBankSlot);
    pSynth->LoadBank(mMetaFxBankSlot, String(pszMetaFx), true);
    pSynth->LoadBank(mMusicSharedBankSlot, String(pszShared), true);
    pSynth->LoadBank(mMusicSwapBankSlot, String(pszSong), true);
}

bool Metagame::AreBanksLoaded() {
    Synth *pSynth = TheSynth;
    return pSynth->IsBankLoaded(mGameFxBankSlot) && pSynth->IsBankLoaded(mMetaFxBankSlot) &&
           pSynth->IsBankLoaded(mMusicSharedBankSlot) && pSynth->IsBankLoaded(mMusicSwapBankSlot);
}

void Metagame::UnloadBanks() {
    TheSynth->UnloadBank(mMetaFxBankSlot);
    TheSynth->UnloadBank(mMusicSharedBankSlot);
    TheSynth->UnloadBank(mMusicSwapBankSlot);
}

void Metagame::CreateMusic() {
    mMusic = new MetaMusic;
    DataArray *pSongs = SystemConfig()->FindArray(kMetagameKey, true)->FindArray(kSongsKey, true);
    int nSong = 0;
    if (mFirstBoot == 0) {
        nSong = RandomInt(0, pSongs->mSize - 1);
    }
    mMusicSong = new MetaMusicSong(mMusic, pSongs->Array(nSong + 1));
}

void Metagame::SetSongScreenMix(bool bIdle) {
    if (bIdle) {
        mMusic->SwitchMix(kIdleMix, kIdleMixTick);
    } else {
        mMusic->SwitchMix(kClipMix, kClipMixTick);
    }
}
