#include "game/netgamescore.h"
#include "msg/bootpkt.h"
#include "msg/changepasswordresultmsg.h"
#include "msg/chatmsg.h"
#include "msg/clientabortedpkt.h"
#include "msg/clientinfopkt.h"
#include "msg/clientratingpkt.h"
#include "msg/clientstatuspkt.h"
#include "msg/createaccountresultmsg.h"
#include "msg/dataackpkt.h"
#include "msg/datablockpkt.h"
#include "msg/deleteaccountresultmsg.h"
#include "msg/findplayermsg.h"
#include "msg/gameendedmsg.h"
#include "msg/gameoverpkt.h"
#include "msg/gameparamspkt.h"
#include "msg/gameparamsupdatemsg.h"
#include "msg/hostlaunchpadsuccessmsg.h"
#include "msg/joinchatroomresultmsg.h"
#include "msg/joinfailedpkt.h"
#include "msg/joinlaunchpadsuccessmsg.h"
#include "msg/launchpadabortedmsg.h"
#include "msg/loadgamepkt.h"
#include "msg/lobbychatroomsmsg.h"
#include "msg/lobbyconnectresultmsg.h"
#include "msg/lobbylaunchpadsmsg.h"
#include "msg/lobbyplayersmsg.h"
#include "msg/loginresultmsg.h"
#include "msg/messagefactory.h"
#include "msg/playerabortedmsg.h"
#include "msg/playerrankmsg.h"
#include "msg/reporemixstatusmsg.h"
#include "msg/shareremixbeginmsg.h"
#include "msg/shareremixpkt.h"
#include "msg/shareremixprogressmsg.h"
#include "msg/startgamemsg.h"
#include "msg/startgamepkt.h"
#include "msg/timerequestpkt.h"
#include "msg/timeresponsepkt.h"
#include "msg/updaterankspkt.h"
#include "msg/welcomepkt.h"
#include "netflow/inetconfigsresultmsg.h"
#include "netflow/inetconnectresultmsg.h"
#include "netflow/inetconnectstatusmsg.h"
#include "netflow/inetdnslookupresultmsg.h"
#include "netflow/lobbymsgtypes.h"
#include "netflow/netlaunchpadinfo.h"
#include "netflow/netlaunchpadplayer.h"
#include "netflow/netreporemix.h"
#include "netflow/reporemixesmsg.h"

namespace {

// The colours of the player slots, and the value of no colour.
enum SlotColor {
    kColorNone = -1,
    kColorGreen = 0,
    kColorPurple = 1,
    kColorRed = 2,
    kColorYellow = 3,
};

// The states of a player of a session, as NetLaunchpadPlayer::mReady.
enum ReadyState {
    kReadyStateNotReady = 0,
    kReadyStateReady = 1,
    kReadyStateLoading = 2,
    kReadyStateDoneLoading = 3,
};

// The value LobbyPlayer's constructor gives its unidentified word and its rank icon.
constexpr int kUnsetField = -1;
constexpr int kNoRankIcon = -1;

// The amount a rating gains once its player leaves the first games behind.
constexpr unsigned int kVeteranRatingBonus = 1000000;

// The wins of a RatingRecord travel in half games. The factor is a hair above two to keep a
// result such as 1.5 from rounding down.
constexpr double kHalfGamesPerWin = 2.0001;
constexpr float kWinsPerHalfGame = 0.5f;

// The European release inserts the identity of DNASStatusMsg after LobbyConnectResultMsg, and
// every later identity is one higher.
#ifdef VIDEO_STANDARD_PAL
constexpr int kDNASTypeShift = 1;
#else
constexpr int kDNASTypeShift = 0;
#endif

// The identities the registrars below pass, the same values the Type() words hold.
constexpr int kWelcomePktType = 335 + kDNASTypeShift;
constexpr int kJoinFailedPktType = 336 + kDNASTypeShift;
constexpr int kClientInfoPktType = 337 + kDNASTypeShift;
constexpr int kClientStatusPktType = 338 + kDNASTypeShift;
constexpr int kClientRatingPktType = 339 + kDNASTypeShift;
constexpr int kTimeRequestPktType = 340 + kDNASTypeShift;
constexpr int kTimeResponsePktType = 341 + kDNASTypeShift;
constexpr int kGameParamsPktType = 342 + kDNASTypeShift;
constexpr int kBootPktType = 343 + kDNASTypeShift;
constexpr int kShareRemixPktType = 344 + kDNASTypeShift;
constexpr int kDataAckPktType = 345 + kDNASTypeShift;
constexpr int kDataBlockPktType = 346 + kDNASTypeShift;
constexpr int kLoadGamePktType = 347 + kDNASTypeShift;
constexpr int kStartGamePktType = 348 + kDNASTypeShift;
constexpr int kClientAbortedPktType = 349 + kDNASTypeShift;
constexpr int kGameOverPktType = 350 + kDNASTypeShift;
constexpr int kUpdateRanksPktType = 351 + kDNASTypeShift;

// NTSC-U/C: 0x00267ab8, PAL: 0x00271790
// Write the colour of a slot.
void PrintColor(PrnStream &stream, int nColor) {
    switch (nColor) {
    case kColorNone:
        stream << " no_clr";
        break;
    case kColorGreen:
        stream << " green";
        break;
    case kColorPurple:
        stream << " purple";
        break;
    case kColorRed:
        stream << " red";
        break;
    case kColorYellow:
        stream << " yellow";
        break;
    default:
        stream << " clr_err:" << nColor;
        break;
    }
}

// NTSC-U/C: 0x00267b98, PAL: 0x00271870
// Write the kind of a network connection. Another value writes nothing.
void PrintConnection(PrnStream &stream, int nConnection) {
    switch (nConnection) {
    case LobbyPlayer::kConnectionNone:
        stream << " none";
        break;
    case LobbyPlayer::kConnectionSlow:
        stream << " slow";
        break;
    case LobbyPlayer::kConnectionFast:
        stream << " fast";
        break;
    case LobbyPlayer::kConnectionOffline:
        stream << " offline";
        break;
    default:
        break;
    }
}

// NTSC-U/C: 0x00267c30, PAL: 0x00271908
// Write the state of a player of a session. Another value writes nothing.
void PrintReady(PrnStream &stream, int nReady) {
    switch (nReady) {
    case kReadyStateNotReady:
        stream << " not-ready";
        break;
    case kReadyStateReady:
        stream << " ready";
        break;
    case kReadyStateLoading:
        stream << " loading";
        break;
    case kReadyStateDoneLoading:
        stream << " done-loading";
        break;
    default:
        break;
    }
}

// Write one byte through the stream's own Write().
void WriteByte(BinStream &stream, int nValue) {
    const signed char cValue = static_cast<signed char>(nValue);
    stream.Write(&cValue, sizeof(cValue));
}

// Read one byte through the stream's own Read(), sign extended.
int ReadByte(BinStream &stream) {
    signed char cValue;
    stream.Read(&cValue, sizeof(cValue));
    return cValue;
}

} // namespace

// NTSC-U/C: 0x003b0b04, PAL: 0x0042f9d0
int g_nInetConfigsResultMsgType = 301;
// NTSC-U/C: 0x003b0b08, PAL: 0x0042f9d4
int g_nInetConnectStatusMsgType = 302;
// NTSC-U/C: 0x003b0b0c, PAL: 0x0042f9d8
int g_nInetConnectResultMsgType = 303;
// NTSC-U/C: 0x003b0b10, PAL: 0x0042f9dc
int g_nInetDisconnectResultMsgType = 304;
// NTSC-U/C: 0x003b0b14, PAL: 0x0042f9e0
int g_nInetDNSLookupResultMsgType = 305;
// NTSC-U/C: 0x003b0b18, PAL: 0x0042f9e4
int g_nInetCheckConnectionResultMsgType = 306;
// NTSC-U/C: 0x003b0b1c, PAL: 0x0042f9e8
int g_nLobbyConnectionLostMsgType = 307;
// NTSC-U/C: 0x003b0b20, PAL: 0x0042f9ec
int g_nLobbyConnectResultMsgType = 308;
#ifdef VIDEO_STANDARD_PAL
// PAL: 0x0042f9f0
int g_nDNASStatusMsgType = 309;
#endif
// NTSC-U/C: 0x003b0b24, PAL: 0x0042f9f4
int g_nCreateAccountResultMsgType = 309 + kDNASTypeShift;
// NTSC-U/C: 0x003b0b28, PAL: 0x0042f9f8
int g_nDeleteAccountResultMsgType = 310 + kDNASTypeShift;
// NTSC-U/C: 0x003b0b2c, PAL: 0x0042f9fc
int g_nChangePasswordResultMsgType = 311 + kDNASTypeShift;
// NTSC-U/C: 0x003b0b30, PAL: 0x0042fa00
int g_nLoginResultMsgType = 312 + kDNASTypeShift;
// NTSC-U/C: 0x003b0b34, PAL: 0x0042fa04
int g_nLobbyDisconnectResultMsgType = 313 + kDNASTypeShift;
// NTSC-U/C: 0x003b0b38, PAL: 0x0042fa08
int g_nLobbyPlayersMsgType = 314 + kDNASTypeShift;
// NTSC-U/C: 0x003b0b3c, PAL: 0x0042fa0c
int g_nPlayerRankMsgType = 315 + kDNASTypeShift;
// NTSC-U/C: 0x003b0b40, PAL: 0x0042fa10
int g_nFindPlayerMsgType = 316 + kDNASTypeShift;
// NTSC-U/C: 0x003b0b44, PAL: 0x0042fa14
int g_nLobbyChatroomsMsgType = 317 + kDNASTypeShift;
// NTSC-U/C: 0x003b0b48, PAL: 0x0042fa18
int g_nLobbyLaunchpadsMsgType = 318 + kDNASTypeShift;
// NTSC-U/C: 0x003b0b4c, PAL: 0x0042fa1c
int g_nJoinChatroomResultMsgType = 319 + kDNASTypeShift;
// NTSC-U/C: 0x003b0b50, PAL: 0x0042fa20
int g_nRepoRemixStatusMsgType = 320 + kDNASTypeShift;
// NTSC-U/C: 0x003b0b54, PAL: 0x0042fa24
int g_nRepoRemixesMsgType = 321 + kDNASTypeShift;
// NTSC-U/C: 0x003b0b58, PAL: 0x0042fa28
int g_nFileListMsgType = 322 + kDNASTypeShift;
// NTSC-U/C: 0x003b0b5c, PAL: 0x0042fa2c
int g_nRemixDownloadMsgType = 323 + kDNASTypeShift;
// NTSC-U/C: 0x003b0b60, PAL: 0x0042fa30
int g_nHostLaunchpadSuccessMsgType = 324 + kDNASTypeShift;
// NTSC-U/C: 0x003b0b64, PAL: 0x0042fa34
int g_nJoinLaunchpadSuccessMsgType = 325 + kDNASTypeShift;
// NTSC-U/C: 0x003b0b68, PAL: 0x0042fa38
int g_nLaunchpadAbortedMsgType = 326 + kDNASTypeShift;
// NTSC-U/C: 0x003b0b6c, PAL: 0x0042fa3c
int g_nGameParamsUpdateMsgType = 327 + kDNASTypeShift;
// NTSC-U/C: 0x003b0b70, PAL: 0x0042fa40
int g_nShareRemixBeginMsgType = 328 + kDNASTypeShift;
// NTSC-U/C: 0x003b0b74, PAL: 0x0042fa44
int g_nShareRemixProgressMsgType = 329 + kDNASTypeShift;
// NTSC-U/C: 0x003b0b78, PAL: 0x0042fa48
int g_nLoadGameMsgType = 330 + kDNASTypeShift;
// NTSC-U/C: 0x003b0b7c, PAL: 0x0042fa4c
int g_nStartGameMsgType = 331 + kDNASTypeShift;
// NTSC-U/C: 0x003b0b80, PAL: 0x0042fa50
int g_nGameEndedMsgType = 332 + kDNASTypeShift;
// NTSC-U/C: 0x003b0b84, PAL: 0x0042fa54
int g_nPlayerAbortedMsgType = 333 + kDNASTypeShift;
// NTSC-U/C: 0x003b0b88, PAL: 0x0042fa58
int g_nChatMsgType = 334 + kDNASTypeShift;
// NTSC-U/C: 0x003b0b8c, PAL: 0x0042fa5c
int g_nWelcomePktType = kWelcomePktType;
// NTSC-U/C: 0x003b0b90, PAL: 0x0042fa60
int g_nJoinFailedPktType = kJoinFailedPktType;
// NTSC-U/C: 0x003b0b94, PAL: 0x0042fa64
int g_nClientInfoPktType = kClientInfoPktType;
// NTSC-U/C: 0x003b0b98, PAL: 0x0042fa68
int g_nClientStatusPktType = kClientStatusPktType;
// NTSC-U/C: 0x003b0b9c, PAL: 0x0042fa6c
int g_nClientRatingPktType = kClientRatingPktType;
// NTSC-U/C: 0x003b0ba0, PAL: 0x0042fa70
int g_nTimeRequestPktType = kTimeRequestPktType;
// NTSC-U/C: 0x003b0ba4, PAL: 0x0042fa74
int g_nTimeResponsePktType = kTimeResponsePktType;
// NTSC-U/C: 0x003b0ba8, PAL: 0x0042fa78
int g_nGameParamsPktType = kGameParamsPktType;
// NTSC-U/C: 0x003b0bac, PAL: 0x0042fa7c
int g_nBootPktType = kBootPktType;
// NTSC-U/C: 0x003b0bb0, PAL: 0x0042fa80
int g_nShareRemixPktType = kShareRemixPktType;
// NTSC-U/C: 0x003b0bb4, PAL: 0x0042fa84
int g_nDataAckPktType = kDataAckPktType;
// NTSC-U/C: 0x003b0bb8, PAL: 0x0042fa88
int g_nDataBlockPktType = kDataBlockPktType;
// NTSC-U/C: 0x003b0bbc, PAL: 0x0042fa8c
int g_nLoadGamePktType = kLoadGamePktType;
// NTSC-U/C: 0x003b0bc0, PAL: 0x0042fa90
int g_nStartGamePktType = kStartGamePktType;
// NTSC-U/C: 0x003b0bc4, PAL: 0x0042fa94
int g_nClientAbortedPktType = kClientAbortedPktType;
// NTSC-U/C: 0x003b0bc8, PAL: 0x0042fa98
int g_nGameOverPktType = kGameOverPktType;
// NTSC-U/C: 0x003b0bcc, PAL: 0x0042fa9c
int g_nUpdateRanksPktType = kUpdateRanksPktType;

Message *WelcomePkt::New() {
    return new WelcomePkt;
}

Message *JoinFailedPkt::New() {
    return new JoinFailedPkt;
}

Message *ClientInfoPkt::New() {
    return new ClientInfoPkt;
}

Message *ClientStatusPkt::New() {
    return new ClientStatusPkt;
}

Message *ClientRatingPkt::New() {
    return new ClientRatingPkt;
}

Message *TimeRequestPkt::New() {
    return new TimeRequestPkt;
}

Message *TimeResponsePkt::New() {
    return new TimeResponsePkt;
}

Message *GameParamsPkt::New() {
    return new GameParamsPkt;
}

Message *BootPkt::New() {
    return new BootPkt;
}

Message *ShareRemixPkt::New() {
    return new ShareRemixPkt;
}

Message *DataAckPkt::New() {
    return new DataAckPkt;
}

Message *DataBlockPkt::New() {
    return new DataBlockPkt;
}

Message *LoadGamePkt::New() {
    return new LoadGamePkt;
}

Message *StartGamePkt::New() {
    return new StartGamePkt;
}

Message *ClientAbortedPkt::New() {
    return new ClientAbortedPkt;
}

Message *GameOverPkt::New() {
    return new GameOverPkt;
}

Message *UpdateRanksPkt::New() {
    return new UpdateRanksPkt;
}

PrnStream &operator<<(PrnStream &stream, const InetConfig &config) {
    PrnStream &next = stream << config.mId << " ";
    next.Print(config.mDescription.c_str());
    (next << " ").Print(config.mName.c_str());
    if (config.mDefault != 0) {
        stream << " [default]";
    }
    return stream;
}

PrnStream &operator<<(PrnStream &stream, const LobbyLocation &location) {
    stream.Print(location.mName.c_str());
    stream << " " << location.mId;
    return stream;
}

PrnStream &operator<<(PrnStream &stream, const NetChatroomInfo &room) {
    PrnStream &next = stream << room.mId << ".";
    next.Print(room.mName.c_str());
    next << " (" << room.mPlayerCount << "/" << room.mMaxPlayers << ")";
    return stream;
}

PrnStream &operator<<(PrnStream &stream, const LobbyPlayer &player) {
    stream.Printf("%d.%s %d/%d sk:%d gp:%d (g:%d %.2f,%d/%d)",
                  player.mId,
                  player.mName.c_str(),
                  player.mRating,
                  player.mRank,
                  player.mRankIcon,
                  player.mGames,
                  player.mRecord.mGames,
                  static_cast<double>(player.mRecord.mWins),
                  player.mRecord.mPoints,
                  player.mRecord.mOpponents);
    if (player.mMuted != 0) {
        stream << " mute";
    }
    PrintConnection(stream, player.mConnectionType);
    return stream;
}

BinStream &operator<<(BinStream &stream, const RatingRecord &record) {
    const short nHalfWins = static_cast<short>(record.mWins * kHalfGamesPerWin);
    const short nGames = static_cast<short>(record.mGames);
    const short nOpponents = static_cast<short>(record.mOpponents);
    stream.WriteEndian(&nHalfWins, sizeof(nHalfWins));
    stream.WriteEndian(&nGames, sizeof(nGames));
    const int nPoints = record.mPoints;
    stream.WriteEndian(&nPoints, sizeof(nPoints));
    stream.WriteEndian(&nOpponents, sizeof(nOpponents));
    return stream;
}

BinStream &operator>>(BinStream &stream, RatingRecord &record) {
    short nHalfWins;
    short nGames;
    short nOpponents;
    stream.ReadEndian(&nHalfWins, sizeof(nHalfWins));
    stream.ReadEndian(&nGames, sizeof(nGames));
    stream.ReadEndian(&record.mPoints, sizeof(record.mPoints));
    stream.ReadEndian(&nOpponents, sizeof(nOpponents));
    record.mOpponents = nOpponents;
    record.mWins = static_cast<float>(nHalfWins) * kWinsPerHalfGame;
    record.mGames = nGames;
    return stream;
}

PrnStream &operator<<(PrnStream &stream, const NetRepoRemix &remix) {
    stream.Print(remix.mFile.c_str());
    (stream << " " << remix.mInfo << "\n").Print(remix.mNote.c_str());
    return stream;
}

BinStream &operator<<(BinStream &stream, const NetRepoRemix &remix) {
    stream.WriteString(remix.mFile.c_str()).WriteString(remix.mNote.c_str()) << remix.mInfo;
    return stream;
}

BinStream &operator>>(BinStream &stream, NetRepoRemix &remix) {
    stream >> remix.mFile >> remix.mNote >> remix.mInfo;
    return stream;
}

PrnStream &operator<<(PrnStream &stream, const NetLaunchpadInfo &launchpad) {
    stream << launchpad.mLaunchpadId << "." << launchpad.mParams
           << " #pl:" << launchpad.mPlayerCount << " st:" << launchpad.mStatus;
    PrintConnection(stream, launchpad.mConnectionType);
    return stream;
}

PrnStream &operator<<(PrnStream &stream, const NetLaunchpadPlayer &player) {
    const LobbyPlayer &lobby = player.mPlayer;
    stream.Printf("%d %d.%s %d/%d sk:%d gp:%d (g:%d %.2f,%d/%d)",
                  player.mId,
                  lobby.mId,
                  lobby.mName.c_str(),
                  lobby.mRating,
                  lobby.mRank,
                  lobby.mRankIcon,
                  lobby.mGames,
                  lobby.mRecord.mGames,
                  static_cast<double>(lobby.mRecord.mWins),
                  lobby.mRecord.mPoints,
                  lobby.mRecord.mOpponents);
    if (lobby.mMuted != 0) {
        stream << " mute";
    }
    PrintColor(stream, player.mColor);
    PrintReady(stream, player.mReady);
    return stream;
}

BinStream &operator<<(BinStream &stream, const NetLaunchpadPlayer &player) {
    const LobbyPlayer &lobby = player.mPlayer;
    const int nAccount = lobby.mId;
    stream.WriteEndian(&nAccount, sizeof(nAccount));
    BinStream &body = stream.WriteString(lobby.mName.c_str());
    WriteByte(body, lobby.mConnectionType);
    const int nRank = lobby.mRank;
    body.WriteEndian(&nRank, sizeof(nRank));
    WriteByte(body, lobby.mMuted);
    const int nRating = lobby.mRating;
    body.WriteEndian(&nRating, sizeof(nRating));
    WriteByte(body, lobby.mRankIcon);
    const int nGames = lobby.mGames;
    body.WriteEndian(&nGames, sizeof(nGames));
    body << lobby.mLastOnline;
    lobby.mAvatar.Save(body);
    body << lobby.mRecord;
    WriteByte(body, player.mColor);
    WriteByte(body, player.mReady);
    const int nId = player.mId;
    body.WriteEndian(&nId, sizeof(nId));
    return stream;
}

BinStream &operator>>(BinStream &stream, NetLaunchpadPlayer &player) {
    LobbyPlayer &lobby = player.mPlayer;
    stream.ReadEndian(&lobby.mId, sizeof(lobby.mId));
    BinStream &body = stream >> lobby.mName;
    const int nConnection = ReadByte(body);
    body.ReadEndian(&lobby.mRank, sizeof(lobby.mRank));
    lobby.mMuted = static_cast<unsigned char>(ReadByte(body)) != 0;
    body.ReadEndian(&lobby.mRating, sizeof(lobby.mRating));
    const int nRankIcon = ReadByte(body);
    body.ReadEndian(&lobby.mGames, sizeof(lobby.mGames));
    body >> lobby.mLastOnline;
    lobby.mAvatar.Load(body);
    body >> lobby.mRecord;
    const int nColor = ReadByte(body);
    const int nReady = ReadByte(body);
    body.ReadEndian(&player.mId, sizeof(player.mId));
    lobby.mRankIcon = nRankIcon;
    player.mColor = nColor;
    player.mReady = nReady;
    lobby.mConnectionType = nConnection;
    return stream;
}

PrnStream &operator<<(PrnStream &stream, const NetGameScore &score) {
    stream << score.mNetOrder << ":" << score.mScore;
    return stream;
}

BinStream &operator<<(BinStream &stream, const NetGameScore &score) {
    const int nNetOrder = score.mNetOrder;
    stream.WriteEndian(&nNetOrder, sizeof(nNetOrder));
    const int nScore = score.mScore;
    stream.WriteEndian(&nScore, sizeof(nScore));
    return stream;
}

BinStream &operator>>(BinStream &stream, NetGameScore &score) {
    stream.ReadEndian(&score.mNetOrder, sizeof(score.mNetOrder));
    stream.ReadEndian(&score.mScore, sizeof(score.mScore));
    return stream;
}

PrnStream &operator<<(PrnStream &stream, const UpdateRanksPkt::Info &info) {
    stream << info.mPlayer << ":" << info.mRank;
    return stream;
}

BinStream &operator<<(BinStream &stream, const UpdateRanksPkt::Info &info) {
    const int nPlayer = info.mPlayer;
    stream.WriteEndian(&nPlayer, sizeof(nPlayer));
    const int nRank = info.mRank;
    stream.WriteEndian(&nRank, sizeof(nRank));
    return stream;
}

BinStream &operator>>(BinStream &stream, UpdateRanksPkt::Info &info) {
    stream.ReadEndian(&info.mPlayer, sizeof(info.mPlayer));
    stream.ReadEndian(&info.mRank, sizeof(info.mRank));
    return stream;
}

LobbyPlayer::LobbyPlayer()
    : mId(0), mConnectionType(kConnectionOffline), mRank(0), mMuted(0), mChannelId(kUnsetField),
      mRating(0), mGames(0), mRankIcon(kNoRankIcon), mLastOnline(0), mRecord() {
}

bool LobbyPlayer::IsNewbie() const {
    return static_cast<unsigned int>(mRating) < kVeteranRatingBonus;
}

void InetConfigsResultMsg::PrintExtra(PrnStream &stream) const {
    stream << "Result:" << mResult;
    if (mResult == 0) {
        stream << "\n" << mConfigs;
    }
}

void InetConnectStatusMsg::PrintExtra(PrnStream &stream) const {
    stream.Print(mStatus.c_str());
}

void InetConnectResultMsg::PrintExtra(PrnStream &stream) const {
    stream << "Result:" << mResult;
#ifndef VIDEO_STANDARD_PAL
    if (mError != "") {
        (stream << " Err:").Print(mError.c_str());
    }
#endif
}

void InetDNSLookupResultMsg::PrintExtra(PrnStream &stream) const {
    stream << "Result:" << mResult << " ip:" << mAddress;
}

void LobbyConnectResultMsg::PrintExtra(PrnStream &stream) const {
    stream << "Result:" << mResult;
    if (mResult == 0) {
        stream << "\n" << mLocations;
    }
}

void CreateAccountResultMsg::PrintExtra(PrnStream &stream) const {
    stream << "Result:" << mResult;
}

void DeleteAccountResultMsg::PrintExtra(PrnStream &stream) const {
    stream << "Result:" << mResult;
}

void ChangePasswordResultMsg::PrintExtra(PrnStream &stream) const {
    stream << "Result:" << mResult;
}

void LoginResultMsg::PrintExtra(PrnStream &stream) const {
    stream << "Result:" << mResult;
    if (mResult == 0) {
        stream << "\n" << mChatroom << "\nnews:" << mNews << "\nlegal:" << mEula;
    }
}

void JoinChatroomResultMsg::PrintExtra(PrnStream &stream) const {
    stream << "Result:" << mResult;
    if (mResult == 0) {
        stream << "\n" << mChatroom;
    }
}

void RepoRemixStatusMsg::PrintExtra(PrnStream &stream) const {
    stream << "Result:" << mStatus << "prg:" << mProgress;
}

void RepoRemixesMsg::PrintExtra(PrnStream &stream) const {
    stream << "Result:" << mResult;
    if (mRemixes != nullptr) {
        stream << "\n" << *mRemixes << "\n";
    }
}

void LobbyPlayersMsg::PrintExtra(PrnStream &stream) const {
    if (mLaunchpad != kNoLaunchpad) {
        stream << "Launchpad=" << mLaunchpad << "\n";
    }
    stream << *mPlayers;
}

void PlayerRankMsg::PrintExtra(PrnStream &stream) const {
    stream << mAccount << ": " << mRank;
}

void FindPlayerMsg::PrintExtra(PrnStream &stream) const {
    if (mResult != 0) {
        stream << "Error:" << mResult;
    } else {
        stream << mPlayer << " @ " << mChatroom;
    }
}

void LobbyChatroomsMsg::PrintExtra(PrnStream &stream) const {
    stream << *mChatrooms;
}

void LobbyLaunchpadsMsg::PrintExtra(PrnStream &stream) const {
    if (mLaunchpads != nullptr) {
        stream << *mLaunchpads;
    }
}

void HostLaunchpadSuccessMsg::PrintExtra(PrnStream &stream) const {
    stream << mPlayer;
}

void JoinLaunchpadSuccessMsg::PrintExtra(PrnStream &stream) const {
    stream << mParams << "\n" << mPlayer;
}

void LaunchpadAbortedMsg::PrintExtra(PrnStream &stream) const {
    stream << "Result:" << mReason;
}

void GameParamsUpdateMsg::PrintExtra(PrnStream &stream) const {
    stream << mParams;
}

void ShareRemixBeginMsg::PrintExtra(PrnStream &stream) const {
    stream << mInfo;
}

void ShareRemixProgressMsg::PrintExtra(PrnStream &stream) const {
    stream << mProgress << " shareonly:" << (mReceived != 0);
}

void StartGameMsg::PrintExtra(PrnStream &stream) const {
    stream << "Rand:" << mSeed;
}

void GameEndedMsg::PrintExtra(PrnStream &stream) const {
    stream << "Result:" << mResult;
    if (!mScores.empty()) {
        stream << mScores;
    }
}

void PlayerAbortedMsg::PrintExtra(PrnStream &stream) const {
    stream << mNetOrder;
}

void ChatMsg::PrintExtra(PrnStream &stream) const {
    stream << mAccountId << ":" << mText;
}

void Packet::PrintExtra(PrnStream &stream) const {
    if (mSender != kUnset) {
        stream << "from:" << mSender << " ";
    }
    if (mReceiver != kUnset) {
        stream << "to:" << mReceiver << " ";
    }
}

void WelcomePkt::PrintExtra(PrnStream &stream) const {
    Packet::PrintExtra(stream);
    PrintColor(stream, mColor);
    stream << " " << mParams;
}

void WelcomePkt::saveGuts(BinStream &stream) const {
    WriteByte(stream, mColor);
    stream << mParams;
}

void WelcomePkt::restoreGuts(BinStream &stream) {
    const int nColor = ReadByte(stream);
    stream >> mParams;
    mColor = nColor;
}

void JoinFailedPkt::PrintExtra(PrnStream &stream) const {
    stream << mReason;
}

void JoinFailedPkt::saveGuts(BinStream &stream) const {
    const int nReason = mReason;
    stream.WriteEndian(&nReason, sizeof(nReason));
}

void JoinFailedPkt::restoreGuts(BinStream &stream) {
    mReason = ReadByte(stream); // The binary reads one byte of the four saveGuts() writes.
}

void ClientInfoPkt::PrintExtra(PrnStream &stream) const {
    Packet::PrintExtra(stream);
    stream << mPlayer;
}

void ClientInfoPkt::saveGuts(BinStream &stream) const {
    stream << mPlayer;
}

void ClientInfoPkt::restoreGuts(BinStream &stream) {
    stream >> mPlayer;
}

void ClientStatusPkt::PrintExtra(PrnStream &stream) const {
    Packet::PrintExtra(stream);
    PrintReady(stream, mReady);
}

void ClientStatusPkt::saveGuts(BinStream &stream) const {
    WriteByte(stream, mReady);
}

void ClientStatusPkt::restoreGuts(BinStream &stream) {
    mReady = ReadByte(stream);
}

void ClientRatingPkt::PrintExtra(PrnStream &stream) const {
    Packet::PrintExtra(stream);
    stream.Printf("%d/%d gp:%d (g:%d %.2f,%d/%d)",
                  mRating,
                  mRank,
                  mGames,
                  mRecord.mGames,
                  static_cast<double>(mRecord.mWins),
                  mRecord.mPoints,
                  mRecord.mOpponents);
}

void ClientRatingPkt::saveGuts(BinStream &stream) const {
    const int nRating = mRating;
    stream.WriteEndian(&nRating, sizeof(nRating));
    const int nRank = mRank;
    stream.WriteEndian(&nRank, sizeof(nRank));
    const int nGames = mGames;
    stream.WriteEndian(&nGames, sizeof(nGames));
    stream << mRecord;
}

void ClientRatingPkt::restoreGuts(BinStream &stream) {
    stream.ReadEndian(&mRating, sizeof(mRating));
    stream.ReadEndian(&mRank, sizeof(mRank));
    stream.ReadEndian(&mGames, sizeof(mGames));
    stream >> mRecord;
}

void TimeRequestPkt::PrintExtra(PrnStream &stream) const {
    Packet::PrintExtra(stream);
    stream << mSendTime;
}

void TimeRequestPkt::saveGuts(BinStream &stream) const {
    const float fSendTime = mSendTime;
    stream.WriteEndian(&fSendTime, sizeof(fSendTime));
}

void TimeRequestPkt::restoreGuts(BinStream &stream) {
    stream.ReadEndian(&mSendTime, sizeof(mSendTime));
}

void TimeResponsePkt::PrintExtra(PrnStream &stream) const {
    Packet::PrintExtra(stream);
    stream << " req:" << mRequestTime << " send:" << mSendTime;
}

void TimeResponsePkt::saveGuts(BinStream &stream) const {
    const float fRequestTime = mRequestTime;
    stream.WriteEndian(&fRequestTime, sizeof(fRequestTime));
    const float fSendTime = mSendTime;
    stream.WriteEndian(&fSendTime, sizeof(fSendTime));
}

void TimeResponsePkt::restoreGuts(BinStream &stream) {
    stream.ReadEndian(&mRequestTime, sizeof(mRequestTime));
    stream.ReadEndian(&mSendTime, sizeof(mSendTime));
}

void GameParamsPkt::PrintExtra(PrnStream &stream) const {
    Packet::PrintExtra(stream);
    stream << mParams;
}

void GameParamsPkt::saveGuts(BinStream &stream) const {
    stream << mParams;
}

void GameParamsPkt::restoreGuts(BinStream &stream) {
    stream >> mParams;
}

void BootPkt::PrintExtra(PrnStream &stream) const {
    Packet::PrintExtra(stream);
    stream << mPlayer;
}

void BootPkt::saveGuts(BinStream &stream) const {
    const int nPlayer = mPlayer;
    stream.WriteEndian(&nPlayer, sizeof(nPlayer));
}

void BootPkt::restoreGuts(BinStream &stream) {
    stream.ReadEndian(&mPlayer, sizeof(mPlayer));
}

void ShareRemixPkt::PrintExtra(PrnStream &stream) const {
    Packet::PrintExtra(stream);
    stream << mInfo << " shareonly:" << (mShareOnly != 0);
}

void ShareRemixPkt::saveGuts(BinStream &stream) const {
    stream << mInfo;
    WriteByte(stream, mShareOnly);
}

void ShareRemixPkt::restoreGuts(BinStream &stream) {
    stream >> mInfo;
    const int nShareOnly = static_cast<unsigned char>(ReadByte(stream));
    mInfo.mSource = RemixInfo::kSourceNet;
    mShareOnly = nShareOnly != 0;
}

void DataBlockPkt::PrintExtra(PrnStream &stream) const {
    Packet::PrintExtra(stream);
    stream << "blk:" << mBlock << " sz:" << mSize << " " << mProgress << "%";
}

void DataBlockPkt::saveGuts(BinStream &stream) const {
    const short nBlock = mBlock;
    stream.WriteEndian(&nBlock, sizeof(nBlock));
    const short nSize = mSize;
    stream.WriteEndian(&nSize, sizeof(nSize));
    const float fProgress = mProgress;
    stream.WriteEndian(&fProgress, sizeof(fProgress));
    stream.Write(mData, mSize);
}

void DataBlockPkt::restoreGuts(BinStream &stream) {
    stream.ReadEndian(&mBlock, sizeof(mBlock));
    stream.ReadEndian(&mSize, sizeof(mSize));
    stream.ReadEndian(&mProgress, sizeof(mProgress));
    stream.Read(mBuffer, mSize);
}

void StartGamePkt::PrintExtra(PrnStream &stream) const {
    Packet::PrintExtra(stream);
    stream << mTime << " " << mSeed;
}

void StartGamePkt::saveGuts(BinStream &stream) const {
    const float fTime = mTime;
    stream.WriteEndian(&fTime, sizeof(fTime));
    const int nSeed = mSeed;
    stream.WriteEndian(&nSeed, sizeof(nSeed));
}

void StartGamePkt::restoreGuts(BinStream &stream) {
    stream.ReadEndian(&mTime, sizeof(mTime));
    stream.ReadEndian(&mSeed, sizeof(mSeed));
}

void GameOverPkt::PrintExtra(PrnStream &stream) const {
    Packet::PrintExtra(stream);
    stream << mScores;
}

void GameOverPkt::saveGuts(BinStream &stream) const {
    stream << mScores;
}

void GameOverPkt::restoreGuts(BinStream &stream) {
    stream >> mScores;
}

void UpdateRanksPkt::PrintExtra(PrnStream &stream) const {
    Packet::PrintExtra(stream);
    stream << mRanks;
}

void UpdateRanksPkt::saveGuts(BinStream &stream) const {
    stream << mRanks;
}

void UpdateRanksPkt::restoreGuts(BinStream &stream) {
    stream >> mRanks;
}

namespace {

// The unit's static initialiser at NTSC-U/C: 0x00269eb8, PAL: 0x00273a48, and its global
// constructor at NTSC-U/C: 0x0026a070, PAL: 0x00273c00, construct the factories.
// NTSC-U/C: 0x0043e580
const MessageFactory kWelcomePktFactory(kWelcomePktType, WelcomePkt::New);
// NTSC-U/C: 0x0043e588
const MessageFactory kJoinFailedPktFactory(kJoinFailedPktType, JoinFailedPkt::New);
// NTSC-U/C: 0x0043e590
const MessageFactory kClientInfoPktFactory(kClientInfoPktType, ClientInfoPkt::New);
// NTSC-U/C: 0x0043e598
const MessageFactory kClientStatusPktFactory(kClientStatusPktType, ClientStatusPkt::New);
// NTSC-U/C: 0x0043e5a0
const MessageFactory kClientRatingPktFactory(kClientRatingPktType, ClientRatingPkt::New);
// NTSC-U/C: 0x0043e5a8
const MessageFactory kTimeRequestPktFactory(kTimeRequestPktType, TimeRequestPkt::New);
// NTSC-U/C: 0x0043e5b0
const MessageFactory kTimeResponsePktFactory(kTimeResponsePktType, TimeResponsePkt::New);
// NTSC-U/C: 0x0043e5b8
const MessageFactory kGameParamsPktFactory(kGameParamsPktType, GameParamsPkt::New);
// NTSC-U/C: 0x0043e5c0
const MessageFactory kBootPktFactory(kBootPktType, BootPkt::New);
// NTSC-U/C: 0x0043e5c8
const MessageFactory kShareRemixPktFactory(kShareRemixPktType, ShareRemixPkt::New);
// NTSC-U/C: 0x0043e5d0
const MessageFactory kDataAckPktFactory(kDataAckPktType, DataAckPkt::New);
// NTSC-U/C: 0x0043e5d8
const MessageFactory kDataBlockPktFactory(kDataBlockPktType, DataBlockPkt::New);
// NTSC-U/C: 0x0043e5e0
const MessageFactory kLoadGamePktFactory(kLoadGamePktType, LoadGamePkt::New);
// NTSC-U/C: 0x0043e5e8
const MessageFactory kStartGamePktFactory(kStartGamePktType, StartGamePkt::New);
// NTSC-U/C: 0x0043e5f0
const MessageFactory kClientAbortedPktFactory(kClientAbortedPktType, ClientAbortedPkt::New);
// NTSC-U/C: 0x0043e5f8
const MessageFactory kGameOverPktFactory(kGameOverPktType, GameOverPkt::New);
// NTSC-U/C: 0x0043e600
const MessageFactory kUpdateRanksPktFactory(kUpdateRanksPktType, UpdateRanksPkt::New);

} // namespace
