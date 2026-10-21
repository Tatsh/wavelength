#include "netflow/joinpadrt.h"

#include <cstring>

#include "game/gamedb.h"
#include "msg/bootpkt.h"
#include "msg/chatmsg.h"
#include "msg/clientinfopkt.h"
#include "msg/dataackpkt.h"
#include "msg/datablockpkt.h"
#include "msg/gameendedmsg.h"
#include "msg/gameoverpkt.h"
#include "msg/gameparamspkt.h"
#include "msg/gameparamsupdatemsg.h"
#include "msg/joinfailedpkt.h"
#include "msg/joinlaunchpadsuccessmsg.h"
#include "msg/launchpadabortedmsg.h"
#include "msg/shareremixbeginmsg.h"
#include "msg/shareremixpkt.h"
#include "msg/shareremixprogressmsg.h"
#include "msg/timeresponsepkt.h"
#include "msg/welcomepkt.h"
#include "netflow/clocksynctask.h"
#include "netflow/ladderpositiontask.h"
#include "netflow/lobbyrt.h"
#include "netflow/mediusjoingametask.h"
#include "netflow/netconnecttask.h"
#include "netflow/netcrypt.h"
#include "netflow/netdisconnecttask.h"
#include "netflow/transportrt.h"
#include "netflow/waitforkeytask.h"
#include "netflow/waitforwelcometask.h"
#include "os/system.h"

namespace {

// The text that asks the host for the key of the session.
constexpr char kKeyRequest[] = "req";

#ifdef VIDEO_STANDARD_PAL
// The milliseconds the host may take to start the game once this console loads.
constexpr float kStartWaitMs = 20000.0f;
#endif

} // namespace

JoinpadRT *TheJoinpadRT;

void JoinpadRT::Create(MsgSink *pScreen, MsgSink *pGame, int nWorldId, int nHostAccount) {
    TheLaunchpadRT = std::make_unique<JoinpadRT>(pScreen, pGame, nWorldId, nHostAccount);
}

JoinpadRT::JoinpadRT(MsgSink *pScreen, MsgSink *pGame, int nWorldId, int nHostAccount)
    : LaunchpadRT(pGame), mKeyReceived(0), mClockOffset(0.0f) {
    TheJoinpadRT = this;
    RequestKey(nHostAccount);
    mConnectTask.Add(new LadderPositionTask(&TheLobbyRT->GetLocalPlayer()));
    mConnectTask.Add(new MediusJoinGameTask());
    mConnectTask.Add(new WaitForKeyTask());
    mConnectTask.Add(new NetConnectTask());
    mConnectTask.Add(new WaitForWelcomeTask());
    mExitTask.Add(new NetDisconnectTask());
    mWorldId = nWorldId;
    mClockSync = std::make_unique<ClockSyncTask>();
    mConnectTask.Start(pScreen);
}

JoinpadRT::~JoinpadRT() {
    mClockSync->Stop();
    TheJoinpadRT = nullptr;
}

void JoinpadRT::PollPriv() {
    (void)mClockSync->Poll();
#ifdef VIDEO_STANDARD_PAL
    PollStart();
#endif
}

#ifdef VIDEO_STANDARD_PAL
void JoinpadRT::NotifyLoaded() {
    LaunchpadRT::NotifyLoaded();
    mSyncDeadline = SystemMs() + kStartWaitMs;
}

void JoinpadRT::PollStart() {
    if (mSyncDeadline == 0.0f) {
        return;
    }
    if (mSyncDeadline < SystemMs()) {
        Exit(mGameSink, LaunchpadAbortedMsg::kReasonHost);
        mSyncDeadline = 0.0f;
    }
}
#endif

void JoinpadRT::RemovePlayer(int nId) {
    LaunchpadRT::RemovePlayer(nId);
    if (nId == kHostId) {
        Exit(mGameSink, LaunchpadAbortedMsg::kReasonHost);
    }
}

void JoinpadRT::OnConnected() {
    const NetLaunchpadPlayer *pPlayer = GetLocalPlayer();
    JoinLaunchpadSuccessMsg msg(mParams, *pPlayer);
    mConnectTask.Notify(&msg);
}

float JoinpadRT::GetTime() {
    return SystemMs() + mClockOffset;
}

void JoinpadRT::RequestKey(int nHostAccount) {
    mKeyReceived = 0;
    ChatMsg::SendPrivate(kKeyChannel, nHostAccount, kKeyRequest);
}

void JoinpadRT::OnClockSynced(float fOffset, float fRoundTrip) {
    (void)fRoundTrip;
    mClockOffset = fOffset;
    mState = kStateSynced;
    CheckReady();
}

bool JoinpadRT::OnMsg(WelcomePkt *pPkt) {
    if (!TheGameDb->HasSong(pPkt->mParams.mSong.c_str())) {
        mError = LaunchpadAbortedMsg::kReasonIncompatible;
        mState = kStateFailed;
        return false;
    }
    mParams = pPkt->mParams;
    const NetLaunchpadPlayer player = {
        TheLobbyRT->GetLocalPlayer(), mLocalId, pPkt->mColor, kReadyNo};
    AddPlayer(player);
    mState = kStateWelcomed;
    mClockSync->Start();
    return false;
}

bool JoinpadRT::OnMsg(JoinFailedPkt *pPkt) {
    mState = kStateFailed;
    mError = pPkt->mReason;
    return false;
}

bool JoinpadRT::OnMsg(ClientInfoPkt *pPkt) {
    AddPlayer(pPkt->mPlayer);
    return false;
}

bool JoinpadRT::OnMsg(TimeResponsePkt *pPkt) {
    mClockSync->AddSample(pPkt->mRequestTime, pPkt->mSendTime);
    return false;
}

bool JoinpadRT::OnMsg(GameParamsPkt *pPkt) {
    if (TheGameDb->HasSong(pPkt->mParams.mSong.c_str())) {
        GameParamsUpdateMsg msg(pPkt->mParams);
        mGameSink->Dispatch(&msg);
    } else {
        Exit(mGameSink, LaunchpadAbortedMsg::kReasonIncompatible);
    }
    return false;
}

bool JoinpadRT::OnMsg(BootPkt *pPkt) {
    if (pPkt->mPlayer == mLocalId) {
        Exit(mGameSink, LaunchpadAbortedMsg::kReasonBoot);
    } else {
        RemovePlayer(pPkt->mPlayer);
    }
    return false;
}

bool JoinpadRT::OnMsg(GameOverPkt *pPkt) {
    if (mInGame == 0) {
        return false;
    }
    RateGame(pPkt->mScores);
    {
        GameEndedMsg msg(GameEndedMsg::kResultFinished, pPkt->mScores);
        TheTransportRT->Notify(&msg);
    }
    EndGame();
    return false;
}

bool JoinpadRT::OnMsg(ShareRemixPkt *pPkt) {
    mRemixInfo = pPkt->mInfo;
    mShareOnly = pPkt->mShareOnly;
    if (mShareOnly != 0) {
        SetReady(kReadyNo);
    }
    TheGameDb->SetRemixBuffer(mRemixInfo.mDataSize);
    ShareRemixBeginMsg msg(pPkt->mInfo);
    mGameSink->Dispatch(&msg);
    return false;
}

bool JoinpadRT::OnMsg(DataBlockPkt *pPkt) {
    memcpy(TheGameDb->GetRemixBuffer() + pPkt->mBlock * DataBlockPkt::kMaxBlockSize,
           pPkt->mBuffer,
           pPkt->mSize);
    {
        DataAckPkt pkt;
        Send(&pkt);
    }
    if (pPkt->mProgress == 1.0f) {
        TheGameDb->SetRemix(&mRemixInfo);
    }
    ShareRemixProgressMsg msg(pPkt->mProgress, mShareOnly);
    mGameSink->Dispatch(&msg);
    return false;
}

bool JoinpadRT::OnMsg(ChatMsg *pMsg) {
    unsigned char key[kKeySize];
    const char *pszText = pMsg->mText;
    for (int i = 0; i < kKeySize; ++i) {
        key[i] = DecodeByte(pszText);
        pszText += 2;
    }
    mKeyReceived = 1;
    NetCryptSetKey(key);
    return false;
}

bool JoinpadRT::DispatchPriv(Message *pMsg) {
    const int nType = pMsg->Type();
    if (nType == g_nWelcomePktType) {
        return OnMsg(static_cast<WelcomePkt *>(pMsg));
    }
    if (nType == g_nJoinFailedPktType) {
        return OnMsg(static_cast<JoinFailedPkt *>(pMsg));
    }
    if (nType == g_nClientInfoPktType) {
        return OnMsg(static_cast<ClientInfoPkt *>(pMsg));
    }
    if (nType == g_nTimeResponsePktType) {
        return OnMsg(static_cast<TimeResponsePkt *>(pMsg));
    }
    if (nType == g_nGameParamsPktType) {
        return OnMsg(static_cast<GameParamsPkt *>(pMsg));
    }
    if (nType == g_nBootPktType) {
        return OnMsg(static_cast<BootPkt *>(pMsg));
    }
    if (nType == g_nGameOverPktType) {
        return OnMsg(static_cast<GameOverPkt *>(pMsg));
    }
    if (nType == g_nShareRemixPktType) {
        return OnMsg(static_cast<ShareRemixPkt *>(pMsg));
    }
    if (nType == g_nDataBlockPktType) {
        return OnMsg(static_cast<DataBlockPkt *>(pMsg));
    }
    if (nType == g_nChatMsgType) {
        return OnMsg(static_cast<ChatMsg *>(pMsg));
    }
    return LaunchpadRT::DispatchPriv(pMsg);
}
