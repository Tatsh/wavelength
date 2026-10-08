#include "game/triggermgr.h"

#include <algorithm>

#include "math/rand.h"
#include "os/debug.h"
#include "os/system.h"
#include "ui/uicomponent.h"
#include "ui/uipanel.h"
#include "ui/uiscreen.h"

namespace {

constexpr char kTriggerTag[] = "trigger";
constexpr char kDebugTag[] = "debug";

// The value of mGameState from the begin event on.
constexpr int kGameStateBegin = 4;

constexpr int DebugBit(TriggerEvent::Type type) {
    return 1 << type;
}

const char *NameOrNull(const UIComponent *pComponent) {
    return pComponent != nullptr ? pComponent->mName : nullptr;
}

const char *NameOrNull(const UIScreen *pScreen) {
    return pScreen != nullptr ? pScreen->mName : nullptr;
}

} // namespace

// NTSC-U/C: 0x0043b700
TriggerMgr TheTriggerMgr;

TriggerMgr::TriggerMgr() : mEvents(TriggerEvent::kNumTypes), mPlayer(0), mClock(0), mDebug(0) {
    for (int i = kNumTracks - 1; i >= 0; --i) {
        mCapturer[i] = kNoPlayer;
    }
}

TriggerMgr::~TriggerMgr() {
    Terminate();
}

void TriggerMgr::Init() {
    DataArray *pDebug = SystemConfig()->FindArray(kTriggerTag, true)->FindArray(kDebugTag, false);
    if (pDebug == nullptr) {
        return;
    }
    for (int i = 1; i < pDebug->Size(); ++i) {
        mDebug |= 1 << TriggerEvent::FindType(pDebug, i);
    }
}

void TriggerMgr::SetPaths(const std::vector<bool> &paths) {
    mPaths = paths;
}

bool TriggerMgr::IsPathUnlocked(unsigned int nPath) const {
    return nPath < mPaths.size() && mPaths[nPath];
}

bool TriggerMgr::PathExists(unsigned int nPath) const {
    return nPath < mPaths.size();
}

void TriggerMgr::Schedule(TriggerAction *pAction, float flDelay) {
    const float flTime = flDelay + mTime[mClock];
    std::list<std::pair<TriggerAction *, float>> &scheduled = mScheduled[mClock];
    auto it = std::find_if(scheduled.begin(), scheduled.end(), [flTime](const auto &entry) {
        return flTime < entry.second;
    });
    scheduled.insert(it, std::make_pair(pAction, flTime));
}

void TriggerMgr::AddRunning(TriggerAction *pAction) {
    mRunning.remove(pAction);
    mRunning.push_back(pAction);
}

void TriggerMgr::Poll() {
    for (int nClock = kClockGame; nClock < kNumClocks; ++nClock) {
        mClock = nClock;
        std::list<std::pair<TriggerAction *, float>> &scheduled = mScheduled[nClock];
        auto it = scheduled.begin();
        while (it != scheduled.end() && it->second <= mTime[nClock]) {
            it->first->Exec();
            it = scheduled.erase(it);
        }
    }
    for (auto it = mRunning.begin(); it != mRunning.end();) {
        mClock = (*it)->mClock;
        if ((*it)->Poll()) {
            it = mRunning.erase(it);
        } else {
            ++it;
        }
    }
}

void TriggerMgr::Load(const char *pszFile, BinStream *pStream) {
    DataArray *pTriggers = DataArray::Read(pszFile, pStream);
    AddTriggers(pTriggers);
    pTriggers->Release();
}

void TriggerMgr::AddTriggers(DataArray *pTriggers) {
    for (int i = 0; i < pTriggers->Size(); ++i) {
        DataArray *pTrigger = pTriggers->Array(i);
        // An unrecognised kind indexes one past the last event, as in the original.
        mEvents[TriggerEvent::FindType(pTrigger, 0)].AddHandler(pTrigger);
    }
}

void TriggerMgr::Terminate() {
    for (unsigned int i = 0; i < mEvents.size(); ++i) {
        mEvents[i].Clear();
    }
    mRunning.clear();
    for (auto &scheduled : mScheduled) {
        scheduled.clear();
    }
}

void TriggerMgr::BeatEvent(char chNote) {
    mNote = chNote;
    mEvents[TriggerEvent::kTypeBeat].Fire();
    if ((mDebug & DebugBit(TriggerEvent::kTypeBeat)) != 0) {
        DebugPrint("Beat event (note %c)\n", chNote);
    }
}

void TriggerMgr::ComponentSelectEvent(UIComponentSelectMsg *pMsg) {
    mComponent = pMsg->mComponent->mName;
    mPanel = pMsg->mPanel->mName;
    mScreen = pMsg->mScreen->mName;
    mEvents[TriggerEvent::kTypeComponentSelect].Fire();
    if ((mDebug & DebugBit(TriggerEvent::kTypeComponentSelect)) != 0) {
        DebugPrint("Component select event (component %s, panel %s, screen %s)\n",
                   mComponent.c_str(),
                   mPanel.c_str(),
                   mScreen.c_str());
    }
}

void TriggerMgr::ComponentSelectStartEvent(UIComponentSelectStartMsg *pMsg) {
    mComponent = pMsg->mComponent->mName;
    mPanel = pMsg->mPanel->mName;
    mScreen = pMsg->mScreen->mName;
    mEvents[TriggerEvent::kTypeComponentSelectStart].Fire();
    if ((mDebug & DebugBit(TriggerEvent::kTypeComponentSelectStart)) != 0) {
        DebugPrint("Component select start event (component %s, panel %s, screen %s)\n",
                   mComponent.c_str(),
                   mPanel.c_str(),
                   mScreen.c_str());
    }
}

void TriggerMgr::ComponentFocusEvent(UIComponentFocusChangeMsg *pMsg) {
    mComponent = NameOrNull(pMsg->mComponent);
    mOldComponent = NameOrNull(pMsg->mOldComponent);
    mPanel = pMsg->mPanel->mName;
    mScreen = pMsg->mScreen->mName;
    mEvents[TriggerEvent::kTypeComponentFocus].Fire();
    if ((mDebug & DebugBit(TriggerEvent::kTypeComponentFocus)) != 0) {
        DebugPrint("Component focus change event (component %s, old component %s, panel %s, "
                   "screen %s)\n",
                   mComponent.c_str(),
                   mOldComponent.c_str(),
                   mPanel.c_str(),
                   mScreen.c_str());
    }
}

void TriggerMgr::ComponentFocusEvent(const char *pszComponent,
                                     const char *pszPanel,
                                     const char *pszScreen) {
    mComponent = pszComponent;
    mOldComponent = static_cast<const char *>(nullptr);
    mPanel = pszPanel;
    mScreen = pszScreen;
    mEvents[TriggerEvent::kTypeComponentFocus].Fire();
    if ((mDebug & DebugBit(TriggerEvent::kTypeComponentFocus)) != 0) {
        DebugPrint("Component focus change event (component %s, old component %s, panel %s, "
                   "screen %s)\n",
                   mComponent.c_str(),
                   mOldComponent.c_str(),
                   mPanel.c_str(),
                   mScreen.c_str());
    }
}

void TriggerMgr::ScreenChangeEvent(UIScreenChangeMsg *pMsg) {
    mScreen = NameOrNull(pMsg->mScreen);
    mOldScreen = NameOrNull(pMsg->mOldScreen);
    mEvents[TriggerEvent::kTypeScreenChange].Fire();
    if ((mDebug & DebugBit(TriggerEvent::kTypeScreenChange)) != 0) {
        DebugPrint("Screen change event (screen %s, old screen %s)\n",
                   mScreen.c_str(),
                   mOldScreen.c_str());
    }
}

void TriggerMgr::MetagameEvent(float flStart, float flTime, float flValue) {
    mTime[kClockGame] = flStart;
    mTime[kClockReal] = flTime;
    mDistance = flValue;
    mRandom = RandomFraction();
    mEvents[TriggerEvent::kTypeTime].Fire();
}

void TriggerMgr::ButtonEvent(int nPlayer, int nButtons) {
    mPlayer = nPlayer;
    mButtons = nButtons;
    mEvents[TriggerEvent::kTypeButton].Fire();
    if ((mDebug & DebugBit(TriggerEvent::kTypeButton)) != 0) {
        DebugPrint("Button event (player %d, buttons %x)\n", nPlayer, nButtons);
    }
}

void TriggerMgr::BeginEvent(int nRuleSet) {
    mGameState = kGameStateBegin;
    mRuleSet = nRuleSet;
    mEvents[TriggerEvent::kTypeBegin].Fire();
    if ((mDebug & DebugBit(TriggerEvent::kTypeBegin)) != 0) {
        DebugPrint("Begin event\n");
    }
}

void TriggerMgr::StageCompleteEvent(int nPlayer) {
    mPlayer = nPlayer;
    mEvents[TriggerEvent::kTypeStageComplete].Fire();
    if ((mDebug & DebugBit(TriggerEvent::kTypeStageComplete)) != 0) {
        DebugPrint("Stage Complete event (player %d)\n", nPlayer);
    }
}

void TriggerMgr::EndEvent(int nState, int nPlayer) {
    mGameState = nState;
    mPlayer = nPlayer;
    mEvents[TriggerEvent::kTypeEnd].Fire();
    if ((mDebug & DebugBit(TriggerEvent::kTypeEnd)) != 0) {
        DebugPrint("End event (state %d, player %d)\n", nState, nPlayer);
    }
}

void TriggerMgr::ScoreEvent(int nPlayer, int nScore) {
    mPlayer = nPlayer;
    mScore[nPlayer] = nScore;
    mEvents[TriggerEvent::kTypeScore].Fire();
    if ((mDebug & DebugBit(TriggerEvent::kTypeScore)) != 0) {
        DebugPrint("Score event (player %d, score %d)\n", nPlayer, nScore);
    }
}

void TriggerMgr::HitEvent(int nPlayer, int nGemPos) {
    mPlayer = nPlayer;
    mGemPos = nGemPos;
    mEvents[TriggerEvent::kTypeHit].Fire();
    if ((mDebug & DebugBit(TriggerEvent::kTypeHit)) != 0) {
        DebugPrint("Hit event (player %d, gemPos %d)\n", nPlayer, nGemPos);
    }
}

void TriggerMgr::MissEvent(int nPlayer, int nGemPos) {
    mPlayer = nPlayer;
    mGemPos = nGemPos;
    mEvents[TriggerEvent::kTypeMiss].Fire();
    if ((mDebug & DebugBit(TriggerEvent::kTypeMiss)) != 0) {
        DebugPrint("Miss event (player %d, gemPos %d)\n", nPlayer, nGemPos);
    }
}

void TriggerMgr::GemEvent(int nTrack, int nGemPos) {
    const int nPlayer = mCapturer[nTrack];
    if (nPlayer == kNoPlayer || mTrack[nPlayer] != nTrack) {
        return;
    }
    mGemPos = nGemPos;
    mPlayer = nPlayer;
    mEvents[TriggerEvent::kTypeGem].Fire();
    if ((mDebug & DebugBit(TriggerEvent::kTypeGem)) != 0) {
        DebugPrint("Gem event (track %d, gemPos %d)\n", nTrack, nGemPos);
    }
}

void TriggerMgr::NewBarEvent(int nBar) {
    mBar = nBar;
    mEvents[TriggerEvent::kTypeNewBar].Fire();
    if ((mDebug & DebugBit(TriggerEvent::kTypeNewBar)) != 0) {
        DebugPrint("New bar event (bar %d)\n", nBar);
    }
}

void TriggerMgr::PhraseEndEvent(int nTrack) {
    mCapturer[nTrack] = kNoPlayer;
    mEvents[TriggerEvent::kTypePhraseEnd].Fire();
    if ((mDebug & DebugBit(TriggerEvent::kTypePhraseEnd)) != 0) {
        DebugPrint("Phrase end event (track %d)\n", nTrack);
    }
}

void TriggerMgr::PhraseCaptureEvent(int nPlayer, int nStreak) {
    mPlayer = nPlayer;
    mStreak[nPlayer] = nStreak;
    mCapturer[mTrack[mPlayer]] = nPlayer;
    mEvents[TriggerEvent::kTypePhraseCapture].Fire();
    if ((mDebug & DebugBit(TriggerEvent::kTypePhraseCapture)) != 0) {
        DebugPrint("Phrase capture event (player %d, streak %d)\n", nPlayer, nStreak);
    }
}

void TriggerMgr::PhraseMissEvent(int nPlayer) {
    mPlayer = nPlayer;
    mStreak[nPlayer] = 0;
    mEvents[TriggerEvent::kTypePhraseMiss].Fire();
    if ((mDebug & DebugBit(TriggerEvent::kTypePhraseMiss)) != 0) {
        DebugPrint("Phrase miss event (player %d)\n", nPlayer);
    }
}

void TriggerMgr::NewTrackEvent(int nPlayer, int nTrack, int nInstrument, int nMode) {
    mPlayer = nPlayer;
    mTrack[nPlayer] = nTrack;
    mInstrument[nPlayer] = nInstrument;
    mMode[nPlayer] = nMode;
    mEvents[TriggerEvent::kTypeNewTrack].Fire();
    if ((mDebug & DebugBit(TriggerEvent::kTypeNewTrack)) != 0) {
        DebugPrint("New track event (player %d, track %d, instr %d, mode %d)\n",
                   nPlayer,
                   nTrack,
                   nInstrument,
                   nMode);
    }
}

void TriggerMgr::HealthEvent(int nPlayer, int nHealth) {
    mPlayer = nPlayer;
    mHealth[nPlayer] = nHealth;
    mEvents[TriggerEvent::kTypeHealth].Fire();
    if ((mDebug & DebugBit(TriggerEvent::kTypeHealth)) != 0) {
        DebugPrint("Health event (player %d, health %d)\n", nPlayer, nHealth);
    }
}

void TriggerMgr::DyingEvent(int nPlayer, int nDying) {
    mPlayer = nPlayer;
    if (mDying[nPlayer] == nDying) {
        return;
    }
    mDying[nPlayer] = nDying;
    mEvents[TriggerEvent::kTypeDyingChange].Fire();
    if ((mDebug & DebugBit(TriggerEvent::kTypeDyingChange)) != 0) {
        DebugPrint("Dying event (player %d, dying %d)\n", nPlayer, nDying);
    }
}

void TriggerMgr::BossJourneyEvent() {
    mEvents[TriggerEvent::kTypeBossJourney].Fire();
    if ((mDebug & DebugBit(TriggerEvent::kTypeBossJourney)) != 0) {
        DebugPrint("Boss Journey event\n");
    }
}

void TriggerMgr::LyricEvent(const char *pszLyric) {
    mLyric = pszLyric;
    mEvents[TriggerEvent::kTypeLyric].Fire();
    if ((mDebug & DebugBit(TriggerEvent::kTypeLyric)) != 0) {
        DebugPrint("Lyric event (lyric %s)\n", pszLyric);
    }
}

void TriggerMgr::PathUnlockedEvent(int nPath) {
    mEvents[TriggerEvent::kTypePathUnlocked].Fire();
    if ((mDebug & DebugBit(TriggerEvent::kTypePathUnlocked)) != 0) {
        DebugPrint("Path unlocked event (path %d)\n", nPath);
    }
}
