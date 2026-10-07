#include "met/metagamearena.h"

#include <vector>

#include "game/gamedb.h"
#include "game/triggermgr.h"
#include "met/metagame.h"
#include "os/system.h"
#include "rnd/manager.h"
#include "rnd/transanim.h"
#include "script/dataarray.h"
#include "synth/fxmidi.h"
#include "ui/uimanager.h"

namespace {

constexpr char kMetagameTag[] = "metagame";
constexpr char kArenaPrefixTag[] = "arena_prefix";
constexpr char kTriggerFileTag[] = "trigger_file";
constexpr char kSceneFormat[] = "Metagame/Arena/%s.rnd";
constexpr char kViewFormat[] = "%s.view";
constexpr char kTransitionView[] = "Transition.view";
constexpr char kProjectorAnim[] = "projector_body.tnm";
constexpr char kFifthUnlockView[] = "ArenaUnlockAnim_05.view";
constexpr char kUnlockViewFormat[] = "ArenaUnlockAnim_0%d.view";
constexpr char kParticlesFormat[] = "arena0%d_part.pnm";
constexpr char kArenaScreen[] = "s_g_sel_arena";

constexpr int kNoArena = -1;
constexpr int kFifthArena = 5;
constexpr int kNumUnlockViews = 5;

constexpr int kSceneLoadFlags =
    RndLoader::kAsync | RndLoader::kPostLoad | RndLoader::kDeleteObjects;

// The bytes of the trigger file the load waits for, which is more than any file holds.
constexpr int kWholeFile = 999999999;

// The frame of the projector before the first arena, and the frames between two arenas.
constexpr float kProjectorFifthArenaFrame = 2250.0f;
constexpr int kProjectorFramesPerArena = 500;

constexpr int kMaxPathLength = 0x80;

} // namespace

MetagameArena::MetagameArena()
    : mLoader(nullptr), mView(nullptr), mTransitionView(nullptr), mPrefix(), mRevealArena(kNoArena),
      mRevealStart(0.0f), mParticlesStart(0.0f), mRevealView(nullptr), mRevealLength(0.0f),
      mTriggerStream(nullptr), mLoadState(kLoadStateIdle) {
}

MetagameArena::~MetagameArena() {
    delete mLoader;
}

void MetagameArena::StartLoad() {
    DataArray *pMetagame = SystemConfig()->FindArray(kMetagameTag, false);
    mPrefix = pMetagame->FindArray(kArenaPrefixTag, false)->Sym(1);
    mLoader = Rnd::TheManager.AddLoader(
        FormatString(kSceneFormat, mPrefix.c_str()), kSceneLoadFlags, nullptr, nullptr);
    mLoadState = kLoadStateScene;
}

bool MetagameArena::IsLoaded() const {
    return mLoadState == kLoadStateDone;
}

void MetagameArena::PollLoad([[maybe_unused]] float flTime) {
    if (mLoadState == kLoadStateTriggers) {
        if (mTriggerStream == nullptr || !mTriggerStream->Ready(kWholeFile)) {
            return;
        }
        TheTriggerMgr.Load(mTriggerFile, mTriggerStream);
        delete mTriggerStream;
        mTriggerStream = nullptr;
        TheTriggerMgr.BeginEvent(0);
        mLoadState = kLoadStateDone;
        if (mRevealArena >= 0) {
            Rnd::TransAnim *pProjector =
                dynamic_cast<Rnd::TransAnim *>(Rnd::TheManager.Find(kProjectorAnim));
            if (mRevealArena == 0) {
                pProjector->SetFrame(kProjectorFifthArenaFrame);
            } else {
                pProjector->SetFrame(
                    static_cast<float>((mRevealArena - 1) * kProjectorFramesPerArena));
            }
        }
        ShowUnlocks();
        return;
    }
    if (mLoadState != kLoadStateScene || mLoader == nullptr || !mLoader->IsLoaded()) {
        return;
    }

    mView =
        dynamic_cast<Rnd::View *>(Rnd::TheManager.Find(FormatString(kViewFormat, mPrefix.c_str())));
    mTransitionView = dynamic_cast<Rnd::View *>(Rnd::TheManager.Find(kTransitionView));
    for (Rnd::Animatable *pAnim : mView->mAnims) {
        Rnd::View *pView = dynamic_cast<Rnd::View *>(pAnim);
        if (pView != nullptr) {
            pView->SetActiveRange(-pView->mActiveStart, -pView->mActiveEnd);
        }
    }

    mTriggerFile =
        SystemConfig()->FindArray(kMetagameTag, false)->FindArray(kTriggerFileTag, false)->Sym(1);
    char szPath[kMaxPathLength];
    DataArray::MakeCompiledPath(szPath, mTriggerFile, false);
    mTriggerStream = new AsyncStream(szPath, true);
    mLoadState = kLoadStateTriggers;
}

void MetagameArena::ShowUnlocks() {
    std::vector<const char *> arenas;
    TheGameDb->GetUnlockedArenas(
        &arenas, TheGameDb->mSkillLevel, TheGameDb->mCommunity == GameDb::kCommunitySolo, false);
    const int nUnlocked = static_cast<int>(arenas.size());

    Rnd::View *pView = dynamic_cast<Rnd::View *>(Rnd::TheManager.Find(kFifthUnlockView));
    if (nUnlocked > 0 && mRevealArena != 0) {
        pView->SetFrame(pView->FilteredFrameEnd());
    } else {
        pView->SetFrame(0.0f);
    }
    for (int i = 1; i < kNumUnlockViews; ++i) {
        pView = dynamic_cast<Rnd::View *>(Rnd::TheManager.Find(FormatString(kUnlockViewFormat, i)));
        if (i < nUnlocked && mRevealArena != i) {
            pView->SetFrame(pView->FilteredFrameEnd());
        } else {
            pView->SetFrame(0.0f);
        }
    }

    TheTriggerMgr.ComponentFocusEvent(
        TheMetagame.mSelectedArena.c_str(), kArenaScreen, kArenaScreen);
}

void MetagameArena::Unload() {
    TheTriggerMgr.Terminate();
    mView = nullptr;
    delete mLoader;
    mLoadState = kLoadStateIdle;
    mLoader = nullptr;
}

void MetagameArena::Poll(int nState, int nLoadStage, float flTime, float flTick) {
    if (nState == Metagame::kStateFrontEnd && mLoadState == kLoadStateDone &&
        nLoadStage == Metagame::kLoadStageDone) {
        TheTriggerMgr.MetagameEvent(flTime, flTime, 0.0f);
        TheTriggerMgr.Poll();
        const float flFrame = mTransitionView->mFrame;
        for (Rnd::Animatable *pAnim : mView->mAnims) {
            Rnd::View *pView = dynamic_cast<Rnd::View *>(pAnim);
            if (pView == nullptr || !(pView->mActiveEnd < pView->mActiveStart)) {
                continue;
            }
            int nActive = 0;
            if (-pView->mActiveStart <= flFrame && flFrame < -pView->mActiveEnd) {
                nActive = 1;
            }
            pView->SetActive(nActive);
        }
        mView->SetFrame(flTick);
    }
    if (mLoadState != kLoadStateDone) {
        PollLoad(flTime);
    }

    if (mRevealStart != 0.0f && mRevealView != nullptr) {
        const float flElapsed = flTime - mRevealStart;
        mRevealView->SetFrame(flElapsed);
        if (mRevealLength < flElapsed) {
            mRevealStart = 0.0f;
            mRevealView = nullptr;
            mRevealArena = kNoArena;
            TheUI.GotoScreen(kArenaScreen);
        }
    }
    if (mParticlesStart != 0.0f && mParticles != nullptr) {
        const float flElapsed = flTime - mParticlesStart;
        mParticles->SetFrame(flElapsed);
        if (mParticlesLength < flElapsed) {
            mParticlesStart = 0.0f;
            mParticles = nullptr;
        }
    }
}

void MetagameArena::Draw(int nState) {
    if (nState != Metagame::kStateFrontEnd || mLoadState != kLoadStateDone) {
        return;
    }
    mView->UpdateWorldXfm(nullptr, 0);
    mView->Draw();
}

void MetagameArena::SetRevealArena(int nArena) {
    mRevealArena = nArena - 1;
}

void MetagameArena::StartReveal(float flTime) {
    if (mRevealArena == 0) {
        mRevealArena = kFifthArena;
    }
    mRevealView = dynamic_cast<Rnd::View *>(
        Rnd::TheManager.Find(FormatString(kUnlockViewFormat, mRevealArena)));
    mRevealStart = flTime;
    FxMidi::PlayArenaUnlock();
    const float flRevealLength = mRevealView->FilteredFrameEnd();
    mParticlesStart = flTime;
    mRevealLength = flRevealLength;
    mParticles = dynamic_cast<Rnd::ParticleSysAnim *>(
        Rnd::TheManager.Find(FormatString(kParticlesFormat, mRevealArena)));
    mParticlesLength = mParticles->UnfilterFrame(mParticles->FilteredFrameEnd());
}
