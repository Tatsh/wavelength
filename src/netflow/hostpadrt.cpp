#include "netflow/hostpadrt.h"

#include <algorithm>
#include <cstring>

#include <librtmgcl.h>

#include "game/gamedb.h"
#include "game/remixinfo.h"
#include "math/rand.h"
#include "msg/bootpkt.h"
#include "msg/chatmsg.h"
#include "msg/clientinfopkt.h"
#include "msg/clientratingpkt.h"
#include "msg/dataackpkt.h"
#include "msg/datablockpkt.h"
#include "msg/gameoverpkt.h"
#include "msg/gameparamspkt.h"
#include "msg/hostlaunchpadsuccessmsg.h"
#include "msg/joinfailedpkt.h"
#include "msg/launchpadabortedmsg.h"
#include "msg/shareremixpkt.h"
#include "msg/shareremixprogressmsg.h"
#include "msg/startgamepkt.h"
#include "msg/timerequestpkt.h"
#include "msg/timeresponsepkt.h"
#include "msg/updaterankspkt.h"
#include "msg/welcomepkt.h"
#include "netflow/banplayertask.h"
#include "netflow/finishupdategameparamstask.h"
#include "netflow/ladderpositiontask.h"
#include "netflow/loadgametask.h"
#include "netflow/lobbyrt.h"
#include "netflow/mediussendworldreporttask.h"
#include "netflow/mgclauthenticationtask.h"
#include "netflow/mgclconnecttask.h"
#include "netflow/mgclcreategametask.h"
#include "netflow/mgcldisconnecttask.h"
#include "netflow/mgclsendserverreporttask.h"
#include "netflow/mgclsessionbegintask.h"
#include "netflow/mgclsessionendtask.h"
#include "netflow/netcrypt.h"
#include "netflow/netdisconnecttask.h"
#include "netflow/nethosttask.h"
#include "netflow/textfiltertask.h"
#include "netflow/waitforsharedonetask.h"
#include "os/system.h"
#include "os/vectorutil.h"

namespace {

// The wait between two reports to the servers, in milliseconds.
constexpr float kReportPeriodMs = 30000.0f;

// The wait for the guests to load the song once the host loaded it, in milliseconds.
constexpr float kLoadWaitMs = 20000.0f;

// The delay from StartGamePkt to the start of the song, in milliseconds.
constexpr float kStartDelayMs = 1000.0f;

// The range of the seed of a game.
constexpr int kSeedRange = 0xffff;

// The wait for the guests to acknowledge a block of the remix, in milliseconds.
constexpr float kBlockWaitMs = 7000.0f;

// The values GetWorldStatus() reports.
constexpr int kWorldStatusOpen = 1;
constexpr int kWorldStatusPlaying = 2;

// The value AssignColor() reports when every colour is taken.
constexpr int kNoColor = -1;

// The mode TextFilterTask filters the name of the remix in.
constexpr int kFilterRemixName = 1;

// Copy the settings of the game from the game database into mParams.
inline void CopyGameParams(NetGameParams &params) {
    params.mSong = TheGameDb->GetSong();
    params.mLoadRemix = TheGameDb->GetLoadRemix();
    params.mRemixReadOnly = TheGameDb->GetRemixReadOnly();
    params.mRemixName = TheGameDb->GetRemixName();
    params.mPracticeMode = TheGameDb->GetPracticeMode();
    params.mTutorial = TheGameDb->GetTutorial();
    params.mSkillLevel = TheGameDb->GetSkillLevel();
    params.mPowerupLevel = TheGameDb->GetPowerupLevel();
    params.mRuleSet = TheGameDb->GetRuleSet();
    params.mCommunity = TheGameDb->GetCommunity();
    params.mMaxPlayers = TheGameDb->GetMaxPlayers();
}

} // namespace

HostpadRT *TheHostpadRT;

void HostpadRT::Create(MsgSink *pScreen, MsgSink *pGame) {
    TheLaunchpadRT = std::make_unique<HostpadRT>(pScreen, pGame);
}

HostpadRT::HostpadRT(MsgSink *pScreen, MsgSink *pGame)
    : LaunchpadRT(pGame), mMgclConnected(0), mLost(0), mServerReport(kReportPeriodMs),
      mWorldReport(kReportPeriodMs),
#ifndef VIDEO_STANDARD_PAL
      mSyncDeadline(0.0f),
#endif
      mSharing(0), mBlockCount(0), mRemixShared(0), mBlockDeadline(0.0f) {
    NetCryptMakeKey(mKey);
    NetCryptSetKey(mKey);
    TheHostpadRT = this;
    for (int nColor = 0; nColor < kMaxPlayers; ++nColor) {
        mColors.push_back(ClientClr{nColor, kNoId});
    }

    CopyGameParams(mParams);
    if (mParams.mLoadRemix == 0) {
        mParams.mRemixName.Clear();
    }

    mConnectTask.Add(
        new TextFilterTask(&mParams.mRemixName, &mParams.mRemixName, kFilterRemixName));
    mConnectTask.Add(new LadderPositionTask(&TheLobbyRT->GetLocalPlayer()));
    mConnectTask.Add(new MGCLConnectTask(MGCLConnectTask::kServerLobby));
    mConnectTask.Add(new MGCLSessionBeginTask());
    mConnectTask.Add(new MGCLAuthenticationTask());
    mConnectTask.Add(new MGCLDisconnectTask());
    mConnectTask.Add(new MGCLConnectTask(MGCLConnectTask::kServerGame));
    mConnectTask.Add(new MGCLSendServerReportTask());
    mConnectTask.Add(new NetHostTask());
    mConnectTask.Add(new MGCLCreateGameTask());
    mExitTask.Add(new NetDisconnectTask());
    mExitTask.Add(new MGCLSessionEndTask());
    mExitTask.Add(new MGCLDisconnectTask());
    mServerReport.SetTask(new MGCLSendServerReportTask());
    mWorldReport.SetTask(new MediusSendWorldReportTask());
    mShareTask.Add(new WaitForShareDoneTask());
    mShareTask.Add(new LoadGameTask());
    mConnectTask.Start(pScreen);
}

HostpadRT::~HostpadRT() {
    TheHostpadRT = nullptr;
    mShareTask.Stop();
}

void HostpadRT::EndEdit() {
    mRemixShared = 0;
    CopyGameParams(mParams);
    if (mParams.mLoadRemix == 0) {
        mParams.mRemixName.Clear();
    }
    NetMultiTask *pTask = new NetMultiTask();
    // Yes, the binary adds these steps without making the request their parent.
    pTask->SerialTasks::Add(
        new TextFilterTask(&mParams.mRemixName, &mParams.mRemixName, kFilterRemixName));
    pTask->SerialTasks::Add(new FinishUpdateGameParamsTask());
    mTasks.Add(pTask);
}

void HostpadRT::SendGameParams() {
    {
        GameParamsPkt pkt(mParams);
        Send(&pkt);
    }
    mWorldReport.RunNow();
}

bool HostpadRT::ShareRemix() {
    return StartShare(1);
}

bool HostpadRT::StartShare(int nShareOnly) {
    if (mSharing != 0) {
        return false;
    }
    for (const auto &player : mPlayers) {
        if (player.mReady != kReadyYes) {
            return false;
        }
    }
    if (mRemixShared != 0) {
        return false;
    }

    mShareOnly = nShareOnly;
    mRecipients.clear();
    mAwaiting.clear();
    for (auto &player : mPlayers) {
        if (player.mId == mLocalId) {
            continue;
        }
        mRecipients.push_back(player.mId);
        if (mShareOnly != 0) {
            player.mReady = kReadyNo;
        }
    }
    PlayersChanged();
    if (mRecipients.empty()) {
        return false;
    }

    mSharing = 1;
    const RemixInfo *pInfo = TheGameDb->GetRemixInfo();
    mBlock = 0;
    mBlockCount =
        (pInfo->mDataSize + DataBlockPkt::kMaxBlockSize - 1) / DataBlockPkt::kMaxBlockSize;
    (void)(TheGameDb->GetRemixName() == pInfo->mName); // Yes, the binary discards this comparison.

    ShareRemixPkt pkt(*pInfo, mShareOnly);
    strncpy(pkt.mInfo.mName, mParams.mRemixName.c_str(), kRemixInfoNameSize - 1);
    Send(&pkt);
    ShareRemixProgressMsg msg(0.0f, mShareOnly);
    mGameSink->Dispatch(&msg);
    return true;
}

int HostpadRT::Launch() {
    for (const auto &player : mPlayers) {
        if (player.mReady != kReadyYes) {
            return 0;
        }
    }
    mStarting = 1;
    mInGame = 1;
    ReportNow();
    mSyncDeadline = PeriodicTask::kNever;
    if (mParams.mLoadRemix != 0) {
        StartShare(0);
        mShareTask.Start(nullptr);
        return 1;
    }
    LoadGameTask task;
    task.Start();
    return 1;
}

void HostpadRT::BootPlayer(int nPlayer) {
    Kick(nPlayer, 1);
}

void HostpadRT::Kick(int nId, int nBan) {
    if (nBan != 0) {
        const NetLaunchpadPlayer *pPlayer = FindPlayer(nId);
        if (pPlayer != nullptr) {
            BanPlayerTask task(mWorldId, pPlayer->mPlayer.mId);
            task.Start();
            task.Stop();
        }
    }
    if (nId == mLocalId) {
        return;
    }
    {
        BootPkt pkt;
        pkt.mPlayer = nId;
        Send(&pkt);
    }
    RemovePlayer(nId);
}

void HostpadRT::SendScores(const std::vector<NetGameScore> &scores) {
    {
        GameOverPkt pkt(scores);
        Send(&pkt);
    }
    RateGame(scores);
    EndGame();
}

void HostpadRT::EndGame() {
    LaunchpadRT::EndGame();
    ReportNow();
}

void HostpadRT::PollPriv() {
    if (mWorldId != kNoWorld) {
        (void)mServerReport.Poll();
        (void)mWorldReport.Poll();
        (void)mPlayerReport.Poll();
    }
    (void)mShareTask.Poll();
    if (IsMgclReady() != 0 && rtMgclUpdate() != 0) {
        mLost = 1;
        if (mExitTask.GetState() != Task::kStateRunning) {
            Exit(mGameSink, LaunchpadAbortedMsg::kReasonLost);
        }
    }
    PollStart();
    PollShare();
}

void HostpadRT::PollStart() {
    if (mSyncDeadline == 0.0f) {
        return;
    }
    const float fNow = SystemMs();
    if (mSyncDeadline < fNow) {
        KickUnloaded();
    }
    for (const auto &player : mPlayers) {
        if (player.mReady != kReadyLoaded) {
            return;
        }
        if (player.mId == kHostId && mSyncDeadline == PeriodicTask::kNever) {
            mSyncDeadline = fNow + kLoadWaitMs;
        }
    }

#ifndef VIDEO_STANDARD_PAL
    mSyncDeadline = 0.0f;
#endif
    const float fStart = GetTime() + kStartDelayMs;
    StartGamePkt pkt;
    pkt.mTime = fStart;
    pkt.mSeed = RandomInt(0, kSeedRange);
    Send(&pkt);
    Dispatch(&pkt);
}

void HostpadRT::KickUnloaded() {
    auto it = mPlayers.begin();
    while (it != mPlayers.end()) {
        if (it->mReady == kReadyLoaded) {
            ++it;
        } else {
            Kick(it->mId, 0);
            it = mPlayers.begin();
        }
    }
}

void HostpadRT::PollShare() {
    if (mSharing == 0) {
        return;
    }
    if (!mAwaiting.empty()) {
        if (SystemMs() < mBlockDeadline) {
            return;
        }
        while (mAwaiting.begin() != mAwaiting.end()) {
            Kick(*mAwaiting.begin(), 0);
        }
    }
    if (mRecipients.empty()) {
        mBlock = mBlockCount;
    }

    float fProgress;
    if (mBlock == mBlockCount) {
        mRemixShared = 1;
        mSharing = 0;
        fProgress = 1.0f;
    } else {
        const float fNow = SystemMs();
        fProgress = static_cast<float>(mBlock) / static_cast<float>(mBlockCount);
        float fNext = static_cast<float>(mBlock + 1) / static_cast<float>(mBlockCount);
        mBlockDeadline = fNow + kBlockWaitMs;
        if (mBlock + 1 == mBlockCount) {
            fNext = 1.0f;
        }
        mAwaiting = mRecipients;

        const int nOffset = mBlock * DataBlockPkt::kMaxBlockSize;
        const RemixInfo *pInfo = TheGameDb->GetRemixInfo();
        const int nEnd = nOffset + DataBlockPkt::kMaxBlockSize;
        const int nSize = std::min(nEnd, pInfo->mDataSize) - nOffset;
        DataBlockPkt pkt(TheGameDb->GetRemixBuffer() + nOffset,
                         static_cast<short>(mBlock),
                         static_cast<short>(nSize),
                         fNext);
        Send(&pkt);
        ++mBlock;
    }

    ShareRemixProgressMsg msg(fProgress, mShareOnly);
    mGameSink->Dispatch(&msg);
}

void HostpadRT::ReportNow() {
    LaunchpadRT::ReportNow();
    if (mWorldId != kNoWorld) {
        mServerReport.RunNow();
        mWorldReport.RunNow();
    }
}

void HostpadRT::SetLocalId(int nId) {
    LaunchpadRT::SetLocalId(nId);
    const int nColor = AssignColor(nId);
    const NetLaunchpadPlayer player = {TheLobbyRT->GetLocalPlayer(), nId, nColor, kReadyNo};
    AddPlayer(player);
    mClientData.push_back(ClientData(player.mId, player.mPlayer.mRank, player.mPlayer.mRating));
    mState = kStateSynced;
}

void HostpadRT::AddClient(int nId) {
    LaunchpadRT::AddClient(nId);
    if (mInGame != 0 || mSharing != 0) {
        JoinFailedPkt pkt(nId, LaunchpadAbortedMsg::kReasonInProgress);
        Send(&pkt);
        return;
    }
    if (mPlayers.size() < static_cast<unsigned int>(mParams.mMaxPlayers)) {
        const int nColor = AssignColor(nId);
        if (nColor != kNoColor) {
            mRemixShared = 0;
            WelcomePkt pkt(nId, nColor, mParams);
            Send(&pkt);
            return;
        }
    }
    JoinFailedPkt pkt(nId, LaunchpadAbortedMsg::kReasonFull);
    Send(&pkt);
}

void HostpadRT::RemovePlayer(int nId) {
    FreeColor(nId);
    VectorRemove(mRecipients, nId);
    VectorRemove(mAwaiting, nId);
    VectorRemove(mClientData, ClientData(nId, 0, 0));
    ReportNow();
    LaunchpadRT::RemovePlayer(nId);
}

int HostpadRT::CanJoin() {
    if (mInGame != 0 || mSharing != 0) {
        return 0;
    }
    return mPlayers.size() < static_cast<unsigned int>(mParams.mMaxPlayers);
}

int HostpadRT::GetWorldStatus() {
    return mInGame == 0 ? kWorldStatusOpen : kWorldStatusPlaying;
}

int HostpadRT::AssignColor(int nId) {
    for (auto &slot : mColors) {
        if (slot.mPlayer == kNoId) {
            slot.mPlayer = nId;
            return slot.mColor;
        }
    }
    return kNoColor;
}

void HostpadRT::FreeColor(int nId) {
    for (auto &slot : mColors) {
        if (slot.mPlayer == nId) {
            slot.mPlayer = kNoId;
        }
    }
}

void HostpadRT::OnConnected() {
    HostLaunchpadSuccessMsg msg(*GetLocalPlayer());
    mConnectTask.Notify(&msg);
}

float HostpadRT::GetTime() {
    return SystemMs();
}

void HostpadRT::SendRanks() {
    std::sort(mClientData.begin(), mClientData.end(), ClientData::HigherRating);
    std::vector<UpdateRanksPkt::Info> ranks;
    int nRank = -1;
    for (const auto &data : mClientData) {
        nRank = nRank < data.mRank ? data.mRank : nRank + 1;
        ranks.push_back(UpdateRanksPkt::Info{data.mId, nRank});
    }
    UpdateRanksPkt pkt(ranks);
    Send(&pkt);
    Dispatch(&pkt);
}

void HostpadRT::SendRating(int nRank) {
    LaunchpadRT::SendRating(nRank);
    ClientData *pData = VectorFind(mClientData, ClientData(mLocalId, 0, 0));
    if (pData != nullptr) {
        pData->mRank = nRank;
        pData->mRating = GetLocalPlayer()->mPlayer.mRating;
    }
}

bool HostpadRT::OnMsg(ClientInfoPkt *pPkt) {
    if (CanJoin() != 0) {
        AddPlayer(pPkt->mPlayer);
        mClientData.push_back(ClientData(
            pPkt->mPlayer.mId, pPkt->mPlayer.mPlayer.mRank, pPkt->mPlayer.mPlayer.mRating));
        ReportNow();
    } else {
        Kick(pPkt->mPlayer.mId, 0);
    }
    return false;
}

bool HostpadRT::OnMsg(TimeRequestPkt *pPkt) {
    const float fNow = SystemMs();
    TimeResponsePkt pkt(pPkt->mSender, fNow, pPkt->mSendTime);
    Send(&pkt);
    return false;
}

bool HostpadRT::OnMsg(DataAckPkt *pPkt) {
    VectorRemove(mAwaiting, pPkt->mSender);
    return false;
}

bool HostpadRT::OnMsg(ChatMsg *pMsg) {
    char szKey[kKeySize * 2 + 1];
    for (int i = 0; i < kKeySize; ++i) {
        EncodeByte(&szKey[i * 2], mKey[i]);
    }
    ChatMsg::SendPrivate(kKeyChannel, pMsg->mAccountId, szKey);
    return false;
}

bool HostpadRT::OnMsg(ClientRatingPkt *pPkt) {
    LaunchpadRT::OnMsg(pPkt);
    ClientData *pData = VectorFind(mClientData, ClientData(pPkt->mSender, 0, 0));
    if (pData != nullptr) {
        pData->mRating = pPkt->mRating;
        pData->mRank = pPkt->mRank;
        SendRanks();
    }
    return false;
}

bool HostpadRT::DispatchPriv(Message *pMsg) {
    const int nType = pMsg->Type();
    if (nType == g_nClientInfoPktType) {
        return OnMsg(static_cast<ClientInfoPkt *>(pMsg));
    }
    if (nType == g_nTimeRequestPktType) {
        return OnMsg(static_cast<TimeRequestPkt *>(pMsg));
    }
    if (nType == g_nDataAckPktType) {
        return OnMsg(static_cast<DataAckPkt *>(pMsg));
    }
    if (nType == g_nChatMsgType) {
        return OnMsg(static_cast<ChatMsg *>(pMsg));
    }
    if (nType == g_nClientRatingPktType) {
        return OnMsg(static_cast<ClientRatingPkt *>(pMsg));
    }
    return LaunchpadRT::DispatchPriv(pMsg);
}
