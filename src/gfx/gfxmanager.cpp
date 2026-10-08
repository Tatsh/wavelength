#include "gfx/gfxmanager.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

#include "game/campaign.h"
#include "game/gamedb.h"
#include "game/triggermgr.h"
#include "gfx/camslide.h"
#include "gfx/gfxconfig.h"
#include "os/debug.h"
#include "os/locale.h"
#include "os/system.h"
#include "os/timer.h"
#include "rnd/environ.h"
#include "rnd/manager.h"
#include "rnd/ps.h"
#include "rnd/rndrenderer.h"
#include "script/scriptfunction.h"
#include "synth/fxmidi.h"

namespace {

// The ticks of one bar of a song.
constexpr int kTicksPerBar = 1920;

// The frame the triggers of the metagame see before a world starts.
constexpr float kPreSongTick = -7680.0f;

// The tick and the time the display starts at before a song is built.
constexpr float kNoTick = -1.0e9f;

// The scroll speed and its reciprocal before a song is built.
constexpr float kNoScrollSpeed = -1.0f;

// The value of mHudFlags before `toggle_hud` changes it.
constexpr int kInitialHudFlags = 5;

// The bits of mHudFlags.
constexpr int kHudFlagDraw = 1;
constexpr int kHudFlagStatsText = 2;
constexpr int kHudFlagSkipMask = 5;
constexpr int kHudFlagSkipValue = 4;
constexpr int kHudFlagLast = 7;
constexpr int kNumHudFlagValues = 8;

// The number of shapes the `circular` cheat cycles through.
constexpr int kNumTunnelShapes = 3;

// The value of the ready test of a stream, larger than any file.
constexpr int kStreamReadyBytes = 999999999;

// The size of the path of a compiled data file.
constexpr int kMaxPathLength = 128;

// The load flags of a file of the `load` list.
constexpr int kLoaderFlags = 5;

// The difference in length of the camera path above which it is reported.
constexpr float kCamPathTolerance = 50.0f;

// The number of the first localview, which the view names count from.
constexpr int kFirstLocalView = 1;

// The scroll speed of a song with no entry in the configuration.
constexpr float kDefaultTempo = 120.0f;
constexpr float kDefaultTunnelScale = 1.0f;
constexpr float kDefaultSongTicks = 184320.0f;
constexpr float kDefaultCamPathLength = 850.0f;

// The scale of a percentage.
constexpr float kPercent = 100.0f;

// The energy shown in practice, and the fraction of its zone.
constexpr float kPracticeEnergy = 1.0f;
constexpr float kPracticeZoneFraction = 100.0f;

// The zones of the energy meter.
constexpr int kEnergyZoneLow = 0;
constexpr int kEnergyZoneMiddle = 1;
constexpr int kEnergyZoneHigh = 2;

// The look of the echo of the `drugs` cheat.
constexpr float kDrugsAlpha = 0.9f;
constexpr int kDrugsOffset = 60;

// The components of the blur rectangle, x, y, w, and h.
constexpr int kNumRectComponents = 4;

// The multiplier from which the text of a multiplier is shown.
constexpr int kFirstShownMultiplier = 2;

// The length of the texts of SetStreakMultiplier() and ShowPendingPoints().
constexpr int kMultiplierTextLength = 16;

// The pose of the first player's avatar once the play field is torn down.
constexpr char kTeardownPose[] = "fm_none";

// The `HealthEvent` value of a won song.
constexpr int kFullHealth = 100;

// The end states EndEvent() receives.
constexpr int kEndStateWon = 0;
constexpr int kEndStateLost = 1;
constexpr int kEndStatePractice = 3;

// The arena of the remix rule set and the arena of the `no_arena` cheat.
constexpr char kTutorialArena[] = "Tutorial";
constexpr char kNoArena[] = "No arena";

// The names of the instruments and of the player colours in the configuration.
const char *const kInstrumentNames[] = {"drum", "bass", "synth", "guitar", "vocal", "fx"};
const char *const kInstrumentMacros[] = {"kDrum", "kBass", "kSynth", "kGuitar", "kVocal", "kFX"};
const char *const kPlayerColorNames[] = {"green", "purple", "red", "yellow", "null", "unknown"};

// The format of the unrecognised sub-commands of the display commands.
constexpr char kUnrecognizedCommand[] = "unrecognized %s command: %s";

// The value of a node of a command that a short command leaves at its default.
constexpr float kDefaultPosition = -1.0f;
constexpr int kDefaultDialogPosition = -1;

// The indices of the arguments of the display commands.
enum CommandArg {
    kArgCommand = 0,
    kArg1 = 1,
    kArg2 = 2,
    kArg3 = 3,
    kArg4 = 4,
    kArg5 = 5,
    kArg6 = 6,
};

// Report a sub-command a display command does not know.
void WarnUnrecognized(DataArray *pCommand, const char *pszSubCommand) {
    DebugWarn(kUnrecognizedCommand, pCommand->Sym(kArgCommand), pszSubCommand);
}

// Draw a drawable that may be null, as the display draws the current camera and environment.
void DrawIfSet(Rnd::Drawable *pDraw) {
    if (pDraw != nullptr) {
        pDraw->Draw();
    }
}

// Bring a view's transforms up to date and draw it.
void DrawView(Rnd::View *pView) {
    static_cast<Rnd::Transformable *>(pView)->UpdateWorldXfm(nullptr, 0);
    static_cast<Rnd::Drawable *>(pView)->Draw();
}

// Find a view of the scene.
Rnd::View *FindView(const char *pszName) {
    return dynamic_cast<Rnd::View *>(Rnd::TheManager.Find(pszName));
}

} // namespace

// The static initialiser of the unit, and the two routines that run it to construct and to destroy
// the colour tables and TheGfxManager.
// NTSC-U/C: 0x001b6258, PAL: 0x001beff8
// NTSC-U/C: 0x001b6500, PAL: 0x001bf2a0
// NTSC-U/C: 0x001b6520, PAL: 0x001bf2c0

Color GfxManager::sInstrumentColors[kNumInstruments] = {
    {1.0f, 0.1f, 1.0f, 1.0f},
    {0.35f, 0.35f, 1.0f, 1.0f},
    {1.0f, 1.0f, 0.0f, 1.0f},
    {0.75f, 0.1f, 0.1f, 1.0f},
    {0.0f, 1.0f, 0.0f, 1.0f},
    {1.0f, 0.5f, 0.0f, 1.0f},
};

Color GfxManager::sInstrumentBgColors[kNumInstruments] = {
    {0.5f, 0.0f, 0.5f, 1.0f},
    {0.0f, 0.5f, 0.75f, 1.0f},
    {0.95f, 0.65f, 0.0f, 1.0f},
    {0.65f, 0.0f, 0.0f, 1.0f},
    {0.0f, 0.65f, 0.0f, 1.0f},
    {0.75f, 0.35f, 0.0f, 1.0f},
};

Color GfxManager::sPlayerColors[kNumPlayerColors] = {
    {0.0f, 1.0f, 0.0f, 1.0f},
    {0.65f, 0.0f, 1.0f, 1.0f},
    {1.0f, 0.0f, 0.0f, 1.0f},
    {1.0f, 1.0f, 0.0f, 1.0f},
    {1.0f, 1.0f, 1.0f, 1.0f},
    {0.0f, 1.0f, 1.0f, 1.0f},
};

Color GfxManager::sPlayerBgColors[kNumPlayerColors] = {
    {0.0f, 0.5f, 0.0f, 1.0f},
    {0.45f, 0.0f, 0.45f, 1.0f},
    {0.75f, 0.0f, 0.0f, 1.0f},
    {0.75f, 0.5f, 0.0f, 1.0f},
    {1.0f, 1.0f, 1.0f, 1.0f},
    {0.0f, 1.0f, 1.0f, 1.0f},
};

DataArray *GfxManager::sMergesAlwaysAllowed;
float GfxManager::sEnergyHighZone = 0.666666f;
float GfxManager::sEnergyLowZone = 0.333333f;
int GfxManager::sInitialized;
int GfxManager::sTerminated;

GfxManager TheGfxManager;

#pragma mark - GfxLoader

GfxManager::GfxLoader::GfxLoader(bool bDone, bool bMergeAll, DataArray *pForceMerge)
    : mLoader(nullptr), mForceMerge(pForceMerge), mDone(bDone), mMergeAll(bMergeAll) {
}

GfxManager::GfxLoader::~GfxLoader() = default;

int GfxManager::GfxLoader::ShouldLoad(Rnd::Object *pExisting,
                                      const char *pszName,
                                      const char *pszClass) {
    if (pExisting == nullptr) {
        return 1;
    }
    if (mForceMerge != nullptr) {
        for (int i = 1; i < mForceMerge->Size(); ++i) {
            if (strcmp(pszName, mForceMerge->Sym(i)) == 0) {
                return 1;
            }
        }
    }
    if (mMergeAll != 0 || strcmp(pszClass, "Tex") != 0) {
        return 0;
    }
    int i = 1;
    while (i < sMergesAlwaysAllowed->Size() && strcmp(pszName, sMergesAlwaysAllowed->Sym(i)) != 0) {
        ++i;
    }
    if (i == sMergesAlwaysAllowed->Size()) {
        mSkipped.Printf("  %s\n", pszName);
    }
    return 0;
}

void GfxManager::GfxLoader::Start(const char *pszFile, int nFlags) {
    mLoader = Rnd::TheManager.AddLoader(pszFile, nFlags, this, nullptr);
}

void GfxManager::GfxLoader::Finish() {
    mDone = 1;
    if (mSkipped.mLength != 0) {
        DebugNotify(
            "These objects in %s\nwon't be loaded because they\nalready exist, and are not\n"
            "in the allowed list:\n%s",
            mLoader->mFile.c_str(),
            mSkipped.c_str());
    }
    mSkipped.Clear();
    mForceMerge = nullptr;
}

#pragma mark - Construction

GfxManager::GfxManager()
    : mVortex(nullptr), mVortexAfterViews(0), mLoadStage(kLoadStageHud), mHudLoader(nullptr),
      mLoaded(0), mLoadStartMs(0.0f), mReserved34(0), mState(kStateIdle), mLastTick(kNoTick),
      mLastTime(kNoTick), mScrollSpeed(kNoScrollSpeed), mInvScrollSpeed(kNoScrollSpeed),
      mSongTicks(0.0f), mCamPathScale(kNoScrollSpeed), mVictoryLap(0), mArenaUnlocked(0),
      mNoArena(0), mMonkeyGems(0), mDrugs(0), mBlackPanels(0), mNoPanels(0), mTunnelShape(0),
      mHudFlags(kInitialHudFlags), mTunnel(nullptr), mOverlay(nullptr), mArena(nullptr),
      mTunnelView(nullptr), mMainView(nullptr), mPanelView(nullptr), mLeader(0), mWinnerDrawn(0),
      mWinner(-1), mMsPerTick(nullptr), mMultiplierFormat(nullptr) {
    for (int i = kMaxPlayers - 1; i >= 0; --i) {
        mShips[i] = nullptr;
    }
}

GfxManager::~GfxManager() = default;

#pragma mark - Script commands

void GfxManager::OnHandleBeat([[maybe_unused]] DataArray *pCommand,
                              [[maybe_unused]] void *pUserData) {
    TheGfxManager.HandleBeat(TheTriggerMgr.mNote);
}

void GfxManager::OnDebug(DataArray *pCommand, [[maybe_unused]] void *pUserData) {
    TheGfxManager.Debug(pCommand);
}

void GfxManager::OnCheat(DataArray *pCommand, [[maybe_unused]] void *pUserData) {
    TheGfxManager.Cheat(pCommand);
}

void GfxManager::OnShow(DataArray *pCommand, [[maybe_unused]] void *pUserData) {
    TheGfxManager.Show(pCommand);
}

void GfxManager::OnAddCheckpoint(DataArray *pCommand, [[maybe_unused]] void *pUserData) {
    const char *pszLabel = nullptr;
    if (pCommand->Size() > kArg3) {
        pszLabel = TheLocale.Localize(pCommand->Sym(kArg3), true);
    }
    const int nBar = static_cast<int>(TheGameDb->mSongTick / kTicksPerBar);
    const float fTick = static_cast<float>((pCommand->Int(kArg1) + nBar) * kTicksPerBar);
    TheGfxManager.AddCheckpoint(pszLabel, fTick, pCommand->Float(kArg2), 1.0f);
}

void GfxManager::OnViewPullback(DataArray *pCommand, [[maybe_unused]] void *pUserData) {
    TheGfxManager.SetScrollSpeed(pCommand->Float(kArg1));
}

void GfxManager::OnButtonIcon(DataArray *pCommand, [[maybe_unused]] void *pUserData) {
    const int nLane = pCommand->Int(kArg1);
    const char *pszSubCommand = pCommand->Sym(kArg2);
    if (strcmp(pszSubCommand, "start") == 0) {
        const Vector2 from{pCommand->Float(kArg3), pCommand->Float(kArg4)};
        const Vector2 to{pCommand->Float(kArg5), pCommand->Float(kArg6)};
        TheGfxManager.ShowButtonIcon(nLane, &from, &to);
    } else if (strcmp(pszSubCommand, "set_flashing") == 0) {
        TheGfxManager.SetButtonIconHighlight(nLane, pCommand->Int(kArg3) != 0);
    } else if (strcmp(pszSubCommand, "hide") == 0) {
        TheGfxManager.HideButtonIcon(nLane);
    } else if (strcmp(pszSubCommand, "set_text") == 0) {
        TheGfxManager.SetButtonIconGlyph(nLane, pCommand->Sym(kArg3));
    } else {
        WarnUnrecognized(pCommand, pszSubCommand);
    }
}

void GfxManager::OnPlaceBox(DataArray *pCommand, [[maybe_unused]] void *pUserData) {
    const float fX0 = pCommand->Float(kArg1);
    const float fY0 = pCommand->Float(kArg2);
    const float fX1 = pCommand->Float(kArg3);
    const float fY1 = pCommand->Float(kArg4);
    TheGfxManager.SetBoxRect(fX0, fY0, fX1, fY1, pCommand->Float(kArg5));
}

void GfxManager::OnPlaceArrow(DataArray *pCommand, [[maybe_unused]] void *pUserData) {
    float fDuration = 0.0f;
    if (pCommand->Size() > kArg4) {
        fDuration = pCommand->Float(kArg4);
    }
    const float fX = pCommand->Float(kArg1);
    const float fY = pCommand->Float(kArg2);
    TheGfxManager.SetArrowTarget(fX, fY, pCommand->Float(kArg3), fDuration);
}

void GfxManager::OnSetJuice(DataArray *pCommand, [[maybe_unused]] void *pUserData) {
    TheGfxManager.SetEnergy(0, pCommand->Float(kArg1));
}

void GfxManager::OnController(DataArray *pCommand, [[maybe_unused]] void *pUserData) {
    const char *pszSubCommand = pCommand->Sym(kArg1);
    if (strcmp(pszSubCommand, "showing") == 0) {
        float fPosition = kDefaultPosition;
        if (pCommand->Size() > kArg3) {
            fPosition = pCommand->Float(kArg3);
        }
        TheGfxManager.ShowController(pCommand->Int(kArg2) != 0, fPosition);
    } else if (strcmp(pszSubCommand, "highlit") == 0) {
        TheGfxManager.SetControllerHighlight(pCommand->Int(kArg2) != 0);
    } else if (strcmp(pszSubCommand, "button") == 0) {
        const int nButton = pCommand->Int(kArg2);
        TheGfxManager.ShowControllerButton(nButton, pCommand->Int(kArg3) != 0);
    } else {
        WarnUnrecognized(pCommand, pszSubCommand);
    }
}

void GfxManager::OnDialog(DataArray *pCommand, [[maybe_unused]] void *pUserData) {
    const char *pszSubCommand = pCommand->Sym(kArg1);
    if (strcmp(pszSubCommand, "show") == 0) {
        int nPosition = kDefaultDialogPosition;
        if (pCommand->Size() > kArg4) {
            nPosition = pCommand->Int(kArg4);
        }
        const char *pszText = TheLocale.Localize(pCommand->Sym(kArg2), true);
        TheGfxManager.OpenDialog(pszText, pCommand->Float(kArg3), nPosition);
    } else if (strcmp(pszSubCommand, "hide") == 0) {
        TheGfxManager.CloseDialog();
    } else {
        WarnUnrecognized(pCommand, pszSubCommand);
    }
}

void GfxManager::OnShowMessage(DataArray *pCommand, [[maybe_unused]] void *pUserData) {
    const char *pszText = pCommand->Sym(kArg1);
    if (pszText != nullptr && *pszText != '\0') {
        pszText = TheLocale.Localize(pszText, true);
    }
    bool bFirstLine = false;
    if (pCommand->Size() > kArg2) {
        bFirstLine = pCommand->Int(kArg2) != 0;
    }
    TheGfxManager.SetLyricText(pszText, bFirstLine);
}

void GfxManager::OnStickDiagram(DataArray *pCommand, [[maybe_unused]] void *pUserData) {
    const char *pszSubCommand = pCommand->Sym(kArg1);
    if (strcmp(pszSubCommand, "show") == 0) {
        const int nDirection = pCommand->Int(kArg2);
        const float fX = static_cast<float>(pCommand->Int(kArg3));
        const Vector2 position{fX, static_cast<float>(pCommand->Int(kArg4))};
        TheGfxManager.ShowStick(nDirection, &position);
    } else if (strcmp(pszSubCommand, "hide") == 0) {
        TheGfxManager.HideStick();
    } else {
        WarnUnrecognized(pCommand, pszSubCommand);
    }
}

void GfxManager::DrawDrugs() {
    const float aflRect[] = {0.0f, 0.0f, 1.0f, 1.0f};
    float aflSavedRect[kNumRectComponents];
    memcpy(aflSavedRect, ThePs.mBlurRect, sizeof(aflSavedRect));
    const float fSavedAmount = ThePs.mBlurAmount;
    const int nSavedCount = ThePs.mBlurCount;
    ThePs.mBlurCount = kDrugsOffset;
    ThePs.mBlurAmount = kDrugsAlpha;
    memcpy(ThePs.mBlurRect, aflRect, sizeof(aflRect));
    ThePs.DrawBlur();
    ThePs.mBlurAmount = fSavedAmount;
    ThePs.mBlurCount = nSavedCount;
    memcpy(ThePs.mBlurRect, aflSavedRect, sizeof(aflSavedRect));
}

#pragma mark - Life cycle

void GfxManager::LoadConfig(bool bReload) {
    sMergesAlwaysAllowed = nullptr;
    if (!bReload) {
        for (int i = kInstrumentDrum; i < kNumInstruments; ++i) {
            DataArray *pValue = DataArray::New(1);
            pValue->SetNode(0, i, DataArray::kNodeInt);
            DataArray::DefineMacro(kInstrumentMacros[i], pValue);
            pValue->Release();
        }
    }
    DataArray *pMode = GetModeGfxConfig();
    DataArray *pGfx = GetGfxConfig();
    if (mArena != nullptr) {
        mArena->LoadConfig();
    }
    for (int i = kInstrumentDrum; i < kNumInstruments; ++i) {
        const char *pszName = kInstrumentNames[i];
        FindConfigColor(
            pMode, pGfx, FormatString("inst_color_%s", pszName), &sInstrumentColors[i], true);
        FindConfigColor(
            pMode, pGfx, FormatString("inst_bg_color_%s", pszName), &sInstrumentBgColors[i], true);
    }
    for (int i = kPlayerColorGreen; i < kNumPlayerColors; ++i) {
        const char *pszName = kPlayerColorNames[i];
        FindConfigColor(
            pMode, pGfx, FormatString("player_color_%s", pszName), &sPlayerColors[i], true);
        FindConfigColor(
            pMode, pGfx, FormatString("player_bg_color_%s", pszName), &sPlayerBgColors[i], true);
    }
    Overlay::LoadConfig(pMode, pGfx, mTunnel != nullptr);
}

void GfxManager::Init() {
    sInitialized = 1;
    const char *pszTextureCache;
    SystemConfig()
        ->FindArray("gfx", true)
        ->FindSymbol("texture_cache_file", &pszTextureCache, true);
    Rnd::TheManager.LoadFile(pszTextureCache);
    ScriptFunction::Register(OnHandleBeat, "gfx_handle_beat", nullptr);
    ScriptFunction::Register(OnDebug, "gfx", nullptr);
    ScriptFunction::Register(OnCheat, "gfx_cheat", nullptr);
    ScriptFunction::Register(OnShow, "gfx_show", nullptr);
    ScriptFunction::Register(OnShowMessage, "gfx_show_message", nullptr);
    ScriptFunction::Register(OnStickDiagram, "gfx_stick_diagram", nullptr);
    ScriptFunction::Register(OnAddCheckpoint, "gfx_add_checkpoint", nullptr);
    ScriptFunction::Register(OnViewPullback, "gfx_view_pullback", nullptr);
    ScriptFunction::Register(OnButtonIcon, "gfx_button_icon", nullptr);
    ScriptFunction::Register(OnController, "gfx_controller", nullptr);
    ScriptFunction::Register(OnDialog, "gfx_dialog", nullptr);
    ScriptFunction::Register(OnPlaceBox, "place_hlbox", nullptr);
    ScriptFunction::Register(OnPlaceArrow, "place_hlarrow", nullptr);
    ScriptFunction::Register(OnSetJuice, "set_juice", nullptr);
    TheTriggerMgr.Terminate();
    LoadConfig(false);
    mVortex = new Vortex("launch");
    mMultiplierFormat = TheLocale.Localize("MULTIPLIER_FORMAT", true);
}

void GfxManager::Terminate() {
    sTerminated = 1;
    switch (mState) {
    case kStatePlaying:
        Teardown();
        break;
    case kStateIdle:
    case kStateLoading:
    case kStateUnloading:
        break;
    default:
        DebugWarn("weird mode");
        break;
    }
    delete mVortex;
    mVortex = nullptr;
    ScriptFunction::Unregister(OnDebug);
    ScriptFunction::Unregister(OnCheat);
    ScriptFunction::Unregister(OnShow);
    ScriptFunction::Unregister(OnStickDiagram);
    ScriptFunction::Unregister(OnShowMessage);
    ScriptFunction::Unregister(OnAddCheckpoint);
    ScriptFunction::Unregister(OnViewPullback);
    ScriptFunction::Unregister(OnDialog);
    ScriptFunction::Unregister(OnController);
    ScriptFunction::Unregister(OnButtonIcon);
    ScriptFunction::Unregister(OnPlaceArrow);
    ScriptFunction::Unregister(OnPlaceBox);
    ScriptFunction::Unregister(OnSetJuice);
    ScriptFunction::Unregister(OnHandleBeat);
}

void GfxManager::Reset() {
    mVortex->Start(kDefaultPosition, Vortex::kModeOut);
    mState = kStateIdle;
}

void GfxManager::Load(float fTime) {
    mLoadStartMs = SystemMs();
    Timer timer;
    timer.Start();

    const char *pszArena;
    if (mNoArena != 0) {
        pszArena = kNoArena;
    } else if (TheGameDb->mRuleSet == GameDb::kRuleSetRemix) {
        pszArena = kTutorialArena;
    } else {
        pszArena = TheGameDb->mArena.c_str();
    }
    mArena = GfxArena::Create(pszArena);

    DataArray *pGfx = SystemConfig()->FindArray("gfx", true);
    DataArray *pPreloads = pGfx->FindArray("data_preloads", true);
    for (int i = 1; i < pPreloads->Size(); ++i) {
        AddPreload(pPreloads->Sym(i));
    }
    sMergesAlwaysAllowed =
        SystemConfig()->FindArray("gfx", true)->FindArray("merges_always_allowed", true);

    DataArray *pLoad = pGfx->FindArray("load", false);
    if (pLoad != nullptr) {
        int i = 1;
        while (i < pLoad->Size()) {
            DataArray *pEntry = pLoad->Array(i);
            const char *pszName = pEntry->Sym(0);
            ++i;
            if (strcmp(pszName, "hud") == 0) {
                // Yes, the binary takes the loader of the entry before the `hud` entry.
                mHudLoader = mLoaders.back().mLoader;
                continue;
            }
            DataArray *pRuleSets = pEntry->FindArray("rule_set", false);
            if (pRuleSets != nullptr) {
                bool bMatch = false;
                for (int j = 1; j < pRuleSets->Size(); ++j) {
                    const int nType = pRuleSets->Type(j);
                    if (nType == DataArray::kNodeSymbol) {
                        if (strcmp(pRuleSets->Sym(j), "tutorial") == 0 &&
                            TheGameDb->mTutorial != 0) {
                            bMatch = true;
                        }
                    } else if (nType == DataArray::kNodeInt) {
                        if (pRuleSets->Int(j) == TheGameDb->mRuleSet) {
                            bMatch = true;
                            break;
                        }
                    } else {
                        DebugWarn("weird type for ruleset node at %d of %s",
                                  pRuleSets->mLine,
                                  pRuleSets->mFile);
                    }
                }
                if (!bMatch) {
                    continue;
                }
            }
            DataArray *pCommunities = pEntry->FindArray("comm", false);
            if (pCommunities != nullptr) {
                bool bMatch = false;
                for (int j = 1; j < pCommunities->Size(); ++j) {
                    if (pCommunities->Int(j) == TheGameDb->mCommunity) {
                        bMatch = true;
                        break;
                    }
                }
                if (!bMatch) {
                    continue;
                }
            }
            const bool bMergeAll = pEntry->FindArray("merge_all", false) != nullptr;
            DataArray *pForceMerge = pEntry->FindArray("force_merge", false);
            bool bArena = false;
            const char *pszFile = pszName;
            if (strcmp(pszName, "arena") == 0) {
                bArena = true;
                pszFile = mArena->mConfig->Sym(1);
                if (pszFile == nullptr) {
                    continue;
                }
                if (*pszFile == '\0') {
                    pszFile = nullptr;
                }
            } else if (strcmp(pszName, "movie") == 0) {
                const char *pszSong = TheGameDb->mSong.c_str();
                if (pszSong == nullptr || *pszSong == '\0') {
                    continue;
                }
                pszFile = FormatString("Songs/%s/movie.rnd", pszSong);
            }
            if (pszFile == nullptr) {
                continue;
            }
            mLoaders.push_back(GfxLoader(false, bMergeAll, pForceMerge));
            mLoaders.back().Start(pszFile, kLoaderFlags);
            if (bArena) {
                mArena->mLoader = mLoaders.back().mLoader;
            }
        }
    }

    if (mVortex->mMode != Vortex::kModeIn && mVortex->mMode != Vortex::kModeHold) {
        mVortex->Start(kDefaultPosition, Vortex::kModeIn);
    }
    mVortexAfterViews = 0;
    mLoadStage = kLoadStageConfig;
    mLoaded = 0;
    mState = kStateLoading;
    UpdateScrollSpeed(fTime);
    Ship::LoadConfig(GetModeGfxConfig(), GetGfxConfig(), false);
    TheTriggerMgr.Terminate();
    TheTriggerMgr.MetagameEvent(kPreSongTick, kPreSongTick, 0.0f);
    timer.Stop();
}

void GfxManager::BuildTracks(float fStartTick,
                             float fEndTick,
                             const float *pMsPerTick,
                             const std::vector<int> &instruments,
                             const std::vector<int> &trackTypes,
                             int nOption,
                             const std::vector<bool> &unlocked) {
    sMergesAlwaysAllowed = nullptr;
    Timer timer;
    timer.Start();
    Timer partTimer;
    partTimer.Start();

    (void)CheckLoaders(); // Yes, the binary discards this call's result.
    mState = kStatePlaying;
    if (mSongTicks != 0.0f && mSongTicks != fEndTick) {
        DebugPrint("Potential song tick mismatch; got %f from db,\nand %f from game.",
                   mSongTicks,
                   fEndTick);
    }
    mArena->SetSongTicks(mSongTicks);
    mMsPerTick = pMsPerTick;
    mLastTick = fStartTick;
    mLastTime = fStartTick * *pMsPerTick;

    DataArray *pGfx = GetGfxConfig();
    (void)GetModeGfxConfig(); // Yes, the binary discards this call's result.
    const char *pszTunnelView;
    pGfx->FindSymbol("tunnelview", &pszTunnelView, true);
    mTunnelView = FindView(pszTunnelView);
    const char *pszLocalView;
    pGfx->FindSymbol("localview", &pszLocalView, true);
    bool bSplitScreen = false;
    FindConfigBool(GetModeGfxConfig(), pGfx, "splitscreen", &bSplitScreen, true);
    const int nLocalViews = bSplitScreen ? TheGameDb->GetNumPads() : 1;
    mLocalViews.clear();
    mLocalViews.reserve(nLocalViews);
    Rnd::View *pTransparent = FindView("tnl transparent");
    for (int i = 0; i < nLocalViews; ++i) {
        const int nView = i + kFirstLocalView;
        LocalViews views;
        views.mArena = FindView(FormatString(pszLocalView, nView, " arena"));
        views.mMain = FindView(FormatString(pszLocalView, nView, ""));
        views.mPart2 = FindView(FormatString(pszLocalView, nView, " part2"));
        static_cast<Rnd::Drawable *>(views.mPart2)->RemoveDraw(pTransparent);
        views.mPart3 = FindView(FormatString(pszLocalView, nView, " part3"));
        views.mPart4 = FindView(FormatString(pszLocalView, nView, " part4"));
        mLocalViews.push_back(views);
    }
    if (TheGameDb->mCommunity == GameDb::kCommunitySolo ||
        TheGameDb->mRuleSet != GameDb::kRuleSetGame) {
        mPanelView = FindView("panel mat render.view");
    } else {
        mPanelView = nullptr;
    }
    partTimer.Stop();
    partTimer.Reset();
    partTimer.Start();

    mTunnel = new GfxTunnel(instruments, trackTypes, mArena, nOption, fStartTick, mSongTicks);
    GfxTunnel::sCurrent = mTunnel;
    if (mSongTicks > 0.0f) {
        const float fDataLength = mArena->mCamPathLength;
        const float fLength = mArena->mCamPath->ArcLength(kPreSongTick, mArena->mSongTicks);
        if (fabsf(fDataLength - fLength) > kCamPathTolerance) {
            DebugNotify("Arena cam path length (%f) different from data (%f)",
                        mArena->mCamPath->ArcLength(kPreSongTick, mArena->mSongTicks),
                        mArena->mCamPathLength);
        }
    }
    partTimer.Stop();
    partTimer.Reset();

    const int nUnlockIndex = mArena->mUnlockIndex;
    if (nUnlockIndex >= 0 && static_cast<unsigned>(nUnlockIndex) < unlocked.size()) {
        mArenaUnlocked = unlocked[nUnlockIndex];
    } else {
        mArenaUnlocked = 0;
    }
    TheTriggerMgr.SetPaths(unlocked);
    TheTriggerMgr.BeginEvent(TheGameDb->mRuleSet);
    for (int i = 0; i < TheGameDb->GetNumPlayers(); ++i) {
        ShowPlayerOnTrack(i, 0);
    }
    mVortex->Start(kDefaultPosition, Vortex::kModeOut);
    mVortexAfterViews = 0;
    for (AsyncStream *pStream : mPreloads) {
        delete pStream;
    }
    mPreloads.clear();
    mWinner = -1;
    mLeader = 0;
    mWinnerDrawn = 0;
    mVictoryLap = 0;
    timer.Stop();
}

void GfxManager::Teardown() {
    Timer timer;
    timer.Start();
    CamSlide::Terminate();
    mPanelView = nullptr;
    mLocalViews.clear();
    mTunnelView = nullptr;
    mMainView = nullptr;
    delete mArena;
    mArena = nullptr;
    delete mOverlay;
    mOverlay = nullptr;
    GfxTunnel::sCurrent = nullptr;
    delete mTunnel;
    mTunnel = nullptr;
    for (int i = 0; i < kMaxPlayers; ++i) {
        delete mShips[i];
        mShips[i] = nullptr;
    }
    mDebugMsgs.clear();
    while (!mLoaders.empty()) {
        delete mLoaders.front().mLoader;
        mLoaders.pop_front();
    }
    TheGameDb->GetProfile(0)->mAvatar.SetPose(kTeardownPose);
    mWinner = -1;
    mVictoryLap = 0;
    mMsPerTick = nullptr;
    mVortexAfterViews = 0;
    mWinnerDrawn = 0;
    timer.Stop();
}

void GfxManager::Unload() {
    TheTriggerMgr.Terminate();
    TheTriggerMgr.MetagameEvent(0.0f, 0.0f, 0.0f);
    mVortex->Start(kDefaultPosition, Vortex::kModeIn);
    Teardown();
    mState = kStateUnloading;
}

float GfxManager::StartIntro(float fLength) {
    Vortex *pVortex = mVortex;
    mVortexAfterViews = 1;
    pVortex->Start(fLength, Vortex::kModeIn);
    return pVortex->GetInLength();
}

bool GfxManager::IsIntroFinished() const {
    return mVortex->mMode != Vortex::kModeIn;
}

float GfxManager::RestartIntro() {
    Vortex *pVortex = mVortex;
    pVortex->Start(kDefaultPosition, Vortex::kModeOut);
    return pVortex->GetOutLength();
}

float GfxManager::StartOutro(float fLength) {
    Vortex *pVortex = mVortex;
    pVortex->Start(fLength, Vortex::kModeOutro);
    mVortexAfterViews = 1;
    return pVortex->GetOutroLength();
}

bool GfxManager::IsOutroDone() {
    return mVortex->mFrame == mVortex->mInterp.mY1;
}

void GfxManager::UpdateScrollSpeed(float fSongTicks) {
    if (!GetGfxConfig()->FindFloat("tunnel_scale", &mScrollSpeed, false)) {
        mSongTicks = 0.0f;
        float fTempo = kDefaultTempo;
        float fTunnelScale = kDefaultTunnelScale;
        const char *pszSong = TheGameDb->mSong.c_str();
        if (pszSong != nullptr && *pszSong != '\0') {
            const SongEntry song{TheGameDb->FindSong(pszSong)};
            fTunnelScale = song.GetTunnelScale();
            fTempo = song.GetBpm();
            int nBars;
            if (TheGameDb->mRuleSet == GameDb::kRuleSetRemix) {
                nBars = 0;
            } else if (TheGameDb->mTutorial != 0) {
                nBars = 0;
            } else if (TheGameDb->mRuleSet == GameDb::kRuleSetDuel) {
                nBars = song.GetDuelBars();
            } else if (TheGameDb->mLoadRemix != 0) {
                nBars = song.GetRemixBars();
            } else {
                nBars = song.GetBars();
            }
            mSongTicks = static_cast<float>(nBars) * kTicksPerBar;
        } else if (fSongTicks > 0.0f) {
            mSongTicks = fSongTicks;
        }
        const float fDifficultyScale = GetGfxConfig()
                                           ->FindArray("difficulty_scale_tweaks", true)
                                           ->Float(TheGameDb->mSkillLevel + 1);
        float fSongLength = mSongTicks;
        float fCamPathLength;
        if (fSongLength <= 0.0f || mArena == nullptr) {
            fSongLength = kDefaultSongTicks;
            fCamPathLength = kDefaultCamPathLength;
        } else {
            fCamPathLength = mArena->mCamPathLength;
        }
        mCamPathScale = fCamPathLength / fSongLength;
        mScrollSpeed = fDifficultyScale * fTunnelScale * kDefaultSongTicks * kDefaultTempo *
                       fCamPathLength / (fSongLength * fTempo * kDefaultCamPathLength);
    }
    mInvScrollSpeed = 1.0f / mScrollSpeed;
}

int GfxManager::Poll(float fTime) {
    static Timer *sTimer = Timer::Find("gfx ee");
    Timer *pTimer = sTimer;
    if (pTimer != nullptr) {
        pTimer->Start();
    }
    int nResult = 0;
    const float fTick = TheGameDb->mSongTick;
    switch (mState) {
    case kStateLoading: {
        mVortex->Poll();
        if (mLoaded != 0) {
            for (int i = TheGameDb->GetNumPlayers() - 1; i >= 0; --i) {
                mShips[i]->UpdateTransform();
            }
            nResult = kPollWorldReady;
            break;
        }
        const bool bHudStarted = mLoadStage >= kLoadStageObjects;
        bool bStreamsReady = true;
        for (AsyncStream *pStream : mPreloads) {
            if (!pStream->Ready(kStreamReadyBytes)) {
                bStreamsReady = false;
            }
        }
        bool bReady = false;
        if (bStreamsReady) {
            bReady = CheckLoaders();
        }
        mArena->PollLoad(bHudStarted);
        if (bReady && mArena->mNoMovie == 0) {
            bReady = false;
            if (mArena->mMovieStream == nullptr) {
                mArena->LoadMovie();
            }
        }
        switch (mLoadStage) {
        case kLoadStageConfig:
            Overlay::LoadConfig(GetModeGfxConfig(), GetGfxConfig(), true);
            mLoadStage = kLoadStageHud;
            break;
        case kLoadStageHud:
            if (mHudLoader->IsLoaded()) {
                DataArray *pMode = GetModeGfxConfig();
                DataArray *pGfx = GetGfxConfig();
                mOverlay = new Overlay(pMode, pGfx);
                mHudLoader = nullptr;
                CamSlide::Init();
                mMainView = GfxTunnel::CreateMainView();
                for (int i = 0; i < TheGameDb->GetNumPlayers(); ++i) {
                    mShips[i] = new Ship(TheGameDb->GetPlayerColor(i), i);
                    mShips[i]->UpdateTransform();
                }
                mLoadStage = kLoadStageObjects;
            }
            break;
        case kLoadStageObjects:
            if (bReady) {
                mLoaded = 1;
                mOverlay->CreateTutorialParts();
            }
            for (int i = TheGameDb->GetNumPlayers() - 1; i >= 0; --i) {
                mShips[i]->UpdateTransform();
            }
            break;
        default:
            break;
        }
        nResult = mLoaded != 0;
        break;
    }
    case kStatePlaying: {
        mVortex->Poll();
        const float fTicks = fTick - mLastTick;
        mLastTick = fTick;
        const float fSongTime = TheGameDb->mSongTime;
        const float fTimeDelta = fSongTime - mLastTime;
        mLastTime = fSongTime;
        mArena->Poll(fTick, fTime);
        mTunnel->Poll(fTicks, fTimeDelta);
        mOverlay->SetCameraOffset(mTunnel->GetHudOffset());
        mOverlay->Poll(fTick, fTicks, fSongTime, fTimeDelta, fTime);
        mTunnel->SetLetterbox(mOverlay->GetLetterbox());
        static_cast<Rnd::Animatable *>(mTunnelView)->SetFrame(fTick);
        for (int i = TheGameDb->GetNumPlayers() - 1; i >= 0; --i) {
            mShips[i]->Poll(fTicks, fTimeDelta);
        }
        mTunnel->PollCameras(fTicks, fTimeDelta);
        static_cast<Rnd::Transformable *>(mTunnelView)->UpdateWorldXfm(nullptr, 0);
        break;
    }
    case kStateUnloading:
        mVortex->Poll();
        nResult = kPollUnloaded;
        break;
    case kStateIdle:
        mVortex->Poll();
        break;
    default:
        break;
    }
    if (pTimer != nullptr) {
        pTimer->Stop();
    }
    return nResult;
}

void GfxManager::Draw([[maybe_unused]] float fTick) {
    switch (mState) {
    case kStateLoading:
        mVortex->Draw();
        if (mLoadStage >= kLoadStageObjects) {
            static_cast<Rnd::Drawable *>(mMainView)->Draw();
            Ship::DrawShared();
            for (int i = TheGameDb->GetNumPlayers() - 1; i >= 0; --i) {
                mShips[i]->DrawTrack();
            }
            for (int i = TheGameDb->GetNumPlayers() - 1; i >= 0; --i) {
                mShips[i]->DrawEffects();
            }
        }
        break;
    case kStatePlaying:
        DrawLocalViews(true);
        break;
    case kStateUnloading:
    case kStateIdle:
        mVortex->Draw();
        break;
    default:
        break;
    }
}

void GfxManager::DrawLocalViews(bool bDrawVortex) {
    for (unsigned i = 0; i < mLocalViews.size(); ++i) {
        DrawView(mLocalViews[i].mArena);
        if (mPanelView != nullptr) {
            Rnd::Cam *pCam = Rnd::Cam::sCurrent;
            Rnd::Environ *pEnviron = Rnd::Environ::sCurrent;
            static_cast<Rnd::Drawable *>(mPanelView)->Draw();
            DrawIfSet(pCam);
            DrawIfSet(pEnviron);
        }
        DrawView(mLocalViews[i].mMain);
        if (mDrugs != 0) {
            DrawDrugs();
        }
        mTunnel->DrawBackground();
        DrawView(mLocalViews[i].mPart2);
        mTunnel->DrawTracks();
        DrawView(mLocalViews[i].mPart3);
        mTunnel->DrawCameras();
        if (bDrawVortex && mVortexAfterViews == 0) {
            Rnd::Cam *pCam = Rnd::Cam::sCurrent;
            Rnd::Environ *pEnviron = Rnd::Environ::sCurrent;
            mVortex->Draw();
            DrawIfSet(pCam);
            DrawIfSet(pEnviron);
        }
        for (int j = TheGameDb->GetNumPlayers() - 1; j >= 0; --j) {
            mShips[j]->DrawTrack();
        }
        for (int j = TheGameDb->GetNumPlayers() - 1; j >= 0; --j) {
            mShips[j]->DrawEffects();
        }
        DrawView(mLocalViews[i].mPart4);
        mTunnel->DrawForeground();
    }
    if ((mHudFlags & kHudFlagDraw) != 0) {
        mOverlay->Draw();
    }
    if (bDrawVortex && mVortexAfterViews != 0) {
        mVortex->Draw();
    }
}

bool GfxManager::CheckLoaders() {
    for (GfxLoader &loader : mLoaders) {
        if (!loader.mLoader->IsLoaded()) {
            return false;
        }
        if (loader.mDone == 0) {
            loader.Finish();
        }
    }
    return true;
}

void GfxManager::AddPreload(const char *pszFile) {
    char szPath[kMaxPathLength];
    DataArray::MakeCompiledPath(szPath, pszFile, false);
    mPreloads.push_back(new AsyncStream(szPath, true));
}

AsyncStream *GfxManager::FindPreload(const char *pszFile) {
    char szPath[kMaxPathLength];
    DataArray::MakeCompiledPath(szPath, pszFile, false);
    for (AsyncStream *pStream : mPreloads) {
        if (strcmp(pStream->GetFileName(), szPath) == 0) {
            pStream->Ready(kStreamReadyBytes);
            return pStream;
        }
    }
    DebugWarn("File %s not in preload list.", szPath);
    return nullptr;
}

void GfxManager::HandleBeat(signed char nEvent) {
    mArena->HandleBeat(nEvent);
}

int GfxManager::GetTrackInstrument(int nTrack) {
    return mTunnel->GetTrackInstrument(nTrack);
}

int GfxManager::GetTrackType(int nTrack) {
    return mTunnel->GetTrackType(nTrack);
}

#pragma mark - Debugging

void GfxManager::Show(DataArray *pCommand) {
    const char *pszThing = pCommand->Sym(kArg1);
    const int nPlayer = pCommand->Int(kArg2);
    const bool bShow = pCommand->Int(kArg3) != 0;
    if (strcmp(pszThing, "ship") == 0) {
        SetPlayerShown(nPlayer, bShow);
    } else if (strcmp(pszThing, "targets") == 0) {
        SetCatcherShown(nPlayer, bShow);
    } else if (strcmp(pszThing, "juice") == 0) {
        SetEnergyShown(nPlayer, bShow);
    } else if (strcmp(pszThing, "score") == 0) {
        SetScoreShown(nPlayer, bShow);
    } else if (strcmp(pszThing, "freq") == 0) {
        SetLaneShown(nPlayer, bShow);
    } else if (strcmp(pszThing, "letterbox") == 0) {
        mOverlay->SetLetterbox(bShow);
    } else if (strcmp(pszThing, "hlbox") == 0) {
        ShowBox(bShow);
    } else if (strcmp(pszThing, "hlarrow") == 0) {
        ShowArrow(bShow);
    } else if (strcmp(pszThing, "hud") == 0) {
        ShowHud(bShow);
    } else if (strcmp(pszThing, "songpos") == 0) {
        ShowSongPos(bShow);
    } else if (strcmp(pszThing, "powerup") == 0) {
        mOverlay->SetPowerupEnabled(nPlayer, bShow);
    } else if (strcmp(pszThing, "remix") == 0) {
        ShowRemix(nPlayer, bShow);
        ShowSections(bShow);
    } else if (strcmp(pszThing, "track_label") == 0) {
        ShowTrackLabels(bShow);
    } else {
        DebugNotify("unknown thing to show: %s\nat line %d of %s",
                    pszThing,
                    pCommand->mLine,
                    pCommand->mFile);
    }
}

void GfxManager::Cheat(DataArray *pCommand) {
    FxMidi::PlayCheat();
    const char *pszCheat = pCommand->Sym(kArg1);
    if (strcmp(pszCheat, "monkey") == 0) {
        mMonkeyGems ^= 1;
        DebugPrint("CHEAT: monkey gems now %d\n", mMonkeyGems);
    } else if (strcmp(pszCheat, "drugs") == 0) {
        mDrugs ^= 1;
        DebugPrint("CHEAT: drugs now %d\n", mDrugs);
    } else if (strcmp(pszCheat, "black_panels") == 0) {
        mBlackPanels ^= 1;
        DebugPrint("CHEAT: black panels now %d\n", mBlackPanels);
    } else if (strcmp(pszCheat, "no_panels") == 0) {
        mNoPanels ^= 1;
        DebugPrint("CHEAT: no panels now %d\n", mNoPanels);
    } else if (strcmp(pszCheat, "circular") == 0) {
        mTunnelShape = static_cast<signed char>((mTunnelShape + 1) % kNumTunnelShapes);
        DebugPrint("CHEAT: tunnel shape now %d\n", mTunnelShape);
    }
}

void GfxManager::Debug(DataArray *pCommand) {
    const char *pszCommand = pCommand->Sym(kArg1);
    if (strcmp(pszCommand, "save") == 0) {
        *this << "Saving RND state to " << pCommand->Sym(kArg2) << "...\n";
        Rnd::TheManager.SaveFile(pCommand->Sym(kArg2));
    } else if (strcmp(pszCommand, "vortex_in") == 0) {
        StartIntro(kDefaultPosition);
    } else if (strcmp(pszCommand, "vortex_out") == 0) {
        RestartIntro();
    } else if (strcmp(pszCommand, "dump_timers") == 0) {
        DebugPrint("----------------- Timers -----------------\n");
        int nWidth = 1;
        for (const Timer &timer : Timer::sTimers) {
            const int nLength = static_cast<int>(strlen(timer.mName));
            if (nWidth < nLength) {
                nWidth = nLength;
            }
        }
        for (const Timer &timer : Timer::sTimers) {
            DebugPrint("   %s:", timer.mName);
            for (int i = static_cast<int>(strlen(timer.mName)); i < nWidth; ++i) {
                DebugPrint(" ");
            }
            DebugPrint(" %8.2f\n", timer.mLastMs);
        }
    } else if (mState == kStateIdle) {
        if (strcmp(pszCommand, "no_arena") == 0) {
            mNoArena ^= 1;
            DebugPrint("CHEAT: no_arena cheat %s.\n", mNoArena != 0 ? "enabled" : "disabled");
        }
    } else if (mState == kStatePlaying) {
        if (strcmp(pszCommand, "toggle_hud") == 0) {
            do {
                ++mHudFlags;
                if (mHudFlags >= kNumHudFlagValues) {
                    mHudFlags = 0;
                    break;
                }
            } while ((mHudFlags & kHudFlagSkipMask) == kHudFlagSkipValue ||
                     (mHudFlags != kHudFlagLast && (mHudFlags & kHudFlagStatsText) != 0));
            TheRnd->SetShowStats(0);
            TheRnd->SetShowTimers(0, TheRnd->mTimerMaxMs);
            TheRnd->SetShowRate((mHudFlags & kHudFlagStatsText) != 0);
        } else if (strcmp(pszCommand, "particle_report") == 0) {
            mTunnel->ReportParticles();
        }
    }
}

#pragma mark - Forwarders

void GfxManager::SetEnergy(int nPlayer, float fEnergy) {
    if (TheGameDb->mCommunity != GameDb::kCommunitySolo) {
        return;
    }
    int nZone;
    float fZoneFraction;
    if (TheGameDb->mPracticeMode != 0) {
        nZone = kEnergyZoneHigh;
        fZoneFraction = kPracticeZoneFraction;
        fEnergy = kPracticeEnergy;
    } else {
        float fZoneStart;
        float fZoneEnd;
        if (sEnergyHighZone <= fEnergy) {
            nZone = kEnergyZoneHigh;
            fZoneStart = sEnergyHighZone;
            fZoneEnd = 1.0f;
        } else if (sEnergyLowZone <= fEnergy) {
            nZone = kEnergyZoneMiddle;
            fZoneStart = sEnergyLowZone;
            fZoneEnd = sEnergyHighZone;
        } else {
            nZone = kEnergyZoneLow;
            fZoneStart = 0.0f;
            fZoneEnd = sEnergyLowZone;
        }
        const float fZoneLength = fZoneEnd - fZoneStart;
        fZoneFraction = 1.0f;
        if (fZoneLength != 0.0f) {
            fZoneFraction = (fEnergy - fZoneStart) / fZoneLength;
        }
    }
    mOverlay->SetEnergy(fEnergy, nZone);
    mTunnel->SetEnergy(fEnergy, nZone, fZoneFraction);
    mShips[nPlayer]->SetEnergy(fEnergy, nZone, fZoneFraction);
    mArena->SetEnergy(nZone, fEnergy);
    TheTriggerMgr.HealthEvent(nPlayer, static_cast<int>(fEnergy * kPercent));
}

void GfxManager::SetDying(int nPlayer, bool bDying) {
    mOverlay->SetEnergyWarning(bDying);
    TheTriggerMgr.DyingEvent(nPlayer, bDying);
}

void GfxManager::SetScore(int nPlayer,
                          int nScore,
                          [[maybe_unused]] int nMultiplier,
                          [[maybe_unused]] int nPoints) {
    mOverlay->SetScore(nPlayer, nScore);
    TheTriggerMgr.ScoreEvent(nPlayer, nScore);
}

void GfxManager::SetStreakMultiplier(int nPlayer,
                                     int nMultiplier,
                                     int nNextMultiplier,
                                     bool bBoosted) {
    char szNext[kMultiplierTextLength];
    char szMultiplier[kMultiplierTextLength];
    szNext[0] = '\0';
    szMultiplier[0] = '\0';
    if (TheGameDb->IsLocalPlayer(nPlayer)) {
        if (nNextMultiplier >= kFirstShownMultiplier) {
            sprintf(szNext, mMultiplierFormat, nNextMultiplier);
        }
        if (nMultiplier >= kFirstShownMultiplier) {
            sprintf(szMultiplier, mMultiplierFormat, nMultiplier);
        }
    }
    mOverlay->SetMultiplier(nPlayer, nMultiplier, bBoosted, szMultiplier);
    mTunnel->SetStreakMultiplier(
        nPlayer, nMultiplier, nNextMultiplier, szMultiplier, szNext, bBoosted);
    mShips[nPlayer]->SetMultiplier(nMultiplier);
}

void GfxManager::ShowPendingPoints(int nPlayer, int nPoints) {
    char szPoints[kMultiplierTextLength];
    if (TheGameDb->IsLocalPlayer(nPlayer)) {
        sprintf(szPoints, "%d", nPoints);
    } else {
        szPoints[0] = '\0';
    }
    mOverlay->ShowPoints(nPlayer, nPoints, szPoints);
}

void GfxManager::HidePendingPoints(int nPlayer) {
    mOverlay->HideMultiplier(nPlayer);
}

void GfxManager::SetPendingPointsResult(int nPlayer, PendingPointsResult result) {
    mOverlay->EndPoints(nPlayer, result);
}

void GfxManager::SetLeader(int nPlayer, bool bLeader) {
    mOverlay->SetPlaying(nPlayer, bLeader);
    if (bLeader) {
        mLeader = nPlayer;
    }
}

void GfxManager::ShowSongPos(bool bShow) {
    mOverlay->ShowSongPos(bShow);
}

void GfxManager::ShowTrackLabels(bool bShow) {
    mOverlay->ShowTrackLabel(bShow);
}

void GfxManager::ShowPowerup(int nPlayer, int nPowerup) {
    mOverlay->ShowPowerup(nPlayer, nPowerup);
}

void GfxManager::SetMultiplierEndTick(int nPlayer, float fTick) {
    mTunnel->SetMultiplierEndTick(nPlayer, fTick);
}

void GfxManager::ShowSlowdown(int nPlayer, float fStartTick, float fEndTick, float fStopTick) {
    mTunnel->ShowSlowdown(nPlayer, fStartTick, fEndTick, fStopTick);
}

void GfxManager::ShowCripple(int nAttacker, int nVictim) {
    mTunnel->ShowCripple(nAttacker, nVictim);
}

void GfxManager::ShowBump(int nAttacker, int nVictim, int nTrack) {
    mTunnel->ShowBump(nAttacker, nVictim, nTrack);
    mShips[nAttacker]->StartBump(false, nTrack);
    mShips[nVictim]->StartBump(true, nTrack);
}

void GfxManager::ShowMessage(const char *pszLine,
                             const char *pszSecondLine,
                             int nPlayer,
                             float fDurationMs,
                             float fScale,
                             float fOffsetY,
                             float fOffsetX) {
    mOverlay->ShowTextMessage(
        pszLine, pszSecondLine, fDurationMs, fScale, nPlayer, fOffsetY, fOffsetX);
}

void GfxManager::SetLyricText(const char *pszText, bool bFirstLine) {
    mOverlay->SetMessage(pszText, bFirstLine);
}

void GfxManager::ShowButtonIcon(int nLane, const Vector2 *pPosition, const Vector2 *pTarget) {
    mOverlay->FlyButton(nLane, pPosition, pTarget);
}

void GfxManager::SetButtonIconHighlight(int nLane, int nHighlight) {
    mOverlay->SetButtonPulse(nLane, nHighlight);
}

void GfxManager::HideButtonIcon(int nLane) {
    mOverlay->HideButton(nLane);
}

void GfxManager::SetButtonIconGlyph(int nLane, const char *pszGlyph) {
    mOverlay->SetButtonGlyph(nLane, pszGlyph);
}

void GfxManager::ShowController(bool bShow, float fPosition) {
    mOverlay->ShowController(bShow, fPosition);
}

void GfxManager::SetControllerHighlight(bool bHighlight) {
    mOverlay->SetControllerHighlight(bHighlight);
}

void GfxManager::ShowControllerButton(int nButton, bool bShow) {
    mOverlay->ShowControllerButton(nButton, bShow);
}

void GfxManager::OpenDialog(const char *pszText, float fSize, int nPosition) {
    mOverlay->OpenDialog(pszText, fSize, static_cast<float>(nPosition));
}

void GfxManager::CloseDialog() {
    mOverlay->CloseDialog();
}

void GfxManager::SetLetterbox(bool bClosed) {
    mOverlay->SetLetterbox(bClosed);
}

void GfxManager::ShowBox(bool bShow) {
    mOverlay->ShowBox(bShow);
}

void GfxManager::ShowArrow(bool bShow) {
    mOverlay->ShowArrow(bShow);
}

void GfxManager::SetBoxRect(float fX0, float fY0, float fX1, float fY1, float fDuration) {
    mOverlay->SetBoxRect(fX0, fY0, fX1, fY1, fDuration);
}

void GfxManager::SetArrowTarget(float fX, float fY, float fAngle, float fDuration) {
    mOverlay->SetArrowTarget(fX, fY, fAngle, fDuration);
}

void GfxManager::ShowStick(int nDirection, const Vector2 *pPosition) {
    mOverlay->ShowStick(nDirection, pPosition);
}

void GfxManager::HideStick() {
    mOverlay->HideStick();
}

void GfxManager::SetSectionCount(int nCount) {
    mOverlay->SetSectionCount(nCount);
}

void GfxManager::SetSectionLabel(int nIndex, const char *pszLabel) {
    mOverlay->SetSectionLabel(nIndex, pszLabel);
}

void GfxManager::ShowSections(bool bShow) {
    mOverlay->ShowSections(bShow);
}

void GfxManager::HighlightSection(int nIndex) {
    mOverlay->HighlightSection(nIndex);
}

void GfxManager::SetRemixPanelText(int nPlayer,
                                   int nIndex,
                                   const char *pszText,
                                   RemixSubPanel ePanel) {
    mOverlay->SetRemixPanelText(nPlayer, nIndex, pszText, ePanel);
}

void GfxManager::SelectRemixPanelEntry(int nPlayer, int nIndex, RemixSubPanel ePanel) {
    mOverlay->SelectRemixPanelEntry(nPlayer, nIndex, ePanel);
}

void GfxManager::FlashRemixPanel(int nPlayer, RemixSubPanel ePanel) {
    mOverlay->FlashRemixPanel(nPlayer, ePanel);
}

void GfxManager::SetRemixPanelLit(int nPlayer, int nIndex, int nLit, RemixSubPanel ePanel) {
    mOverlay->SetRemixPanelLit(nPlayer, nIndex, nLit, ePanel);
}

void GfxManager::ShowRemixPanel(int nPlayer, RemixSubPanel ePanel) {
    mOverlay->ShowRemixPanel(nPlayer, ePanel);
}

void GfxManager::ShowRemix(int nPlayer, bool bShow) {
    mOverlay->ShowRemix(nPlayer, bShow);
}

void GfxManager::StartBossJourney() {
    TheTriggerMgr.BossJourneyEvent();
    mTunnel->StartBossJourney();
}

bool GfxManager::IsIdle() {
    return mTunnel->mIdle != 0;
}

int GfxManager::IsBossJourneyDone() {
    return mTunnel->mBossJourneyDone;
}

void GfxManager::SetFreqSize(int nPlayer, int nSize) {
    mOverlay->SetFreqSize(nPlayer, nSize);
}

void GfxManager::SetWinner(int nPlayer) {
    mWinner = nPlayer;
}

void GfxManager::SetTrackPlayers(int nTrack, int nPlayers, const int *pPlayers) {
    mTunnel->SetTrackPlayers(nTrack, nPlayers, pPlayers);
    for (int i = 0; i < nPlayers; ++i) {
        mOverlay->ShowPlayerOnTrack(pPlayers[i], i);
    }
}

void GfxManager::ShowPlayerOnTrack(int nPlayer, int nTrack) {
    const int nInstrument = GetTrackInstrument(nTrack);
    const int nTrackType = GetTrackType(nTrack);
    mTunnel->SetPlayerTrack(nPlayer, nTrack, nTrackType, nInstrument);
    mShips[nPlayer]->SetTrack(nTrack, nTrackType, nInstrument);
    mOverlay->SetTrack(nPlayer, nInstrument, nTrackType);
    TheTriggerMgr.NewTrackEvent(nPlayer, nTrack, nInstrument, nTrackType);
}

void GfxManager::SetTrackInstrument(int nTrack, int nInstrument) {
    const int nTrackType = GetTrackType(nTrack);
    mTunnel->SetTrackInstrument(nTrack, nInstrument);
    for (int i = TheGameDb->GetNumPlayers() - 1; i >= 0; --i) {
        if (mTunnel->GetPlayerTrack(i) != nTrack) {
            continue;
        }
        mShips[i]->SetTrack(nTrack, nTrackType, nInstrument);
        mOverlay->SetTrack(i, nInstrument, nTrackType);
        TheTriggerMgr.NewTrackEvent(i, nTrack, nInstrument, nTrackType);
    }
}

void GfxManager::ShowPlayerOnFreestyleTrack(
    int nPlayer, int nType, int nInstrument, bool bVictory, int nReserved) {
    mTunnel->SetPlayerFreestyle(nPlayer, bVictory, nReserved);
    mOverlay->SetTrack(nPlayer, nInstrument, nType);
    TheTriggerMgr.NewTrackEvent(nPlayer, mTunnel->mNumTracks, nInstrument, nType);
}

int GfxManager::GetViewedTrack(int nPlayer) {
    return mTunnel->GetViewedTrack(nPlayer);
}

void GfxManager::SetFreestylePosition(int nPlayer, const Vector3 &position) {
    mTunnel->SetFreestylePosition(nPlayer, position);
}

void GfxManager::ShowScratchNote(int nPlayer, float fTick, float fDuration) {
    mTunnel->ShowScratchNote(nPlayer, fTick, fDuration);
}

void GfxManager::PlaceGem(int nTrack, int nSlot, int nPlayer, float fTick, int nStyle, int nFlags) {
    mTunnel->PlaceGem(nTrack, nSlot, nPlayer, fTick, nStyle, nFlags);
}

void GfxManager::RemoveGem(int nTrack, int nSlot, float fTick) {
    mTunnel->RemoveGem(nTrack, nSlot, fTick);
}

void GfxManager::ClearGems(int nTrack, bool bAll, float fStartTick, float fEndTick) {
    mTunnel->ClearGems(nTrack, bAll, fStartTick, fEndTick);
}

void GfxManager::HideNextPhrase(int nPlayer) {
    mTunnel->HideNextPhrase(nPlayer);
}

void GfxManager::ShowNextPhrase(int nTrack, float fTick, int nGemType) {
    mTunnel->ShowNextPhrase(nTrack, fTick, nGemType);
}

void GfxManager::SetCatchMeter(int nPlayer, float fLevel) {
    mTunnel->SetCatchMeter(nPlayer, fLevel);
    mOverlay->FlashPoints(nPlayer, fLevel);
}

void GfxManager::ShowGemResult(
    int nTrack, int nSlot, bool bHit, int nPlayer, int nFlags, float fTick) {
    mTunnel->ShowGemResult(nTrack, nSlot, bHit, nPlayer, nFlags, fTick);
    mShips[nPlayer]->Fire(nSlot, static_cast<signed char>(nTrack), bHit);
    if (nPlayer != mLeader && TheGameDb->mRuleSet == GameDb::kRuleSetGame) {
        return;
    }
    if (bHit) {
        TheTriggerMgr.HitEvent(nPlayer, nSlot);
    } else {
        TheTriggerMgr.MissEvent(nPlayer, nSlot);
    }
}

void GfxManager::HitGem(int nPlayer, int nTrack, int nSlot, float fTick) {
    mTunnel->HitGem(nPlayer, nTrack, nSlot, fTick);
    mShips[nPlayer]->Fire(nSlot, static_cast<signed char>(nTrack), true);
    TheTriggerMgr.HitEvent(nPlayer, nSlot);
}

void GfxManager::SetFreestyle(int nPlayer, bool bActive, int nColumn) {
    mTunnel->SetFreestyle(nPlayer, bActive, nColumn);
}

void GfxManager::ResetFreestyle(int nPlayer) {
    mTunnel->ResetFreestyle(nPlayer);
}

void GfxManager::ShowCapture(
    int nPlayer, int nTrack, int nStyle, bool bQuiet, float fStartTick, float fEndTick) {
    TheTriggerMgr.PhraseCaptureEvent(nPlayer, 1);
    mTunnel->ShowCapture(nPlayer, nTrack, nStyle, bQuiet, fStartTick, fEndTick);
    if (!bQuiet) {
        mOverlay->ShowCapture(nPlayer);
    }
}

void GfxManager::CompleteStage(int nPlayer, [[maybe_unused]] int nSection) {
    mOverlay->ShowCheckpoint();
    if (TheGameDb->mCommunity == GameDb::kCommunitySolo) {
        TheTriggerMgr.StageCompleteEvent(nPlayer);
    }
}

void GfxManager::ShowResult(bool bWon, int nPlayer, bool bUnlocked) {
    bool bCampaignWin = false;
    int nEndState;
    if (bWon) {
        bCampaignWin = TheGameDb->mPracticeMode == 0;
        TheTriggerMgr.HealthEvent(nPlayer, kFullHealth);
        nEndState = TheGameDb->mPracticeMode == 0 ? kEndStateWon : kEndStatePractice;
    } else {
        bUnlocked = false;
        nEndState = TheGameDb->mPracticeMode == 0 ? kEndStateLost : kEndStatePractice;
    }
    TheTriggerMgr.EndEvent(nEndState, nPlayer);
    mOverlay->ShowResult(bWon, nPlayer);
    mTunnel->ShowResult(bWon, nPlayer, bCampaignWin, bUnlocked);
    mArena->ShowResult(bWon);
}

void GfxManager::ShowPhrase(int nPlayer,
                            int nTrack,
                            bool bPlayable,
                            float fStartTick,
                            float fEndTick,
                            int nStyle,
                            bool bSlide) {
    mTunnel->ShowPhrase(nPlayer, nTrack, bPlayable, fStartTick, fEndTick, nStyle, bSlide);
}

void GfxManager::SetBar(int nTrack,
                        int nPlayer,
                        int nOwner,
                        bool bInSong,
                        bool bVisible,
                        int nFlags,
                        signed char nRiff,
                        float fTick,
                        float fTicks) {
    mTunnel->SetBar(nTrack, nPlayer, nOwner, bInSong, bVisible, nFlags, nRiff, fTick, fTicks);
}

void GfxManager::DimBar(int nTrack, float fTick) {
    mTunnel->DimBar(nTrack, fTick);
}

void GfxManager::SetTrackEnabled(int nTrack, bool bEnabled) {
    mTunnel->SetTrackEnabled(nTrack, bEnabled);
}

void GfxManager::AddCheckpoint(const char *pszLabel, float fTick, float fOffset, float fScale) {
    mTunnel->AddCheckpoint(fTick, fScale);
    const float fSongTicks = mArena->mSongTicks;
    if (fSongTicks != 0.0f) {
        mOverlay->AddCheckpoint(fTick / fSongTicks, pszLabel);
    } else {
        mOverlay->AddCheckpoint(fOffset, pszLabel);
    }
}

void GfxManager::ClearCheckpoints() {
    mOverlay->ClearCheckpoints();
    mTunnel->ClearCheckpoints();
}

void GfxManager::SetOption(bool bOption) {
    mTunnel->SetOption(bOption);
}

bool GfxManager::GetOption() {
    return mTunnel->GetOption();
}

void GfxManager::SetScrollSpeed(float fSpeed) {
    mTunnel->SetScrollSpeed(fSpeed);
}

void GfxManager::SetPlayerShown(int nPlayer, bool bShown) {
    mShips[nPlayer]->SetShown(bShown);
}

void GfxManager::SetCatcherShown(int nPlayer, bool bShown) {
    mTunnel->SetCatcherShown(nPlayer, bShown);
}

void GfxManager::SetEnergyShown([[maybe_unused]] int nPlayer, bool bShown) {
    mOverlay->ShowEnergy(bShown);
}

void GfxManager::SetScoreShown(int nPlayer, bool bShown) {
    mOverlay->ShowScore(nPlayer, bShown);
}

void GfxManager::SetLaneShown(int nPlayer, bool bShown) {
    mOverlay->ShowAvatar(nPlayer, bShown);
}

void GfxManager::ShowHud(bool bShow) {
    mOverlay->SetPartsShown(bShow, 0);
}

void GfxManager::ClearLanes() {
    mTunnel->ClearLanes();
    mOverlay->Reset();
}

void GfxManager::ClearAll() {
    mTunnel->ClearAll();
    for (int i = 0; i < kMaxPlayers; ++i) {
        if (mShips[i] != nullptr) {
            mShips[i]->Reset();
        }
    }
    mArena->ResetPanels();
    mOverlay->ClearStarted();
}

void GfxManager::SetActive(bool bActive) {
    mVictoryLap = bActive;
}

#pragma mark - Colours

const Color *GfxManager::GetInstrumentColor(int nInstrument) const {
    return &sInstrumentColors[nInstrument];
}

const Color *GfxManager::GetInstrumentBgColor(int nInstrument) const {
    return &sInstrumentBgColors[nInstrument];
}

const Color *GfxManager::GetPlayerColor(int nPlayer) const {
    return &sPlayerColors[TheGameDb->GetPlayerSlot(nPlayer)];
}

const Color *GfxManager::GetPlayerBgColor(int nPlayer) const {
    return &sPlayerBgColors[TheGameDb->GetPlayerSlot(nPlayer)];
}
