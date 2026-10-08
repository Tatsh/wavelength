#include "met/metagame.h"

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <list>
#include <vector>

#include "app/cutscene.h"
#include "game/campaign.h"
#include "game/gamedb.h"
#include "game/remixinfo.h"
#include "game/songentry.h"
#include "game/songrecord.h"
#include "game/triggermgr.h"
#include "gfx/gfxmanager.h"
#include "math/color.h"
#include "math/rand.h"
#include "memcard/mcmanager.h"
#include "met/arenaunlockscreen.h"
#include "met/autosavedonescreen.h"
#include "met/avatarpanel.h"
#include "met/bonuspicpanel.h"
#include "met/bootscreen.h"
#include "met/bossjourneyscreen.h"
#include "met/bossunlockscreen.h"
#include "met/choosememcardscreen.h"
#include "met/controllerconfigscreen.h"
#include "met/controllerpanel.h"
#include "met/creditsscreen.h"
#include "met/dialogpanel.h"
#include "met/errorscreen.h"
#include "met/freestylelaptipscreen.h"
#include "met/freqconfirmscreen.h"
#include "met/freqcopydelscreen.h"
#include "met/freqloadlist.h"
#include "met/freqmakercustomscreen.h"
#include "met/freqmakeremblemscreen.h"
#include "met/freqmakererrorscreen.h"
#include "met/freqmakermainscreen.h"
#include "met/freqmakerpartconfigscreen.h"
#include "met/freqpanel.h"
#include "met/freqscreen.h"
#include "met/freqselpanel.h"
#include "met/fullrankedpanel.h"
#include "met/gameoptionsscreen.h"
#include "met/helppanel.h"
#include "met/introscreen.h"
#include "met/jukeboxscreen.h"
#include "met/keyboardkey.h"
#include "met/keyboardpanel.h"
#include "met/keyboardscreen.h"
#include "met/launchmtvpanel.h"
#include "met/launchscreen.h"
#include "met/mainoptionsscreen.h"
#include "met/marketingscreen.h"
#include "met/mcdialogpanel.h"
#include "met/metaarenascreen.h"
#include "met/metagameutil.h"
#include "met/metamainscreen.h"
#include "met/metaskillscreen.h"
#include "met/metasongscreen.h"
#include "met/metastartscreen.h"
#include "met/modescreen.h"
#include "met/multiendgamescreen.h"
#include "met/multifreqselectscreen.h"
#include "met/multigamestatspanel.h"
#include "met/multisetupremixsavescreen.h"
#include "met/netchangepassword.h"
#include "met/netchangepasswordscreen.h"
#include "met/netcreateaccount.h"
#include "met/netcreateconfigscreen.h"
#include "met/netcreateuserscreen.h"
#include "met/netdodownloadscreen.h"
#include "met/netdoplayersearchscreen.h"
#include "met/netdouploadscreen.h"
#include "met/netendgamescreen.h"
#include "met/neteulascreen.h"
#include "met/netfoundplayerpanel.h"
#include "met/nethostattemptscreen.h"
#include "met/nethostingscreen.h"
#include "met/netinetdisconnect.h"
#include "met/netjoinlpadscreen.h"
#include "met/netlaunchpadquitscreen.h"
#include "met/netlaunchscreen.h"
#include "met/netlobbyconnect.h"
#include "met/netlobbydisconnect.h"
#include "met/netloginscreen.h"
#include "met/netlpadgamepanel.h"
#include "met/netlpadplayerpanel.h"
#include "met/netlpadscreen.h"
#include "met/netmainlaunchpadslist.h"
#include "met/netmainplayerslist.h"
#include "met/netnewchatroomscreen.h"
#include "met/netrankedplayerslist.h"
#include "met/netrankedplayerspanel.h"
#include "met/netrankingscreen.h"
#include "met/netreadonlywarnscreen.h"
#include "met/netsearchscreen.h"
#include "met/netserverlogin.h"
#include "met/netserverselscreen.h"
#include "met/netsongpanel.h"
#include "met/netsorteddatapanel.h"
#include "met/netsortedlaunchpadslist.h"
#include "met/netsortedlaunchpadspanel.h"
#include "met/netsortedscreen.h"
#include "met/netswitchlobbyscreen.h"
#include "met/netwelcomescreen.h"
#include "met/nocontrollerscreen.h"
#include "met/numplayersscreen.h"
#include "met/partunlockdonescreen.h"
#include "met/partunlockscreen.h"
#include "met/pausescreen.h"
#include "met/playersearchresultsscreen.h"
#include "met/playersearchsetupscreen.h"
#include "met/powerupscreen.h"
#include "met/prelaunchscreen.h"
#include "met/puptipsscreen.h"
#include "met/readonlycheckscreen.h"
#include "met/readonlysavescreen.h"
#include "met/remixcontrollerscreen.h"
#include "met/remixcopydelscreen.h"
#include "met/remixcreatemodescreen.h"
#include "met/remixdiscardscreen.h"
#include "met/remixdownloadlist.h"
#include "met/remixloadlist.h"
#include "met/remixorfreqscreen.h"
#include "met/remixselectscreen.h"
#include "met/remixsongscreen.h"
#include "met/remixtypescreen.h"
#include "met/saveeditedfreqscreen.h"
#include "met/savefreqscreen.h"
#include "met/saveremixscreen.h"
#include "met/selloadedfreqscreen.h"
#include "met/setupnewchatroomscreen.h"
#include "met/setupremixsavescreen.h"
#include "met/shareremixscreen.h"
#include "met/soloendgamescreen.h"
#include "met/sololosetipspanel.h"
#include "met/solowinstatspanel.h"
#include "met/songdecryptscreen.h"
#include "met/songpicpanel.h"
#include "met/songpreview.h"
#include "met/songsellist.h"
#include "met/tipsscreen.h"
#include "met/transitionerrorscreen.h"
#include "met/transitionmusic.h"
#include "met/transitionscreen.h"
#include "met/unlockavatarpanel.h"
#include "met/unlockscreen.h"
#include "met/uploadnotescreen.h"
#include "met/xproceedscreen.h"
#include "netflow/lobbymsgtypes.h"
#include "netflow/netlaunchpad.h"
#include "netflow/netlaunchpadplayer.h"
#include "netflow/netlobby.h"
#include "os/debug.h"
#include "os/file.h"
#include "os/joypad.h"
#include "os/keyboard.h"
#include "os/locale.h"
#include "os/scheduler.h"
#include "os/string.h"
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
#include "script/symbol.h"
#include "synth/fxmidi.h"
#include "synth/synth.h"
#include "ui/uimanager.h"

// The unit's static initialiser at NTSC-U/C: 0x00169300, PAL: 0x0016c488, and its global
// constructor at NTSC-U/C: 0x00169340, PAL: 0x0016c4c8, construct and destroy it.
Metagame TheMetagame;
Scheduler TheMetaScheduler;

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
constexpr char kHostLaunchpadPlayScreen[] = "fn_h_lpad_play";
constexpr char kGuestLaunchpadPlayScreen[] = "fn_g_lpad_play";
constexpr char kKeyboardScreen[] = "kb_screen";
constexpr char kShareRemixScreen[] = "net_share_remix";
constexpr char kNetLaunchScreen[] = "net_launch";
constexpr char kNetLaunchSequenceScreen[] = "pre_net2launchseq";
constexpr char kUntitledRemix[] = "Untitled";

// The longest name and player name strncpy() copies into a RemixInfo.
constexpr size_t kRemixNameCopyLength = 29;
constexpr size_t kRemixCreatorCopyLength = 15;

// The first argument of GameDb::AddPlayer() for the player of this console.
constexpr int kRemotePlayer = 0;
constexpr int kLocalPlayer = 1;
constexpr char kLobbyErrorScreen[] = "lobby_error";
constexpr char kLaunchpadErrorScreen[] = "lpad_error";
constexpr char kUnlockArenaAnimScreen[] = "unlockarena_anim";
constexpr char kSelectArenaScreen[] = "s_g_sel_arena";
constexpr char kJukeboxScreen[] = "jbox_redbook";

// The arguments of the unlock_all command.
constexpr char kUnlockSongsArg[] = "songs";
constexpr char kUnlockFreqsArg[] = "freqs";

constexpr char kDefaultNameToken[] = "default_name_1";

// The screens a song leads to in a remix and in an online game.
constexpr char kSoloRemixModeScreen[] = "game2soloremix_mode";
constexpr char kMultiRemixModeScreen[] = "game2multiremix_mode";
constexpr char kLostLaunchpadScreen[] = "game2lost_launchpad";
constexpr char kNetLaunchHostScreen[] = "game2netlaunch_host";
constexpr char kNetLaunchGuestScreen[] = "game2netlaunch_guest";
constexpr char kNetEndRemixScreen[] = "game2net_end_remix";
constexpr char kLostLobbyScreen[] = "game2lost_lobby";

// The arena of the campaign's win sequence, and the song a finished campaign plays again.
constexpr char kWinArena[] = "Winarena";
constexpr char kFallbackSong[] = "DRUMNBASS";

// The share of a song played and of its gems blasted, in percent.
constexpr float kPercent = 100.0f;
constexpr int kPracticeResult = 100;

// The messages of the launchpad error screen, by reason.
constexpr char kLaunchpadIncompatibleError[] = "net_lpad_incompatible";
constexpr char kLaunchpadHostError[] = "net_lpad_abort_host";
constexpr char kLaunchpadBootError[] = "net_lpad_abort_boot";
constexpr char kLaunchpadDiedError[] = "net_lpad_abort_died";
constexpr char kLaunchpadBadError[] = "net_lpad_abort_bad";
constexpr char kLaunchpadLostError[] = "net_lpad_lost_error";
constexpr char kErrorMessageFormat[] = "%s_msg";

// The messages and the transition of the lobby error screen.
constexpr char kInternetDownLog[] = "Inet down error\n";
constexpr char kOtherErrorLog[] = "Some other error\n";
constexpr char kLostInternetToken[] = "lost_internet_error_msg";
constexpr char kLostLobbyToken[] = "lost_lobby_error_msg";
constexpr char kErrorOkComponent[] = "ok";
constexpr char kNetConfigScreen[] = "fn_config";
constexpr char kNetPortalScreen[] = "net_portal";

// The screens the front end returns to after a song.
constexpr char kSoloArenaScreen[] = "game2soloarena";
constexpr char kDuelSelectSongScreen[] = "game2duel_sel_song";
constexpr char kMultiCustomScreen[] = "game2multicustom";
constexpr char kMultiSelectSongScreen[] = "game2multi_sel_song";
constexpr char kMultiEndRemixScreen[] = "game2multi_end_remix";
constexpr char kNetEndSaveReadOnlyScreen[] = "game2net_end_save_read_only";
constexpr char kSaveRemixScreen[] = "save_remix";
constexpr char kNoEndScreenLog[] = "end game screen not being set\n";
constexpr char kLaunchpadLaunchScreen[] = "launchpad2launchseq";
constexpr char kSoloTutorialLaunchScreen[] = "solotut2launchseq";
constexpr char kSoloCustomScreen[] = "game2solocustom";
constexpr char kSoloMetaScreen[] = "game2solometa";
constexpr char kSoloRemixScreen[] = "game2soloremix";

// The screens that follow a song.
constexpr char kUnlockArenaScreen[] = "game2unlockarena";
constexpr char kSongDecryptScreen[] = "song_decrypt";
constexpr char kUnlockPartsScreen[] = "unlock_parts";
constexpr char kUnlockBossScreen[] = "unlock_boss";
constexpr char kAutoSaveFreqScreen[] = "auto_save_freq";
constexpr char kAutoSaveSettingsScreen[] = "auto_save_settings";
constexpr char kFreestyleTipScreen[] = "freestyle_lap_tip";

// The dialogs over a song.
constexpr char kMultiGamePauseScreen[] = "m_g_pause";
constexpr char kSoloPauseScreen[] = "s_pause";
constexpr char kSoloRemixPauseScreen[] = "s_r_pause";
constexpr char kMultiRemixPauseScreen[] = "m_r_pause";
constexpr char kNoControllerScreen[] = "no_controller";
constexpr char kPracticeEndScreen[] = "s_g_end_practice";
constexpr char kSoloWinScreen[] = "s_g_end_win";
constexpr char kSoloLoseScreen[] = "s_g_end_lose";
constexpr char kTieCountFormat[] = "tie%d";
constexpr char kTieSuffix[] = "tie";
constexpr char kWinSuffix[] = "win";
constexpr char kOnlineEndScreenFormat[] = "fn_g_end_%dpl_%s";
constexpr char kLocalEndScreenFormat[] = "m_g_end_%dpl_%s";
constexpr int kNamedTieWinners = 3;
constexpr int kTieWinners = 2;

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

// NTSC-U/C: 0x00165828, PAL: 0x00168738
void SoloSongScreen([[maybe_unused]] DataArray *pCommand, [[maybe_unused]] void *pUserData) {
    TheGameDb->SetRuleSet(GameDb::kRuleSetGame);
    TheGameDb->SetCommunity(GameDb::kCommunitySolo);
    TheGameDb->ClearPlayers();
    Campaign profile;
    profile.mName = TheLocale.Localize(kDefaultNameToken, true);
    TheGameDb->AddPlayer(&profile);
    TheGameDb->UnlockAllSongs();
    TheUI.GotoScreen(kSelectArenaScreen);
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

// NTSC-U/C: 0x00168ce0, PAL: 0x0016be68
void UnlockAll(DataArray *pCommand, [[maybe_unused]] void *pUserData) {
    FxMidi::PlayCheat();
    if (strcmp(pCommand->Sym(1), kUnlockSongsArg) == 0) {
        TheGameDb->UnlockAllSongs();
        dynamic_cast<JukeboxScreen *>(TheUI.FindScreen(kJukeboxScreen, false))->mShowAllSongs = 1;
    } else if (strcmp(pCommand->Sym(1), kUnlockFreqsArg) == 0) {
        Campaign::UnlockAllParts();
    }
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
    mChatroom.mId = -1;
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
    // 31 panel classes, and 12 component classes here, in this order.
    TheUI.RegisterScreenType(FreqScreen::New, "freq_screen");
    TheUI.RegisterScreenType(TransitionScreen::New, "transition_screen");
    TheUI.RegisterScreenType(IntroScreen::New, "intro_screen");
    TheUI.RegisterScreenType(MarketingScreen::New, "mktg_screen");
    TheUI.RegisterScreenType(CreditsScreen::New, "credits_screen");
    TheUI.RegisterScreenType(LaunchScreen::New, "launch_screen");
    TheUI.RegisterScreenType(PreLaunchScreen::New, "pre_launch_screen");
    TheUI.RegisterScreenType(NoControllerScreen::New, "no_controller_screen");
    TheUI.RegisterScreenType(ErrorScreen::New, "error_screen");
    TheUI.RegisterScreenType(XProceedScreen::New, "x_proceed_screen");
    TheUI.RegisterScreenType(MetaStartScreen::New, "meta_start_screen");
    TheUI.RegisterScreenType(MetaMainScreen::New, "meta_main_screen");
    TheUI.RegisterScreenType(ModeScreen::New, "mode_screen");
    TheUI.RegisterScreenType(RemixTypeScreen::New, "remix_type_screen");
    TheUI.RegisterScreenType(NumPlayersScreen::New, "num_players_screen");
    TheUI.RegisterScreenType(TipsScreen::New, "tips_screen");
    TheUI.RegisterScreenType(PupTipsScreen::New, "pup_tips_screen");
    TheUI.RegisterScreenType(MetaSkillScreen::New, "meta_skill_screen");
    TheUI.RegisterScreenType(PowerupScreen::New, "powerup_screen");
    TheUI.RegisterScreenType(MetaArenaScreen::New, "meta_arena_screen");
    TheUI.RegisterScreenType(MetaSongScreen::New, "meta_song_screen");
    TheUI.RegisterScreenType(RemixSongScreen::New, "remix_song_screen");
    TheUI.RegisterScreenType(SoloEndGameScreen::New, "solo_end_screen");
    TheUI.RegisterScreenType(AutoSaveDoneScreen::New, "solo_end_done_screen");
    TheUI.RegisterScreenType(MultiEndGameScreen::New, "m_end_screen");
    TheUI.RegisterScreenType(NetEndGameScreen::New, "fn_end_screen");
    TheUI.RegisterScreenType(PauseScreen::New, "pause_screen");
    TheUI.RegisterScreenType(FreqMakerPartConfigScreen::New, "f_maker_part_screen");
    TheUI.RegisterScreenType(FreqMakerEmblemScreen::New, "f_maker_emblem_screen");
    TheUI.RegisterScreenType(FreqMakerMainScreen::New, "f_maker_main_screen");
    TheUI.RegisterScreenType(FreqMakerCustomScreen::New, "f_maker_custom_screen");
    TheUI.RegisterScreenType(FreqMakerErrorScreen::New, "f_maker_error_screen");
    TheUI.RegisterScreenType(MultiFreqSelectScreen::New, "f_m_sel_screen");
    TheUI.RegisterScreenType(SelLoadedFreqScreen::New, "sel_loaded_f_screen");
    TheUI.RegisterScreenType(FreqConfirmScreen::New, "confirm_screen");
    TheUI.RegisterScreenType(UnlockScreen::New, "unlock_screen");
    TheUI.RegisterScreenType(PartUnlockScreen::New, "parts_unlock_screen");
    TheUI.RegisterScreenType(PartUnlockDoneScreen::New, "parts_unlock_done_screen");
    TheUI.RegisterScreenType(BossUnlockScreen::New, "boss_unlock_screen");
    TheUI.RegisterScreenType(BossJourneyScreen::New, "boss_journey_screen");
    TheUI.RegisterScreenType(ArenaUnlockScreen::New, "arena_unlock_screen");
    TheUI.RegisterScreenType(SongDecryptScreen::New, "song_decrypt_screen");
    TheUI.RegisterScreenType(FreestyleLapTipScreen::New, "freestyl_lap_tip_screen");
    TheUI.RegisterScreenType(SetupRemixSaveScreen::New, "setup_remix_save_screen");
    TheUI.RegisterScreenType(MultiSetupRemixSaveScreen::New, "multi_setup_remix_save_screen");
    TheUI.RegisterScreenType(ChooseMemCardScreen::New, "choose_mem_card_screen");
    TheUI.RegisterScreenType(RemixOrFreqScreen::New, "mc_r_or_f_screen");
    TheUI.RegisterScreenType(RemixCopyDelScreen::New, "mc_remix_screen");
    TheUI.RegisterScreenType(FreqCopyDelScreen::New, "mc_freq_screen");
    TheUI.RegisterScreenType(RemixSelectScreen::New, "sel_remix_screen");
    TheUI.RegisterScreenType(RemixDiscardScreen::New, "remix_discard_screen");
    TheUI.RegisterScreenType(RemixCreateModeScreen::New, "remix_create_mode_screen");
    TheUI.RegisterScreenType(RemixControllerScreen::New, "remix_controller_screen");
    TheUI.RegisterScreenType(MainOptionsScreen::New, "main_options_screen");
    TheUI.RegisterScreenType(GameOptionsScreen::New, "game_options_screen");
    TheUI.RegisterScreenType(ControllerConfigScreen::New, "control_config_screen");
    TheUI.RegisterScreenType(KeyboardScreen::New, "keyboard_screen");
    TheUI.RegisterPanelType(KeyboardPanel::New, "kb_panel");
    TheUI.RegisterPanelType(ControllerPanel::New, "controller_map_panel");
    TheUI.RegisterScreenType(NetCreateConfigScreen::New, "create_config_screen");
    TheUI.RegisterScreenType(NetInetDisconnect::New, "disconnect_internet_screen");
    TheUI.RegisterScreenType(NetLobbyConnect::New, "net_connect_lobby_screen");
    TheUI.RegisterScreenType(NetLobbyDisconnect::New, "net_disconnect_lobby_screen");
    TheUI.RegisterScreenType(NetLoginScreen::New, "net_login_screen");
    TheUI.RegisterScreenType(NetCreateUserScreen::New, "net_create_user_screen");
    TheUI.RegisterScreenType(NetChangePasswordScreen::New, "net_setup_pass_screen");
    TheUI.RegisterScreenType(NetCreateAccount::New, "net_create_account_screen");
    TheUI.RegisterScreenType(NetChangePassword::New, "net_change_pass_screen");
    TheUI.RegisterScreenType(NetServerSelScreen::New, "net_server_sel_screen");
    TheUI.RegisterScreenType(NetServerLogin::New, "net_server_login_screen");
    TheUI.RegisterScreenType(NetEULAScreen::New, "net_eula_screen");
    TheUI.RegisterScreenType(NetWelcomeScreen::New, "net_welcome_screen");
    TheUI.RegisterScreenType(NetHostingScreen::New, "net_hosting_screen");
    TheUI.RegisterScreenType(NetHostAttemptScreen::New, "net_host_attempt_screen");
    TheUI.RegisterScreenType(NetSearchScreen::New, "net_search_screen");
    TheUI.RegisterScreenType(NetSortedScreen::New, "net_sorted_screen");
    TheUI.RegisterScreenType(NetLaunchpadQuitScreen::New, "net_launchpad_quit_screen");
    TheUI.RegisterScreenType(NetSwitchLobbyScreen::New, "net_switch_lobby_screen");
    TheUI.RegisterScreenType(NetNewChatroomScreen::New, "net_create_lobby_screen");
    TheUI.RegisterScreenType(NetJoinLPadScreen::New, "net_join_lpad_screen");
    TheUI.RegisterScreenType(NetLpadScreen::New, "lpad_screen");
    TheUI.RegisterScreenType(NetLaunchScreen::New, "net_launch_screen");
    TheUI.RegisterScreenType(ShareRemixScreen::New, "share_remix_screen");
    TheUI.RegisterScreenType(BootScreen::New, "boot_screen");
    TheUI.RegisterScreenType(UploadNoteScreen::New, "upload_note_screen");
    TheUI.RegisterScreenType(NetDoUploadScreen::New, "do_upload_screen");
    TheUI.RegisterScreenType(NetDoDownloadScreen::New, "do_download_screen");
    TheUI.RegisterScreenType(NetRankingScreen::New, "net_ranking_screen");
    TheUI.RegisterScreenType(PlayerSearchSetupScreen::New, "setup_p_search_screen");
    TheUI.RegisterScreenType(NetDoPlayerSearchScreen::New, "do_p_search_screen");
    TheUI.RegisterScreenType(PlayerSearchResultsScreen::New, "player_found_screen");
    TheUI.RegisterScreenType(SetupNewChatroomScreen::New, "new_lobby_screen");
    TheUI.RegisterScreenType(ReadOnlyCheckScreen::New, "read_only_check_screen");
    TheUI.RegisterScreenType(ReadOnlySaveScreen::New, "read_only_save_screen");
    TheUI.RegisterScreenType(NetReadOnlyWarnScreen::New, "net_ro_warn_screen");
    TheUI.RegisterScreenType(SaveFreqScreen::New, "save_freq_screen");
    TheUI.RegisterScreenType(SaveEditedFreqScreen::New, "save_edited_freq_screen");
    TheUI.RegisterPanelType(UnlockAvatarPanel::New, "unlock_avatar_panel");
    TheUI.RegisterPanelType(BonusPicPanel::New, "bonus_pic_panel");
    TheUI.RegisterPanelType(FreqPanel::New, "freq_panel");
    TheUI.RegisterPanelType(AvatarPanel::New, "avatar_panel");
    TheUI.RegisterPanelType(HelpPanel::New, "help_panel");
    TheUI.RegisterPanelType(DialogPanel::New, "dialog_panel");
    TheUI.RegisterPanelType(MCDialogPanel::New, "mc_dialog_panel");
    TheUI.RegisterPanelType(SongPicPanel::New, "song_pic_panel");
    TheUI.RegisterPanelType(SoloWinStatsPanel::New, "solo_win_stats_panel");
    TheUI.RegisterPanelType(SoloLoseTipsPanel::New, "solo_lose_tip_panel");
    TheUI.RegisterPanelType(MultiGameStatsPanel::New, "multi_stats_panel");
    TheUI.RegisterPanelType(FullRankedPanel::New, "full_ranked_panel");
    TheUI.RegisterPanelType(NetRankedPlayersPanel::New, "fn_main_ranked_panel");
    TheUI.RegisterPanelType(NetSortedLaunchpadsPanel::New, "fn_sorted_launchpads_panel");
    TheUI.RegisterPanelType(NetSortedDataPanel::New, "fn_sorted_data_panel");
    TheUI.RegisterPanelType(NetSongPanel::New, "net_song_panel");
    TheUI.RegisterPanelType(NetLPadGamePanel::New, "lpad_data_game_panel");
    TheUI.RegisterPanelType(NetLPadPlayerPanel::New, "lpad_data_player_panel");
    TheUI.RegisterPanelType(FreqSelPanel::New, "f_sel_panel");
    TheUI.RegisterPanelType(LaunchMTVPanel::New, "launch_mtv_panel");
    TheUI.RegisterPanelType(NetFoundPlayerPanel::New, "found_player_panel");
    TheUI.RegisterComponentType(FreqLoadList::New, "freqs_list_comp");
    TheUI.RegisterComponentType(RemixLoadList::New, "remix_list_comp");
    TheUI.RegisterComponentType(SongSelList::New, "song_list_comp");
    TheUI.RegisterComponentType(NetMainPlayersList::New, "players_list_comp");
    TheUI.RegisterComponentType(NetRankedPlayersList::New, "ranked_list_comp");
    TheUI.RegisterComponentType(NetMainLaunchpadsList::New, "launchpads_list_comp");
    TheUI.RegisterComponentType(NetSortedLaunchpadsList::New, "sorted_launchpads_list_comp");
    TheUI.RegisterComponentType(RemixDownloadList::New, "downloads_list_comp");
    TheUI.RegisterComponentType(KeyboardKey::New, "key_comp");
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
    // Yes, the binary times the unload with a timer it never reads.
    Timer timer{};
    timer.Start();
    mArena->Unload();
    delete mMusic;
    mMusic = nullptr;
    UnloadBanks();
    mFirstBoot = 0;
    timer.Stop();

    if (TheUI.mEditMode) {
        TheUI.UnloadGroup(kFrontEndGroup);
    }

    if (TheGameDb->GetDemo() != nullptr) {
        mNextScreen = kStartScreen;
        return;
    }

    const int nRuleSet = TheGameDb->mRuleSet;
    const int nCommunity = TheGameDb->mCommunity;
    if (TheGameDb->mTutorial) {
        mNextScreen = nRuleSet == GameDb::kRuleSetRemix ? kSoloRemixModeScreen : kSoloArenaScreen;
    } else if (nCommunity == GameDb::kCommunityLocal) {
        if (nRuleSet == GameDb::kRuleSetDuel) {
            mNextScreen = kDuelSelectSongScreen;
        } else if (nRuleSet == GameDb::kRuleSetGame) {
            mNextScreen = TheGameDb->mLoadRemix ? kMultiCustomScreen : kMultiSelectSongScreen;
        } else {
            mNextScreen = kMultiEndRemixScreen;
        }
    } else if (nCommunity == GameDb::kCommunityOnline) {
        if (TheNetLaunchpad == nullptr) {
            mNextScreen = kLostLaunchpadScreen;
        } else if (nRuleSet == GameDb::kRuleSetGame || nRuleSet == GameDb::kRuleSetDuel) {
            if (!TheNetLaunchpad->IsGuest()) {
                mNextScreen = kNetLaunchHostScreen;
            } else if (nRuleSet != GameDb::kRuleSetGame || !TheGameDb->mLoadRemix) {
                mNextScreen = kNetLaunchGuestScreen;
            } else if (TheGameDb->mRemixReadOnly) {
                mNextScreen = kNetEndSaveReadOnlyScreen;
                dynamic_cast<SaveRemixScreen *>(TheUI.FindScreen(kSaveRemixScreen, false))
                    ->mRemixName = TheGameDb->GetRemixInfo()->mName;
            } else {
                mNextScreen = kNetEndRemixScreen;
            }
        } else if (nRuleSet == GameDb::kRuleSetRemix) {
            mNextScreen = kNetEndRemixScreen;
        } else {
            DebugWarn(kNoEndScreenLog);
        }
    } else if (strcmp(TheUI.mCurrentScreen->mName, kLaunchpadLaunchScreen) != 0 &&
               strcmp(TheUI.mCurrentScreen->mName, kSoloTutorialLaunchScreen) != 0) {
        mNextScreen = kStartScreen;
    } else if (nRuleSet == GameDb::kRuleSetGame) {
        mNextScreen = TheGameDb->mLoadRemix ? kSoloCustomScreen : kSoloMetaScreen;
    } else {
        mNextScreen =
            TheGameDb->GetRemixInfo()->mReadOnly ? kSoloRemixModeScreen : kSoloRemixScreen;
    }
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
    const int nRuleSet = TheGameDb->mRuleSet;
    if (nRuleSet != GameDb::kRuleSetRemix || TheGameDb->GetNetRemixEnded()) {
        return;
    }

    const int nCommunity = TheGameDb->mCommunity;
    if (nCommunity == GameDb::kCommunitySolo) {
        mNextScreen = kSoloRemixModeScreen;
    } else if (nCommunity == GameDb::kCommunityLocal) {
        mNextScreen = kMultiRemixModeScreen;
    } else if (nCommunity == GameDb::kCommunityOnline) {
        if (TheNetLaunchpad == nullptr) {
            mNextScreen = kLostLaunchpadScreen;
        } else if (TheNetLaunchpad->IsGuest()) {
            mNextScreen = kNetLaunchGuestScreen;
        } else {
            mNextScreen = kNetLaunchHostScreen;
        }
    }
}

void Metagame::ExitLeaving() {
}

void Metagame::StartRestarting() {
    const int nSkillLevel = TheGameDb->mSkillLevel;
    if (TheGameDb->IsWinSequence()) {
        TheGameDb->SetArena(kWinArena);
    } else {
        const char *pszSong = TheGameDb->GetProfile(0)->FindFirstUnfinishedSong(nSkillLevel);
        if (pszSong != nullptr) {
            TheGameDb->SetSong(pszSong);
        } else {
            TheGameDb->SetSong(LookupSymbol(kFallbackSong));
        }
    }
    mSharedMusicFadedIn = 0;
    LoadFxBanks();
    mEffects.Apply();
    mLeaveTime = SystemMs() + kLeaveDelayMs;
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
    if (TheGameDb->mLoadRemix || TheGameDb->mPracticeMode || TheGameDb->GetDemo() != nullptr) {
        return;
    }

    SongRecord record;
    const SongEntry entry{TheGameDb->FindSong(TheGameDb->mSong.c_str())};
    record.mSong = entry.GetName();
    record.mScore = static_cast<unsigned short>(TheGameDb->GetPlayerScore(0));
    record.mSkillLevel = static_cast<unsigned char>(TheGameDb->mSkillLevel);
    record.mBestStreak = static_cast<unsigned char>(TheGameDb->GetBestStreak());
    record.mBlasted = static_cast<unsigned char>(TheGameDb->GetEnergized() * kPercent);
    record.mPercentDone = static_cast<unsigned char>(TheGameDb->GetProgress() * kPercent);
    record.mFullMixBars = static_cast<unsigned char>(TheGameDb->GetFullMixBars());
    TheGameDb->GetProfile(0)->RecordResult(&record, &mUnlocks);
}

void Metagame::QueueUnlocks() {
    int nBossIndex = -1;
    bool bPartsQueued = false;
    bool bCampaignEnded = false;
    bool bSongQueued = false;
    mUnlockScreens.clear();

    for (unsigned int i = 0; i < mUnlocks.size(); ++i) {
        const UnlockableItem &item = mUnlocks[i];
        switch (item.mKind) {
        case UnlockableItem::kKindArena: {
            mNextScreen = kUnlockArenaScreen;
            std::vector<const char *> arenas;
            TheGameDb->GetUnlockedArenas(&arenas, TheGameDb->mSkillLevel, true, false);
            mArena->SetRevealArena(static_cast<int>(arenas.size()));
            if (mDialogAction == kDialogActionResume) {
                mDialogAction = kDialogActionEnd;
            }
            break;
        }
        case UnlockableItem::kKindBossSong:
            dynamic_cast<SongDecryptScreen *>(TheUI.FindScreen(kSongDecryptScreen, false))
                ->SetSong(item.mName);
            mUnlockScreens.push_back(kUnlockEventSong);
            nBossIndex = i;
            break;
        case UnlockableItem::kKindSong:
        case UnlockableItem::kKindBonusSong:
            dynamic_cast<SongDecryptScreen *>(TheUI.FindScreen(kSongDecryptScreen, false))
                ->SetSong(item.mName);
            mUnlockScreens.push_back(kUnlockEventSong);
            bSongQueued = true;
            break;
        case UnlockableItem::kKindCampaignEnd:
            bCampaignEnded = true;
            break;
        default: {
            PartUnlockScreen *pScreen =
                dynamic_cast<PartUnlockScreen *>(TheUI.FindScreen(kUnlockPartsScreen, false));
            if (pScreen->AddItem(item) && !bPartsQueued) {
                mUnlockScreens.push_back(kUnlockEventParts);
                bPartsQueued = true;
            }
            break;
        }
        }
    }

    if (bCampaignEnded && !bSongQueued) {
        mNextScreen = kStartScreen;
    }
    if (TheGameDb->GetProfile(0)->IsModified()) {
        mUnlockScreens.push_back(kUnlockEventSaveFreq);
    }
    if (TheGameDb->GetOptions()->mModified) {
        mUnlockScreens.push_back(kUnlockEventSaveSettings);
    }

    if (nBossIndex >= 0 && mDialogAction == kDialogActionResume) {
        dynamic_cast<BossUnlockScreen *>(TheUI.FindScreen(kUnlockBossScreen, false))->mSong =
            mUnlocks[nBossIndex].mName;
        mUnlockScreens.push_back(kUnlockEventBoss);
    } else if (TheGameDb->mCommunity == GameDb::kCommunitySolo &&
               !TheGameDb->GetProfile(0)->HasSeenFreestyleTip() &&
               mDialogAction == kDialogActionContinue) {
        mUnlockScreens.push_back(kUnlockEventFreestyleTip);
        TheGameDb->GetProfile(0)->SetSeenFreestyleTip();
    }

    mUnlocks.clear();
}

void Metagame::ShowDialog(DialogType type, DialogCallback pfnCallback, void *pUserData, int nPad) {
    mDialogCallback = pfnCallback;
    mDialogUserData = pUserData;
    mDialogShowing = 1;
    mDialogAction = kDialogActionNone;

    if (type == kDialogTutorialEnd) {
        ShowEndGameScreens(kDialogActionEnd);
        return;
    }

    const char *pszScreen = nullptr;
    if (type == kDialogPause) {
        const int nRuleSet = TheGameDb->mRuleSet;
        const int nCommunity = TheGameDb->mCommunity;
        if (TheGameDb->mTutorial) {
            pszScreen = kMultiGamePauseScreen;
        } else if (nCommunity == GameDb::kCommunitySolo) {
            pszScreen = nRuleSet == GameDb::kRuleSetGame ? kSoloPauseScreen : kSoloRemixPauseScreen;
        } else if (nCommunity == GameDb::kCommunityLocal &&
                   (nRuleSet == GameDb::kRuleSetGame || nRuleSet == GameDb::kRuleSetDuel)) {
            pszScreen = kMultiGamePauseScreen;
        } else {
            pszScreen = kMultiRemixPauseScreen;
        }
        dynamic_cast<PauseScreen *>(TheUI.FindScreen(pszScreen, false))->mPad = nPad;
    } else if (type == kDialogNoController) {
        pszScreen = kNoControllerScreen;
        dynamic_cast<NoControllerScreen *>(TheUI.FindScreen(pszScreen, false))->mPad = nPad;
    } else if (TheGameDb->mPracticeMode) {
        pszScreen = kPracticeEndScreen;
    } else if (type == kDialogSoloWon || type == kDialogSoloLost) {
        pszScreen = type == kDialogSoloWon ? kSoloWinScreen : kSoloLoseScreen;
        TheGfxManager.ShowHud(false);
    } else if (type == kDialogEndGame) {
        int nWinners = 0;
        for (int i = 0; i < TheGameDb->GetNumPlayers(); ++i) {
            if (TheGameDb->GetPlayerRank(i) == 0) {
                ++nWinners;
            }
        }
        const bool bOnline = TheGameDb->mCommunity == GameDb::kCommunityOnline;
        {
            String suffix;
            if (nWinners >= kNamedTieWinners) {
                suffix = FormatString(kTieCountFormat, TheGameDb->GetNumPlayers());
            } else if (nWinners >= kTieWinners) {
                suffix = kTieSuffix;
            } else {
                suffix = kWinSuffix;
            }
            pszScreen = FormatString(bOnline ? kOnlineEndScreenFormat : kLocalEndScreenFormat,
                                     TheGameDb->GetNumPlayers(),
                                     suffix.c_str());
        }
        TheGfxManager.ShowHud(false);
    }
    // Yes, the binary goes to a null screen for the remaining dialog types.
    TheUI.GotoScreen(pszScreen);
}

void Metagame::RecordPracticeResult() {
    SongRecord record;
    const SongEntry entry{TheGameDb->FindSong(TheGameDb->mSong.c_str())};
    record.mSong = entry.GetName();
    // Yes, the binary records a score of 100 for a practice song.
    record.mScore = kPracticeResult;
    record.mSkillLevel = static_cast<unsigned char>(TheGameDb->mSkillLevel);
    record.mBestStreak = static_cast<unsigned char>(TheGameDb->GetBestStreak());
    record.mPercentDone = kPracticeResult;
    record.mBlasted = 0;
    record.mFullMixBars = 0;
    TheGameDb->GetProfile(0)->RecordResult(&record, &mUnlocks);
}

void Metagame::ShowEndGameScreens(DialogAction action) {
    mDialogAction = action;
    if (TheGameDb->mTutorial) {
        if (action != kDialogActionEnd) {
            FinishDialog();
            return;
        }
        RecordPracticeResult();
    }
    if (action != kDialogActionEnd || TheGameDb->mCommunity != GameDb::kCommunitySolo) {
        FinishDialog();
        return;
    }
    if (TheGameDb->GetProfile(0)->IsModified() || TheGameDb->GetOptions()->mModified) {
        QueueUnlocks();
        AdvanceUnlocks();
    } else {
        FinishDialog();
    }
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
    if (mUnlockScreens.empty()) {
        FinishDialog();
        return;
    }

    switch (mUnlockScreens.front()) {
    case kUnlockEventBoss:
        TheUI.GotoScreen(kUnlockBossScreen);
        break;
    case kUnlockEventParts:
        TheUI.GotoScreen(kUnlockPartsScreen);
        break;
    case kUnlockEventSong:
        TheUI.GotoScreen(kSongDecryptScreen);
        break;
    case kUnlockEventSaveFreq: {
        SaveFreqScreen *pScreen =
            dynamic_cast<SaveFreqScreen *>(TheUI.FindScreen(kAutoSaveFreqScreen, false));
        pScreen->mOverwriteStatus = 1;
        TheUI.GotoScreen(pScreen);
        break;
    }
    case kUnlockEventSaveSettings:
        TheUI.GotoScreen(kAutoSaveSettingsScreen);
        break;
    case kUnlockEventFreestyleTip:
        TheUI.GotoScreen(kFreestyleTipScreen);
        break;
    default:
        break;
    }
    mUnlockScreens.pop_front();
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
        FxMidi::PlayBack();
    }
    mLastSelectButton = kPadNone;
    return false;
}

bool Metagame::OnLaunchpadAborted(LaunchpadAbortedMsg *pMsg) {
    if (pMsg->mReason == LaunchpadAbortedMsg::kReasonNone) {
        return false;
    }
    UIScreen *pCurrent = TheUI.mCurrentScreen;
    // Yes, the binary reads the next screen of a null current screen.
    if (pCurrent != nullptr && strcmp(pCurrent->mName, kLobbyErrorScreen) == 0) {
        return false;
    }
    UIScreen *pNext = pCurrent->mNextScreen;
    if (pNext != nullptr && strcmp(pNext->mName, kLobbyErrorScreen) == 0) {
        return false;
    }

    const char *pszError;
    switch (pMsg->mReason) {
    case LaunchpadAbortedMsg::kReasonIncompatible:
        pszError = kLaunchpadIncompatibleError;
        break;
    case LaunchpadAbortedMsg::kReasonHost:
        pszError = kLaunchpadHostError;
        break;
    case LaunchpadAbortedMsg::kReasonBoot:
        pszError = kLaunchpadBootError;
        break;
    case LaunchpadAbortedMsg::kReasonDied:
        pszError = kLaunchpadDiedError;
        break;
    case LaunchpadAbortedMsg::kReasonBad:
        pszError = kLaunchpadBadError;
        break;
    default:
        pszError = kLaunchpadLostError;
        break;
    }
    const String message(TheLocale.Localize(FormatString(kErrorMessageFormat, pszError), true));
    TransitionErrorScreen *pError =
        dynamic_cast<TransitionErrorScreen *>(TheUI.FindScreen(kLaunchpadErrorScreen, false));
    pError->mMessage = message.c_str();

    if (mReserved144) {
        TheUI.GotoScreen(pError);
    } else if (strcmp(kLostLobbyScreen, mNextScreen) != 0) {
        mNextScreen =
            TheGameDb->mRuleSet == GameDb::kRuleSetRemix && TheGameDb->GetNetRemixEnded() == 1 ?
                kNetEndRemixScreen :
                kLostLaunchpadScreen;
        mNetScreenPending = 1;
        mNetScreen = kLaunchpadErrorScreen;
    }
    return false;
}

bool Metagame::OnLostInternet(LobbyConnectionLostMsg *pMsg) {
    TransitionErrorScreen *pError =
        dynamic_cast<TransitionErrorScreen *>(TheUI.FindScreen(kLobbyErrorScreen, false));
    if (pMsg->mError == LobbyConnectionLostMsg::kInternetDown) {
        printf(kInternetDownLog);
        pError->mMessage = TheLocale.Localize(kLostInternetToken, true);
        pError->ClearTransitions();
        pError->AddTransition(kErrorOkComponent, kPadNone, kNetConfigScreen);
    } else {
        printf(kOtherErrorLog);
        pError->mMessage = TheLocale.Localize(kLostLobbyToken, true);
        pError->ClearTransitions();
        pError->AddTransition(kErrorOkComponent, kPadNone, kNetPortalScreen);
    }

    if (mReserved144) {
        TheUI.GotoScreen(pError);
    } else {
        mNextScreen =
            TheGameDb->mRuleSet == GameDb::kRuleSetRemix && TheGameDb->GetNetRemixEnded() == 1 ?
                kNetEndRemixScreen :
                kLostLobbyScreen;
        mNetScreenPending = 1;
        mNetScreen = kLobbyErrorScreen;
    }
    return false;
}

bool Metagame::OnGameParamsUpdate(GameParamsUpdateMsg *pMsg) {
    TheGameDb->SetGameParams(&pMsg->mParams);
    RemixInfo info = *TheGameDb->GetRemixInfo();
    info.mReadOnly = static_cast<signed char>(pMsg->mParams.mRemixReadOnly);
    TheGameDb->SetRemix(&info);
    auto *pLaunchpad = dynamic_cast<NetLpadScreen *>(TheUI.mCurrentScreen);
    if (pLaunchpad != nullptr) {
        pLaunchpad->UpdateGameParams(&pMsg->mParams);
    }
    return false;
}

bool Metagame::OnLoadGame([[maybe_unused]] Message *pMsg) {
    Campaign profile(*TheGameDb->GetProfile(0));
    TheGameDb->ClearPlayers();
    if (TheNetLaunchpad == nullptr) {
        NetJoinLPadScreen::ShowLaunchpadLost();
        return false;
    }
    mReserved144 = 0;
    std::list<NetLaunchpadPlayer> *pPlayers = TheNetLaunchpad->GetPlayers();
    for (const auto &player : *pPlayers) {
        if (player.mId == TheNetLaunchpad->IsGuest()) {
            TheGameDb->AddPlayer(kLocalPlayer, &profile, player.mId, player.mDifficulty);
            break;
        }
    }
    for (const auto &player : *pPlayers) {
        if (player.mId == TheNetLaunchpad->IsGuest()) {
            continue;
        }
        Campaign other;
        other.mName = player.mPlayer.mName.c_str();
        other.mAvatar = player.mPlayer.mAvatar;
        TheGameDb->AddPlayer(kRemotePlayer, &other, player.mId, player.mDifficulty);
    }

    if (TheGameDb->mRuleSet == GameDb::kRuleSetRemix) {
        if (TheGameDb->mLoadRemix == 0) {
            TheGameDb->SetRemixBuffer(0);
            TheGameDb->SetRemixActive(1);
            RemixInfo info;
            info.Reset();
            strncpy(info.mName, kUntitledRemix, kRemixNameCopyLength);
            info.mSource = RemixInfo::kSourceNone;
            info.mDataSize = 0;
            info.mDate.ReadClock();
            info.mReadOnly = 0;
            TheGameDb->SetRemix(&info);
        }
        RemixInfo info = *TheGameDb->GetRemixInfo();
        strcpy(info.mSong, TheGameDb->mSong.c_str());
        for (int i = 0; i < kRemixInfoCreatorCount; ++i) {
            const char *pszCreator =
                i < TheGameDb->GetNumPlayers() ? TheGameDb->GetPlayerName(i) : "";
            strncpy(info.mCreators[i], pszCreator, kRemixCreatorCopyLength);
        }
        TheGameDb->SetRemix(&info);
    }
    TheUI.GotoScreen(kNetLaunchSequenceScreen);
    return false;
}

bool Metagame::OnShareRemixBegin([[maybe_unused]] Message *pMsg) {
    if (strcmp(TheUI.mCurrentScreen->mName, kGuestLaunchpadScreen) == 0 ||
        strcmp(TheUI.mCurrentScreen->mName, kGuestLaunchpadPlayScreen) == 0 ||
        strcmp(TheUI.mCurrentScreen->mName, kKeyboardScreen) == 0) {
        if (TheNetLaunchpad != nullptr) {
            TheUI.GotoScreen(kShareRemixScreen);
        } else {
            NetJoinLPadScreen::ShowLaunchpadLost();
        }
    }
    return false;
}

bool Metagame::OnShareRemixProgress(ShareRemixProgressMsg *pMsg) {
    const char *const screens[] = {kHostLaunchpadScreen,
                                   kHostLaunchpadPlayScreen,
                                   kGuestLaunchpadScreen,
                                   kGuestLaunchpadPlayScreen,
                                   kShareRemixScreen,
                                   kNetLaunchScreen,
                                   kKeyboardScreen};
    for (const char *pszScreen : screens) {
        if (strcmp(TheUI.mCurrentScreen->mName, pszScreen) == 0) {
            auto *pShare =
                dynamic_cast<ShareRemixScreen *>(TheUI.FindScreen(kShareRemixScreen, false));
            // Yes, the binary discards the result, and passes on a null screen.
            (void)pShare->HandleProgress(pMsg);
            break;
        }
    }
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
        return OnLaunchpadAborted(static_cast<LaunchpadAbortedMsg *>(pMsg));
    }
    if (nType == g_nLobbyConnectionLostMsgType) {
        return OnLostInternet(static_cast<LobbyConnectionLostMsg *>(pMsg));
    }
    if (nType == g_nGameParamsUpdateMsgType) {
        return OnGameParamsUpdate(static_cast<GameParamsUpdateMsg *>(pMsg));
    }
    if (nType == g_nShareRemixBeginMsgType) {
        return OnShareRemixBegin(pMsg);
    }
    if (nType == g_nShareRemixProgressMsgType) {
        return OnShareRemixProgress(static_cast<ShareRemixProgressMsg *>(pMsg));
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
    mMusic = new Mix;
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
