#include "game/duellogic.h"

#include <algorithm>
#include <utility>
#include <vector>

#include "game/forcefeedbackmgr.h"
#include "game/gameconfig.h"
#include "game/gamedb.h"
#include "game/localplayer.h"
#include "game/mixer.h"
#include "game/netgamescore.h"
#include "game/remoteplayer.h"
#include "game/stats.h"
#include "game/track.h"
#include "game/triggermgr.h"
#include "gfx/gfxmanager.h"
#include "math/rand.h"
#include "netflow/nettransport.h"
#include "os/debug.h"
#include "os/joypad.h"
#include "os/locale.h"
#include "os/memfuncommand.h"
#include "os/scheduler.h"
#include "os/string.h"
#include "os/system.h"
#include "synth/fxmidi.h"
#include "synth/synth.h"

namespace {

constexpr int kSideCount = 2;
constexpr int kFirstSide = 0;
constexpr int kSecondSide = 1;

constexpr int kPhraseBars = 2;

constexpr int kWinningScore = 9;

// The score one point short of a win, at which the duel plays its last coaching sounds.
constexpr int kMatchPointScore = kWinningScore - 1;

// The phases that open the duel with a coaching sound at the easiest difficulty.
constexpr int kCoachedPhaseCount = 4;

constexpr int kEasiestDifficulty = 1;

constexpr int kNoPlayer = -1;
constexpr int kNoGem = -1;
constexpr int kMinMissSoundGems = 4;
constexpr int kNoPad = -1;
constexpr int kNoSeat = -1;

constexpr int kTicksPerBeat = 480;
constexpr int kMinFadeTicks = 960;
constexpr int kEndTickMargin = 480;
constexpr int kMinGemLeadTicks = 960;
constexpr int kOneLetterDelayTicks = 720;
constexpr int kPraiseWindowTicks = 60;
constexpr int kBlankScreenTick = -3840;
constexpr int kConnectionLostDelayTicks = 3500;

constexpr float kOutroNow = -1.0f;
constexpr float kAnnounceDelay = 1500.0f;
constexpr float kHostAbortedDelay = 3500.0f;

constexpr float kCheckpointOffset = 0.0f;
constexpr float kCheckpointScale = 1.0f;

constexpr float kCueMessageDuration = 1600.0f;
constexpr float kCueMessageScale = 0.7f;
constexpr float kCueMessageOffsetX = 75.0f;
constexpr float kCueMessageOffsetY = 110.0f;

constexpr float kAbortMessageDuration = 3200.0f;
constexpr float kAbortMessageScale = 0.9f;
constexpr float kAbortMessageOffset = 0.0f;

constexpr double kPhraseCueLead = 0.5;
constexpr double kSoundCueLead = 0.25;
constexpr double kSoundCueDelay = 1.0;

constexpr int kStatsPlayerCount = 4;

constexpr unsigned kMidiControlChange = 0xb0;
constexpr unsigned char kMidiVolumeController = 17;
constexpr int kTrackChannelCount = 15;

constexpr unsigned char kFullVolume = 127;
constexpr unsigned char kMutedVolume = 50;

constexpr int kVolumeRampTicks = 0;

// The pending points result shows the side that scored rather than what happened to the points.
constexpr GfxManager::PendingPointsResult kPointForFirstSide = GfxManager::kPendingPointsCaptured;
constexpr GfxManager::PendingPointsResult kPointForSecondSide = GfxManager::kPendingPointsLost;

constexpr int kGfxPlayer = 0;
constexpr int kAllPlayers = -1;
constexpr int kGemStyle = 0;
constexpr int kGemFlags = 0;
constexpr int kPhraseStyle = 0;
constexpr bool kPhraseSlide = true;

constexpr char kPitchPhraseKey[] = "PITCH_PHRASE";
constexpr char kCatchPhraseKey[] = "CATCH_PHRASE";
constexpr char kClientAbortedKey[] = "CLIENT_ABORTED";
constexpr char kHostAborted1Key[] = "HOST_ABORTED_1";
constexpr char kHostAborted2Key[] = "HOST_ABORTED_2";
constexpr char kConnectionLost1Key[] = "CONNECTION_LOST_1";
constexpr char kConnectionLost2Key[] = "CONNECTION_LOST_2";

// NTSC-U/C: 0x003af6f0
void (*const kMissSounds[])() = {&FxMidi::PlayDuelAlmost, &FxMidi::PlayDuelAww};

// NTSC-U/C: 0x003af6f8
void (*const kCatchSounds[])() = {
    &FxMidi::PlayDuelNice, &FxMidi::PlayDuelYouGotIt, &FxMidi::PlayDuelPerfect};

constexpr int kMissSoundCount = sizeof(kMissSounds) / sizeof(kMissSounds[0]);
constexpr int kCatchSoundCount = sizeof(kCatchSounds) / sizeof(kCatchSounds[0]);

} // namespace

DuelLogic::DuelLogic(Song *pSong)
    : mPitcher(nullptr), mCatcher(nullptr), mPhase(kPhasePitch), mState(kStateNone), mPaused(0),
      mQuit(0), mEndTick(0), mCommandsStopped(0), mPitcherTrack(nullptr), mCatcherTrack(nullptr),
      mCatchMissed(0), mCatchMade(0), mSong(pSong), mTicksPerBar(pSong->mBuilder->mTicksPerBar),
      mPhraseBars(kPhraseBars), mPlayMap(pSong->GetPlayMap()), mReserved5c(0), mSectionStartBar(0),
      mNextSectionBar(pSong->GetSections()->SectionEnd(0)), mSection(0), mTrackSelector(nullptr),
      mBarCommand(NewMemFunCommand(this, &DuelLogic::OnBarTick)),
      mScrollCommand(NewMemFunCommand(this, &DuelLogic::OnScrollTick)),
      mPhraseCueCommand(NewMemFunCommand(this, &DuelLogic::OnPhraseCueTick)),
      mSoundCueCommand(NewMemFunCommand(this, &DuelLogic::OnSoundCueTick)),
      mAnnounceCommand(NewMemFunCommand(this, &DuelLogic::AnnounceWinner)), mCatchMadeTick(0),
      mLocalPlayer(nullptr), mUpdateVersions(TheGameDb->GetNumPlayers(), 0),
      mScoresReceived(TheGameDb->GetNumPlayers(), false), mCuesPlayed(0), mOneLetterPlayed(0),
      mMissThisPlayed(0) {
    for (int nPad = 0; nPad < TheGameDb->GetNumPads(); ++nPad) {
        JoypadSetMenuControl(nPad, true);
    }
    mPlayMap->AddLoop(0, mSong->mNumBars, 0);

    const float *pTickDuration = mSong->GetMsPerTick();
    mState = kStatePlaying;
    mSlopTicks = static_cast<int>(static_cast<float>(TheGameConfig->mSlopMs) / *pTickDuration);

    for (int nSide = 0; nSide < kSideCount; ++nSide) {
        mGems.push_back(new CatchTrackData(mTicksPerBar * mSong->mNumBars));
    }

    int nLocalPlayers = 0;
    for (int nPlayer = 0; nPlayer < TheGameDb->GetNumPlayers(); ++nPlayer) {
        Player *pPlayer;
        if (TheGameDb->IsLocalPlayer(nPlayer)) {
            pPlayer = new LocalPlayer(nPlayer, mTicksPerBar, nLocalPlayers++);
        } else {
            pPlayer = new RemotePlayer(nPlayer, mTicksPerBar);
        }
        mPlayers.push_back(pPlayer);
        TheGfxManager.SetStreakMultiplier(pPlayer->GetIndex(), 1, 1, 0);
    }

    if (TheGameDb->mCommunity == GameDb::kCommunityOnline &&
        !(TheGameDb->GetPlayerNetOrder(mPlayers[kFirstSide]->GetIndex()) <
          TheGameDb->GetPlayerNetOrder(mPlayers[kSecondSide]->GetIndex()))) {
        mInputPlayers.push_back(kSecondSide);
        mInputPlayers.push_back(kFirstSide);
        std::swap(mPlayers[kFirstSide], mPlayers[kSecondSide]);
    } else {
        mInputPlayers.push_back(kFirstSide);
        mInputPlayers.push_back(kSecondSide);
    }

    BuildBarData();

    const int nFirstTrack = mBarData[0].mTrack;
    for (int nSide = 0; nSide < kSideCount; ++nSide) {
        const int nOtherSide = kSecondSide - nSide;
        DuelTrack *pTrack = new DuelTrack(this,
                                          mPlayers[nSide],
                                          mGems[nSide],
                                          mGems[nOtherSide],
                                          mSong->GetTrackRiffData(nFirstTrack),
                                          mSong->GetDuelPatterns(),
                                          mSong->GetMsPerTick(),
                                          mPlayMap,
                                          mSong->GetSections(),
                                          GetPlayerSide(nSide),
                                          GetPlayerSide(nOtherSide),
                                          mSong->mIntroBars,
                                          mSong->mNumBars,
                                          mTicksPerBar,
                                          mPhraseBars,
                                          mSong->mDifficulty == kEasiestDifficulty);
        pTrack->Start();
        mTracks.push_back(pTrack);
    }

    TheMixer->Init(kSideCount, Mixer::kInstrumentNone);

    for (int nTrack = 0; nTrack < mSong->GetNumTracks(); ++nTrack) {
        if (mSong->GetTrackType(nTrack) == Song::kTrackTypePitch &&
            mSong->GetTrackRiffData(nTrack)->IsEnabled()) {
            PitchTrack *pPitchTrack = new PitchTrack(nTrack,
                                                     mSong->mIntroBars,
                                                     mSong->mNumBars,
                                                     mSong->mBuilder->mTicksPerBar,
                                                     mSong->GetTrackRiffData(nTrack),
                                                     mSong->GetTrackPitchData(nTrack),
                                                     mPlayMap,
                                                     mSong->GetSections(),
                                                     mSong->GetSlotGrid(),
                                                     false);
            pPitchTrack->SelectPattern(0, 0, mSong->mNumBars);
            pPitchTrack->RebuildGems();
            pPitchTrack->Start();
            mPitchTracks.push_back(pPitchTrack);
        }
    }

    for (int nChannel = 0; nChannel < kTrackChannelCount; ++nChannel) {
        TheSynth->SendMessage(static_cast<unsigned char>(kMidiControlChange | nChannel),
                              kMidiVolumeController,
                              GetTrackVolume(nChannel, true),
                              0);
    }

    mSong->GetBankTrack()->Start();

    const int nStartTick = mTicksPerBar - mTicksPerBar * mSong->mIntroBars;
    TheForceFeedbackMgr->Start(mSong->GetMsPerTick(), mTicksPerBar);
    TheForceFeedbackMgr->StartMetronome(kTicksPerBeat, nStartTick);
    for (int nSide = 0; nSide < kSideCount; ++nSide) {
        if (TheGameDb->IsLocalPlayer(mPlayers[nSide]->GetIndex())) {
            TheForceFeedbackMgr->SetBeatEnabled(mPlayers[nSide]->GetPadNum(), false);
        }
    }

    std::vector<Track *> tracks;
    tracks.insert(tracks.end(), mTracks.begin(), mTracks.end());
    mTrackSelector = new GameTrackSelector(tracks, nullptr, 0, 0);
    if (TheGameDb->mCommunity == GameDb::kCommunityOnline &&
        !TheGameDb->IsLocalPlayer(mPlayers[kFirstSide]->GetIndex())) {
        mTrackSelector->AddPlayer(mPlayers[kSecondSide], kSecondSide);
        mTrackSelector->AddPlayer(mPlayers[kFirstSide], kFirstSide);
    } else {
        mTrackSelector->AddPlayer(mPlayers[kFirstSide], kFirstSide);
        mTrackSelector->AddPlayer(mPlayers[kSecondSide], kSecondSide);
    }

    if (TheGameDb->mCommunity == GameDb::kCommunityOnline) {
        for (int nPlayer = 0; nPlayer < TheGameDb->GetNumPlayers(); ++nPlayer) {
            const int nIndex = mPlayers[nPlayer]->GetIndex();
            if (TheGameDb->IsLocalPlayer(nIndex)) {
                mLocalPlayer = mPlayers[nPlayer];
            } else {
                mRemotePlayer = mPlayers[nPlayer];
            }
            const unsigned int nSeat =
                static_cast<unsigned int>(TheGameDb->GetPlayerNetOrder(nIndex));
            if (nSeat >= mSeatPlayers.size()) {
                mSeatPlayers.resize(nSeat + 1, kNoSeat);
            }
            mSeatPlayers[nSeat] = nPlayer;
        }
    }
}

DuelLogic::~DuelLogic() {
    TheSongScheduler.Cancel(mAnnounceCommand.Get());
    StopCommands();
    delete mTrackSelector;
    for (unsigned int i = 0; i < mPlayers.size(); ++i) {
        delete mPlayers[i];
    }
    for (unsigned int i = 0; i < mTracks.size(); ++i) {
        delete mTracks[i];
    }
    for (unsigned int i = 0; i < mPitchTracks.size(); ++i) {
        delete mPitchTracks[i];
    }
    for (unsigned int i = 0; i < mGems.size(); ++i) {
        delete mGems[i];
    }
    mPlayMap->ClearLoops();
}

void DuelLogic::StopCommands() {
    if (mCommandsStopped) {
        return;
    }
    mCommandsStopped = 1;
    TheSongScheduler.Cancel(mBarCommand.Get());
    TheSongScheduler.Cancel(mScrollCommand.Get());
    TheSongScheduler.Cancel(mPhraseCueCommand.Get());
    TheSongScheduler.Cancel(mSoundCueCommand.Get());
}

void DuelLogic::BuildBarData() {
    const int nTrackCount = static_cast<int>(mSong->mDuelTrackOrder.size());
    const SectionBoundaries *pSections = mSong->GetSections();
    for (int nBar = 0; nBar < mSong->mNumBars; ++nBar) {
        const int nSection = pSections->SectionAt(nBar);
        mBarData.push_back(BarData{mSong->mDuelTrackOrder[nSection % nTrackCount]});
    }
}

int DuelLogic::GetBarInstrument(int nBar) {
    // Yes, the binary checks the bar count against zero and discards the wrapped bar.
    (void)(nBar % mSong->mNumBars);
    return mSong->GetTrackInstrument(mBarData[nBar].mTrack);
}

void DuelLogic::Start() {
    for (int i = 0; i < mSong->GetNumBackMusic(); ++i) {
        mSong->GetBackMusic(i)->Start(mPlayMap);
    }
    mSong->GetLyric()->Start();

    mPitcher = mPlayers[kFirstSide];
    mCatcher = mPlayers[kSecondSide];
    mPitcherTrack = mTracks[kFirstSide];
    mCatcherTrack = mTracks[kSecondSide];
    mPitcherGems = mGems[kFirstSide];
    mCatcherGems = mGems[kSecondSide];
    mPhase = kPhasePitch;
    StartFirstPhrase();

    const SectionBoundaries *pSections = mSong->GetSections();
    TheGfxManager.AddCheckpoint(kGfxPlayer, 0.0f, kCheckpointOffset, kCheckpointScale);
    for (int nSection = 1; nSection < pSections->NumSections(); ++nSection) {
        TheGfxManager.AddCheckpoint(
            kGfxPlayer,
            static_cast<float>(pSections->SectionStart(nSection) * mTicksPerBar),
            kCheckpointOffset,
            kCheckpointScale);
    }
    TheGfxManager.AddCheckpoint(kGfxPlayer,
                                static_cast<float>(mSong->mNumBars * mTicksPerBar),
                                kCheckpointOffset,
                                kCheckpointScale);

    const int nStartTick = 0;
    TheSongScheduler.PostAt(mBarCommand.Get(), -mSlopTicks, false);
    TheSongScheduler.PostAt(mScrollCommand.Get(), -mSlopTicks, false);
    TheSongScheduler.PostAt(mPhraseCueCommand.Get(),
                            static_cast<int>(nStartTick - mTicksPerBar * kPhraseCueLead),
                            false);
    TheSongScheduler.PostAt(
        mSoundCueCommand.Get(),
        static_cast<int>(nStartTick - mTicksPerBar * kSoundCueLead + mSlopTicks + kSoundCueDelay),
        false);

    mNextPhraseBar = 0;
    mCuePhase = kPhasePitch;
    mCuePlayer = 0;
    TheSongScheduler.PostAt(
        new MemFunCommand<void (Metagame::*)(), Metagame>(&TheMetagame, &Metagame::ShowBlankScreen),
        kBlankScreenTick,
        false);

    for (int i = 0; i < mSong->GetNumIntroMuses(); ++i) {
        mSong->GetIntroMuse(i)->Play(&TheSongScheduler);
    }
    ApplyBarTrack(0);
    ScheduleControllerCheck();
}

void DuelLogic::Stop() {
    mState = kStateFinished;
}

void DuelLogic::SetPaused(bool bPaused, int nPad, int nReason) {
    if (bPaused && mState != kStatePlaying) {
        return;
    }
    if (mPaused == bPaused) {
        return;
    }
    mPaused = bPaused;

    if (bPaused) {
        TheForceFeedbackMgr->Pause();
        mStateBeforePause = mState;
        mState = kStatePaused;
        const Metagame::DialogType type =
            nReason != 0 ? Metagame::kDialogNoController : Metagame::kDialogPause;
        if (TheGameDb->mCommunity == GameDb::kCommunityOnline) {
            TheMetagame.ShowDialog(type, OnOnlinePauseDialog, this, nPad);
        } else {
            TheSongScheduler.Pause();
            AllNotesOff();
            TheMetagame.ShowDialog(type, OnPauseDialog, this, nPad);
        }
        SystemSetPadCheck(true);
    } else {
        TheForceFeedbackMgr->Resume();
        mState = mStateBeforePause;
        if (TheGameDb->mCommunity != GameDb::kCommunityOnline) {
            TheSongScheduler.Resume();
        }
        if (TheGameDb->mCommunity == GameDb::kCommunitySolo) {
            TheGfxManager.SetFreqSize(kGfxPlayer, TheGameDb->GetOptions()->mFreqSize);
        }
    }
    WorldLogic::SetPaused(bPaused, nPad, nReason);
}

void DuelLogic::OnPauseDialog(Metagame::DialogAction action, void *pUserData) {
    DuelLogic *pLogic = static_cast<DuelLogic *>(pUserData);
    if (pLogic->mState != kStatePaused) {
        return;
    }
    switch (action) {
    case Metagame::kDialogActionResume:
        pLogic->SetPaused(false, kNoPad, 0);
        break;
    case Metagame::kDialogActionQuit:
        pLogic->mQuit = 1;
        pLogic->StartEnding();
        break;
    case Metagame::kDialogActionEnd:
        pLogic->StartEnding();
        break;
    default:
        DebugWarn("illegal dialog response");
        break;
    }
    SystemSetPadCheck(false);
}

void DuelLogic::OnOnlinePauseDialog(Metagame::DialogAction action, void *pUserData) {
    DuelLogic *pLogic = static_cast<DuelLogic *>(pUserData);
    if (pLogic->mState != kStatePaused) {
        return;
    }
    switch (action) {
    case Metagame::kDialogActionResume:
        pLogic->SetPaused(false, kNoPad, 0);
        break;
    case Metagame::kDialogActionQuit:
        break;
    case Metagame::kDialogActionEnd:
        TheNetTransport->Leave();
        TheStats->PlayerAborted(0, TheSongScheduler.mTick);
        pLogic->StartEnding();
        break;
    default:
        DebugWarn("illegal dialog response");
        break;
    }
}

void DuelLogic::OnGameOverDialog(Metagame::DialogAction action, void *pUserData) {
    DuelLogic *pLogic = static_cast<DuelLogic *>(pUserData);
    switch (action) {
    case Metagame::kDialogActionQuit:
        pLogic->mQuit = 1;
        pLogic->StartEnding();
        break;
    case Metagame::kDialogActionResume:
    case Metagame::kDialogActionEnd:
        pLogic->StartEnding();
        break;
    default:
        DebugWarn("illegal choice");
        break;
    }
}

float DuelLogic::CueOffsetY(bool bSecondSide) {
    return bSecondSide ? kCueMessageOffsetY : -kCueMessageOffsetY;
}

int DuelLogic::GetPlayerSide(int nPlayer) const {
    return nPlayer;
}

bool DuelLogic::IsFinished() const {
    return mState == kStateFinished;
}

void DuelLogic::StartEnding() {
    float fOutro = 0.0f;
    if (!mQuit) {
        fOutro = TheGfxManager.StartOutro(kOutroNow);
    }
    if (mState != kStatePaused) {
        const int nOutroTicks = static_cast<int>(fOutro / *mSong->GetMsPerTick());
        const int nFadeTicks = std::max(kMinFadeTicks, nOutroTicks);
        TheMixer->FadeOut(nFadeTicks);
        mEndTick = TheSongScheduler.mTick + nFadeTicks + kEndTickMargin;
        TheSongScheduler.PostAt(
            new MemFunCommand<void (WorldLogic::*)(), DuelLogic>(this, &WorldLogic::AllNotesOff),
            mEndTick - 1,
            false);
    }
    mState = kStateEnding;
}

int DuelLogic::HasQuit() const {
    return mQuit;
}

int DuelLogic::GetTick() {
    return TheSongScheduler.GetClockTick();
}

float DuelLogic::GetTime() {
    return TheSongScheduler.GetClockTime();
}

void DuelLogic::Poll() {
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

void DuelLogic::StartFirstPhrase() {
    int nBar = mPhraseBars;
    while (IsBarSkipped(nBar)) {
        ++nBar;
    }

    for (int nSide = 0; nSide < kSideCount; ++nSide) {
        int nMode = DuelTrack::kModePitch;
        int nNextMode = DuelTrack::kModeIdle;
        if (mPlayers[nSide] != mPitcher) {
            mCuePhase = kPhaseCatch;
            mCuePlayer = nSide;
            nMode = DuelTrack::kModeIdle;
            nNextMode = DuelTrack::kModeCatch;
        }
        DuelTrack *pTrack = mTracks[nSide];
        pTrack->SetMode(nMode, 0);
        pTrack->ShowPhrase(nNextMode, nBar, nBar + mPhraseBars);
        if (nMode == DuelTrack::kModePitch) {
            TheGfxManager.ShowPhrase(pTrack->GetPlayer()->GetIndex(),
                                     pTrack->mIndex,
                                     true,
                                     static_cast<float>(mTicksPerBar),
                                     static_cast<float>(mPhraseBars * mTicksPerBar),
                                     kPhraseStyle,
                                     kPhraseSlide);
        } else if (nNextMode != DuelTrack::kModeIdle) {
            TheGfxManager.ShowPhrase(pTrack->GetPlayer()->GetIndex(),
                                     pTrack->mIndex,
                                     true,
                                     static_cast<float>(nBar * mTicksPerBar),
                                     static_cast<float>((nBar + mPhraseBars) * mTicksPerBar),
                                     kPhraseStyle,
                                     kPhraseSlide);
        }
    }
}

void DuelLogic::ShowNextPhrase(int nBar, int nPhase) {
    mNextPhraseBar = nBar + mPhraseBars;
    while (IsBarSkipped(mNextPhraseBar)) {
        ++mNextPhraseBar;
    }

    if (nPhase == kPhasePitch) {
        for (int nSide = 0; nSide < kSideCount; ++nSide) {
            TheGfxManager.ShowPendingPoints(mPlayers[nSide]->GetIndex(),
                                            mPlayers[nSide]->GetScore() + 1);
        }
    }

    for (int nSide = 0; nSide < kSideCount; ++nSide) {
        int nMode = DuelTrack::kModeIdle;
        int nNextMode = DuelTrack::kModeIdle;
        if (nBar < mSong->mNumBars) {
            if (nPhase == kPhaseCatch) {
                if (mPlayers[nSide] == mCatcher) {
                    nMode = DuelTrack::kModeCatch;
                    nNextMode = DuelTrack::kModePitch;
                    mCuePhase = kPhasePitch;
                    mCuePlayer = nSide;
                }
            } else if (nPhase == kPhasePitch) {
                if (mPlayers[nSide] == mPitcher) {
                    nMode = DuelTrack::kModePitch;
                } else {
                    mCuePhase = kPhaseCatch;
                    nMode = DuelTrack::kModeIdle;
                    nNextMode = DuelTrack::kModeCatch;
                    mCuePlayer = nSide;
                }
            }
        }

        DuelTrack *pTrack = mTracks[nSide];
        if (mPhase != kPhaseBreak) {
            pTrack->SetMode(nMode, nBar);
        }
        if (mNextPhraseBar < mSong->mNumBars) {
            pTrack->ShowPhrase(nNextMode, mNextPhraseBar, mNextPhraseBar + mPhraseBars);
        }
        if (nMode == DuelTrack::kModePitch) {
            TheGfxManager.ShowPhrase(pTrack->GetPlayer()->GetIndex(),
                                     pTrack->mIndex,
                                     true,
                                     static_cast<float>(nBar * mTicksPerBar),
                                     static_cast<float>((nBar + mPhraseBars) * mTicksPerBar),
                                     kPhraseStyle,
                                     kPhraseSlide);
        } else if (nNextMode == DuelTrack::kModeCatch && mNextPhraseBar < mSong->mNumBars) {
            TheGfxManager.ShowPhrase(
                pTrack->GetPlayer()->GetIndex(),
                pTrack->mIndex,
                true,
                static_cast<float>(mNextPhraseBar * mTicksPerBar),
                static_cast<float>((mNextPhraseBar + mPhraseBars) * mTicksPerBar),
                kPhraseStyle,
                kPhraseSlide);
        }
    }
}

void DuelLogic::OnScrollTick() {
    const int nBar = (TheSongScheduler.mTick + mSlopTicks) / mTicksPerBar;
    if (mState == kStateDecided) {
        for (unsigned int i = 0; i < mTracks.size(); ++i) {
            mTracks[i]->SetMode(DuelTrack::kModeIdle, nBar);
        }
        return;
    }

    const int nDropBars = nBar - mPhraseBars;
    if (nDropBars > 0) {
        for (auto it = mTracks.begin(); it < mTracks.end(); ++it) {
            (*it)->DropBars(nDropBars, nBar);
        }
    }
    const int nNextBar = AdvancePhrase(nBar);
    TheSongScheduler.PostAt(mScrollCommand.Get(), nNextBar * mTicksPerBar - mSlopTicks, false);
}

bool DuelLogic::IsBarSkipped(int nBar) const {
    return std::binary_search(mSong->mSkippedBars.begin(), mSong->mSkippedBars.end(), nBar);
}

int DuelLogic::AdvancePhrase(int nBar) {
    if (nBar != 0) {
        if (mPhase == kPhasePitch) {
            mPhase = kPhaseCatch;
            mCatchMissed = 0;
            mCatchMade = 0;
        } else if (mPhase == kPhaseCatch) {
            if (mCatchMade) {
                TheTriggerMgr.PhraseEndEvent(mCatcherTrack->mIndex);
            }
            std::swap(mPitcher, mCatcher);
            std::swap(mPitcherTrack, mCatcherTrack);
            std::swap(mPitcherGems, mCatcherGems);
            mPhase = kPhasePitch;
            mCatchMissed = 0;
            mCatchMade = 0;
        }
    }

    int nNextBar = nBar + mPhraseBars;
    if (IsBarSkipped(nBar)) {
        nNextBar = nBar;
        while (IsBarSkipped(nNextBar)) {
            ++nNextBar;
        }
        mPhase = kPhaseBreak;
        for (int nSide = 0; nSide < kSideCount; ++nSide) {
            mTracks[nSide]->SetMode(DuelTrack::kModeIdle, nBar);
        }
    } else if (mPhase == kPhaseBreak) {
        mPhase = kPhasePitch;
    }

    int nPhraseBar = nBar;
    while (IsBarSkipped(nPhraseBar)) {
        ++nPhraseBar;
    }
    ShowNextPhrase(nPhraseBar, mPhase == kPhaseBreak ? kPhasePitch : mPhase);
    return nNextBar;
}

float DuelLogic::GetProgress() {
    return static_cast<float>(GetTick()) / static_cast<float>(mTicksPerBar * mSong->mNumBars);
}

void DuelLogic::HandleInput([[maybe_unused]] const RotateEvent &event) {
}

void DuelLogic::HandleInput(const PlayNoteEvent &event) {
    if (mState != kStatePlaying) {
        return;
    }
    mPlayers[mInputPlayers[event.mPlayer]]->HandleInput(event);
}

void DuelLogic::HandleInput(const BtnEvent<3> &event) {
    if (mState != kStatePlaying) {
        return;
    }
    SetPaused(true, TheGameDb->GetPlayerPad(event.mPlayer), 0);
}

void DuelLogic::HandleInput([[maybe_unused]] const BtnEvent<4> &event) {
}

void DuelLogic::HandleInput([[maybe_unused]] const BtnEvent<5> &event) {
}

void DuelLogic::HandleInput([[maybe_unused]] const StickEvent<2> &event) {
}

void DuelLogic::HandleInput([[maybe_unused]] const StickEvent<6> &event) {
}

bool DuelLogic::CheckForWinner() {
    int nLeader = kNoPlayer;
    int nLeaderScore = 0;
    for (unsigned int i = 0; i < mPlayers.size(); ++i) {
        const int nScore = mPlayers[i]->GetScore();
        if (nScore >= kWinningScore) {
            nLeader = static_cast<int>(i);
            nLeaderScore = nScore;
            break;
        }
    }
    if (nLeader == kNoPlayer) {
        return false;
    }

    // Nine points do not win against eight.
    const int nOtherScore = mPlayers[kSecondSide - nLeader]->GetScore();
    if (nLeaderScore == kWinningScore && nOtherScore == kMatchPointScore) {
        return false;
    }
    DecideDuel(TheGameDb->IsLocalPlayer(mPlayers[nLeader]->GetIndex()));
    return true;
}

void DuelLogic::PlayMissSound() {
    const int nGem = mCatcherGems->FindGem(TheSongScheduler.mTick);
    const int nGemCount = mCatcherGems->GetNumGems();
    if (nGem != kNoGem && nGem < nGemCount - 1) {
        return;
    }
    if (nGemCount < kMinMissSoundGems) {
        return;
    }
    kMissSounds[RandomInt(0, kMissSoundCount)]();
}

void DuelLogic::OnCatchMissed(DuelTrack *pTrack, Player *pPlayer, int nBar) {
    if (mCatchMissed || mCatchMade) {
        return;
    }
    mCatchMissed = 1;

    Player *pCatcher = pTrack->GetPlayer();
    Player *pScorer = mPlayers[kFirstSide];
    if (pScorer == pCatcher) {
        pScorer = mPlayers[kSecondSide];
    }
    TheGfxManager.SetPendingPointsResult(
        kGfxPlayer, pCatcher->GetIndex() != 0 ? kPointForFirstSide : kPointForSecondSide);
    pScorer->AddScore(1);
    TheGfxManager.ShowPhrase(pCatcher->GetIndex(),
                             mCatcherTrack->mIndex,
                             false,
                             static_cast<float>(nBar * mTicksPerBar),
                             static_cast<float>((nBar + mPhraseBars) * mTicksPerBar),
                             kPhraseStyle,
                             kPhraseSlide);
    for (int nDimBar = 0; nDimBar < nBar + mPhraseBars; ++nDimBar) {
        TheGfxManager.DimBar(mCatcherTrack->mIndex, static_cast<float>(nDimBar * mTicksPerBar));
    }
    FxMidi::PlayDuelMiss();

    if (CheckForWinner()) {
        return;
    }
    if (TheGameDb->IsLocalPlayer(pPlayer->GetIndex())) {
        PlayMissSound();
    }
}

void DuelLogic::OnCatchMade(DuelTrack *pTrack, Player *pPlayer, bool bReact) {
    mCatchMade = 1;
    const int nTick = TheSongScheduler.mTick;
    mCatchMadeTick = nTick;

    Player *pCatcher = pTrack->GetPlayer();
    pCatcher->AddScore(1);
    TheGfxManager.SetPendingPointsResult(
        kGfxPlayer, pCatcher->GetIndex() == 0 ? kPointForFirstSide : kPointForSecondSide);
    if (CheckForWinner()) {
        return;
    }
    if (!bReact || !TheGameDb->IsLocalPlayer(pPlayer->GetIndex())) {
        return;
    }
    if (mSong->mDifficulty == kEasiestDifficulty && mCuesPlayed < kCoachedPhaseCount) {
        return;
    }
    if (nTick % (mTicksPerBar * mPhraseBars) < mTicksPerBar - kPraiseWindowTicks) {
        return;
    }
    kCatchSounds[RandomInt(0, kCatchSoundCount)]();
}

void DuelLogic::RewindCatcherTrack() {
    mCatcherTrack->Rewind();
}

void DuelLogic::OnBarTick() {
    const int nBar = (TheSongScheduler.mTick + mTicksPerBar) / mTicksPerBar;
    if (nBar == mNextSectionBar) {
        AdvanceSection(nBar);
    }
    if (mState != kStateDecided) {
        TheSongScheduler.PostIn(mBarCommand.Get(), mTicksPerBar, false);
    }
}

void DuelLogic::AdvanceSection(int nBar) {
    const SectionBoundaries *pSections = mSong->GetSections();
    ApplyBarTrack(nBar);
    mSectionStartBar = nBar;
    ++mSection;
    if (!pSections->IsPastEnd(mSection)) { // Yes, the binary tests the section as a measure.
        mNextSectionBar = pSections->SectionEnd(mSection);
    }
}

void DuelLogic::OnPhraseCueTick() {
    const int nTick = TheSongScheduler.mTick;
    if ((nTick + mTicksPerBar) / mTicksPerBar == mNextPhraseBar && mCuePhase != kPhaseBreak) {
        const bool bSecondSide = mCuePlayer != kFirstSide;
        const char *pszText =
            TheLocale.Localize(mCuePhase == kPhasePitch ? kPitchPhraseKey : kCatchPhraseKey, true);
        TheGfxManager.ShowMessage(pszText,
                                  nullptr,
                                  kAllPlayers,
                                  kCueMessageDuration,
                                  kCueMessageScale,
                                  CueOffsetY(bSecondSide),
                                  kCueMessageOffsetX);
    }
    TheSongScheduler.PostAt(mPhraseCueCommand.Get(), nTick + mTicksPerBar, false);
}

void DuelLogic::OnSoundCueTick() {
    const int nTick = TheSongScheduler.mTick;
    if ((nTick + mTicksPerBar) / mTicksPerBar == mNextPhraseBar) {
        if (mCuesPlayed < kCoachedPhaseCount) {
            const bool bCoach = mSong->mDifficulty == kEasiestDifficulty;
            if (mCuePhase == kPhasePitch) {
                if (bCoach && TheGameDb->IsLocalPlayer(mPlayers[mCuePlayer]->GetIndex())) {
                    FxMidi::PlayDuelLayPattern();
                }
                ++mCuesPlayed;
            } else if (mCuePhase == kPhaseCatch) {
                if (bCoach && TheGameDb->IsLocalPlayer(mPlayers[mCuePlayer]->GetIndex())) {
                    FxMidi::PlayDuelCatchPattern();
                }
                ++mCuesPlayed;
            }
        } else if (mPlayers[kFirstSide]->GetScore() == kMatchPointScore ||
                   mPlayers[kSecondSide]->GetScore() == kMatchPointScore) {
            if (mCuePhase == kPhasePitch && !mOneLetterPlayed &&
                mCatchMadeTick + kOneLetterDelayTicks < nTick) {
                FxMidi::PlayDuelOneLetter();
                mOneLetterPlayed = 1;
            } else if (mCuePhase == kPhaseCatch && mPitcher->GetScore() == kMatchPointScore &&
                       mCatcher->GetScore() < kMatchPointScore && !mMissThisPlayed &&
                       TheGameDb->IsLocalPlayer(mPlayers[mCuePlayer]->GetIndex()) &&
                       mCatcherGems->GetNumGems() > 0) {
                FxMidi::PlayDuelMissThis();
                mMissThisPlayed = 1;
            }
        }
    }
    TheSongScheduler.PostAt(mSoundCueCommand.Get(), nTick + mTicksPerBar, false);
}

unsigned char DuelLogic::GetTrackVolume(int nTrack, bool bMuted) {
    if (!mSong->HasTrackVolumes(nTrack)) {
        return bMuted ? kMutedVolume : kFullVolume;
    }
    return mSong->GetTrackVolumes(nTrack)[bMuted];
}

void DuelLogic::ApplyBarTrack(int nBar) {
    // Yes, the binary checks the bar count against zero and discards the wrapped bar.
    (void)(nBar % mSong->mNumBars);
    const int nTrack = mBarData[nBar].mTrack;

    for (unsigned int i = 0; i < mPitchTracks.size(); ++i) {
        PitchTrack *pPitchTrack = mPitchTracks[i];
        const int nPitchTrack = pPitchTrack->mIndex;
        if (nPitchTrack == nTrack) {
            pPitchTrack->SetActive(true);
            TheMixer->SetVolume(nTrack, GetTrackVolume(nTrack, false), kVolumeRampTicks);
        } else {
            pPitchTrack->SetActive(false);
            TheMixer->SetVolume(nPitchTrack, GetTrackVolume(nPitchTrack, true), kVolumeRampTicks);
        }
    }

    for (unsigned int i = 0; i < mTracks.size(); ++i) {
        mTracks[i]->SetRiffData(mSong->GetTrackRiffData(nTrack));
        TheGfxManager.SetTrackInstrument(static_cast<int>(i), mSong->GetTrackInstrument(nTrack));
    }
}

void DuelLogic::DecideDuel(bool bLocalWinner) {
    TheForceFeedbackMgr->StopAll();
    TheGameDb->SetWon(true);
    TheGfxManager.ShowResult(true, 0, false);
    StopCommands();
    for (unsigned int i = 0; i < mTracks.size(); ++i) {
        mTracks[i]->CancelPhrase();
    }
    for (unsigned int i = 0; i < mPlayers.size(); ++i) {
        TheGameDb->SetPlayerScore(mPlayers[i]->GetIndex(), mPlayers[i]->GetScore());
    }
    if (bLocalWinner) {
        FxMidi::PlayWinSound();
    }
    mState = kStateDecided;

    int nWinner = kNoPlayer;
    for (unsigned int i = 0; i < mPlayers.size(); ++i) {
        if (TheGameDb->GetPlayerRank(static_cast<int>(i)) == 0) {
            nWinner = mPlayers[i]->GetIndex();
        }
    }
    if (nWinner == kNoPlayer || TheGameDb->IsLocalPlayer(nWinner)) {
        FxMidi::PlayDuelCheer();
    }
    TheSongScheduler.PostAfter(mAnnounceCommand.Get(), kAnnounceDelay, false);

    if (TheGameDb->mCommunity != GameDb::kCommunityOnline) {
        LogEndMultiGame();
        TheMetagame.ShowDialog(Metagame::kDialogEndGame, OnGameOverDialog, this, kNoPad);
        return;
    }
    if (TheNetTransport->IsHost()) {
        mScoresReceived[mLocalPlayer->GetIndex()] = true;
        if (AllScoresReceived()) {
            TheMetagame.ShowDialog(Metagame::kDialogEndGame, OnGameOverDialog, this, kNoPad);
            ReportScores();
        }
    } else {
        FinalScorePacket packet;
        packet.mScore = mLocalPlayer->GetScore();
        TheNetTransport->Send(packet);
    }
}

void DuelLogic::AnnounceWinner() {
    const int nFirstScore = mPlayers[kFirstSide]->GetScore();
    const int nSecondScore = mPlayers[kSecondSide]->GetScore();
    if (nFirstScore == nSecondScore) {
        FxMidi::PlayDuelGameTie();
    } else if (nSecondScore < nFirstScore) {
        FxMidi::PlayDuelGreenWins();
    } else {
        FxMidi::PlayDuelPurpleWins();
    }
}

bool DuelLogic::AllScoresReceived() const {
    for (unsigned int i = 0; i < mScoresReceived.size(); ++i) {
        if (!mScoresReceived[i] && !mPlayers[i]->IsAborted()) {
            return false;
        }
    }
    return true;
}

void DuelLogic::ReportScores() {
    std::vector<NetGameScore> scores;
    for (int nPlayer = 0; nPlayer < TheGameDb->GetNumPlayers(); ++nPlayer) {
        if (mPlayers[nPlayer]->IsAborted()) {
            TheGameDb->SetPlayerScore(nPlayer, 0);
        }
        const int nSeat = TheGameDb->GetPlayerNetOrder(nPlayer);
        scores.push_back(NetGameScore{nSeat, TheGameDb->GetPlayerScore(nPlayer)});
    }
    LogEndMultiGame();
    TheNetTransport->ReportScores(scores);
    TheMetagame.ShowDialog(Metagame::kDialogEndGame, OnGameOverDialog, this, kNoPad);
}

void DuelLogic::LogEndMultiGame() {
    std::vector<int> scores(kStatsPlayerCount, 0);
    for (unsigned int i = 0; i < mPlayers.size(); ++i) {
        scores[i] = mPlayers[i]->GetScore();
    }
    TheStats->EndMultiGame(scores[0], scores[1], scores[2], scores[3]);
}

int DuelLogic::OnPlayerUpdate(PlayerUpdatePacket *pPacket) {
    const int nPlayer = mSeatPlayers[pPacket->mSender];
    const unsigned int nVersion = static_cast<unsigned int>(pPacket->mVersion);
    if (nVersion < mUpdateVersions[nPlayer]) {
        return 0;
    }
    mUpdateVersions[nPlayer] = nVersion;
    mPlayers[nPlayer]->SetCatching(pPacket->mCatching);
    return 0;
}

int DuelLogic::OnEditGem(EditGemPacket *pPacket) {
    const int nSide = pPacket->mTrack;
    const int nOtherSide = kSecondSide - nSide;
    CatchTrackData *pGems = mGems[nSide];
    const int nTick = pPacket->mTick;
    const int nSlot = pPacket->mSlot;
    if (nTick < TheSongScheduler.mTick + kMinGemLeadTicks) {
        return 0;
    }

    const float fCatchTick = static_cast<float>(nTick - mTicksPerBar * mPhraseBars);
    const int nBar = (nTick / mTicksPerBar) % mSong->mNumBars;
    Muse *pRiff = mSong->GetTrackRiffData(mBarData[nBar].mTrack)->GetRiff(nTick, nSlot);
    const Gem gem{nSlot, nTick, Ptr<Muse>(pRiff)};
    pGems->AddGem(gem);
    mTracks[nSide]->CountGem();
    TheGfxManager.PlaceGem(
        nSide, nSlot, kNoPlayer, static_cast<float>(nTick), kGemStyle, kGemFlags);
    TheGfxManager.PlaceGem(nOtherSide, nSlot, kNoPlayer, fCatchTick, kGemStyle, kGemFlags);
    TheGfxManager.HitGem(mRemotePlayer->GetIndex(), nOtherSide, nSlot, fCatchTick);
    return 0;
}

int DuelLogic::OnCapture(CapturePacket *pPacket) {
    DuelTrack *pTrack = mTracks[pPacket->mTrack];
    if (pPacket->mValid) {
        pTrack->MakeCatch(pPacket->mBar);
    } else {
        pTrack->MissCatch();
    }
    return 0;
}

int DuelLogic::OnFinalScore(FinalScorePacket *pPacket) {
    (void)TheNetTransport->IsHost(); // Yes, the binary discards the result.
    mScoresReceived[mSeatPlayers[pPacket->mSender]] = true;
    if (AllScoresReceived()) {
        ReportScores();
    }
    return 0;
}

int DuelLogic::OnGameEnded(GameEndedMsg *pMsg) {
    switch (pMsg->mResult) {
    case GameEndedMsg::kResultFinished:
        TheMetagame.ShowDialog(Metagame::kDialogEndGame, OnGameOverDialog, this, kNoPad);
        break;
    case GameEndedMsg::kResultHostAborted: {
        const char *pszLine = TheLocale.Localize(kHostAborted1Key, true);
        const char *pszSecondLine = TheLocale.Localize(kHostAborted2Key, true);
        TheGfxManager.ShowMessage(pszLine,
                                  pszSecondLine,
                                  kAllPlayers,
                                  kAbortMessageDuration,
                                  kAbortMessageScale,
                                  kAbortMessageOffset,
                                  kAbortMessageOffset);
        TheSongScheduler.PostAfter(
            NewMemFunCommand(this, &DuelLogic::StartEnding), kHostAbortedDelay, false);
        break;
    }
    case GameEndedMsg::kResultConnectionLost: {
        const char *pszLine = TheLocale.Localize(kConnectionLost1Key, true);
        const char *pszSecondLine = TheLocale.Localize(kConnectionLost2Key, true);
        TheGfxManager.ShowMessage(pszLine,
                                  pszSecondLine,
                                  kAllPlayers,
                                  kAbortMessageDuration,
                                  kAbortMessageScale,
                                  kAbortMessageOffset,
                                  kAbortMessageOffset);
        TheSongScheduler.PostIn(
            NewMemFunCommand(this, &DuelLogic::StartEnding), kConnectionLostDelayTicks, false);
        break;
    }
    default:
        DebugWarn("illegal game result");
        break;
    }
    return 0;
}

int DuelLogic::OnPlayerAborted(PlayerAbortedMsg *pMsg) {
    const int nPlayer = mSeatPlayers[pMsg->mNetOrder];
    Player *pPlayer = mPlayers[mInputPlayers[nPlayer]];
    if (pPlayer->IsAborted()) {
        return 0;
    }
    pPlayer->Abort();
    mTrackSelector->RemovePlayer(nPlayer);
    const char *pszFormat = TheLocale.Localize(kClientAbortedKey, true);
    const char *pszText = FormatString(pszFormat, TheGameDb->GetPlayerName(nPlayer));
    TheGfxManager.ShowMessage(pszText,
                              nullptr,
                              kAllPlayers,
                              kAbortMessageDuration,
                              kAbortMessageScale,
                              kAbortMessageOffset,
                              kAbortMessageOffset);
    TheNetTransport->Leave();
    TheStats->PlayerAborted(nPlayer, TheSongScheduler.mTick);
    StartEnding();
    return 0;
}

bool DuelLogic::DispatchPriv(Message *pMsg) {
    const int nType = pMsg->Type();
    if (nType == g_nPlayerUpdatePacketType) {
        return OnPlayerUpdate(static_cast<PlayerUpdatePacket *>(pMsg)) != 0;
    }
    if (nType == g_nEditGemPacketType) {
        return OnEditGem(static_cast<EditGemPacket *>(pMsg)) != 0;
    }
    if (nType == g_nCapturePacketType) {
        return OnCapture(static_cast<CapturePacket *>(pMsg)) != 0;
    }
    if (nType == g_nFinalScorePacketType) {
        return OnFinalScore(static_cast<FinalScorePacket *>(pMsg)) != 0;
    }
    if (nType == g_nGameEndedMsgType) {
        return OnGameEnded(static_cast<GameEndedMsg *>(pMsg)) != 0;
    }
    if (nType == g_nPlayerAbortedMsgType) {
        return OnPlayerAborted(static_cast<PlayerAbortedMsg *>(pMsg)) != 0;
    }
    return WorldLogic::DispatchPriv(pMsg);
}
