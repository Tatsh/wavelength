#include "game/remixlogic.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <new>
#include <vector>

#include "game/axetrack.h"
#include "game/difficultyrating.h"
#include "game/forcefeedbackmgr.h"
#include "game/gameconfig.h"
#include "game/gamedb.h"
#include "game/localplayer.h"
#include "game/mixer.h"
#include "game/netgamescore.h"
#include "game/remixinfo.h"
#include "game/remixtutorial.h"
#include "game/remoteplayer.h"
#include "game/scratchtrack.h"
#include "game/track.h"
#include "gfx/gfxmanager.h"
#include "gs/muse.h"
#include "msg/chatmsg.h"
#include "netflow/netlaunchpad.h"
#include "netflow/nettransport.h"
#include "os/bufstream.h"
#include "os/debug.h"
#include "os/joypad.h"
#include "os/keyboard.h"
#include "os/loadfile.h"
#include "os/locale.h"
#include "os/mem.h"
#include "os/memfun1command.h"
#include "os/memfun2command.h"
#include "os/memfun4command.h"
#include "os/memfuncommand.h"
#include "os/memstream.h"
#include "os/scheduler.h"
#include "os/string.h"
#include "os/system.h"
#include "script/dataarray.h"
#include "synth/fxmidi.h"
#include "synth/songspeed.h"

namespace {

constexpr char kGameSection[] = "game";
constexpr char kVoxVolumeKey[] = "remix_vox_volume";
constexpr char kAxeTrackTag[] = "AxeTrack";
constexpr char kScratchTrackTag[] = "ScratchTrack";
constexpr char kRemixBufferTag[] = "remix buf";
constexpr char kDuelAuthoringText[] = "DUEL AUTHORING";
constexpr char kEmptyText[] = "";

constexpr char kNextSectionRequestKey[] = "NEXT_SECTION_REQUEST";
constexpr char kPrevSectionRequestKey[] = "PREV_SECTION_REQUEST";
constexpr char kEditRequestKey[] = "EDIT_REQUEST";
constexpr char kPreviewRequestKey[] = "PREVIEW_REQUEST";
constexpr char kFreestyleStartKey[] = "FREESTYLE_START";
constexpr char kClientAbortedKey[] = "CLIENT_ABORTED";
constexpr char kHostAborted1Key[] = "HOST_ABORTED_REMIX_1";
constexpr char kHostAborted2Key[] = "HOST_ABORTED_2";
constexpr char kConnectionLost1Key[] = "CONNECTION_LOST_1";
constexpr char kConnectionLost2Key[] = "CONNECTION_LOST_2";
constexpr char kBooted1Key[] = "BOOTED_1";
constexpr char kBooted2Key[] = "BOOTED_2";

// The volume of a track, and the default of `remix_vox_volume`.
constexpr int kFullTrackVolume = 120;

constexpr int kTicksPerBeat = 480;
constexpr float kBeatTicks = 480.0f;
constexpr float kMsPerMinute = 60000.0f;

constexpr int kNoBar = -1;
constexpr int kNoPattern = -1;
constexpr int kNoPad = -1;
constexpr int kNoPlayer = -1;
constexpr int kFirstPlayer = 0;
constexpr int kFirstPad = 0;
constexpr int kFirstSection = 0;
constexpr int kFirstPattern = 0;
constexpr int kFirstBar = 0;
constexpr int kNotDisconnected = 0;

// The track a tutorial player starts on.
constexpr int kTutorialTrack = 3;

constexpr int kBlankScreenTick = -3840;

// The ticks after a solo preview starts at which the player enters the freestyle track.
constexpr int kFreestyleDelayTicks = 2000;

// The delays of the section labels, in ticks.
constexpr int kFirstLabelDelayTicks = 300;
constexpr int kLabelDelayStepTicks = 20;

// The delays of the section checkpoints, in milliseconds.
constexpr int kCheckpointDelayMs = 100;
constexpr int kCheckpointDelayStepMs = 10;

constexpr float kArenaBlendDelayMs = 500.0f;
constexpr float kShowPlayersDelayMs = 1000.0f;
constexpr float kGameEndedDelayMs = 3500.0f;

constexpr int kMinFadeTicks = 960;
constexpr int kEndTickMargin = 480;
constexpr int kMixerRampTicks = 0;
constexpr unsigned char kSilentVolume = 0;

constexpr float kOutroNow = -1.0f;

constexpr float kCheckpointOffset = 0.0f;
constexpr float kCheckpointScale = 1.0f;
constexpr double kSectionCheckpointScale = 0.9;

constexpr float kSectionMessageMs = 1600.0f;
constexpr float kSectionMessageScale = 1.0f;
constexpr float kNoticeMessageMs = 3200.0f;
constexpr float kAbortMessageScale = 0.9f;
constexpr float kGameEndedMessageScale = 0.8f;
constexpr float kMessageOffset = 0.0f;

constexpr float kMinSpeed = 0.8f;
constexpr float kMaxSpeed = 1.2f;
constexpr float kTempoStep = 1.0f;
constexpr float kTempoMatch = 0.5f;
constexpr double kNormalSpeed = 1.0;
constexpr double kSpeedMatch = 0.5;

constexpr float kProgressOutsidePreview = 50.0f;

// The section change starts a bar later once this part of the current bar has passed.
constexpr int kSectionChangeLateDivisor = 3;
constexpr int kSectionChangeLeadBars = 2;

// The force feedback metronome beats every beat.
constexpr int kMetronomePeriodTicks = kTicksPerBeat;

// The chat channel of the session notices, and the recipient of every console.
constexpr int kNoticeChannel = 3;
constexpr int kAllConsoles = 0;
constexpr int kEchoChat = 1;

// The kinds of effect an FXPacket switches.
constexpr int kEffectStutter = 0;
constexpr int kEffectFilter = 1;
constexpr int kEffectChorus = 2;

constexpr unsigned char kSaveVersion = 3;

// A packed remix may grow by a thousandth and a header.
constexpr int kPackedSlackDivisor = 1000;
constexpr int kPackedHeaderSize = 12;

constexpr int kCatchSlot = 0;

} // namespace

RemixLogic::RemixLogic(Song *pSong)
    : mSong(pSong), mTicksPerBar(pSong->mBuilder->mTicksPerBar), mNumBars(pSong->mNumBars),
      mState(kStatePlaying), mSavedState(kStatePlaying), mPaused(0), mPlayMap(pSong->GetPlayMap()),
      mSections(pSong->GetSections()), mFreestyleTrack(nullptr), mTrackSelector(nullptr),
      mWorldBeat(&TheSongScheduler, pSong->GetWorldTrack(), mTicksPerBar * mNumBars),
      mSectionIndex(0), mLoopBar(0), mMuteRemote(0), mPreviewing(0), mQuit(0), mModified(0),
      mReadOnly(TheGameDb->mTutorial == 0 ? TheGameDb->GetRemixInfo()->mReadOnly != 0 : 0),
      mBaseTempo(kMsPerMinute / (pSong->GetMsPerTick()[0] * kBeatTicks)), mNetArbiter(nullptr),
      mArbiterVersion(0), mUpdateVersions(TheGameDb->GetNumPlayers(), 0u),
      mPlayerTracks(TheGameDb->GetNumPlayers(), kNoPlayer), mTrackVolumes(pSong->GetNumTracks(), 0),
      mFx(pSong->GetNumTracks() - 1, pSong->GetRemixEffectsBus()),
      mTransitionCmd(NewMemFunCommand(this, &RemixLogic::OnTransition)),
      mSectionStartCmd(NewMemFunCommand(this, &RemixLogic::ApplySectionTracks)),
      mUpdateHudsCmd(NewMemFunCommand(this, &RemixLogic::UpdateHuds)),
      mSectionEndCmd(NewMemFunCommand(this, &RemixLogic::OnSectionEnd)),
      mApplyEffectsCmd(NewMemFunCommand(this, &RemixLogic::ApplySectionEffects)),
      mPlayIntroCmd(NewMemFunCommand(this, &RemixLogic::PlayIntro)),
      mEndIntroCmd(NewMemFunCommand(this, &RemixLogic::EndIntro)), mTutorial(nullptr),
      mSavedMuteRemote(0), mEndTick(0), mRemixLoaded(0) {
    if (TheGameDb->mTutorial == 0) {
        TheSongScheduler.PostAt(
            NewMemFun1Command(&TheGfxManager, &GfxManager::ShowTrackLabels, true), 0, false);
    }

    int nFreestyleType;
    int nFreestyleInstrument;
    BuildTracks(&nFreestyleType, &nFreestyleInstrument);
    if (TheGameDb->mTutorial == 0) {
        LoadRemix();
    }
    TheMixer->Init(static_cast<int>(mRemixTracks.size()) + 1, nFreestyleType);

    int nVoxVolume = kFullTrackVolume;
    (void)SystemConfig()
        ->FindArray(kGameSection, false)
        ->FindInt(kVoxVolumeKey, &nVoxVolume, false);
    (void)TheGameDb->FindSong(TheGameDb->mSong.c_str())->FindInt(kVoxVolumeKey, &nVoxVolume, false);
    for (int i = 0; i < mSong->GetNumTracks(); ++i) {
        if (mSong->GetTrackType(i) == Song::kTrackTypeVox) {
            mTrackVolumes[i] = nVoxVolume;
        } else {
            mTrackVolumes[i] = kFullTrackVolume;
        }
    }

    int nLocalPlayers = 0;
    for (int i = 0; i < TheGameDb->GetNumPlayers(); ++i) {
        if (TheGameDb->IsLocalPlayer(i)) {
            mPlayers.push_back(new LocalPlayer(i, mTicksPerBar, nLocalPlayers++));
        } else {
            mPlayers.push_back(new RemotePlayer(i, mTicksPerBar));
        }
        TheGfxManager.SetEnergyShown(i, false);
        TheGfxManager.SetScoreShown(i, false);
    }

    const char *pszEffect = mSong->GetRemixEffectsName();
    for (int i = 0; i < TheGameDb->GetNumPads(); ++i) {
        RemixHUD *pHud = new RemixHUD(i, pszEffect);
        pHud->SetCallback(OnHudEvent, this);
        if (i == kFirstPad) {
            const double dSpeedOffset = std::fabs(TheSongScheduler.GetSpeed() - kNormalSpeed);
            (void)(dSpeedOffset < kSpeedMatch); // Yes, the binary compares and passes 1 regardless.
            pHud->SetTempo(GetTempo(), 1);
        }
        mHuds.push_back(pHud);
    }

    const std::vector<Track *> tracks(mRemixTracks.begin(), mRemixTracks.end());
    mTrackSelector =
        new GameTrackSelector(tracks, mFreestyleTrack, nFreestyleType, nFreestyleInstrument);

    if (TheGameDb->mCommunity == GameDb::kCommunityOnline) {
        for (int i = 0; i < TheGameDb->GetNumPlayers(); ++i) {
            const unsigned int nNetOrder = TheGameDb->GetPlayerNetOrder(i);
            if (nNetOrder >= mNetOrderPlayers.size()) {
                mNetOrderPlayers.resize(nNetOrder + 1, kNoPlayer);
            }
            mNetOrderPlayers[nNetOrder] = i;
        }
    }

    if (TheGameDb->mCommunity == GameDb::kCommunityOnline) {
        const int nPlayers = TheGameDb->GetNumPlayers();
        std::vector<int> netOrders(nPlayers, 0);
        for (int i = 0; i < nPlayers; ++i) {
            netOrders[i] = TheGameDb->GetPlayerNetOrder(i);
        }
        std::sort(netOrders.begin(), netOrders.end());
        std::vector<int> playerTracks(nPlayers, 0);
        for (int i = 0; i < nPlayers; ++i) {
            playerTracks[GetPlayerFromNetOrder(netOrders[i])] = i;
        }
        for (int i = 0; i < nPlayers; ++i) {
            mTrackSelector->AddPlayer(mPlayers[i], playerTracks[i]);
            mPlayerTracks[i] = playerTracks[i];
        }
    } else if (TheGameDb->mTutorial != 0) {
        mTrackSelector->AddPlayer(mPlayers[kFirstPlayer], kTutorialTrack);
    } else {
        (void)mSong->GetNumTracks(); // Yes, the binary discards this call's result.
        for (unsigned int i = 0; i < mPlayers.size(); ++i) {
            mTrackSelector->AddPlayer(mPlayers[i], static_cast<int>(i));
        }
    }

    if (TheGameDb->mCommunity == GameDb::kCommunityOnline && TheNetTransport->IsHost()) {
        mNetArbiter = new NetArbiter(mTrackSelector);
    }

    SlotGrid *pGrid = mSong->GetSlotGrid();
    mPlayableSections.reserve(pGrid->GetNumFree());
    for (int i = 0; i < pGrid->GetNumSections(); ++i) {
        if (!pGrid->HasPattern(i)) {
            mPlayableSections.push_back(i);
        }
    }
    if (mRemixLoaded == 0) {
        for (int i = 1; i < mSections->NumSections(); ++i) {
            mFx.AddSet(mSections->SectionStart(i));
        }
    }

    if (g_bDuelAuthoring != 0) {
        TheGfxManager.SetLyricText(kDuelAuthoringText, true);
        TheGfxManager.SetSectionCount(1);
        TheGfxManager.SetSectionLabel(kFirstSection, kEmptyText);
    } else {
        TheGfxManager.SetSectionCount(static_cast<int>(mPlayableSections.size()));
        int nDelay = 0;
        int nNextDelay = kFirstLabelDelayTicks;
        for (unsigned int i = 0; i < mPlayableSections.size(); ++i) {
            nDelay = nNextDelay;
            nNextDelay = nDelay + kLabelDelayStepTicks;
            TheSongScheduler.PostIn(NewMemFun2Command(&TheGfxManager,
                                                      &GfxManager::SetSectionLabel,
                                                      static_cast<int>(i),
                                                      mSong->GetSectionLabel(static_cast<int>(i))),
                                    nDelay,
                                    false);
        }
        TheSongScheduler.PostIn(
            NewMemFun1Command(&TheGfxManager, &GfxManager::HighlightSection, kFirstSection),
            nDelay + 1,
            false);
    }

    if (mReadOnly == 0) {
        QueueSectionChange(kFirstSection);
        if (TheGameDb->GetRemixActive() == 0 && TheGameDb->GetRemixBuffer() == nullptr) {
            for (PitchTrack *pTrack : mPitchTracks) {
                pTrack->SelectPattern(kFirstPattern, kFirstBar, mNumBars);
            }
            mModified = 1;
        }
    }
    TheGfxManager.AddCheckpoint(nullptr, 0.0f, kCheckpointOffset, kCheckpointScale);
    KeyboardAddSink(this);
    for (VoxTrack *pTrack : mVoxTracks) {
        pTrack->SetSection(kFirstSection);
    }
    SetMuteRemote(mMuteRemote);
    UpdateHuds();
    if (TheGameDb->mTutorial != 0) {
        mTutorial = new RemixTutorial(this);
    }
}

int RemixLogic::GetPlayerFromNetOrder(int nNetOrder) const {
    return mNetOrderPlayers[nNetOrder];
}

RemixLogic::~RemixLogic() {
    TheGfxManager.SetLyricText(kEmptyText, true);
    if (TheGameDb->mTutorial == 0) {
        SaveRemix();
    }
    if (TheGameDb->mCommunity == GameDb::kCommunityOnline && TheNetTransport->IsHost()) {
        TheNetTransport->ReportScores(std::vector<NetGameScore>());
    }
    KeyboardRemoveSink(this);
    delete mTutorial;
    mTutorial = nullptr;
    delete mTrackSelector;
    for (RemixTrack *pTrack : mRemixTracks) {
        delete pTrack;
    }
    delete mFreestyleTrack;
    for (Player *pPlayer : mPlayers) {
        delete pPlayer;
    }
    for (RemixHUD *pHud : mHuds) {
        delete pHud;
    }
    delete mNetArbiter;
    mPlayMap->ClearLoops();
    TheGameDb->SetNetRemixEnded(mModified);
}

void RemixLogic::Start() {
    for (RemixTrack *pTrack : mRemixTracks) {
        pTrack->Start();
    }
    mFreestyleTrack->Start();
    for (Player *pPlayer : mPlayers) {
        pPlayer->HidePowerup();
    }
    for (int i = 0; i < mSong->GetNumBackMusic(); ++i) {
        mSong->GetBackMusic(i)->Start(mPlayMap);
    }
    PlayIntro();
    mWorldBeat.Start();
    mFx.Activate();

    const int nFirstBeat = mTicksPerBar - mTicksPerBar * mSong->mIntroBars;
    TheForceFeedbackMgr->Start(mSong->GetMsPerTick(), mTicksPerBar);
    TheForceFeedbackMgr->StartMetronome(kMetronomePeriodTicks, nFirstBeat);

    if (mReadOnly != 0) {
        StartPreview(kNoBar, false);
    } else {
        TheSongScheduler.PostAt(NewMemFunCommand(this, &RemixLogic::UpdateHuds), 0, false);
    }
    TheSongScheduler.PostAt(
        NewMemFunCommand(&TheMetagame, &Metagame::ShowBlankScreen), kBlankScreenTick, false);
    if (mTutorial != nullptr) {
        mTutorial->Start();
    }
    ScheduleControllerCheck();
    if (mNetArbiter != nullptr) {
        mNetArbiter->Start();
    }
}

void RemixLogic::Stop() {
    mState = kStateFinished;
    for (int i = 0; i < mSong->GetNumBackMusic(); ++i) {
        mSong->GetBackMusic(i)->Stop();
    }
    for (int i = 0; i < mSong->GetNumIntroMuses(); ++i) {
        mSong->GetIntroMuse(i)->Stop();
    }
    for (RemixTrack *pTrack : mRemixTracks) {
        pTrack->Stop();
    }
    for (Player *pPlayer : mPlayers) {
        pPlayer->CancelMultiplier();
    }
    mWorldBeat.Stop();
    TheForceFeedbackMgr->StopAll();
}

void RemixLogic::Poll() {
    if (mTutorial != nullptr && !mTutorial->Poll()) {
        delete mTutorial;
        mTutorial = nullptr;
        mState = kStateTutorialEnd;
        TheMetagame.ShowDialog(Metagame::kDialogTutorialEnd, OnPauseDialog, this, kFirstPad);
    }
    if (mState != kStateEnding) {
        return;
    }
    if (!TheGfxManager.IsOutroDone()) {
        return;
    }
    if (TheSongScheduler.IsRunning() && TheSongScheduler.mTick < mEndTick) {
        return;
    }
    Stop();
}

bool RemixLogic::IsFinished() const {
    return mState == kStateFinished;
}

int RemixLogic::HasQuit() const {
    return mQuit;
}

bool RemixLogic::IsFreestyling(int nPlayer) {
    return mPlayers[nPlayer]->GetTrack() == mFreestyleTrack;
}

int RemixLogic::GetTick() {
    return TheSongScheduler.GetClockTick();
}

float RemixLogic::GetTime() {
    return TheSongScheduler.GetClockTime();
}

float RemixLogic::GetProgress() {
    if (mPreviewing == 0) {
        return kProgressOutsidePreview;
    }
    const int nSongTicks = mNumBars * mTicksPerBar;
    return static_cast<float>((TheSongScheduler.mTick - mLoopBar * mTicksPerBar) % nSongTicks) /
           static_cast<float>(nSongTicks);
}

void RemixLogic::SetPaused(bool bPaused, int nPad, int nReason) {
    if (mPaused == static_cast<int>(bPaused)) {
        return;
    }
    mPaused = bPaused;
    if (bPaused) {
        TheForceFeedbackMgr->Pause();
        SetHudsActive(0);
        const int nState = mState;
        mState = kStatePaused;
        mSavedState = nState;
        for (int i = 0; i < TheGameDb->GetNumPads(); ++i) {
            JoypadSetMenuControl(i, true);
        }
        TheMetagame.ShowDialog(nReason == 0 ? Metagame::kDialogPause :
                                              Metagame::kDialogNoController,
                               OnPauseDialog,
                               this,
                               nPad);
        SystemSetPadCheck(true);
        if (TheGameDb->mCommunity != GameDb::kCommunityOnline) {
            TheSongScheduler.Pause();
            AllNotesOff();
        }
    } else {
        TheForceFeedbackMgr->Resume();
        UpdateHuds();
        mState = mSavedState;
        for (int i = 0; i < TheGameDb->GetNumPads(); ++i) {
            JoypadSetMenuControl(i, false);
        }
        SystemSetPadCheck(false);
        if (TheGameDb->mCommunity != GameDb::kCommunityOnline) {
            TheSongScheduler.Resume();
        }
        if (TheGameDb->mCommunity == GameDb::kCommunitySolo) {
            TheGfxManager.SetFreqSize(kFirstPlayer, TheGameDb->GetOptions()->mFreqSize);
        }
    }
    if (mTutorial != nullptr) {
        mTutorial->SetPaused(bPaused);
    }
    WorldLogic::SetPaused(bPaused, nPad, nReason);
}

void RemixLogic::OnPauseDialog(Metagame::DialogAction action, void *pUserData) {
    auto *pThis = static_cast<RemixLogic *>(pUserData);
    if (pThis->mState != kStatePaused && pThis->mState != kStateTutorialEnd) {
        return;
    }
    switch (action) {
    case Metagame::kDialogActionResume:
        pThis->SetPaused(false, kNoPad, kNotDisconnected);
        break;
    case Metagame::kDialogActionQuit:
        pThis->mQuit = 1;
        pThis->EndRemix();
        break;
    case Metagame::kDialogActionEnd:
        if (TheGameDb->mCommunity == GameDb::kCommunityOnline) {
            TheNetTransport->Leave();
        }
        pThis->EndRemix();
        break;
    default:
        DebugWarn("illegal dialog response");
        break;
    }
}

void RemixLogic::BuildTracks(int *pFreestyleType, int *pFreestyleInstrument) {
    for (int nTrack = 0; nTrack < mSong->GetNumTracks(); ++nTrack) {
        const int nType = mSong->GetTrackType(nTrack);
        switch (nType) {
        case Song::kTrackTypePitch: {
            auto *pTrack = new PitchTrack(nTrack,
                                          mSong->mIntroBars,
                                          mNumBars,
                                          mSong->mBuilder->mTicksPerBar,
                                          mSong->GetTrackRiffData(nTrack),
                                          mSong->GetTrackPitchData(nTrack),
                                          mPlayMap,
                                          mSections,
                                          mSong->GetSlotGrid(),
                                          true);
            mPitchTracks.push_back(pTrack);
            mRemixTracks.push_back(pTrack);
            break;
        }
        case Song::kTrackTypeVox: {
            auto *pTrack = new VoxTrack(mSong->GetVoxData(nTrack),
                                        mPlayMap,
                                        mSections,
                                        mSong->GetSlotGrid(),
                                        nTrack,
                                        mSong->mIntroBars,
                                        mNumBars,
                                        mTicksPerBar);
            mVoxTracks.push_back(pTrack);
            mRemixTracks.push_back(pTrack);
            break;
        }
        case Song::kTrackTypeAxe: {
            *pFreestyleType = nType;
            *pFreestyleInstrument = mSong->GetTrackInstrument(nTrack);
            void *pBlock = PoolMemAlloc(sizeof(AxeTrack), kAxeTrackTag, 0);
            mFreestyleTrack = new (pBlock) AxeTrack(mSong->GetAxeData(nTrack),
                                                    mSections,
                                                    mSong->GetPlayMap(),
                                                    mSong->GetMsPerTick(),
                                                    nTrack,
                                                    mSong->mIntroBars,
                                                    mNumBars,
                                                    mTicksPerBar);
            break;
        }
        case Song::kTrackTypeScratch: {
            *pFreestyleType = nType;
            *pFreestyleInstrument = mSong->GetTrackInstrument(nTrack);
            void *pBlock = PoolMemAlloc(sizeof(ScratchTrack), kScratchTrackTag, 0);
            mFreestyleTrack = new (pBlock) ScratchTrack(mSong->GetScratchData(nTrack),
                                                        mSections,
                                                        mSong->GetPlayMap(),
                                                        mSong->GetMsPerTick(),
                                                        nTrack,
                                                        mSong->mIntroBars,
                                                        mNumBars,
                                                        mTicksPerBar);
            break;
        }
        default:
            DebugWarn("unknown track type %i for track %i", nType, nTrack);
            break;
        }
    }
}

bool RemixLogic::IsInputLocked(int nPressed, bool bNote) {
    const int nTick = TheSongScheduler.mTick;
    const int nBar = nTick / mTicksPerBar;
    if (nTick >= 0 && nBar >= mLoopBar) {
        return false;
    }
    if (nPressed == 0) {
        return true;
    }
    if (bNote && (mPreviewing != 0 || mSong->mIntroBars >= nBar)) {
        return true;
    }
    FxMidi::PlaySound1();
    return true;
}

void RemixLogic::EndRemix() {
    if (mQuit != 0) {
        Stop();
        return;
    }
    mState = kStateEnding;
    const float fOutroMs = TheGfxManager.StartOutro(kOutroNow);
    const int nFadeTicks =
        std::max(kMinFadeTicks, static_cast<int>(fOutroMs / mSong->GetMsPerTick()[0]));
    TheMixer->FadeOut(nFadeTicks);
    mEndTick = TheSongScheduler.mTick + nFadeTicks + kEndTickMargin;
    TheSongScheduler.PostAt(
        new MemFunCommand<void (WorldLogic::*)(), RemixLogic>(this, &WorldLogic::AllNotesOff),
        mEndTick - 1,
        false);
}

void RemixLogic::HandleInput(const RotateEvent &event) {
    if (mState != kStatePlaying || mPreviewing != 0) {
        return;
    }
    if (mTutorial != nullptr) {
        if (mTutorial->mRotationEnabled == 0) {
            return;
        }
        if (mTutorial->mRotationLeftAllowed == 0 &&
            event.mDirection == RotateEvent::kDirectionPrevious) {
            return;
        }
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
    if (mNetArbiter != nullptr) {
        mNetArbiter->RequestTrack(nPlayer, mTrackSelector->GetPlayerTrack(nPlayer));
    }
    UpdateHuds();
    if (mTutorial != nullptr) {
        mTutorial->NotifyRotate(event.mDirection == RotateEvent::kDirectionNext,
                                mTrackSelector->GetPlayerTrack(kFirstPlayer));
    }
}

void RemixLogic::HandleInput(const PlayNoteEvent &event) {
    if (mState != kStatePlaying) {
        return;
    }
    if (IsInputLocked(event.mState, true)) {
        return;
    }
    if (mPreviewing != 0 && TheGameDb->mCommunity != GameDb::kCommunitySolo) {
        return;
    }
    if (mTutorial != nullptr && mTutorial->mPitchingEnabled == 0) {
        return;
    }
    const int nPlayer = event.mPlayer;
    Player *pPlayer = mPlayers[nPlayer];
    bool bOnAssignedTrack = true;
    if (TheGameDb->mCommunity == GameDb::kCommunityOnline) {
        const int nTrack = pPlayer->GetTrack()->mIndex;
        if (TheNetTransport->IsHost()) {
            bOnAssignedTrack = nTrack == mNetArbiter->GetTrack(nPlayer);
        } else {
            bOnAssignedTrack = nTrack == mPlayerTracks[nPlayer];
        }
    }
    if (bOnAssignedTrack) {
        pPlayer->HandleInput(event);
        mModified = 1;
    }
    if (event.mState != 0 && mTutorial != nullptr) {
        mTutorial->NotifyPitch();
    }
}

void RemixLogic::HandleInput(const BtnEvent<8> &event) {
    if (mState != kStatePlaying || mPreviewing != 0) {
        return;
    }
    if (IsInputLocked(1, false)) {
        return;
    }
    if (mTutorial != nullptr && mTutorial->mErasingEnabled == 0) {
        return;
    }
    mPlayers[event.mPlayer]->HandleInput(event);
    mModified = 1;
}

void RemixLogic::HandleInput(const BtnEvent<3> &event) {
    if (mState != kStatePlaying) {
        return;
    }
    SetPaused(true, TheGameDb->GetPlayerPad(event.mPlayer), kNotDisconnected);
}

void RemixLogic::HandleInput(const ChangeSectionEvent &event) {
    if (mState != kStatePlaying || mPreviewing != 0) {
        return;
    }
    if (IsInputLocked(1, false)) {
        return;
    }
    if (mTutorial != nullptr && (mTutorial->mSectionChangeEnabled == 0 || event.mDirection != 0)) {
        return;
    }
    if (TheGameDb->mCommunity == GameDb::kCommunityOnline && !TheNetTransport->IsHost()) {
        const char *pszFormat = TheLocale.Localize(
            event.mDirection == 0 ? kNextSectionRequestKey : kPrevSectionRequestKey, true);
        (void)ChatMsg::Send(kNoticeChannel,
                            kAllConsoles,
                            FormatString(pszFormat, TheGameDb->GetPlayerName(event.mPlayer)),
                            kEchoChat);
        return;
    }
    if (TheGameDb->mCommunity == GameDb::kCommunityLocal && event.mPlayer != kFirstPlayer) {
        return;
    }
    if (event.mDirection == 0) {
        ++mSectionIndex;
        if (static_cast<unsigned int>(mSectionIndex) >= mPlayableSections.size()) {
            mSectionIndex = 0;
        }
    } else {
        --mSectionIndex;
        if (mSectionIndex < 0) {
            mSectionIndex = static_cast<int>(mPlayableSections.size()) - 1;
        }
    }
    TheGfxManager.ShowMessage(mSong->GetSectionLabel(mSectionIndex),
                              nullptr,
                              kNoPlayer,
                              kSectionMessageMs,
                              kSectionMessageScale,
                              kMessageOffset,
                              kMessageOffset);
    StartTransition();
    QueueSectionChange(mSectionIndex);
    if (TheGameDb->mCommunity == GameDb::kCommunityOnline) {
        SectionChangePacket packet;
        packet.mSection = mSectionIndex;
        packet.mTick = mLoopBar; // Yes, the binary sends the bar the section loops from.
        TheNetTransport->Send(packet);
    }
    if (mTutorial != nullptr) {
        mTutorial->NotifySectionChange(event.mDirection == 0);
    }
}

void RemixLogic::HandleInput(const BtnEvent<9> &event) {
    if (mState != kStatePlaying || mReadOnly != 0) {
        return;
    }
    if (IsInputLocked(1, false)) {
        return;
    }
    const int nCommunity = TheGameDb->mCommunity;
    if (nCommunity == GameDb::kCommunityLocal && event.mPlayer != kFirstPlayer) {
        return;
    }
    if (nCommunity == GameDb::kCommunityOnline && !TheNetTransport->IsHost()) {
        const char *pszFormat =
            TheLocale.Localize(mPreviewing != 0 ? kEditRequestKey : kPreviewRequestKey, true);
        (void)ChatMsg::Send(kNoticeChannel,
                            kAllConsoles,
                            FormatString(pszFormat, TheGameDb->GetPlayerName(event.mPlayer)),
                            kEchoChat);
        return;
    }
    if (mTutorial != nullptr && mTutorial->mPreviewEnabled == 0) {
        return;
    }
    if (mPreviewing != 0) {
        StopPreview(kNoBar);
    } else {
        StartPreview(kNoBar, true);
    }
}

void RemixLogic::HandleInput(const StickEvent<2> &event) {
    if (mState != kStatePlaying) {
        return;
    }
    if (mTutorial != nullptr && mTutorial->mStick2Enabled == 0) {
        return;
    }
    mPlayers[event.mPlayer]->HandleInput(event);
}

void RemixLogic::HandleInput(const StickEvent<6> &event) {
    if (mState != kStatePlaying) {
        return;
    }
    if (mTutorial != nullptr && mTutorial->mStick6Enabled == 0) {
        return;
    }
    mPlayers[event.mPlayer]->HandleInput(event);
}

void RemixLogic::HandleInput(const BtnEvent<10> &event) {
    if (mState != kStatePlaying) {
        return;
    }
    if (IsInputLocked(1, false)) {
        return;
    }
    if (mTutorial != nullptr && mTutorial->mButton10Enabled == 0) {
        return;
    }
    mPlayers[event.mPlayer]->HandleInput(event);
}

PitchTrack *RemixLogic::FindPitchTrack(int nTrack) const {
    for (PitchTrack *pTrack : mPitchTracks) {
        if (pTrack->mIndex == nTrack) {
            return pTrack;
        }
    }
    return nullptr;
}

VoxTrack *RemixLogic::FindVoxTrack(int nTrack) const {
    for (VoxTrack *pTrack : mVoxTracks) {
        if (pTrack->mIndex == nTrack) {
            return pTrack;
        }
    }
    return nullptr;
}

void RemixLogic::SetHudsActive(int nActive) {
    for (RemixHUD *pHud : mHuds) {
        pHud->SetActive(nActive);
    }
}

void RemixLogic::UpdateHud(int nPlayer) {
    int nActive = 0;
    const int nSet = mFx.GetCurrentSet();
    Player *pPlayer = mPlayers[nPlayer];
    const Track *pTrack = pPlayer->GetTrack();
    const int nBar = TheSongScheduler.mTick / mTicksPerBar;
    const int nTrack = pTrack->mIndex;
    if (pTrack->mPlayer == pPlayer && nBar >= mLoopBar) {
        nActive = mPreviewing == 0;
    }

    int nMuted = 0;
    const int nType = mSong->GetTrackType(nTrack);
    const int nSection = mPlayableSections[mSectionIndex];
    if (nType == Song::kTrackTypeVox) {
        nMuted = FindVoxTrack(nTrack)->IsSectionMuted(nSection);
    }

    int nFilter = 0;
    int nStutter = 0;
    int nChorus = 0;
    if (static_cast<unsigned int>(nTrack) < mRemixTracks.size()) {
        nFilter = mFx.GetFilter(nSet, nTrack);
        nStutter = mFx.GetStutter(nSet, nTrack);
        nChorus = mFx.GetChorus(nSet, nTrack);
    }

    RemixHUD *pHud = mHuds[nPlayer];
    pHud->SetActive(nActive);
    pHud->SetChorus(nFilter); // Yes, the binary shows the filter on the chorus panel.
    pHud->SetStutter(nStutter);
    pHud->SetEffect(nChorus);
    pHud->SetLoop(pPlayer->GetRepeat());
    pHud->SetMute(nMuted);
    pHud->SetSolo(mMuteRemote);
    pHud->SetMode(nType);
}

void RemixLogic::UpdateHuds() {
    if (TheGameDb->mCommunity == GameDb::kCommunityOnline) {
        UpdateHud(kFirstPlayer);
        return;
    }
    for (unsigned int i = 0; i < mPlayers.size(); ++i) {
        UpdateHud(static_cast<int>(i));
    }
}

void RemixLogic::QueueSectionChange(int nIndex) {
    const int nTick = TheSongScheduler.mTick;
    const int nBar = std::max(0, nTick / mTicksPerBar + kSectionChangeLeadBars);
    const int nLate = mTicksPerBar / kSectionChangeLateDivisor < nTick % mTicksPerBar;
    ChangeSection(nIndex, nBar + nLate);
}

void RemixLogic::ChangeSection(int nIndex, int nBar) {
    const int nSection = mPlayableSections[nIndex];
    TheSongScheduler.PostAt(mApplyEffectsCmd.Get(), nBar * mTicksPerBar, false);
    int nStartBar;
    int nEndBar;
    mSections->GetSectionRange(nSection, &nStartBar, &nEndBar);
    SetLoop(nStartBar, nEndBar, nBar, nBar);
    mSectionIndex = nIndex;
    UpdateHuds();
    TheGfxManager.HighlightSection(g_bDuelAuthoring != 0 ? kFirstSection : nIndex);
}

void RemixLogic::StartTransition() {
    const int nHalfBar = mTicksPerBar / 2;
    (void)TheGfxManager.StartIntro(static_cast<float>(nHalfBar) * mSong->GetMsPerTick()[0]);
    TheSongScheduler.PostIn(mTransitionCmd.Get(), nHalfBar, false);
}

void RemixLogic::OnTransition() {
    const int nLoopTick = mLoopBar * mTicksPerBar;
    TheGfxManager.AddCheckpoint(
        nullptr, static_cast<float>(nLoopTick), kCheckpointOffset, kCheckpointScale);
    const int nBar = TheSongScheduler.mTick / mTicksPerBar;
    for (RemixTrack *pTrack : mRemixTracks) {
        pTrack->SkipBars(nBar, mLoopBar);
        pTrack->Redraw(mLoopBar);
    }
    for (PitchTrack *pTrack : mPitchTracks) {
        pTrack->SetEditing(mPreviewing != 0);
    }
    TheSongScheduler.PostAt(mSectionStartCmd.Get(), nLoopTick - 1, false);
    TheSongScheduler.PostAt(mUpdateHudsCmd.Get(), nLoopTick, false);
    (void)TheGfxManager.RestartIntro();
}

void RemixLogic::ApplySectionTracks() {
    const int nSection = mPlayableSections[mSectionIndex];
    for (VoxTrack *pTrack : mVoxTracks) {
        pTrack->SetSection(nSection);
    }
    if (mPreviewing != 0) {
        return;
    }
    for (RemixTrack *pTrack : mRemixTracks) {
        pTrack->SetActive(false);
    }
    mFx.Activate();
}

void RemixLogic::RebuildPatterns() {
    for (RemixTrack *pTrack : mRemixTracks) {
        pTrack->RebuildGems();
    }
    SlotGrid *pGrid = mSong->GetSlotGrid();
    for (int i = 0; i < pGrid->GetNumSections(); ++i) {
        const int nPattern = pGrid->GetPattern(i);
        if (nPattern != kNoPattern) {
            mFx.CopySet(nPattern, i);
        }
    }
}

void RemixLogic::OnSectionEnd() {
    const int nTick = TheSongScheduler.mTick;
    const int nPlayTick = mPlayMap->MapTick(nTick);
    const int nPlayBar = nPlayTick / mTicksPerBar;
    const int nSection = mSections->SectionAt(nPlayBar);
    int nPattern = mSong->GetSlotGrid()->GetPattern(nSection);
    if (nPattern == kNoPattern) {
        nPattern = nSection;
    }
    mSectionIndex =
        static_cast<int>(std::find(mPlayableSections.begin(), mPlayableSections.end(), nPattern) -
                         mPlayableSections.begin());
    TheGfxManager.HighlightSection(g_bDuelAuthoring != 0 ? kFirstSection : mSectionIndex);
    const int nEndBar = mSections->SectionEnd(nSection);
    TheSongScheduler.PostIn(mSectionEndCmd.Get(), nEndBar * mTicksPerBar - nPlayTick, false);
    if (mSections->IsPastEnd(nEndBar)) {
        AddSectionCheckpoints(nTick / mTicksPerBar + nEndBar - nPlayBar);
    }
    mFx.ApplySet(nSection);
    for (VoxTrack *pTrack : mVoxTracks) {
        pTrack->SetSection(nSection);
    }
}

void RemixLogic::ApplySectionEffects() {
    mFx.ApplySet(mPlayableSections[mSectionIndex]);
}

void RemixLogic::AddSectionCheckpoints(int nFirstBar) {
    SlotGrid *pGrid = mSong->GetSlotGrid();
    for (int i = 0; i < mSections->NumSections(); ++i) {
        int nPattern = pGrid->GetPattern(i);
        if (nPattern == kNoPattern) {
            nPattern = i;
        }
        const int nIndex = static_cast<int>(
            std::find(mPlayableSections.begin(), mPlayableSections.end(), nPattern) -
            mPlayableSections.begin());
        const char *pszLabel = mSong->GetSectionLabel(nIndex);
        const int nStartBar = mSections->SectionStart(i);
        TheSongScheduler.PostAfter(
            NewMemFun4Command(&TheGfxManager,
                              &GfxManager::AddCheckpoint,
                              pszLabel,
                              (nFirstBar + nStartBar) * mTicksPerBar,
                              static_cast<float>(nStartBar) / static_cast<float>(mNumBars),
                              kSectionCheckpointScale),
            static_cast<float>(i * kCheckpointDelayStepMs + kCheckpointDelayMs),
            false);
    }
}

void RemixLogic::StartPreview(int nBar, bool bTransition) {
    if (mPreviewing != 0) {
        return;
    }
    mPreviewing = 1;
    const int nIntroBars = mSong->mIntroBars;
    const int nTick = TheSongScheduler.mTick;
    const int nCurrentBar = nTick / mTicksPerBar;
    if (nBar == kNoBar) {
        if (bTransition) {
            nBar = std::max(0, nCurrentBar + nIntroBars + kSectionChangeLeadBars);
        } else {
            nBar = nCurrentBar + nIntroBars;
        }
    }
    if (bTransition) {
        const int nSongBar = nBar - nIntroBars;
        const int nFadeTicks = nTick - nSongBar * mTicksPerBar;
        for (int i = 0; i < mSong->GetNumTracks() - 1; ++i) {
            TheMixer->SetVolume(i, kSilentVolume, ~nFadeTicks);
        }
        TheSongScheduler.PostAt(mPlayIntroCmd.Get(), nSongBar * mTicksPerBar, false);
    }
    const int nStartTick = nBar * mTicksPerBar;

    const int nCommunity = TheGameDb->mCommunity;
    if (nCommunity == GameDb::kCommunitySolo) {
        TheGfxManager.ShowRemix(kFirstPlayer, false);
        const int nFreestyleTick = TheSongScheduler.mTick + kFreestyleDelayTicks;
        TheSongScheduler.PostAt(
            NewMemFun2Command(
                mTrackSelector, &GameTrackSelector::EnterFreestyle, kFirstPlayer, false),
            nFreestyleTick,
            false);
        TheSongScheduler.PostAt(NewMemFun2Command(&TheGfxManager,
                                                  &GfxManager::SetLyricText,
                                                  TheLocale.Localize(kFreestyleStartKey, true),
                                                  true),
                                nFreestyleTick,
                                false);
    } else {
        if (nCommunity == GameDb::kCommunityOnline) {
            if (TheNetTransport->IsHost() && bTransition) {
                PreviewPacket packet;
                packet.mSection = nBar; // Yes, the binary sends the bar the preview starts at.
                packet.mPreviewing = 1;
                TheNetTransport->Send(packet);
            }
            TheSongScheduler.PostAfter(
                NewMemFun1Command(&TheGfxManager, &GfxManager::BlendToArenaPath, true),
                kArenaBlendDelayMs,
                false);
        }
        TheGfxManager.SetLetterbox(true);
        TheGfxManager.ShowTrackLabels(false);
        for (int i = 0; i < TheGameDb->GetNumPlayers(); ++i) {
            TheGfxManager.SetPlayerShown(i, false);
            TheGfxManager.SetCatcherShown(i, false);
            TheGfxManager.ShowRemix(i, false);
        }
    }

    TheGfxManager.ClearCheckpoints();
    AddSectionCheckpoints(nBar);
    if (mReadOnly == 0) {
        TheGfxManager.ShowSongPos(true);
    }
    TheGfxManager.ShowSections(false);
    RebuildPatterns();
    mSavedMuteRemote = mMuteRemote;
    SetMuteRemote(0);
    UpdateHuds();
    if (bTransition) {
        StartTransition();
        const int nIntro = mSong->mIntroBars;
        SetLoop(-nIntro, mNumBars, nBar - nIntro, nBar);
        TheSongScheduler.PostAt(
            NewMemFun4Command(this, &RemixLogic::SetLoop, kFirstBar, mNumBars, nBar, nBar),
            nBar * mTicksPerBar,
            false);
    } else {
        SetLoop(kFirstBar, mNumBars, nBar, nBar);
    }
    TheSongScheduler.PostAt(mSectionEndCmd.Get(), nStartTick, false);
    TheGfxManager.HighlightSection(kFirstSection);
}

void RemixLogic::PlayIntro() {
    mFx.Reset();
    for (unsigned int i = 0; i < mRemixTracks.size(); ++i) {
        mRemixTracks[i]->SetActive(true);
        TheMixer->SetVolume(static_cast<int>(i), kFullTrackVolume, kMixerRampTicks);
    }
    for (int i = 0; i < mSong->GetNumIntroMuses(); ++i) {
        mSong->GetIntroMuse(i)->Play(&TheSongScheduler);
    }
    TheSongScheduler.PostIn(mEndIntroCmd.Get(), mSong->mIntroBars * mTicksPerBar - 1, false);
}

void RemixLogic::EndIntro() {
    mFx.Activate();
    for (unsigned int i = 0; i < mRemixTracks.size(); ++i) {
        mRemixTracks[i]->SetActive(false);
        TheMixer->SetVolume(
            static_cast<int>(i), static_cast<unsigned char>(mTrackVolumes[i]), kMixerRampTicks);
    }
}

void RemixLogic::StopPreview(int nBar) {
    if (mPreviewing == 0) {
        return;
    }
    mPreviewing = 0;
    SetMuteRemote(mSavedMuteRemote);
    StartTransition();
    if (nBar == kNoBar) {
        QueueSectionChange(mSectionIndex);
    } else {
        ChangeSection(mSectionIndex, nBar);
    }

    const int nCommunity = TheGameDb->mCommunity;
    if (nCommunity == GameDb::kCommunitySolo) {
        TheGfxManager.ShowRemix(kFirstPlayer, true);
        TheGfxManager.SetLyricText(kEmptyText, true);
        mTrackSelector->MovePlayer(kFirstPlayer, TheGfxManager.GetViewedTrack(kFirstPlayer));
    } else {
        if (nCommunity == GameDb::kCommunityOnline) {
            if (TheNetTransport->IsHost()) {
                PreviewPacket packet;
                packet.mSection = mLoopBar; // Yes, the binary sends the bar of the loop.
                packet.mPreviewing = 0;
                TheNetTransport->Send(packet);
            }
            TheSongScheduler.PostAfter(
                NewMemFun1Command(&TheGfxManager, &GfxManager::BlendToArenaPath, false),
                kArenaBlendDelayMs,
                false);
        }
        TheGfxManager.SetLetterbox(false);
        TheGfxManager.ShowTrackLabels(true);
        for (int i = 0; i < TheGameDb->GetNumPlayers(); ++i) {
            if (!mPlayers[i]->IsAborted()) {
                TheSongScheduler.PostAfter(
                    NewMemFun2Command(&TheGfxManager, &GfxManager::SetPlayerShown, i, true),
                    kShowPlayersDelayMs,
                    false);
                TheSongScheduler.PostAfter(
                    NewMemFun2Command(&TheGfxManager, &GfxManager::SetCatcherShown, i, true),
                    kShowPlayersDelayMs,
                    false);
            }
            TheGfxManager.ShowRemix(i, true);
        }
    }
    TheGfxManager.ShowSongPos(false);
    TheGfxManager.ShowSections(true);
    TheGfxManager.ClearCheckpoints();
    TheSongScheduler.Cancel(mSectionEndCmd.Get());
}

void RemixLogic::SetLoop(int nStartBar, int nEndBar, int nPlayBar, int nLoopBar) {
    mPlayMap->AddLoop(nStartBar, nEndBar - nStartBar, nPlayBar);
    mSong->GetBankTrack()->Restart();
    mFreestyleTrack->Refresh();
    mLoopBar = nLoopBar;
}

float RemixLogic::GetTempo() {
    return mBaseTempo * TheSongScheduler.GetSpeed();
}

void RemixLogic::AdjustTempo(float fDelta) {
    const float fTempo = GetTempo() + fDelta;
    const float fSpeed = fTempo / mBaseTempo;
    if (fSpeed < kMinSpeed || kMaxSpeed < fSpeed) {
        return;
    }
    SetSongSpeed(fSpeed);
    mHuds[kFirstPad]->SetTempo(fTempo, std::fabs(fTempo - mBaseTempo) < kTempoMatch);
}

void RemixLogic::SetLooping(int nPlayer, bool bLooping) {
    Player *pPlayer = mPlayers[nPlayer];
    pPlayer->SetRepeat(bLooping);
    PitchTrack *pTrack = FindPitchTrack(pPlayer->GetTrack()->mIndex);
    if (pTrack != nullptr) {
        pTrack->SetRepeat(bLooping);
    }
    if (static_cast<unsigned int>(nPlayer) < mHuds.size()) {
        UpdateHud(nPlayer);
    }
}

void RemixLogic::MuteVocals(int nTrack, bool bMute) {
    const int nSection = mPlayableSections[mSectionIndex];
    FindVoxTrack(nTrack)->SetSectionMuted(nSection, bMute);
    UpdateHuds();
}

void RemixLogic::SetMuteRemote(int nMuteRemote) {
    for (RemixTrack *pTrack : mRemixTracks) {
        pTrack->SetMuteRemote(nMuteRemote != 0);
    }
    mMuteRemote = nMuteRemote;
    UpdateHud(kFirstPlayer);
}

void RemixLogic::SetFilter(int nTrack, bool bOn) {
    mFx.SetFilter(mFx.GetCurrentSet(), nTrack, bOn);
    UpdateHuds();
}

void RemixLogic::SetStutter(int nTrack, bool bOn) {
    mFx.SetStutter(mFx.GetCurrentSet(), nTrack, bOn);
    UpdateHuds();
}

void RemixLogic::SetChorus(int nTrack, bool bOn) {
    mFx.SetChorus(mFx.GetCurrentSet(), nTrack, bOn);
    UpdateHuds();
}

void RemixLogic::LoadRemix() {
    char *pPacked = TheGameDb->GetRemixBuffer();
    if (pPacked == nullptr) {
        return;
    }
    RemixInfo *pInfo = TheGameDb->GetRemixInfo();
    const int nSize = GzipSizeDecompressed(pPacked, pInfo->mDataSize);
    (void)strcmp(pInfo->mSong, TheGameDb->mSong.c_str()); // Yes, the binary discards the result.
    auto *pBuffer = static_cast<char *>(PoolMemAlloc(nSize, kRemixBufferTag, 0));
    (void)GzipDecompressRamToRam(pPacked, nSize, pBuffer);
    BufStream stream(pBuffer, nSize, true);
    Load(stream);
    PoolMemFree(pBuffer);
    mRemixLoaded = 1;
}

void RemixLogic::SaveRemix() {
    MemStream stream(true);
    Save(stream);
    const int nSize = stream.mTell;
    void *pPacked =
        PoolMemAlloc(nSize + nSize / kPackedSlackDivisor + kPackedHeaderSize, kRemixBufferTag, 0);
    const int nPackedSize = GzipCompressRamToRam(stream.mBuffer.data(), nSize, pPacked);
    TheGameDb->SetRemixBuffer(nPackedSize);
    memcpy(TheGameDb->GetRemixBuffer(), pPacked, nPackedSize);
    PoolMemFree(pPacked);
    RebuildPatterns();

    DifficultyRating rating(GetTempo(), mNumBars, mTicksPerBar);
    for (PitchTrack *pTrack : mPitchTracks) {
        (void)rating.RatePitchTrack(&pTrack->mGems); // Yes, the binary discards the rating.
    }
    for (int i = 0; i < mSong->GetNumTracks(); ++i) {
        if (mSong->GetTrackType(i) == Song::kTrackTypeVox) {
            (void)rating.RateCatchTrack(mSong->GetVoxData(i), kCatchSlot);
        }
    }
    RemixInfo info = *TheGameDb->GetRemixInfo();
    info.mKey = RemixInfo::MakeKey();
    info.mDataSize = nPackedSize;
    info.mSkillLevel = rating.GetSkillTier();
    info.mPlayable = rating.IsBelowEmptyThreshold();
    info.mTempo = GetTempo();
    TheGameDb->SetRemix(&info);
}

void RemixLogic::Save(BinStream &stream) {
    const unsigned char nVersion = kSaveVersion;
    stream.Write(&nVersion, sizeof(nVersion));
    const float fSpeed = TheSongScheduler.GetSpeed();
    stream.WriteEndian(&fSpeed, sizeof(fSpeed));
    mFx.Save(stream);
    for (RemixTrack *pTrack : mRemixTracks) {
        pTrack->Save(stream);
    }
}

void RemixLogic::Load(BinStream &stream) {
    unsigned char nVersion = 0;
    stream.Read(&nVersion, sizeof(nVersion)); // Yes, the binary does not check the version.
    float fSpeed = -1.0f;
    stream.ReadEndian(&fSpeed, sizeof(fSpeed));
    SetSongSpeed(fSpeed);
    mFx.Load(stream);
    for (RemixTrack *pTrack : mRemixTracks) {
        pTrack->Load(stream);
    }
}

void RemixLogic::ClearGems() {
    const int nBar = TheSongScheduler.mTick / mTicksPerBar;
    for (PitchTrack *pTrack : mPitchTracks) {
        pTrack->EraseSection(nBar);
    }
}

void RemixLogic::EnableMenu(bool bEnable) {
    mHuds[kFirstPad]->SetFocused(bEnable);
}

void RemixLogic::OpenTempo() {
    mHuds[kFirstPad]->OpenTempo();
}

void RemixLogic::CloseTempo() {
    mHuds[kFirstPad]->CloseTempo();
}

void RemixLogic::RaiseTempo() {
    AdjustTempo(kTempoStep);
}

void RemixLogic::LowerTempo() {
    AdjustTempo(-kTempoStep);
}

bool RemixLogic::IsTickInCurrentSection(int nTick) {
    if (mPreviewing != 0) {
        return false;
    }
    const int nPlayTick = mPlayMap->MapTick(nTick);
    if (nPlayTick < 0) {
        return false;
    }
    return mSections->SectionAt(nPlayTick / mTicksPerBar) == mPlayableSections[mSectionIndex];
}

int RemixLogic::OnSectionChange(SectionChangePacket *pPacket) {
    StartTransition();
    ChangeSection(pPacket->mSection, pPacket->mTick);
    TheGfxManager.ShowMessage(mSong->GetSectionLabel(mSectionIndex),
                              nullptr,
                              kNoPlayer,
                              kSectionMessageMs,
                              kSectionMessageScale,
                              kMessageOffset,
                              kMessageOffset);
    return 0;
}

int RemixLogic::OnFX(FXPacket *pPacket) {
    switch (pPacket->mType) {
    case kEffectStutter:
        mFx.SetStutter(pPacket->mSection, pPacket->mTrack, pPacket->mOn);
        break;
    case kEffectFilter:
        mFx.SetFilter(pPacket->mSection, pPacket->mTrack, pPacket->mOn);
        break;
    case kEffectChorus:
        mFx.SetChorus(pPacket->mSection, pPacket->mTrack, pPacket->mOn);
        break;
    default:
        DebugWarn("illegal effect");
        break;
    }
    UpdateHuds();
    return 0;
}

int RemixLogic::OnMuteVocals(MuteVocalsPacket *pPacket) {
    const int nSection = mPlayableSections[pPacket->mSection];
    FindVoxTrack(pPacket->mTrack)->SetSectionMuted(nSection, pPacket->mMute);
    UpdateHuds();
    return 0;
}

int RemixLogic::OnPreview(PreviewPacket *pPacket) {
    if (pPacket->mPreviewing) {
        StartPreview(pPacket->mSection, true);
    } else {
        StopPreview(pPacket->mSection);
    }
    return 0;
}

int RemixLogic::OnEditGem(EditGemPacket *pPacket) {
    if (!IsTickInCurrentSection(pPacket->mTick)) {
        return 0;
    }
    const int nPlayer = GetPlayerFromNetOrder(pPacket->mSender);
    (void)FindPitchTrack(pPacket->mTrack)
        ->ToggleGem(static_cast<signed char>(pPacket->mSlot),
                    pPacket->mTick,
                    pPacket->mRepeat,
                    static_cast<signed char>(nPlayer));
    mModified = 1;
    return 0;
}

int RemixLogic::OnEraseBar(EraseBarPacket *pPacket) {
    FindPitchTrack(pPacket->mTrack)->EraseBar(pPacket->mBar);
    mModified = 1;
    return 0;
}

int RemixLogic::OnEraseSection(EraseSectionPacket *pPacket) {
    FindPitchTrack(pPacket->mTrack)->EraseSection(pPacket->mBar);
    mModified = 1;
    return 0;
}

int RemixLogic::OnRemixerUpdate(RemixerUpdatePacket *pPacket) {
    const int nPlayer = GetPlayerFromNetOrder(pPacket->mSender);
    const unsigned int nVersion = pPacket->mVersion;
    if (nVersion < mUpdateVersions[nPlayer]) {
        return 0;
    }
    mUpdateVersions[nPlayer] = nVersion;
    const int nTrack = pPacket->mTrack;
    if (nTrack != mTrackSelector->GetPlayerTrack(nPlayer)) {
        mTrackSelector->MovePlayer(nPlayer, nTrack);
        if (mNetArbiter != nullptr) {
            mNetArbiter->UpdatePlayer(nPlayer, nTrack, static_cast<int>(nVersion));
        }
        UpdateHuds();
    }
    if (mPlayers[nPlayer]->GetRepeat() != pPacket->mRepeat) {
        SetLooping(nPlayer, pPacket->mRepeat);
    }
    return 0;
}

int RemixLogic::OnArbiter(ArbiterPacket *pPacket) {
    if (static_cast<unsigned int>(pPacket->mVersion) < mArbiterVersion) {
        return 0;
    }
    mArbiterVersion = pPacket->mVersion;
    bool bChanged = false;
    for (int i = 0; i < static_cast<int>(pPacket->mPlayers.size()); ++i) {
        const ArbiterPacket::PlayerData &data = pPacket->mPlayers[i];
        const int nPlayer = GetPlayerFromNetOrder(data.mNetOrder);
        if (mPlayers[nPlayer]->IsAborted()) {
            continue;
        }
        if (TheGameDb->IsLocalPlayer(nPlayer)) {
            mUpdateVersions[nPlayer] = LocalPlayer::sRemixerUpdateVersion;
        }
        mPlayerTracks[nPlayer] = data.mTrack;
        const int nTrack = mTrackSelector->GetPlayerTrack(nPlayer);
        if (static_cast<unsigned int>(data.mVersion) < mUpdateVersions[nPlayer] &&
            data.mTrack != nTrack) {
            continue;
        }
        const int nSlot = mTrackSelector->GetPlayerSlot(nPlayer);
        if (nTrack == data.mTrack && nSlot == data.mSlot) {
            continue;
        }
        mTrackSelector->SwapPlayer(nPlayer, data.mTrack, data.mSlot);
        bChanged = true;
    }
    mTrackSelector->Resync();
    if (bChanged) {
        UpdateHuds();
    }
    return 0;
}

int RemixLogic::OnPlayerAborted(PlayerAbortedMsg *pMsg) {
    const int nPlayer = GetPlayerFromNetOrder(pMsg->mNetOrder);
    Player *pPlayer = mPlayers[nPlayer];
    if (pPlayer->IsAborted()) {
        return 0;
    }
    pPlayer->Abort();
    mHuds[kFirstPad]->RemovePeer(nPlayer);
    mTrackSelector->RemovePlayer(nPlayer);
    if (mNetArbiter != nullptr) {
        mNetArbiter->RemovePlayer(nPlayer);
    }
    const char *pszFormat = TheLocale.Localize(kClientAbortedKey, true);
    TheGfxManager.ShowMessage(FormatString(pszFormat, TheGameDb->GetPlayerName(nPlayer)),
                              nullptr,
                              kNoPlayer,
                              kNoticeMessageMs,
                              kAbortMessageScale,
                              kMessageOffset,
                              kMessageOffset);
    return 0;
}

int RemixLogic::OnGameEnded(GameEndedMsg *pMsg) {
    const char *pszLine = nullptr;
    const char *pszSecondLine = nullptr;
    switch (pMsg->mResult) {
    case GameEndedMsg::kResultHostAborted:
        pszLine = TheLocale.Localize(kHostAborted1Key, true);
        pszSecondLine = TheLocale.Localize(kHostAborted2Key, true);
        break;
    case GameEndedMsg::kResultConnectionLost:
        pszLine = TheLocale.Localize(kConnectionLost1Key, true);
        pszSecondLine = TheLocale.Localize(kConnectionLost2Key, true);
        break;
    case GameEndedMsg::kResultBooted:
        pszLine = TheLocale.Localize(kBooted1Key, true);
        pszSecondLine = TheLocale.Localize(kBooted2Key, true);
        break;
    default:
        DebugWarn("illegal remix result");
        break;
    }
    TheGfxManager.ShowMessage(pszLine,
                              pszSecondLine,
                              kNoPlayer,
                              kNoticeMessageMs,
                              kGameEndedMessageScale,
                              kMessageOffset,
                              kMessageOffset);
    TheSongScheduler.PostAfter(
        NewMemFunCommand(this, &RemixLogic::EndRemix), kGameEndedDelayMs, false);
    return 0;
}

void RemixLogic::OnHudEvent(int nPlayer, int nEvent, void *pData) {
    static_cast<RemixLogic *>(pData)->HandleHudEvent(nPlayer, nEvent);
}

void RemixLogic::HandleHudEvent(int nPlayer, int nEvent) {
    const int nTrack = mPlayers[nPlayer]->GetTrack()->mIndex;
    if (mTutorial != nullptr && mTutorial->mHudEnabled == 0) {
        mTutorial->PlaySound();
        return;
    }
    switch (nEvent) {
    case RemixHUD::kEventMuteOn:
    case RemixHUD::kEventMuteOff: {
        if (mTutorial != nullptr) {
            mTutorial->PlaySound();
            return;
        }
        const bool bMute = nEvent == RemixHUD::kEventMuteOn;
        MuteVocals(nTrack, bMute);
        if (TheGameDb->mCommunity == GameDb::kCommunityOnline) {
            MuteVocalsPacket packet;
            packet.mSection = mSectionIndex;
            packet.mTrack = nTrack;
            packet.mMute = bMute;
            TheNetTransport->Send(packet);
        }
        mModified = 1;
        break;
    }
    case RemixHUD::kEventLoopOn:
    case RemixHUD::kEventLoopOff: {
        if (mTutorial != nullptr && mTutorial->mLoopingAllowed == 0) {
            mTutorial->PlaySound();
            return;
        }
        const bool bLooping = nEvent == RemixHUD::kEventLoopOn;
        SetLooping(nPlayer, bLooping);
        if (mTutorial != nullptr) {
            mTutorial->NotifyLoop(bLooping);
        }
        break;
    }
    case RemixHUD::kEventChorusOn:
    case RemixHUD::kEventChorusOff: {
        if (mTutorial != nullptr) {
            mTutorial->PlaySound();
            return;
        }
        const bool bOn = nEvent == RemixHUD::kEventChorusOn;
        SetFilter(nTrack, bOn); // Yes, the chorus panel switches the filter.
        if (TheGameDb->mCommunity == GameDb::kCommunityOnline) {
            FXPacket packet;
            packet.mSection = mSectionIndex;
            packet.mTrack = nTrack;
            packet.mType = kEffectFilter;
            packet.mOn = bOn;
            TheNetTransport->Send(packet);
        }
        mModified = 1;
        break;
    }
    case RemixHUD::kEventStutterOn:
    case RemixHUD::kEventStutterOff: {
        if (mTutorial != nullptr) {
            mTutorial->PlaySound();
            return;
        }
        const bool bOn = nEvent == RemixHUD::kEventStutterOn;
        SetStutter(nTrack, bOn);
        if (TheGameDb->mCommunity == GameDb::kCommunityOnline) {
            FXPacket packet;
            packet.mSection = mSectionIndex;
            packet.mTrack = nTrack;
            packet.mType = kEffectStutter;
            packet.mOn = bOn;
            TheNetTransport->Send(packet);
        }
        mModified = 1;
        break;
    }
    case RemixHUD::kEventEffectOn:
    case RemixHUD::kEventEffectOff: {
        if (mTutorial != nullptr) {
            mTutorial->PlaySound();
            return;
        }
        const bool bOn = nEvent == RemixHUD::kEventEffectOn;
        SetChorus(nTrack, bOn);
        if (TheGameDb->mCommunity == GameDb::kCommunityOnline) {
            FXPacket packet;
            packet.mSection = mSectionIndex;
            packet.mTrack = nTrack;
            packet.mType = kEffectChorus;
            packet.mOn = bOn;
            TheNetTransport->Send(packet);
        }
        mModified = 1;
        break;
    }
    case RemixHUD::kEventSoloOn:
    case RemixHUD::kEventSoloOff:
        if (mTutorial != nullptr) {
            mTutorial->PlaySound();
            return;
        }
        SetMuteRemote(nEvent == RemixHUD::kEventSoloOn);
        break;
    case RemixHUD::kEventTempoUp:
    case RemixHUD::kEventTempoDown:
        if (mTutorial != nullptr && mTutorial->mTempoChangeAllowed == 0) {
            return;
        }
        AdjustTempo(nEvent == RemixHUD::kEventTempoUp ? kTempoStep : -kTempoStep);
        mModified = 1;
        break;
    case RemixHUD::kEventTempoOpened:
        break;
    case RemixHUD::kEventBootFirst:
    case RemixHUD::kEventBootFirst + 1:
    case RemixHUD::kEventBootFirst + 2:
    case RemixHUD::kEventBootFirst + 3:
        TheNetLaunchpad->BootPlayer(
            TheGameDb->GetPlayerNetOrder(nEvent - RemixHUD::kEventBootFirst));
        break;
    default:
        DebugWarn("illegal menu option");
        break;
    }
}

int RemixLogic::OnKeyboardKey([[maybe_unused]] KeyboardKeyMsg *pMsg) {
    return 0;
}

bool RemixLogic::DispatchPriv(Message *pMsg) {
    const int nType = pMsg->Type();
    if (nType == g_nKeyboardKeyMsgType) {
        return OnKeyboardKey(static_cast<KeyboardKeyMsg *>(pMsg)) != 0;
    }
    if (nType == g_nSectionChangePacketType) {
        return OnSectionChange(static_cast<SectionChangePacket *>(pMsg)) != 0;
    }
    if (nType == g_nFXPacketType) {
        return OnFX(static_cast<FXPacket *>(pMsg)) != 0;
    }
    if (nType == g_nMuteVocalsPacketType) {
        return OnMuteVocals(static_cast<MuteVocalsPacket *>(pMsg)) != 0;
    }
    if (nType == g_nPreviewPacketType) {
        return OnPreview(static_cast<PreviewPacket *>(pMsg)) != 0;
    }
    if (nType == g_nEditGemPacketType) {
        return OnEditGem(static_cast<EditGemPacket *>(pMsg)) != 0;
    }
    if (nType == g_nEraseBarPacketType) {
        return OnEraseBar(static_cast<EraseBarPacket *>(pMsg)) != 0;
    }
    if (nType == g_nEraseSectionPacketType) {
        return OnEraseSection(static_cast<EraseSectionPacket *>(pMsg)) != 0;
    }
    if (nType == g_nRemixerUpdatePacketType) {
        return OnRemixerUpdate(static_cast<RemixerUpdatePacket *>(pMsg)) != 0;
    }
    if (nType == g_nArbiterPacketType) {
        return OnArbiter(static_cast<ArbiterPacket *>(pMsg)) != 0;
    }
    if (nType == g_nPlayerAbortedMsgType) {
        return OnPlayerAborted(static_cast<PlayerAbortedMsg *>(pMsg)) != 0;
    }
    if (nType == g_nGameEndedMsgType) {
        return OnGameEnded(static_cast<GameEndedMsg *>(pMsg)) != 0;
    }
    return WorldLogic::DispatchPriv(pMsg);
}
