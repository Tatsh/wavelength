#include "game/worldmgr.h"

#include "game/duel.h"
#include "game/game.h"
#include "game/gameconfig.h"
#include "game/gamedb.h"
#include "game/remix.h"
#include "gfx/gfxmanager.h"
#include "os/debug.h"
#include "os/system.h"
#include "os/timer.h"
#include "synth/synth.h"

namespace {

constexpr int kMidiChannelCount = 16;
constexpr unsigned kMidiControlChange = 0xb0;
constexpr unsigned kMidiResetAllControllers = 121;
constexpr int kMidiDataShift = 8;

constexpr float kFullOutputLevel = 1.0f;
constexpr float kGfxLoadStartTime = -1.0f;
constexpr float kIdleDisplayTime = 0.0f;

} // namespace

void WorldMgr::ResetSynthControllers() {
    for (unsigned nChannel = 0; nChannel < kMidiChannelCount; ++nChannel) {
        TheSynth->SendPackedMessage((kMidiControlChange | nChannel) |
                                    (kMidiResetAllControllers << kMidiDataShift));
    }
    TheSynth->SetOutputLevel(kFullOutputLevel);
}

WorldMgr::WorldMgr()
    : mWorld(nullptr), mInputMgr(nullptr), mState(kStateIdle), mEvent(kEventNone), mInitialized(0),
      mWorldStarted(0) {
}

WorldMgr::~WorldMgr() {
}

void WorldMgr::Init() {
    TheGfxManager.Init();
    TheGameConfig->Init();
    mInitialized = 1;
}

void WorldMgr::Terminate() {
    // The pointers are not cleared after the objects are destroyed.
    delete mWorld;
    delete mInputMgr;
    TheGfxManager.Terminate();
}

WorldMgr *WorldMgr::shared() {
    // The instance is destroyed at exit by NTSC-U/C: 0x00104978, PAL: 0x00106060.
    static WorldMgr sInstance;
    return &sInstance;
}

int WorldMgr::GetState() const {
    return mState;
}

void WorldMgr::Reset() {
    Timer timer{};
    timer.Start();
    TheGfxManager.Reset();
    mState = kStateIdle;
    timer.Stop();
}

void WorldMgr::Load() {
    Timer timer{};
    timer.Start();
    TheGfxManager.Load(kGfxLoadStartTime);
    switch (TheGameDb->mRuleSet) {
    case GameDb::kRuleSetGame:
        mWorld = new Game;
        break;
    case GameDb::kRuleSetRemix:
        mWorld = new Remix;
        break;
    case GameDb::kRuleSetDuel:
        mWorld = new Duel;
        break;
    default:
        DebugWarn("invalid ruleset");
        break;
    }
    mWorldStarted = 0;
    mState = kStateLoading;
    SystemSetPadCheck(false);
    timer.Stop();
}

void WorldMgr::Start() {
    Timer timer{};
    timer.Start();
    ResetSynthControllers();
    mWorld->Start();
    mInputMgr = new InputMgr(mWorld);
    mState = kStatePlaying;
    timer.Stop();
}

void WorldMgr::Unload() {
    Timer timer{};
    timer.Start();
    mWorld->Stop();
    delete mInputMgr;
    mInputMgr = nullptr;
    delete mWorld;
    mWorld = nullptr;
    ResetSynthControllers();
    TheGfxManager.Unload();
    mState = kStateUnloaded;
    SystemSetPadCheck(true);
    timer.Stop();
}

void WorldMgr::UpdateTime() {
    if (mState != kStatePlaying) {
        return;
    }
    const float fTick = static_cast<float>(mWorld->GetTick());
    const float fTime = mWorld->GetTime();
    TheGameDb->mSongTime = fTime;
    TheGameDb->mSongTick = fTick;
}

int WorldMgr::Poll() {
    switch (mState) {
    case kStateIdle:
        (void)TheGfxManager.Poll(kIdleDisplayTime); // Yes, the binary discards the result.
        break;
    case kStateLoading: {
        const int nGfxResult = TheGfxManager.Poll(kIdleDisplayTime);
        mWorld->Poll();
        mEvent = kEventNone;
        if (mWorldStarted != 0) {
            if (mWorld->IsLoaded()) {
                mEvent = kEventLoaded;
            }
        } else if (nGfxResult == GfxManager::kPollWorldReady) {
            mWorld->LoadAssets();
            mWorldStarted = 1;
        }
        break;
    }
    case kStatePlaying:
        mInputMgr->Poll();
        mWorld->Poll();
        (void)TheGfxManager.Poll(mWorld->GetDisplayTime()); // Yes, the binary discards the result.
        if (mWorld->IsFinished()) {
            mEvent = mWorld->IsRestartRequested() ? kEventRestart : kEventFinished;
        }
        break;
    case kStateUnloaded:
        if (TheGfxManager.Poll(kIdleDisplayTime) == GfxManager::kPollUnloaded) {
            mEvent = kEventUnloaded;
        } else {
            mEvent = kEventNone;
        }
        break;
    default:
        break;
    }
    const int nEvent = mEvent;
    mEvent = kEventNone;
    return nEvent;
}

void WorldMgr::Draw() {
    if (mState == kStatePlaying) {
        TheGfxManager.Draw(static_cast<float>(mWorld->GetTick()));
    } else {
        TheGfxManager.Draw(kIdleDisplayTime);
    }
}

// The unit's static initialiser at NTSC-U/C: 0x00104f58, PAL: 0x00106640, and its global
// constructor at NTSC-U/C: 0x00104f90, PAL: 0x00106678, store the instance.
WorldMgr *TheWorldMgr = WorldMgr::shared();
