#include "game/multigamelogic.h"

#include "game/forcefeedbackmgr.h"
#include "game/gamedb.h"
#include "game/helptext.h"
#include "game/localplayer.h"
#include "game/netgamescore.h"
#include "game/stats.h"
#include "gfx/gfxmanager.h"
#include "math/rand.h"
#include "netflow/nettransport.h"
#include "os/debug.h"
#include "os/locale.h"
#include "os/memfun1command.h"
#include "os/memfun3command.h"
#include "os/memfuncommand.h"
#include "os/scheduler.h"
#include "os/string.h"
#include "script/scriptfunction.h"
#include "synth/fxmidi.h"
#include "synth/songspeed.h"

namespace {

constexpr char kWinCheatCommandName[] = "win_cheat";

constexpr int kNoPlayer = -1;
constexpr int kNoPad = -1;
constexpr int kWinCheatPlayer = 1;
constexpr int kWinCheatScore = 400;

constexpr float kLeaderSoundRetryMs = 1500.0f;
constexpr float kEndGameDialogDelayMs = 3200.0f;
constexpr float kLeaveSongDelayMs = 3500.0f;
constexpr int kLeaveSongDelayTicks = 3500;

constexpr float kFullEnergy = 100.0f;
constexpr float kWonSongProgress = 1.0f;

constexpr float kStageMessageMs = 2600.0f;
constexpr float kStageMessageScale = 1.1f;
constexpr int kStageMessageOffsetPerPlayer = -25;
constexpr float kResultMessageMs = 3000.0f;
constexpr float kResultMessageScale = 1.5f;
constexpr float kNoticeMessageMs = 3200.0f;
constexpr float kNoticeMessageScale = 0.9f;

bool IsOnline() {
    return TheGameDb->mCommunity == GameDb::kCommunityOnline;
}

} // namespace

MultiGameLogic::MultiGameLogic(Song *pSong, DataArray *pConfig, int nSeed)
    : GameLogic(pSong, pConfig, nSeed), mLocalPlayer(nullptr), mLeader(kNoPlayer),
      mFinishCommand(NewMemFun1Command(this, &GameLogic::Finish, true)),
      mLeaderCommand(NewMemFunCommand(this, &MultiGameLogic::PlayLeaderSound)),
      mVersions(TheGameDb->GetNumPlayers(), 0), mArbiterVersion(0), mArbiter(nullptr),
      mFinished(TheGameDb->GetNumPlayers(), false) {
    if (IsOnline()) {
        if (TheNetTransport->IsHost()) {
            mArbiter = new NetArbiter(mTrackSelector);
        }
        for (int i = 0; i < TheGameDb->GetNumPlayers(); ++i) {
            if (TheGameDb->IsLocalPlayer(i)) {
                mLocalPlayer = mPlayers[i];
            }
            const unsigned nNetOrder = TheGameDb->GetPlayerNetOrder(i);
            if (nNetOrder >= mNetToPlayer.size()) {
                mNetToPlayer.resize(nNetOrder + 1, kNoPlayer);
            }
            mNetToPlayer[nNetOrder] = i;
        }
    }
    ScriptFunction::Register(WinCheat, kWinCheatCommandName, this);
}

MultiGameLogic::~MultiGameLogic() {
    delete mArbiter;
    SetSongSpeed(mSong->GetSpeed());
    ScriptFunction::Unregister(WinCheat);
}

void MultiGameLogic::OnStart() {
    for (CatchTrack *pTrack : mCatchTracks) {
        pTrack->Enable(0);
    }
    TheSongScheduler.PostAt(mFinishCommand.Get(), mNumBars * mTicksPerBar, false);
    if (mArbiter != nullptr) {
        mArbiter->Start();
    }
}

void MultiGameLogic::SetPaused(bool bPaused, int nPad, int nReason) {
    if (bPaused && mState != kStatePlaying) {
        return;
    }
    if (mPaused == bPaused) {
        return;
    }
    mPaused = bPaused;
    GameLogic::SetPaused(bPaused, nPad, nReason);
    if (IsOnline()) {
        PauseOnline(bPaused, nPad, nReason);
    } else {
        PauseLocal(bPaused, nPad, nReason);
    }
}

void MultiGameLogic::PauseOnline(bool bPaused, int nPad, int nReason) {
    if (bPaused) {
        const Metagame::DialogType type =
            nReason != 0 ? Metagame::kDialogNoController : Metagame::kDialogPause;
        TheMetagame.ShowDialog(type, OnOnlinePauseDialog, this, nPad);
    }
}

void MultiGameLogic::PauseLocal(bool bPaused, int nPad, int nReason) {
    if (bPaused) {
        TheSongScheduler.Pause();
        AllNotesOff();
        const Metagame::DialogType type =
            nReason != 0 ? Metagame::kDialogNoController : Metagame::kDialogPause;
        TheMetagame.ShowDialog(type, OnLocalPauseDialog, this, nPad);
    } else {
        TheSongScheduler.Resume();
    }
}

void MultiGameLogic::OnBar([[maybe_unused]] int nBar) {
    UpdateLeader();
}

void MultiGameLogic::OnSection() {
    if (mSection >= mSong->GetSections()->NumSections()) {
        return;
    }
    TheGfxManager.CompleteStage(0, mSection);
    const char *pszText = FormatString(TheLocale.Localize("STAGE_COMPLETED", true), mSection + 1);
    TheGfxManager.ShowMessage(
        pszText,
        nullptr,
        kNoPlayer,
        kStageMessageMs,
        kStageMessageScale,
        0.0f,
        static_cast<float>(TheGameDb->GetNumPlayers() * kStageMessageOffsetPerPlayer));
}

void MultiGameLogic::OnLocalPauseDialog(Metagame::DialogAction action, void *pUserData) {
    auto *pLogic = static_cast<MultiGameLogic *>(pUserData);
    if (pLogic->mState != kStatePaused) {
        return;
    }
    switch (action) {
    case Metagame::kDialogActionResume:
        pLogic->SetPaused(false, kNoPad, 0);
        break;
    case Metagame::kDialogActionQuit:
        pLogic->mQuit = 1;
        pLogic->EndSong();
        break;
    case Metagame::kDialogActionEnd:
        pLogic->EndSong();
        break;
    default:
        DebugWarn("illegal dialog response");
        break;
    }
}

void MultiGameLogic::OnOnlinePauseDialog(Metagame::DialogAction action, void *pUserData) {
    auto *pLogic = static_cast<MultiGameLogic *>(pUserData);
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
        pLogic->EndSong();
        break;
    default:
        DebugWarn("illegal dialog response");
        break;
    }
}

int MultiGameLogic::GetTick() {
    return TheSongScheduler.GetClockTick();
}

float MultiGameLogic::GetTime() {
    return TheSongScheduler.GetClockTime();
}

void MultiGameLogic::HandleInput(const RotateEvent &event) {
    GameLogic::HandleInput(event);
    if (mArbiter != nullptr) {
        const int nPlayer = event.mPlayer;
        mArbiter->RequestTrack(nPlayer, mTrackSelector->GetPlayerTrack(nPlayer));
    }
}

void MultiGameLogic::PlayLeaderSound() {
    if (TheGameDb->GetNumPlayers() == 1) {
        return;
    }
    if (mLeader == kNoPlayer) {
        return;
    }
    FxMidi::PlayLeaderSound(TheGameDb->GetPlayerSlot(mLeader));
}

void MultiGameLogic::UpdateLeader() {
    std::vector<int> scores(mPlayers.size());
    int nBestScore = -1;
    int nLeader = kNoPlayer;
    for (unsigned i = 0; i < mPlayers.size(); ++i) {
        const int nScore = mPlayers[i]->GetScore();
        scores[i] = nScore;
        if (nScore > nBestScore) {
            nBestScore = nScore;
            nLeader = static_cast<int>(i);
        } else if (nScore == nBestScore) {
            nLeader = kNoPlayer;
        }
    }

    if (nLeader != mLeader) {
        mLeader = nLeader;
        TheSongScheduler.Cancel(mLeaderCommand.Get());
        if (FxMidi::IsLeaderSoundPlaying()) {
            TheSongScheduler.PostAfter(mLeaderCommand.Get(), kLeaderSoundRetryMs, false);
        } else {
            PlayLeaderSound();
        }
    }

    for (unsigned i = 0; i < mPlayers.size(); ++i) {
        const bool bLeader = scores[i] > 0 && scores[i] == nBestScore;
        TheGfxManager.SetLeader(mPlayers[i]->GetIndex(), bLeader);
    }
}

void MultiGameLogic::OnPhraseCaptured(CatchTrack *pTrack,
                                      Player *pPlayer,
                                      [[maybe_unused]] int nBar,
                                      bool bAutocatch) {
    UpdateLeader();
    if (IsOnline() && TheGameDb->IsLocalPlayer(pPlayer->GetIndex())) {
        CapturePacket packet(pTrack->mIndex, TheSongScheduler.mTick / mTicksPerBar, bAutocatch);
        TheNetTransport->Send(packet);
    }
}

void MultiGameLogic::OnPhraseEnded([[maybe_unused]] Track *pTrack) {
}

void MultiGameLogic::OnEndGameDialog(Metagame::DialogAction action, void *pUserData) {
    auto *pLogic = static_cast<MultiGameLogic *>(pUserData);
    switch (action) {
    case Metagame::kDialogActionQuit:
        pLogic->mQuit = 1;
        pLogic->EndSong();
        break;
    case Metagame::kDialogActionResume:
    case Metagame::kDialogActionEnd:
        pLogic->EndSong();
        break;
    default:
        DebugWarn("illegal choice");
        break;
    }
}

void MultiGameLogic::WinCheat([[maybe_unused]] DataArray *pCommand, void *pUserData) {
    static_cast<MultiGameLogic *>(pUserData)->ApplyWinCheat();
}

void MultiGameLogic::ApplyWinCheat() {
    mPlayers[kWinCheatPlayer]->SetScore(
        kWinCheatScore); // Yes, the binary scores the second player.
    Finish(true);
    TheGameDb->SetProgress(kWonSongProgress);
}

void MultiGameLogic::OnFinish(bool bWon) {
    if (bWon) {
        TheGfxManager.SetEnergy(0, kFullEnergy);
        mState = kStateWon;
        FxMidi::PlayWinSound();
    }

    if (!IsOnline()) {
        ShowResults();
        return;
    }
    if (TheNetTransport->IsHost()) {
        mFinished[mLocalPlayer->GetIndex()] = true;
        if (AllPlayersFinished()) {
            SendResults();
        }
    } else {
        FinalScorePacket packet(mLocalPlayer->GetScore());
        TheNetTransport->Send(packet);
    }
}

void MultiGameLogic::ShowResults() {
    LogResults();

    int nWinner = kNoPlayer;
    bool bTie = false;
    for (unsigned i = 0; i < mPlayers.size(); ++i) {
        if (TheGameDb->GetPlayerRank(i) == 0) {
            if (nWinner != kNoPlayer) {
                bTie = true;
            }
            nWinner = static_cast<int>(i);
        }
    }

    const char *pszText;
    if (bTie) {
        pszText = TheLocale.Localize("TIE_MULTI_GAME", true);
    } else {
        FxMidi::PlayWinnerSound(TheGameDb->GetPlayerSlot(nWinner));
        TheGfxManager.SetWinner(nWinner);
        pszText = FormatString(TheLocale.Localize("WON_MULTI_GAME", true),
                               TheGameDb->GetPlayerName(nWinner));
    }
    TheGfxManager.ShowMessage(
        pszText, nullptr, kNoPlayer, kResultMessageMs, kResultMessageScale, 0.0f, 0.0f);

    Command *pDialog = NewMemFun3Command(&TheMetagame,
                                         &Metagame::ShowDialogToAll,
                                         Metagame::kDialogEndGame,
                                         &MultiGameLogic::OnEndGameDialog,
                                         static_cast<void *>(this));
    TheSongScheduler.PostAfter(pDialog, kEndGameDialogDelayMs, false);
}

int MultiGameLogic::GetPlayerFromNetOrder(int nNetOrder) {
    return mNetToPlayer[nNetOrder];
}

bool MultiGameLogic::DeploySlowdown(Player *pPlayer) {
    const bool bDeployed = GameLogic::DeploySlowdown(pPlayer);
    if (bDeployed && IsOnline() && TheGameDb->IsLocalPlayer(pPlayer->GetIndex())) {
        SlowdownPacket packet;
        TheNetTransport->Send(packet);
    }
    return bDeployed;
}

bool MultiGameLogic::DeployFreestyle(Player *pPlayer) {
    if (!IsOnline()) {
        Player *pFreestyler = mFreestyleTrack->mPlayer;
        if (pFreestyler != nullptr) {
            LeaveFreestyle(pFreestyler, RandomInt(0, static_cast<int>(mCatchTracks.size())));
        }
    }
    (void)GameLogic::DeployFreestyle(pPlayer); // Yes, the binary discards this call's result.
    if (mArbiter != nullptr) {
        mArbiter->RequestTrack(pPlayer->GetIndex(), mFreestyleTrack->mIndex);
    }
    return true;
}

void MultiGameLogic::EndFreestyle(Player *pPlayer) {
    (void)TheGameDb->IsLocalPlayer(pPlayer->GetIndex()); // Yes, the binary discards the result.
    GameLogic::EndFreestyle(pPlayer);
    if (mArbiter != nullptr) {
        mArbiter->RequestTrack(pPlayer->GetIndex(), pPlayer->GetTrack()->mIndex);
    }
}

bool MultiGameLogic::DeployBumper(Player *pPlayer) {
    BumperPacket packet;
    const int nAttacker = pPlayer->GetIndex();
    Track *pTrack = pPlayer->GetTrack();
    bool bBumped = false;

    for (Player *pOther : mPlayers) {
        if (pOther == pPlayer || pOther->IsAborted() || pOther->GetTrack() != pTrack) {
            continue;
        }
        const int nVictim = pOther->GetIndex();
        bBumped = true;

        const int nPad = TheGameDb->GetPlayerPad(nVictim);
        if (nPad != kNoPad) {
            TheForceFeedbackMgr->PlayBumpEffect(nPad);
        }

        int nNewTrack = RandomInt(0, static_cast<int>(mCatchTracks.size()) - 1);
        if (nNewTrack >= pTrack->mIndex) {
            ++nNewTrack;
        }
        packet.mVictims.push_back(
            BumperPacket::VictimData{TheGameDb->GetPlayerNetOrder(nVictim), nNewTrack});

        TheGfxManager.ShowBump(nAttacker, nVictim, nNewTrack);
        mTrackSelector->MovePlayer(nVictim, nNewTrack);
        if (mArbiter != nullptr) {
            mArbiter->RequestTrack(nVictim, nNewTrack);
        }
    }

    if (bBumped) {
        ShowPowerupText("BUMPER", nAttacker);
        if (IsOnline()) {
            TheNetTransport->Send(packet);
        }
    } else {
        TheHelpText->ShowBumperFailed();
    }
    return bBumped;
}

bool MultiGameLogic::DeployCrippler(Player *pPlayer) {
    const int nAttacker = pPlayer->GetIndex();
    Track *pTrack = pPlayer->GetTrack();
    bool bCrippled = false;

    for (Player *pOther : mPlayers) {
        if (pOther == pPlayer || pOther->IsAborted() || pOther->GetTrack() != pTrack) {
            continue;
        }
        const int nVictim = pOther->GetIndex();
        bCrippled = true;

        const int nPad = TheGameDb->GetPlayerPad(nVictim);
        if (nPad != kNoPad) {
            TheForceFeedbackMgr->PlayCrippleEffect(nPad);
        }
        TheGfxManager.ShowCripple(nAttacker, nVictim);

        if (IsOnline()) {
            (void)TheGameDb->GetPlayerNetOrder(nAttacker); // Yes, the binary discards this.
            CripplerPacket packet(TheGameDb->GetPlayerNetOrder(nVictim));
            TheNetTransport->Send(packet);
        }
    }

    if (bCrippled) {
        ShowPowerupText("CRIPPLER", nAttacker);
    } else {
        TheHelpText->ShowCripplerFailed();
    }
    return bCrippled;
}

bool MultiGameLogic::AllPlayersFinished() {
    for (unsigned i = 0; i < mFinished.size(); ++i) {
        if (!mFinished[i] && !mPlayers[i]->IsAborted()) {
            return false;
        }
    }
    return true;
}

void MultiGameLogic::SendResults() {
    std::vector<NetGameScore> scores;
    for (int i = 0; i < TheGameDb->GetNumPlayers(); ++i) {
        if (mPlayers[i]->IsAborted()) {
            TheGameDb->SetPlayerScore(i, 0);
        }
        const int nNetOrder = TheGameDb->GetPlayerNetOrder(i);
        scores.push_back(NetGameScore{nNetOrder, TheGameDb->GetPlayerScore(i)});
    }
    TheNetTransport->ReportScores(scores);
    ShowResults();
}

void MultiGameLogic::LogResults() {
    std::vector<int> scores(4, 0);
    for (unsigned i = 0; i < mPlayers.size(); ++i) {
        scores[i] = mPlayers[i]->GetScore();
    }
    TheStats->EndMultiGame(scores[0], scores[1], scores[2], scores[3]);
}

void MultiGameLogic::OnPlayerUpdate(PlayerUpdatePacket *pPacket) {
    if (IsPastEnd()) {
        return;
    }
    const int nPlayer = GetPlayerFromNetOrder(pPacket->mSender);
    const unsigned nVersion = pPacket->mVersion;
    if (nVersion < mVersions[nPlayer]) {
        return;
    }
    mVersions[nPlayer] = nVersion;

    Player *pPlayer = mPlayers[nPlayer];
    pPlayer->SetScore(pPacket->mPoints);
    const int nTrack = pPacket->mTrack;
    if (mTrackSelector->GetPlayerTrack(nPlayer) != nTrack) {
        if (nTrack == mFreestyleTrack->mIndex) {
            mTrackSelector->EnterFreestyle(nPlayer, false);
        } else {
            mTrackSelector->MovePlayer(nPlayer, nTrack);
        }
        if (mArbiter != nullptr) {
            mArbiter->UpdatePlayer(nPlayer, nTrack, static_cast<int>(nVersion));
        }
    }
    pPlayer->SetCatching(pPacket->mCatching);
}

void MultiGameLogic::OnArbiter(ArbiterPacket *pPacket) {
    if (IsPastEnd()) {
        return;
    }
    const unsigned nVersion = pPacket->mVersion;
    if (nVersion < mArbiterVersion) {
        return;
    }
    mArbiterVersion = nVersion;
    mVersions[mLocalPlayer->GetIndex()] = LocalPlayer::sPlayerUpdateVersion;

    for (const ArbiterPacket::PlayerData &data : pPacket->mPlayers) {
        const int nPlayer = GetPlayerFromNetOrder(data.mNetOrder);
        if (mPlayers[nPlayer]->IsAborted()) {
            continue;
        }
        const int nCurrentTrack = mTrackSelector->GetPlayerTrack(nPlayer);
        // An assignment older than the player's own update is applied only within its track.
        if (static_cast<unsigned>(data.mVersion) < mVersions[nPlayer] &&
            data.mTrack != nCurrentTrack) {
            continue;
        }
        const int nCurrentSlot = mTrackSelector->GetPlayerSlot(nPlayer);
        if (nCurrentTrack == data.mTrack && nCurrentSlot == data.mSlot) {
            continue;
        }
        mTrackSelector->SwapPlayer(nPlayer, data.mTrack, data.mSlot);
    }
    mTrackSelector->Resync();
}

void MultiGameLogic::OnCapture(CapturePacket *pPacket) {
    if (IsPastEnd()) {
        return;
    }
    const int nPlayer = GetPlayerFromNetOrder(pPacket->mSender);
    CatchTrack *pTrack = mCatchTracks[pPacket->mTrack];
    Player *pPlayer = mPlayers[nPlayer];
    if (!pPacket->mAutocatch) {
        pTrack->Capture(pPacket->mBar, pPlayer);
        return;
    }
    pTrack->Autocatch(pPacket->mBar, pPlayer);
    FxMidi::PlayPowerupSound(kPowerupAutocatcher);
    if (mLocalPlayer->GetTrack() == pTrack) {
        TheForceFeedbackMgr->PlayAutocatchEffect(TheGameDb->GetPlayerPad(mLocalPlayer->GetIndex()));
    }
}

void MultiGameLogic::OnBumper(BumperPacket *pPacket) {
    if (IsPastEnd()) {
        return;
    }
    const int nAttacker = GetPlayerFromNetOrder(pPacket->mSender);
    FxMidi::PlayPowerupSound(kPowerupBumper);
    for (const BumperPacket::VictimData &victim : pPacket->mVictims) {
        const int nVictim = GetPlayerFromNetOrder(victim.mNetOrder);
        if (mPlayers[nVictim]->IsAborted()) {
            continue;
        }
        const int nNewTrack = victim.mTrack;
        TheGfxManager.ShowBump(nAttacker, nVictim, nNewTrack);
        mTrackSelector->MovePlayer(nVictim, nNewTrack);
        const int nPad = TheGameDb->GetPlayerPad(nVictim);
        if (nPad != kNoPad) {
            TheForceFeedbackMgr->PlayBumpEffect(nPad);
        }
        if (mArbiter != nullptr) {
            mArbiter->UpdatePlayer(nVictim, nNewTrack, -1);
        }
    }
}

void MultiGameLogic::OnCrippler(CripplerPacket *pPacket) {
    if (IsPastEnd()) {
        return;
    }
    const int nAttacker = GetPlayerFromNetOrder(pPacket->mSender);
    const int nVictim = GetPlayerFromNetOrder(pPacket->mVictim);
    if (mPlayers[nVictim]->IsAborted()) {
        return;
    }
    FxMidi::PlayPowerupSound(kPowerupCrippler);
    TheGfxManager.ShowCripple(nAttacker, nVictim);
    const int nPad = TheGameDb->GetPlayerPad(nVictim);
    if (nPad != kNoPad) {
        TheForceFeedbackMgr->PlayCrippleEffect(nPad);
    }
}

void MultiGameLogic::OnSlowdown(SlowdownPacket *pPacket) {
    if (IsPastEnd()) {
        return;
    }
    const int nPlayer = GetPlayerFromNetOrder(pPacket->mSender);
    DeploySlowdown(mPlayers[nPlayer]);
    FxMidi::PlayPowerupSound(kPowerupSlowdown);
}

void MultiGameLogic::OnMultiplier(MultiplierPacket *pPacket) {
    if (IsPastEnd()) {
        return;
    }
    Player *pPlayer = mPlayers[GetPlayerFromNetOrder(pPacket->mSender)];
    if (pPlayer->IsAborted()) {
        return;
    }
    pPlayer->ActivateMultiplier();
    FxMidi::PlayPowerupSound(kPowerupMultiplier);
}

void MultiGameLogic::OnFreestyle(FreestylePacket *pPacket) {
    if (IsPastEnd()) {
        return;
    }
    const int nPlayer = GetPlayerFromNetOrder(pPacket->mSender);
    Player *pPlayer = mPlayers[nPlayer];
    if (pPlayer->IsAborted() || pPlayer->GetTrack() != mFreestyleTrack) {
        return;
    }
    if (pPacket->mActive != mFreestyleTrack->IsActive()) {
        const PlayNoteEvent event(static_cast<unsigned char>(nPlayer),
                                  static_cast<unsigned char>(pPacket->mButton),
                                  pPacket->mActive,
                                  pPacket->mX,
                                  pPacket->mY);
        mFreestyleTrack->HandleInput(pPlayer, event);
    } else if (mFreestyleTrack->IsActive()) {
        const StickEvent<2> event{static_cast<unsigned char>(nPlayer), pPacket->mX, pPacket->mY};
        mFreestyleTrack->HandleInput(pPlayer, event);
    }
}

void MultiGameLogic::OnFreestyleBump(FreestyleBumpPacket *pPacket) {
    if (IsPastEnd()) {
        return;
    }
    Player *pPlayer = mPlayers[GetPlayerFromNetOrder(pPacket->mNetOrder)];
    if (pPlayer->GetTrack() != mFreestyleTrack || pPlayer->IsAborted()) {
        return;
    }
    LeaveFreestyle(pPlayer, pPacket->mTrack);
}

void MultiGameLogic::OnFinalScore(FinalScorePacket *pPacket) {
    (void)TheNetTransport->IsHost(); // Yes, the binary discards this call's result.
    const int nPlayer = GetPlayerFromNetOrder(pPacket->mSender);
    mFinished[nPlayer] = true;
    mPlayers[nPlayer]->SetScore(pPacket->mScore);
    TheGameDb->SetPlayerScore(nPlayer, pPacket->mScore);
    if (AllPlayersFinished()) {
        SendResults();
    }
}

void MultiGameLogic::OnGameEnded(GameEndedMsg *pMsg) {
    for (const NetGameScore &score : pMsg->mScores) {
        for (int i = 0; i < TheGameDb->GetNumPlayers(); ++i) {
            if (TheGameDb->GetPlayerNetOrder(i) == score.mNetOrder) {
                TheGameDb->SetPlayerScore(i, score.mScore);
            }
        }
    }

    switch (pMsg->mResult) {
    case GameEndedMsg::kResultFinished:
        ShowResults();
        break;
    case GameEndedMsg::kResultHostAborted:
        TheGfxManager.ShowMessage(TheLocale.Localize("HOST_ABORTED_1", true),
                                  TheLocale.Localize("HOST_ABORTED_2", true),
                                  kNoPlayer,
                                  kNoticeMessageMs,
                                  kNoticeMessageScale,
                                  0.0f,
                                  0.0f);
        TheSongScheduler.PostAfter(
            NewMemFunCommand(this, &GameLogic::EndSong), kLeaveSongDelayMs, false);
        break;
    case GameEndedMsg::kResultConnectionLost:
        TheGfxManager.ShowMessage(TheLocale.Localize("CONNECTION_LOST_1", true),
                                  TheLocale.Localize("CONNECTION_LOST_2", true),
                                  kNoPlayer,
                                  kNoticeMessageMs,
                                  kNoticeMessageScale,
                                  0.0f,
                                  0.0f);
        TheSongScheduler.PostIn(
            NewMemFunCommand(this, &GameLogic::EndSong), kLeaveSongDelayTicks, false);
        break;
    default:
        DebugWarn("illegal game result");
        break;
    }
}

void MultiGameLogic::OnPlayerAborted(PlayerAbortedMsg *pMsg) {
    const int nPlayer = GetPlayerFromNetOrder(pMsg->mNetOrder);
    Player *pPlayer = mPlayers[nPlayer];
    if (pPlayer->IsAborted()) {
        return;
    }
    pPlayer->Abort();
    mTrackSelector->RemovePlayer(nPlayer);
    if (mArbiter != nullptr) {
        mArbiter->RemovePlayer(nPlayer);
    }

    const char *pszText =
        FormatString(TheLocale.Localize("CLIENT_ABORTED", true), TheGameDb->GetPlayerName(nPlayer));
    TheGfxManager.ShowMessage(
        pszText, nullptr, kNoPlayer, kNoticeMessageMs, kNoticeMessageScale, 0.0f, 0.0f);
    TheStats->PlayerAborted(nPlayer, TheSongScheduler.mTick);

    if (TheNetTransport->IsHost() && AllPlayersFinished()) {
        SendResults();
    }
}

void MultiGameLogic::DispatchPriv(Message *pMsg) {
    const int nType = pMsg->Type();
    if (nType == g_nPlayerUpdatePacketType) {
        OnPlayerUpdate(static_cast<PlayerUpdatePacket *>(pMsg));
    } else if (nType == g_nArbiterPacketType) {
        OnArbiter(static_cast<ArbiterPacket *>(pMsg));
    } else if (nType == g_nCapturePacketType) {
        OnCapture(static_cast<CapturePacket *>(pMsg));
    } else if (nType == g_nBumperPacketType) {
        OnBumper(static_cast<BumperPacket *>(pMsg));
    } else if (nType == g_nCripplerPacketType) {
        OnCrippler(static_cast<CripplerPacket *>(pMsg));
    } else if (nType == g_nSlowdownPacketType) {
        OnSlowdown(static_cast<SlowdownPacket *>(pMsg));
    } else if (nType == g_nMultiplierPacketType) {
        OnMultiplier(static_cast<MultiplierPacket *>(pMsg));
    } else if (nType == g_nFreestylePacketType) {
        OnFreestyle(static_cast<FreestylePacket *>(pMsg));
    } else if (nType == g_nFreestyleBumpPacketType) {
        OnFreestyleBump(static_cast<FreestyleBumpPacket *>(pMsg));
    } else if (nType == g_nFinalScorePacketType) {
        OnFinalScore(static_cast<FinalScorePacket *>(pMsg));
    } else if (nType == g_nGameEndedMsgType) {
        OnGameEnded(static_cast<GameEndedMsg *>(pMsg));
    } else if (nType == g_nPlayerAbortedMsgType) {
        OnPlayerAborted(static_cast<PlayerAbortedMsg *>(pMsg));
    } else {
        WorldLogic::DispatchPriv(pMsg);
    }
}
