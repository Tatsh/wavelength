#include "netflow/launchpadrt.h"

#include <cstring>
#include <memory>

#include "game/gamedb.h"
#include "math/rand.h"
#include "msg/chatmsg.h"
#include "msg/clientabortedpkt.h"
#include "msg/clientinfopkt.h"
#include "msg/clientratingpkt.h"
#include "msg/clientstatuspkt.h"
#include "msg/gameendedmsg.h"
#include "msg/launchpadabortedmsg.h"
#include "msg/loadgamemsg.h"
#include "msg/loadgamepkt.h"
#include "msg/startgamemsg.h"
#include "msg/startgamepkt.h"
#include "msg/updaterankspkt.h"
#include "netflow/hostpadrt.h"
#include "netflow/ladderpositionfasttask.h"
#include "netflow/lobbyrt.h"
#include "netflow/mediussendplayerreporttask.h"
#include "netflow/netcrypt.h"
#include "netflow/transportrt.h"
#include "os/bufstream.h"
#include "os/debug.h"

namespace {

// The wait between two player reports, in milliseconds.
constexpr float kReportPeriodMs = 30000.0f;

// The players a session takes, which mRatings reserves.
constexpr int kMaxPlayers = 4;

// The size of a block of the cipher, which a sent packet is padded to.
constexpr int kCipherBlockSize = 8;

// The one field of the message type of the session, a byte array of the largest message.
constexpr int kMessageFieldSize = 1;
constexpr int kMaxMessageSize = 480;

// The bytes of the length that precedes a message.
constexpr int kLengthSize = sizeof(unsigned short);

// The digits that encode one half of a byte.
constexpr char kDigitBase = '0';
constexpr int kNibbleBits = 4;
constexpr unsigned char kNibbleMask = 0xf;

// The scores StartGame() rates a game with before it ends. This console loses to every other.
constexpr int kStartScoreLocal = 1;
constexpr int kStartScoreOther = 2;

// The score DropRating() gives a player who left.
constexpr int kLeftScore = -1;

// NTSC-U/C: 0x0043e380
char g_sendBuffer[RT_DME_MESSAGE_DATA_SIZE];

} // namespace

// NTSC-U/C: 0x003b0abc, PAL: 0x0042f988
int LaunchpadRT::sMessageType;

// NTSC-U/C: 0x003b0ac0, PAL: 0x0042f98c
int LaunchpadRT::sMgclReady;

// NTSC-U/C: 0x003b0ad4, PAL: 0x0042f9a0
int LaunchpadRT::sPlayersVersion = 1;

// NTSC-U/C: 0x003b0ae0, PAL: 0x0042f9ac
NetTask *LaunchpadRT::sNetTask;

std::unique_ptr<LaunchpadRT> TheLaunchpadRT;

NetLaunchpad *TheNetLaunchpad;

int LaunchpadRT::RegisterMessages() {
    const int nError = rtDmeBeginFields();
    if (nError != 0) {
        return nError;
    }
    (void)rtDmeAddField(nullptr, nullptr, kMessageFieldSize, kMaxMessageSize);
    return rtDmeRegisterMessage(&sMessageType, Receive);
}

void LaunchpadRT::SetMgclReady(int nReady) {
    sMgclReady = nReady;
}

int LaunchpadRT::IsMgclReady() {
    return sMgclReady;
}

unsigned char LaunchpadRT::DecodeByte(const char *pszText) {
    return static_cast<unsigned char>(((pszText[0] - kDigitBase) << kNibbleBits) |
                                      (pszText[1] - kDigitBase));
}

void LaunchpadRT::EncodeByte(char *pszText, unsigned char nByte) {
    pszText[2] = '\0';
    pszText[1] = static_cast<char>((nByte & kNibbleMask) + kDigitBase);
    pszText[0] = static_cast<char>((nByte >> kNibbleBits) + kDigitBase);
}

LaunchpadRT::LaunchpadRT(MsgSink *pGame)
    : mWorldId(kNoWorld), mConnection(kNoWorld), mLocalId(kNoId), mState(kStateConnecting),
      mError(0), mGameSink(pGame), mPlayerReport(kReportPeriodMs), mStartTime(0.0f), mSeed(0),
      mActive(0), mShareOnly(0), mStarting(0), mInGame(0) {
    TheNetLaunchpad = this;
    mPlayerReport.SetTask(new MediusSendPlayerReportTask());
    mRatings.reserve(kMaxPlayers);
    ChatMsg::AddHandler(kKeyChannel, this);
    mRankTask.Add(new LadderPositionFastTask(&TheLobbyRT->GetLocalPlayer()));
}

LaunchpadRT::~LaunchpadRT() {
    mConnectTask.Stop();
    mExitTask.Stop();
    mRankTask.Stop();
    TheNetLaunchpad = nullptr;
    ChatMsg::RemoveSink(kKeyChannel);
}

void LaunchpadRT::Cancel(MsgSink *pSink) {
    Exit(pSink, LaunchpadAbortedMsg::kReasonNone);
}

void LaunchpadRT::Activate() {
    mActive = 1;
    CheckReady();
}

void LaunchpadRT::NotifyLoaded() {
    (void)GetLocalPlayer(); // Yes, the binary discards this call's result.
    SetReady(kReadyLoaded);
}

int LaunchpadRT::IsGuest() {
    return mLocalId;
}

int LaunchpadRT::GetPlayersVersion() {
    return sPlayersVersion;
}

void LaunchpadRT::SetReady(int nReady) {
    GetLocalPlayer()->mReady = nReady;
    ClientStatusPkt pkt(nReady);
    Send(&pkt);
    PlayersChanged();
}

void LaunchpadRT::SendRating(int nRank) {
    const NetLaunchpadPlayer *pPlayer = GetLocalPlayer();
    ClientRatingPkt pkt(pPlayer->mPlayer.GetRating(),
                        nRank,
                        pPlayer->mPlayer.GetGames(),
                        pPlayer->mPlayer.GetRecord());
    Send(&pkt);
}

int LaunchpadRT::Poll() {
    const int nConnectState = mConnectTask.Poll();
    if (nConnectState == Task::kStateFailed) {
        mConnectTask.Stop();
        Exit(mConnectTask.GetSink(), mConnectTask.GetResult());
    } else if (nConnectState == Task::kStateDone) {
        OnConnected();
        mConnectTask.Stop();
        ReportNow();
    }

    if (mExitTask.Poll() == Task::kStateDone) {
        LaunchpadAbortedMsg msg(mExitTask.GetResult());
        mExitTask.Notify(&msg);
        mExitTask.Stop();
        return 1;
    }

    if (mRankTask.Poll() == Task::kStateDone) {
        SendRating(TheLobbyRT->GetLocalPlayer().GetRank());
        mRankTask.Stop();
    }

    mTasks.Poll();

    if (!mClients.empty() && mState >= kStateWelcomed) {
        const NetLaunchpadPlayer *pPlayer = GetLocalPlayer();
        for (const int nClient : mClients) {
            ClientInfoPkt pkt(*pPlayer, nClient);
            Send(&pkt);
        }
        mClients.clear();
    }

    if (mStartTime != 0.0f && mStartTime <= GetTime()) {
        mStartTime = 0.0f;
        StartGame();
    }

    if (mWorldId != kNoWorld) {
        (void)mPlayerReport.Poll();
    }
    PollPriv();
    return 0;
}

void LaunchpadRT::ReportNow() {
    if (mWorldId != kNoWorld) {
        mPlayerReport.RunNow();
    }
}

void LaunchpadRT::SetLocalId(int nId) {
    mLocalId = nId;
    mState = kStateConnected;
}

void LaunchpadRT::AddClient(int nId) {
    mClients.push_back(nId);
}

void LaunchpadRT::RemovePlayer(int nId) {
    bool bFound = false;
    for (auto it = mPlayers.begin(); it != mPlayers.end(); ++it) {
        if (it->mId == nId) {
            TheTransportRT->PlayerAborted(nId);
            mPlayers.erase(it);
            bFound = true;
            break;
        }
    }
    if (bFound) {
        DropRating(nId);
        PlayersChanged();
    }
}

NetLaunchpadPlayer *LaunchpadRT::FindPlayer(int nId) {
    for (auto &player : mPlayers) {
        if (player.mId == nId) {
            return &player;
        }
    }
    return nullptr;
}

NetLaunchpadPlayer *LaunchpadRT::GetLocalPlayer() {
    return FindPlayer(mLocalId);
}

void LaunchpadRT::AddPlayer(const NetLaunchpadPlayer &player) {
    auto it = mPlayers.begin();
    while (it != mPlayers.end() && !(player.mColor < it->mColor)) {
        ++it;
    }
    mPlayers.insert(it, player);
    PlayersChanged();
}

void LaunchpadRT::StartGame() {
    mStarting = 0;
    StartGameMsg msg(mSeed);
    TheTransportRT->Notify(&msg);

    if (TheGameDb->GetRuleSet() != GameDb::kRuleSetGame || mPlayers.size() == 1) {
        mRatings.clear();
        return;
    }

    const RatingStat empty = {};
    mRatings.resize(mPlayers.size(), empty);
    unsigned int nIndex = 0;
    for (auto &player : mPlayers) {
        RatingStat &stat = mRatings[nIndex];
        stat.Set(&player);
        stat.SetScore(player.mId == mLocalId ? kStartScoreLocal : kStartScoreOther);
        if (player.mId == mLocalId) {
            player.mPlayer.SetGames(player.mPlayer.GetGames() + 1);
        }
        ++nIndex;
    }
    UpdateRating();
}

void LaunchpadRT::DropRating(int nId) {
    if (mRatings.empty()) {
        return;
    }
    for (auto it = mRatings.begin(); it != mRatings.end();) {
        if (it->GetId() == nId) {
            it->SetScore(kLeftScore);
        }
        if (nId == kHostId && it->GetId() != mLocalId && it->GetId() != kHostId) {
            it = mRatings.erase(it);
        } else {
            ++it;
        }
    }
    UpdateRating();
}

int LaunchpadRT::FindScore(const std::vector<NetGameScore> &scores, int nId) {
    for (const auto &score : scores) {
        if (score.mNetOrder == nId) {
            return score.mScore;
        }
    }
    return 0;
}

void LaunchpadRT::RateGame(const std::vector<NetGameScore> &scores) {
    if (mRatings.empty()) {
        return;
    }
    for (unsigned int i = 0; i < mRatings.size(); ++i) {
        mRatings[i].SetScore(FindScore(scores, mRatings[i].GetId()));
    }
    UpdateRating();
    mRatings.clear();
}

void LaunchpadRT::UpdateRating() {
    RatingStat stat;
    stat.Rate(mLocalId, &mRatings);
    NetLaunchpadPlayer *pPlayer = GetLocalPlayer();
    pPlayer->mPlayer.SetRating(&stat);
    TheLobbyRT->UpdateStats(&pPlayer->mPlayer);
}

void LaunchpadRT::Exit(MsgSink *pSink, int nReason) {
    mConnectTask.Stop();
    {
        GameEndedMsg msg(nReason == LaunchpadAbortedMsg::kReasonBoot ?
                             GameEndedMsg::kResultBooted :
                             GameEndedMsg::kResultConnectionLost);
        TheTransportRT->Notify(&msg);
    }
    TheTransportRT->SetOpen(false);
    mExitTask.SetResult(nReason);
    mExitTask.Start(pSink);
}

int LaunchpadRT::FindIdByAccount(int nAccountId) {
    for (const auto &player : mPlayers) {
        if (player.mPlayer.GetId() == nAccountId) {
            return player.mId;
        }
    }
    return kNoId;
}

void LaunchpadRT::Abort(int nReason) {
    Exit(mGameSink, nReason);
}

void LaunchpadRT::Send(Packet *pPacket) {
    memset(g_sendBuffer, 0, sizeof(g_sendBuffer));
    BufStream stream(g_sendBuffer, sizeof(g_sendBuffer), true);
    const unsigned char nSalt = static_cast<unsigned char>(RandomWord());
    stream.Write(&nSalt, sizeof(nSalt));
    stream << pPacket;
    const int nSize = ((stream.Tell() - 1) / kCipherBlockSize + 1) * kCipherBlockSize;
    NetCryptEncrypt(g_sendBuffer, nSize);
    stream.Seek(0, BinStream::kSeekBegin);

    rtDmeMessage message;
    stream.Read(message.data, nSize);
    message.length = static_cast<unsigned short>(nSize);
    int nFlags = (pPacket->GetFlags() & Packet::kFlagOrdered) != 0 ?
                     RT_DME_SEND_BASE | RT_DME_SEND_ORDERED :
                     RT_DME_SEND_BASE;
    if ((pPacket->GetFlags() & Packet::kFlagGuaranteed) != 0) {
        nFlags |= RT_DME_SEND_GUARANTEED;
    }
    rtDmeSend(static_cast<signed char>(nFlags),
              mConnection,
              pPacket->GetReceiver(),
              sMessageType,
              static_cast<unsigned short>(nSize + kLengthSize),
              &message);
}

int LaunchpadRT::Receive(int nConnection, int nType, int nSender, rtDmeMessage *pMessage) {
    (void)nConnection;
    (void)nType;
    if (TheLaunchpadRT == nullptr) {
        return pMessage->length + kLengthSize;
    }

    NetCryptDecrypt(pMessage->data, pMessage->length);
    BufStream stream(pMessage->data, pMessage->length, true);
    unsigned char nSalt;
    stream.Read(&nSalt, sizeof(nSalt));
    Message *pRead = nullptr;
    stream >> pRead;
    std::unique_ptr<Message> pMsg(pRead);
    if (!stream.Fail()) {
        Packet *pPacket = dynamic_cast<Packet *>(pMsg.get());
        if (pPacket != nullptr) {
            pPacket->SetSender(nSender);
            const int nRoute = pPacket->Route();
            if ((nRoute & Packet::kRouteLaunchpad) != 0) {
                static_cast<MsgSink *>(TheLaunchpadRT.get())->Dispatch(pMsg.get());
            }
            if ((nRoute & Packet::kRouteTransport) != 0) {
                TheTransportRT->Receive(pPacket);
            }
        }
    }
    return pMessage->length + kLengthSize;
}

void LaunchpadRT::OnNetConnect(int nConnection, int nReserved1, int nReserved2, int nStatus) {
    (void)nReserved1;
    (void)nReserved2;
    if (sNetTask == nullptr) {
        return;
    }
    TheLaunchpadRT->SetConnection(nConnection);
    int nClient;
    if (nStatus == 0) {
        nStatus = rtDmeGetLocalClient(&nClient, nConnection);
    }
    int nReason = 0;
    bool bSucceeded = false;
    if (nStatus == 0) {
        bSucceeded = TheHostpadRT != nullptr ? nClient == kHostId : nClient != kHostId;
    }
    if (bSucceeded) {
        TheLaunchpadRT->SetLocalId(nClient);
        bSucceeded = rtDmeEnableConnection(TheLaunchpadRT->GetConnection(), 1) == 0;
    } else {
        nReason = TheHostpadRT != nullptr ? LaunchpadAbortedMsg::kReasonHostFailed :
                                            LaunchpadAbortedMsg::kReasonConnectFailed;
    }
    sNetTask->Finish(bSucceeded, nReason);
}

void LaunchpadRT::OnNetClientJoin(int nConnection, int nClient, const char *pszAddress) {
    (void)nConnection;
    (void)pszAddress;
    if (TheLaunchpadRT != nullptr) {
        TheLaunchpadRT->AddClient(nClient);
    }
}

void LaunchpadRT::OnNetClientLeave(int nConnection, int nClient, const char *pszAddress) {
    (void)nConnection;
    (void)pszAddress;
    if (TheLaunchpadRT != nullptr) {
        TheLaunchpadRT->RemovePlayer(nClient);
    }
}

void LaunchpadRT::Leave() {
    EndGame();
    ClientAbortedPkt pkt;
    Send(&pkt);
}

void LaunchpadRT::PlayersChanged() {
    ++sPlayersVersion;
}

void LaunchpadRT::CheckReady() {
    if (GetLocalPlayer()->mReady == kReadyNo && mActive != 0 && mState >= kStateSynced) {
        SetReady(kReadyYes);
    }
}

void LaunchpadRT::EndGame() {
    if (mInGame == 0) {
        return;
    }
    mInGame = 0;
    TheTransportRT->SetOpen(false);
    mRatings.clear();
    SetReady(kReadyNo);
    mActive = 0;
    mRankTask.Task::Start(); // Yes, the binary starts the chain without a sink.
}

bool LaunchpadRT::OnMsg(ClientStatusPkt *pPkt) {
    NetLaunchpadPlayer *pPlayer = FindPlayer(pPkt->GetSender());
    if (pPlayer != nullptr) {
        pPlayer->mReady = pPkt->GetReady();
        PlayersChanged();
    }
    return false;
}

bool LaunchpadRT::OnMsg(ClientRatingPkt *pPkt) {
    NetLaunchpadPlayer *pPlayer = FindPlayer(pPkt->GetSender());
    if (pPlayer != nullptr) {
        pPlayer->mPlayer.SetRating(pPkt->GetRating());
        pPlayer->mPlayer.SetGames(pPkt->GetGames());
        pPlayer->mPlayer.SetRecord(pPkt->GetRecord());
        PlayersChanged();
    }
    return false;
}

bool LaunchpadRT::OnMsg(LoadGamePkt *pPkt) {
    (void)pPkt;
    mInGame = 1;
    mStarting = 1;
    (void)GetLocalPlayer(); // Yes, the binary discards this call's result.
    SetReady(kReadyLoading);
    TheTransportRT->SetOpen(true);
    LoadGameMsg msg;
    mGameSink->Dispatch(&msg);
    return false;
}

bool LaunchpadRT::OnMsg(StartGamePkt *pPkt) {
#ifdef VIDEO_STANDARD_PAL
    mSyncDeadline = 0.0f;
#endif
    const float fNow = GetTime();
    if (pPkt->GetTime() < fNow) {
        DebugPrint("Net Problem: Starting late by %d milliseconds!\n",
                   static_cast<int>(fNow - pPkt->GetTime()));
    }
    mStartTime = pPkt->GetTime();
    mSeed = pPkt->GetSeed();
    return false;
}

bool LaunchpadRT::OnMsg(ClientAbortedPkt *pPkt) {
    const int nId = pPkt->GetSender();
    DropRating(nId);
    if (nId == kHostId) {
        {
            GameEndedMsg msg(GameEndedMsg::kResultHostAborted);
            TheTransportRT->Notify(&msg);
        }
        EndGame();
    } else {
        TheTransportRT->PlayerAborted(nId);
    }
    return false;
}

bool LaunchpadRT::OnMsg(UpdateRanksPkt *pPkt) {
    for (const auto &info : pPkt->GetRanks()) {
        NetLaunchpadPlayer *pPlayer = FindPlayer(info.mPlayer);
        if (pPlayer != nullptr) {
            pPlayer->mPlayer.SetRank(info.mRank);
        }
    }
    PlayersChanged();
    return false;
}

bool LaunchpadRT::DispatchPriv(Message *pMsg) {
    const int nType = pMsg->Type();
    if (nType == g_nClientStatusPktType) {
        return OnMsg(static_cast<ClientStatusPkt *>(pMsg));
    }
    if (nType == g_nClientRatingPktType) {
        return OnMsg(static_cast<ClientRatingPkt *>(pMsg));
    }
    if (nType == g_nLoadGamePktType) {
        return OnMsg(static_cast<LoadGamePkt *>(pMsg));
    }
    if (nType == g_nStartGamePktType) {
        return OnMsg(static_cast<StartGamePkt *>(pMsg));
    }
    if (nType == g_nClientAbortedPktType) {
        return OnMsg(static_cast<ClientAbortedPkt *>(pMsg));
    }
    if (nType == g_nUpdateRanksPktType) {
        return OnMsg(static_cast<UpdateRanksPkt *>(pMsg));
    }
    return false;
}
