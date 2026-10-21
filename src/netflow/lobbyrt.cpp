#include "netflow/lobbyrt.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <memory>

#include <librtmedius.h>

#include "game/gamedb.h"
#include "game/gameoptions.h"
#include "game/remixinfo.h"
#include "game/songentry.h"
#include "msg/changepasswordresultmsg.h"
#include "msg/createaccountresultmsg.h"
#include "msg/deleteaccountresultmsg.h"
#include "msg/filelistmsg.h"
#include "msg/findplayermsg.h"
#include "msg/joinchatroomresultmsg.h"
#include "msg/lobbychatroomsmsg.h"
#include "msg/lobbyconnectionlostmsg.h"
#include "msg/lobbyconnectresultmsg.h"
#include "msg/lobbydisconnectresultmsg.h"
#include "msg/lobbylaunchpadsmsg.h"
#include "msg/lobbyplayersmsg.h"
#include "msg/loginresultmsg.h"
#include "msg/remixdownloadmsg.h"
#include "msg/reporemixstatusmsg.h"
#include "netflow/accountdeletetask.h"
#include "netflow/accountgetidtask.h"
#include "netflow/accountlogintask.h"
#include "netflow/accountregistrationtask.h"
#include "netflow/accountupdatepasswordtask.h"
#include "netflow/accountupdatestatstask.h"
#include "netflow/allocatebuffertask.h"
#include "netflow/anonymouslogintask.h"
#include "netflow/channelinfotask.h"
#include "netflow/checkinettask.h"
#include "netflow/cleargamelistfiltertask.h"
#include "netflow/closemgcltask.h"
#include "netflow/closerttask.h"
#include "netflow/createchanneltask.h"
#include "netflow/dnastask.h"
#include "netflow/dodnstask.h"
#include "netflow/fileclosetask.h"
#include "netflow/filecreatetask.h"
#include "netflow/filedeletetask.h"
#include "netflow/filedownloadtask.h"
#include "netflow/filegetattributestask.h"
#include "netflow/filelisttask.h"
#include "netflow/fileupdateattributestask.h"
#include "netflow/fileuploadtask.h"
#include "netflow/findplayertask.h"
#include "netflow/getannouncementstask.h"
#include "netflow/getchannelstask.h"
#include "netflow/getgameplayerstask.h"
#include "netflow/getgamestask.h"
#include "netflow/getlobbyplayerstask.h"
#include "netflow/getpolicytask.h"
#include "netflow/initmgcltask.h"
#include "netflow/initrttask.h"
#include "netflow/joinchanneltask.h"
#include "netflow/killlaunchpadtask.h"
#include "netflow/ladderlisttask.h"
#include "netflow/ladderposition2task.h"
#include "netflow/ladderpositiontask.h"
#include "netflow/mediusconnecttask.h"
#include "netflow/mediusdisconnecttask.h"
#include "netflow/netinet.h"
#include "netflow/playerinfotask.h"
#include "netflow/reporemixesmsg.h"
#include "netflow/sessionbegintask.h"
#include "netflow/sessionendtask.h"
#include "netflow/setgamelistfiltertask.h"
#include "netflow/setlocparamstask.h"
#include "netflow/setuplocalplayertask.h"
#include "netflow/updateclientstatetask.h"
#include "netflow/versionservertask.h"
#include "os/base64.h"
#include "os/bufstream.h"
#include "os/datetime.h"
#include "os/debug.h"
#include "os/file.h"
#include "os/mem.h"
#include "os/system.h"
#include "script/dataarray.h"
#include "script/scriptfunction.h"

namespace {

// The wait between checks of the network connection, in milliseconds.
constexpr float kCheckInetPeriod = 1000.0f;

// The entries of a page of the launchpads and of the ranking.
constexpr int kPageSize = 10;

// The value of mLaunchpadId before RequestLaunchpadPlayers().
constexpr int kNoLaunchpadId = -2;

// The file the lobby server is read from, and the size of the buffer of the file.
const char *const kServerConfig = "server.cfg";
constexpr int kServerConfigSize = 1024;

// The nodes of `server.cfg`: the name of the server, its port, and its security.
constexpr int kServerConfigNodes = 3;
constexpr int kServerConfigName = 0;
constexpr int kServerConfigPort = 1;
constexpr int kServerConfigSecure = 2;

// The file of the remix directory.
const char *const kDirectoryFile = "directory.rx";

// The prefix of the files of the uploaded remixes.
const char *const kRemixPrefix = "r_";

// The location the connection reports when the server lists none.
const char *const kDefaultLocation = "home sweet home";

// The uploads of a remix tried before the upload fails.
constexpr int kMaxUploadAttempts = 10;

// The progress of a transfer when it starts and when it ends.
constexpr float kProgressStart = 0.01f;
constexpr float kProgressDone = 1.0f;

// The size of the length a stream writes before a string, and before the list of the directory.
constexpr int kLengthSize = 4;

// The entries of a page of the launchpads, and of the page RequestLaunchpads() lists them all in.
constexpr int kLaunchpadsPageSize = 7;
constexpr int kLaunchpadsAllPageSize = 32;

// The value of a filter that matches every game.
constexpr int kAnyValue = -1;

// The value of RTFilter::kFieldGenericField3 for a host that takes players.
constexpr int kHostJoinable = 1;

// The first field of an encoded game name and of an encoded player.
constexpr int kGameNameVersion = 3;
constexpr int kPlayerVersion = 5;

// The digits of each field of an encoded game name.
constexpr int kFlagDigits = 1;
constexpr int kSongDigits = 2;
constexpr int kAccountDigits = 10;

// The digits of each field of an encoded player.
constexpr int kRatingDigits = 10;
constexpr int kRankIconDigits = 1;
constexpr int kGamesDigits = 7;
constexpr int kSecondsDigits = 10;
constexpr int kWinsDigits = 3;
constexpr int kRecordGamesDigits = 3;
constexpr int kPointsDigits = 8;
constexpr int kOpponentsDigits = 3;
constexpr int kChannelDigits = 10;

// The size of the buffers of an encoded game name and an encoded player.
constexpr int kGameNameBufferSize = 64;
constexpr int kPlayerBufferSize = 256;
constexpr int kNumberBufferSize = 32;

// The size of the saved Freq of an encoded player, as bytes and as characters.
constexpr int kAvatarBufferSize = 128;
constexpr int kAvatarTextSize = 60;

// The smallest saved Freq an encoded player loads.
constexpr int kMinAvatarSize = 11;

// The character that pads the saved Freq of an encoded player.
constexpr char kAvatarPadding = '.';

// The games won are encoded as a count of half games.
constexpr double kWinsEncodeScale = 2.0001;
constexpr float kWinsDecodeScale = 0.5f;

// The highest skill level.
constexpr int kMaxSkillLevel = 3;

// The rules value of a skill level of a duel, and of a remix.
constexpr int kDuelRulesBase = 5;
constexpr int kRemixRules = 4;

// The connection classes of the lobby server.
constexpr int kConnectionClassSlow = 0;
constexpr int kConnectionClassFast = 1;
constexpr int kConnectionClassOther = 2;
constexpr int kConnectionClassUnknown = 0xffffff;

// The state of a game on the lobby server that takes players.
constexpr int kWorldStatusOpen = 1;

// The years DateTime counts from 1900 before the two digits of a remix file.
constexpr int kCenturyYears = 100;

// The format of the file of an upload of a remix: year, month, day, song, name, and attempt.
const char *const kRemixFileFormat = "r_%02d%02d%02d_%s_%s.%d";

#ifdef VIDEO_STANDARD_PAL
// DecodePlayer() rejects a text with a minus sign in its first characters, and marks the player
// with the channel kSignedChannel.
constexpr int kSignScanLength = 43;
constexpr int kSignedChannel = -666;
#endif

} // namespace

// NTSC-U/C: 0x003b0aec, PAL: 0x0042f9b8
NetLobby *TheNetLobby;

// NTSC-U/C: 0x003b0af0, PAL: 0x0042f9bc
std::unique_ptr<LobbyRT> TheLobbyRT;

// NTSC-U/C: 0x003b0af4, PAL: 0x0042f9c0
int LobbyRT::sUnlimitedUpload;

#ifdef VIDEO_STANDARD_PAL
// PAL: 0x0041d180
int LobbyRT::sApplicationId = 10314;

// PAL: 0x004db300. FileGetAttributesTask writes the access of the owner of the downloaded file
// here, and no other instruction of the binary forms the address.
static int sDownloadPermissions;
#endif

void LobbyRT::AllowUnlimitedUpload(DataArray *pCommand, void *pUserData) {
    (void)pCommand;
    (void)pUserData;
    sUnlimitedUpload = 1;
}

bool LobbyRT::ReadNumber(int *pnValue, const char **ppszText, int *pnRemaining, int nDigits) {
    if (*pnRemaining < nDigits) {
        return false;
    }
    char szNumber[kNumberBufferSize];
    memcpy(szNumber, *ppszText, nDigits);
    szNumber[nDigits] = '\0';
    *ppszText += nDigits;
    *pnRemaining -= nDigits;
    *pnValue = atoi(szNumber);
    return true;
}

int LobbyRT::ConnectionTypeOf(int nClass) {
    switch (nClass) {
    case kConnectionClassFast:
        return LobbyPlayer::kConnectionFast;
    case kConnectionClassSlow:
    case kConnectionClassOther:
        return LobbyPlayer::kConnectionSlow;
    default:
        return LobbyPlayer::kConnectionOffline;
    }
}

int LobbyRT::ConnectionClassOf(int nType) {
    switch (nType) {
    case LobbyPlayer::kConnectionSlow:
        return kConnectionClassSlow;
    case LobbyPlayer::kConnectionFast:
        return kConnectionClassFast;
    default:
        return kConnectionClassUnknown;
    }
}

void LobbyRT::EncodeGameName(String *pName,
                             const NetGameParams *pParams,
                             int nPlayers,
                             int nConnectionType,
                             int nAccountId) {
    int nLevel = 0;
    EncodeSong(&nLevel, pParams->GetSong().c_str());
    char szName[kGameNameBufferSize];
    sprintf(szName,
#ifdef VIDEO_STANDARD_PAL
            "%u%u%u%u%u%u%u%u%u%02u%010u%s",
#else
            "%d%d%d%d%d%d%d%d%d%02d%010d%s",
#endif
            kGameNameVersion,
            pParams->GetLoadRemix(),
            pParams->GetRemixReadOnly(),
            pParams->GetSkillLevel(),
            pParams->GetRuleSet(),
            pParams->GetMaxPlayers(),
            pParams->GetPowerupLevel(),
            nPlayers,
            nConnectionType,
            nLevel,
            nAccountId,
            pParams->GetRemixName().c_str());
    szName[strlen(szName)] = '\0'; // Yes, the binary terminates the text again.
    *pName = szName;
}

void LobbyRT::DecodeGameName(NetGameParams *pParams,
                             int *pnPlayers,
                             int *pnConnectionType,
                             int *pnAccountId,
                             const char *pszName) {
    pParams->GetSong().Clear();
    pParams->SetRuleSet(NetGameParams::kDefaultSelection);
    pParams->SetMaxPlayers(NetGameParams::kDefaultMaxPlayers);
    pParams->SetLoadRemix(0);
    pParams->SetPracticeMode(0);
    pParams->SetTutorial(0);
    pParams->SetSkillLevel(0);
    pParams->SetPowerupLevel(NetGameParams::kDefaultSelection);
    pParams->SetCommunity(GameDb::kCommunityOnline);
    *pnPlayers = 0;
    *pnConnectionType = LobbyPlayer::kConnectionFast;
    *pnAccountId = 0;

    const char *pszText = pszName;
    int nRemaining = static_cast<int>(strlen(pszName));
    int nValue = 0;
    if (!ReadNumber(&nValue, &pszText, &nRemaining, kFlagDigits) || nValue != kGameNameVersion) {
        return;
    }
    if (!ReadNumber(&nValue, &pszText, &nRemaining, kFlagDigits)) {
        return;
    }
    pParams->SetLoadRemix(nValue != 0);
    if (!ReadNumber(&nValue, &pszText, &nRemaining, kFlagDigits)) {
        return;
    }
    pParams->SetRemixReadOnly(nValue != 0);
    if (!ReadNumber(&nValue, &pszText, &nRemaining, kFlagDigits)) {
        return;
    }
    pParams->SetSkillLevel(nValue);
    if (!ReadNumber(&nValue, &pszText, &nRemaining, kFlagDigits)) {
        return;
    }
    pParams->SetRuleSet(nValue);
    if (!ReadNumber(&nValue, &pszText, &nRemaining, kFlagDigits)) {
        return;
    }
    pParams->SetMaxPlayers(nValue);
    if (!ReadNumber(&nValue, &pszText, &nRemaining, kFlagDigits)) {
        return;
    }
    pParams->SetPowerupLevel(nValue);
    if (!ReadNumber(&nValue, &pszText, &nRemaining, kFlagDigits)) {
        return;
    }
    *pnPlayers = nValue;
    if (!ReadNumber(&nValue, &pszText, &nRemaining, kFlagDigits)) {
        return;
    }
    *pnConnectionType = nValue;
    if (!ReadNumber(&nValue, &pszText, &nRemaining, kSongDigits)) {
        return;
    }
    DecodeSong(pParams, nValue);
    if (!ReadNumber(&nValue, &pszText, &nRemaining, kAccountDigits)) {
        return;
    }
    *pnAccountId = nValue;
    pParams->SetRemixName(pszText);
}

void LobbyRT::EncodePlayer(String *pText, const LobbyPlayer *pPlayer) {
    const unsigned int nSeconds = pPlayer->GetLastOnline().ToSeconds();
    char szText[kPlayerBufferSize];
    sprintf(szText,
#ifdef VIDEO_STANDARD_PAL
            "%u%010u%u%07u%010u%03u%03u%08u%03u",
#else
            "%d%010d%d%07d%010d%03d%03d%08d%03d",
#endif
            kPlayerVersion,
            pPlayer->GetRating(),
            pPlayer->GetRankIcon() + 1,
            pPlayer->GetGames(),
            nSeconds,
            static_cast<int>(static_cast<double>(pPlayer->GetRecord().mWins) * kWinsEncodeScale),
            pPlayer->GetRecord().mGames,
            pPlayer->GetRecord().mPoints,
            pPlayer->GetRecord().mOpponents);
    int nLength = static_cast<int>(strlen(szText));

    char avatar[kAvatarBufferSize];
    BufStream stream(avatar, kAvatarBufferSize, true);
    pPlayer->GetAvatar().Save(stream);
    int nEncoded;
    Base64Encode(&szText[nLength],
                 &nEncoded,
                 reinterpret_cast<const unsigned char *>(avatar),
                 stream.Tell());
    const int nPadding = kAvatarTextSize - nEncoded;
    nLength += nEncoded;
    memset(&szText[nLength], kAvatarPadding, nPadding);
    nLength += nPadding;
    sprintf(&szText[nLength], "%010d", pPlayer->GetChannelId() + 1);
    szText[strlen(szText)] = '\0'; // Yes, the binary terminates the text again.
    *pText = szText;
}

void LobbyRT::DecodePlayer(LobbyPlayer *pPlayer, const char *pszText) {
    pPlayer->SetRankIcon(-1);
    pPlayer->SetRating(0);
    pPlayer->SetGames(0);
    pPlayer->SetLastOnline(DateTime(0));
    pPlayer->SetAvatar(AvatarPartSet());
    pPlayer->SetRecord(RatingRecord());
    pPlayer->SetChannelId(-1);

    const char *pszCursor = pszText;
    int nRemaining = static_cast<int>(strlen(pszText));
#ifdef VIDEO_STANDARD_PAL
    // Yes, the binary scans the characters whatever the length of the text.
    for (int i = 0; i < kSignScanLength; ++i) {
        if (pszText[i] == '-') {
            pPlayer->SetChannelId(kSignedChannel);
            return;
        }
    }
#endif
    int nVersion = 0;
    if (!ReadNumber(&nVersion, &pszCursor, &nRemaining, kFlagDigits) ||
        nVersion != kPlayerVersion) {
        return;
    }
    int nValue;
    if (!ReadNumber(&nValue, &pszCursor, &nRemaining, kRatingDigits)) {
        return;
    }
    pPlayer->SetRating(nValue);
    if (!ReadNumber(&nValue, &pszCursor, &nRemaining, kRankIconDigits)) {
        return;
    }
    pPlayer->SetRankIcon(nValue - 1);
    if (!ReadNumber(&nValue, &pszCursor, &nRemaining, kGamesDigits)) {
        return;
    }
    pPlayer->SetGames(nValue);
    if (!ReadNumber(&nValue, &pszCursor, &nRemaining, kSecondsDigits)) {
        return;
    }
    pPlayer->SetLastOnline(DateTime(nValue));
    if (!ReadNumber(&nValue, &pszCursor, &nRemaining, kWinsDigits)) {
        return;
    }
    pPlayer->GetRecord().mWins =
        static_cast<float>(static_cast<unsigned int>(nValue)) * kWinsDecodeScale;
    if (!ReadNumber(&nValue, &pszCursor, &nRemaining, kRecordGamesDigits)) {
        return;
    }
    pPlayer->GetRecord().mGames = nValue;
    if (!ReadNumber(&nValue, &pszCursor, &nRemaining, kPointsDigits)) {
        return;
    }
    pPlayer->GetRecord().mPoints = nValue;
    if (!ReadNumber(&nValue, &pszCursor, &nRemaining, kOpponentsDigits)) {
        return;
    }
    pPlayer->GetRecord().mOpponents = nValue;

    char avatar[kAvatarBufferSize];
    int nSize;
    Base64Decode(reinterpret_cast<unsigned char *>(avatar), &nSize, pszCursor, nRemaining);
    if (nSize >= kMinAvatarSize) {
        BufStream stream(avatar, kAvatarBufferSize, true);
        pPlayer->GetAvatar().Load(stream);
    }
    if (strlen(pszCursor) < static_cast<size_t>(kAvatarTextSize)) {
        return;
    }
    pszCursor += kAvatarTextSize; // Yes, the binary leaves nRemaining unchanged here.
    if (!ReadNumber(&nValue, &pszCursor, &nRemaining, kChannelDigits)) {
        return;
    }
    pPlayer->SetChannelId(nValue - 1);
}

void LobbyRT::EncodeRules(int *pnValue, int *pnComparison, int nRuleSet, int nSkillLevel) {
    switch (nRuleSet) {
    case GameDb::kRuleSetGame:
        switch (nSkillLevel) {
        case 0:
        case 1:
        case 2:
        case kMaxSkillLevel:
            *pnValue = nSkillLevel;
            *pnComparison = RTFilter::kComparisonEqual;
            break;
        default:
            *pnValue = kMaxSkillLevel;
            *pnComparison = RTFilter::kComparisonLessOrEqual;
            break;
        }
        break;
    case GameDb::kRuleSetDuel:
        switch (nSkillLevel) {
        case 0:
        case 1:
        case 2:
        case kMaxSkillLevel:
            *pnValue = kDuelRulesBase + nSkillLevel;
            *pnComparison = RTFilter::kComparisonEqual;
            break;
        default:
            *pnValue = kDuelRulesBase;
            *pnComparison = RTFilter::kComparisonMoreOrEqual;
            break;
        }
        break;
    case GameDb::kRuleSetRemix:
        *pnValue = kRemixRules;
        *pnComparison = RTFilter::kComparisonEqual;
        break;
    default:
        break;
    }
}

void LobbyRT::EncodeSong(int *pnLevel, const char *pszSong) {
    const SongEntry song{TheGameDb->FindSong(pszSong)};
    *pnLevel = song.GetId();
}

void LobbyRT::DecodeSong(NetGameParams *pParams, int nLevel) {
    DataArray *pData = nullptr;
    if (TheGameDb->FindSongById(&pData, nLevel)) {
        pParams->SetSong(SongEntry{pData}.GetName());
    } else {
        pParams->SetSong("");
    }
}

int LobbyRT::LaunchpadStatusOf(int nWorldStatus) {
    if (nWorldStatus == kWorldStatusOpen) {
        return NetLaunchpadInfo::kStatusOpen;
    }
    return NetLaunchpadInfo::kStatusBusy;
}

void LobbyRT::Page(
    int *pnPage, int *pnPageSize, int nDirection, int nPageSize, int nLargePageSize, int nCount) {
    switch (nDirection) {
    case kPageFirst:
        *pnPage = 0;
        *pnPageSize = nPageSize;
        break;
    case kPageNext:
        if (nCount < nPageSize) {
            *pnPage = 0;
        } else {
            ++*pnPage;
        }
        *pnPageSize = nPageSize;
        break;
    case kPageCurrent:
        if (nCount == 0) {
            *pnPage = 0;
        }
        *pnPageSize = nPageSize;
        break;
    case kPagePrevious:
        if (--*pnPage < 0) {
            *pnPage = 0;
        }
        *pnPageSize = nPageSize;
        break;
    case kPageFirstAll:
        *pnPage = 0;
        *pnPageSize = nLargePageSize;
        break;
    default:
        break;
    }
}

LobbyRT::LobbyRT()
    : mDisconnectReason(kReasonDisconnect), mDisconnected(0), mCheckInet(kCheckInetPeriod),
      mRtReady(0), mMediusConnected(0), mServerName(nullptr), mServerPort(0), mLocationId(0),
      mJoinChatroomId(0), mLaunchpadsPage(0), mLaunchpadsPageSize(kPageSize),
      mLaunchpadId(kNoLaunchpadId), mRanksPage(0), mRanksPageSize(kPageSize), mRanksMode(0),
      mTransferSize(0), mTransferBuffer(nullptr), mTransferMode(kTransferNone),
      mDeleteBeforeUpload(0) {
    TheNetLobby = this;
    LoadConfig();

    int nAnonymous = 0;
    int nPrintVersion = 0;
    SystemConfig()->FindArray("net", false)->FindBool("anon_login", &nAnonymous, false);
    SystemConfig()->FindArray("net", false)->FindBool("print_version", &nPrintVersion, false);
    ScriptFunction::Register(AllowUnlimitedUpload, "unlimited_upload", nullptr);

#ifdef VIDEO_STANDARD_PAL
    mConnectTask.Add(new CheckInetTask);
    mConnectTask.Add(new InitRTTask);
    mConnectTask.Add(new DNASTask);
    mConnectTask.Add(new InitMGCLTask);
    mConnectTask.Add(new DoDNSTask);
    mConnectTask.Add(new MediusConnectTask(MediusConnectTask::kServerLobby));
    mConnectTask.Add(new SessionBeginTask);
    mConnectTask.Add(new SetLocParamsTask);
#else
    mConnectTask.Add(new CheckInetTask);
    mConnectTask.Add(new DoDNSTask);
    mConnectTask.Add(new InitRTTask);
    mConnectTask.Add(new InitMGCLTask);
    mConnectTask.Add(new MediusConnectTask(MediusConnectTask::kServerLobby));
    mConnectTask.Add(new SessionBeginTask);
#endif
    if (nPrintVersion != 0) {
        mConnectTask.Add(new VersionServerTask);
    }

    mDisconnectTask.Add(new KillLaunchpadTask);
    mDisconnectTask.Add(new SessionEndTask);
    mDisconnectTask.Add(new MediusDisconnectTask);
    mDisconnectTask.Add(new CloseMGCLTask);
    mDisconnectTask.Add(new CloseRTTask);

    if (nAnonymous != 0) {
        mLoginTask.Add(new AnonymousLoginTask);
    } else {
        mLoginTask.Add(new AccountLoginTask);
    }
    mLoginTask.Add(new MediusDisconnectTask);
    mLoginTask.Add(new MediusConnectTask(MediusConnectTask::kServerLocation));
    if (nPrintVersion != 0) {
        mLoginTask.Add(new VersionServerTask);
    }
    if (nAnonymous == 0) {
        mLoginTask.Add(new PlayerInfoTask(&mFoundPlayer, 1));
        mLoginTask.Add(new SetupLocalPlayerTask(&mFoundPlayer, &mLocalPlayer));
        mLoginTask.Add(new AccountUpdateStatsTask);
        mLoginTask.Add(new LadderPositionTask(&mLocalPlayer));
    }
    mLoginTask.Add(new UpdateClientStateTask);
    mLoginTask.Add(new ChannelInfoTask(&mChatroom));
    mLoginTask.Add(new GetAnnouncementsTask);
    mLoginTask.Add(new GetPolicyTask);

    mCreateAccountTask.Add(new AccountRegistrationTask);
    mDeleteAccountTask.Add(new AccountDeleteTask);
    mChangePasswordTask.Add(new AccountUpdatePasswordTask);
    mChatroomsTask.Add(new GetChannelsTask);
    mLobbyPlayersTask.Add(new GetLobbyPlayersTask);
    mLaunchpadPlayersTask.Add(new GetGamePlayersTask);
    mRanksTask.Add(new LadderListTask);

    mLaunchpadsTask.Add(new ClearGameListFilterTask);
    mLaunchpadsTask.Add(new SetGameListFilterTask);
    mLaunchpadsTask.Add(new GetGamesTask);

    mFindPlayerTask.Add(new AccountGetIDTask(&mFoundPlayer));
    mFindPlayerTask.Add(new PlayerInfoTask(&mFoundPlayer, 1));
    mFindPlayerTask.Add(new LadderPositionTask(&mFoundPlayer));
    mFindPlayerTask.Add(new FindPlayerTask(&mFoundChatroom, &mFoundPlayer));
    mFindPlayerTask.Add(new ChannelInfoTask(&mFoundChatroom));

    mJoinChatroomTask.Add(new JoinChannelTask);
    mJoinChatroomTask.Add(new MediusDisconnectTask);
    mJoinChatroomTask.Add(new MediusConnectTask(MediusConnectTask::kServerLocation));
    mJoinChatroomTask.Add(new UpdateClientStateTask);
    mJoinChatroomTask.Add(new AccountUpdateStatsTask);
    mJoinChatroomTask.Add(new ChannelInfoTask(&mChatroom));

    mCreateChatroomTask.Add(new CreateChannelTask);
    mCreateChatroomTask.Add(new JoinChannelTask);
    mCreateChatroomTask.Add(new MediusDisconnectTask);
    mCreateChatroomTask.Add(new MediusConnectTask(MediusConnectTask::kServerLocation));
    mCreateChatroomTask.Add(new UpdateClientStateTask);
    mCreateChatroomTask.Add(new AccountUpdateStatsTask);
    mCreateChatroomTask.Add(new ChannelInfoTask(&mChatroom));

#ifdef VIDEO_STANDARD_PAL
    mDownloadTask.Add(
        new FileGetAttributesTask(&mTransferName, &mTransferSize, &sDownloadPermissions));
#else
    mDownloadTask.Add(new FileGetAttributesTask(&mTransferName, &mTransferSize));
#endif
    mDownloadTask.Add(new AllocateBufferTask(&mTransferBuffer, &mTransferSize));
    mDownloadTask.Add(new FileDownloadTask(&mTransferName, &mTransferBuffer, &mTransferSize));

    mUploadTask.Add(new FileDeleteTask(&mTransferName, &mDeleteBeforeUpload));
    mUploadTask.Add(new FileCreateTask(&mTransferName, &mTransferSize));
    mUploadTask.Add(new FileUploadTask(&mTransferName, &mTransferBuffer, &mTransferSize));
    mUploadTask.Add(new FileCloseTask(&mTransferName));

    mFileListTask.Add(new FileListTask(&mTransferName, &mFiles));

#ifdef VIDEO_STANDARD_PAL
    mDeleteFileTask.Add(new FileUpdateAttributesTask(&mTransferName));
#else
    // Yes, the binary gives the step the address of a local of the constructor.
    int nDelete = 1;
    mDeleteFileTask.Add(new FileDeleteTask(&mTransferName, &nDelete));
#endif

    mCheckInet.SetTask(new CheckInetTask);
}

void LobbyRT::LoadConfig() {
    std::unique_ptr<File> pFile(File::New(kServerConfig, File::kModeRead, File::kFlagNoArchive));
    if (pFile != nullptr) {
        char *pBuffer = static_cast<char *>(PoolMemAlloc(kServerConfigSize, "nettmp", 0));
        memset(pBuffer, 0, kServerConfigSize);
        const int nRead = pFile->Read(pBuffer, kServerConfigSize);
        BufStream stream(pBuffer, nRead, true);
        DataArray *pConfig = DataArray::ReadText(kServerConfig, &stream);
        if (pConfig != nullptr && pConfig->Size() >= kServerConfigNodes) {
            mServerName = pConfig->Sym(kServerConfigName);
            mServerPort = pConfig->Int(kServerConfigPort);
            mSecure = pConfig->Int(kServerConfigSecure) != 0;
            pConfig->Release();
        }
        PoolMemFree(pBuffer);
    }
    if (mServerName == nullptr || mServerPort == 0) {
        DebugWarn("Error reading \"server.cfg\". Could not get server name, port, security");
    }
    // Yes, the binary reads mSecure even when `server.cfg` did not set it.
    if (mSecure != 0) {
        rtNetGenerateKeyPair(&mKeyPair.modulus, &mKeyPair.privateExponent);
    }
}

LobbyRT::~LobbyRT() {
    TheNetLobby = nullptr;
}

void LobbyRT::SetProfile(Campaign *pProfile) {
    mLocalPlayer.SetRankIcon(pProfile->GetHighestBeatenSkillLevel());
    mLocalPlayer.SetAvatar(pProfile->GetAvatar());
    mLocalPlayer.SetName(pProfile->GetName().c_str());
    mPassword = "";
}

void LobbyRT::Disconnect(MsgSink *pSink) {
    Fail(pSink, 0, kReasonDisconnect);
}

void LobbyRT::Connect(MsgSink *pSink) {
    (void)mDisconnectTask.GetState(); // Yes, the binary discards the state.
    mConnectTask.Start(pSink);
    mCheckInet.RunNow();
}

void LobbyRT::Login(MsgSink *pSink, int nLocation, const char *pszPassword) {
    (void)mDisconnectTask.GetState(); // Yes, the binary discards the state.
    mLocalPlayer.SetRating(0);
    mLocalPlayer.SetRank(0);
    mLocalPlayer.SetGames(0);
    mLocalPlayer.SetRecord(RatingRecord());
    mLocalPlayer.SetChannelId(-1);
    (void)mLocalPlayer.GetLastOnline().ReadClock(); // Yes, the binary discards the result.
    mLocalPlayer.SetConnectionType(TheNetInet->GetConnectionType());
    mFoundPlayer = LobbyPlayer();
    mLocationId = nLocation;
    mRanksPage = 0;
    mLaunchpadsPage = 0;
    mPassword = pszPassword;
    mLoginTask.Start(pSink);
}

int LobbyRT::GetAccountId() {
    return mLocalPlayer.GetId();
}

void LobbyRT::CreateAccount(MsgSink *pSink, const char *pszPassword) {
    mPassword = pszPassword;
    mCreateAccountTask.Start(pSink);
}

void LobbyRT::DeleteAccount(MsgSink *pSink) {
    mDeleteAccountTask.Start(pSink);
}

void LobbyRT::ChangePassword(MsgSink *pSink,
                             const char *pszOldPassword,
                             const char *pszNewPassword) {
    mPassword = pszOldPassword;
    mNewPassword = pszNewPassword;
    mChangePasswordTask.Start(pSink);
}

void LobbyRT::SetMuted(int nAccount, bool bMuted) {
    if (bMuted) {
        mMuted.insert(nAccount);
    } else {
        mMuted.erase(nAccount);
    }
}

void LobbyRT::ApplyMuted(std::list<LobbyPlayer> *pPlayers) {
    for (LobbyPlayer &player : *pPlayers) {
        player.SetMuted(IsMuted(player.GetId()));
    }
}

void LobbyRT::MoveLocalPlayerToFront(std::list<LobbyPlayer> *pPlayers) {
    for (auto it = pPlayers->begin(); it != pPlayers->end(); ++it) {
        if (it->GetId() == GetAccountId()) {
            pPlayers->splice(pPlayers->begin(), *pPlayers, it);
            return;
        }
    }
}

void LobbyRT::MoveChatroomToFront() {
    for (auto it = mChatrooms.begin(); it != mChatrooms.end(); ++it) {
        if (it->mId == mChatroom.mId) {
            mChatrooms.splice(mChatrooms.begin(), mChatrooms, it);
            return;
        }
    }
}

void LobbyRT::RequestLobbyPlayers(MsgSink *pSink) {
    mLobbyPlayersTask.Start(pSink);
}

void LobbyRT::AddLadderListener(MsgSink *pSink, int nAccount) {
    mTasks.Add(new LadderPosition2Task(pSink, nAccount));
}

void LobbyRT::RequestLaunchpadPlayers(MsgSink *pSink, int nLaunchpad) {
    if (mLaunchpadPlayersTask.GetState() == Task::kStateRunning) {
        return;
    }
    mLaunchpadId = nLaunchpad;
    mLaunchpadPlayersTask.Start(pSink);
}

void LobbyRT::RequestLaunchpads(
    MsgSink *pSink, int nStart, int nMode, const char *pszArena, int nRuleSet, int nSkillLevel) {
    mFilters.clear();
    if (nMode != 0) {
        mFilters.push_back(
            RTFilter{RTFilter::kFieldGenericField2, RTFilter::kComparisonEqual, mChatroom.mId});
    } else {
        mFilters.push_back(
            RTFilter{RTFilter::kFieldGenericField3, RTFilter::kComparisonEqual, kHostJoinable});
        if (nRuleSet != 0) {
            int nValue;
            int nComparison;
            // Yes, the binary adds the filter unset for a game mode EncodeRules() does not know.
            EncodeRules(&nValue, &nComparison, nRuleSet, nSkillLevel);
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
            mFilters.push_back(RTFilter{RTFilter::kFieldRulesSet, nComparison, nValue});
#pragma GCC diagnostic pop
        }
        if (pszArena != nullptr && pszArena[0] != '\0') {
            int nLevel;
            EncodeSong(&nLevel, pszArena);
            mFilters.push_back(
                RTFilter{RTFilter::kFieldGameLevel, RTFilter::kComparisonEqual, nLevel});
        }
    }
    Page(&mLaunchpadsPage,
         &mLaunchpadsPageSize,
         nStart,
         kLaunchpadsPageSize,
         kLaunchpadsAllPageSize,
         static_cast<int>(mLaunchpads.size()));
    mLaunchpadsTask.Start(pSink);
}

void LobbyRT::FindPlayer(MsgSink *pSink, const char *pszName) {
    mFoundPlayer.SetName(pszName);
    mFindPlayerTask.Start(pSink);
}

void LobbyRT::RequestChatrooms(MsgSink *pSink) {
    mChatroomsTask.Start(pSink);
}

void LobbyRT::JoinChatroom(MsgSink *pSink, int nChatroomId) {
    mJoinChatroomId = nChatroomId;
    mJoinChatroomTask.Start(pSink);
}

void LobbyRT::CreateChatroom(MsgSink *pSink, const String &name) {
    mNewChatroomName = name;
    mCreateChatroomTask.Start(pSink);
}

void LobbyRT::RequestRanks(MsgSink *pSink, int nCount, int nDirection, int nMode) {
    if (mRanksTask.GetState() == Task::kStateRunning) {
        return;
    }
    Page(&mRanksPage,
         &mRanksPageSize,
         nDirection,
         nCount,
         nCount,
         static_cast<int>(mRankedPlayers.size()));
    mRanksMode = nMode;
    mRanksTask.Start(pSink);
}

void LobbyRT::UploadRemix(MsgSink *pSink, const char *pszNote) {
    if (sUnlimitedUpload == 0 && TheGameDb->GetOptions()->UploadedToday()) {
        RepoRemixStatusMsg msg(RepoRemixStatusMsg::kStatusAlreadyExists, kProgressDone);
        pSink->Dispatch(&msg);
        return;
    }
    mDeleteBeforeUpload = 0;
    mUploadAttempts = 0;
    mTransferMode = kTransferUploadRemix;
    RemixInfo info = *TheGameDb->GetRemixInfo();
    info.mReadOnly = 1;
    String note(pszNote);
    mTransferSize = note.GetLength() + RemixInfo::WireSize() + kLengthSize + info.mDataSize;
    mTransferBuffer = static_cast<char *>(PoolMemAlloc(mTransferSize, "Upload", 0));
    BufStream stream(mTransferBuffer, mTransferSize, true);
    (stream << info).WriteString(note.c_str());
    stream.Write(TheGameDb->GetRemixBuffer(), info.mDataSize);
    (void)StartUpload(pSink); // Yes, the binary discards the result.
}

bool LobbyRT::StartUpload(MsgSink *pSink) {
    if (mUploadAttempts == kMaxUploadAttempts) {
        DebugPrint("upload failed on 10th attempt\n");
        return true;
    }
    const RemixInfo *pInfo = TheGameDb->GetRemixInfo();
    DateTime now{};
    (void)now.ReadClock(); // Yes, the binary discards the result.
    mTransferName.Clear();
    mTransferName.Printf(kRemixFileFormat,
                         now.GetYear() - kCenturyYears,
                         now.GetMonth() + 1,
                         now.GetDay(),
                         pInfo->mSong,
                         pInfo->mName,
                         mUploadAttempts);
    RepoRemixStatusMsg msg(RepoRemixStatusMsg::kStatusProgress, kProgressStart);
    pSink->Dispatch(&msg);
    mUploadTask.Start(pSink);
    return false;
}

void LobbyRT::UploadDirectory(MsgSink *pSink, std::list<NetRepoRemix> *pRemixes) {
    mTransferMode = kTransferUploadDirectory;
    mTransferName = kDirectoryFile;
    mTransferSize = kLengthSize;
    mDeleteBeforeUpload = 1;
    for (const NetRepoRemix &remix : *pRemixes) {
        mTransferSize += remix.mFile.GetLength() +
                         (remix.mNote.GetLength() + kLengthSize + kLengthSize) +
                         RemixInfo::WireSize();
    }
    mTransferBuffer = static_cast<char *>(PoolMemAlloc(mTransferSize, "Upload", 0));
    BufStream stream(mTransferBuffer, mTransferSize, true);
    stream << *pRemixes;
    mUploadTask.Start(pSink);
}

void LobbyRT::OnUploadDone(NetMultiTask *pTask) {
    bool bEnded = true;
    const int nResult = pTask->GetResult();
    switch (mTransferMode) {
    case kTransferUploadRemix:
        if (nResult == 0) {
            RepoRemixStatusMsg msg(RepoRemixStatusMsg::kStatusDone, kProgressDone);
            pTask->Notify(&msg);
            TheGameDb->GetOptions()->RecordUpload();
        } else {
            ++mUploadAttempts;
            bEnded = StartUpload(pTask->GetSink());
            if (bEnded) {
                RepoRemixStatusMsg msg(RepoRemixStatusMsg::kStatusFailed, kProgressDone);
                pTask->Notify(&msg);
            }
        }
        break;
    case kTransferUploadDirectory:
        if (nResult == 0) {
            RepoRemixStatusMsg msg(RepoRemixStatusMsg::kStatusDone, kProgressDone);
            pTask->Notify(&msg);
        } else {
            RepoRemixStatusMsg msg(nResult, kProgressDone);
            pTask->Notify(&msg);
        }
        break;
    default:
        break;
    }
    if (bEnded) {
        mTransferSize = 0;
        PoolMemFree(mTransferBuffer);
        mTransferMode = kTransferNone;
        mTransferBuffer = nullptr;
    }
}

void LobbyRT::RequestRemixes(MsgSink *pSink) {
    mTransferMode = kTransferDirectory;
    mTransferName = kDirectoryFile;
    mDownloadTask.Start(pSink);
}

void LobbyRT::DownloadRemix(MsgSink *pSink, const char *pszName) {
    mTransferMode = kTransferRemix;
    mTransferName = pszName;
    RepoRemixStatusMsg msg(RepoRemixStatusMsg::kStatusProgress, kProgressStart);
    pSink->Dispatch(&msg);
    mDownloadTask.Start(pSink);
}

void LobbyRT::OnDownloadDone(NetMultiTask *pTask) {
    const int nResult = pTask->GetResult();
    if (mTransferMode == kTransferDirectory) {
        if (nResult != 0) {
            RepoRemixesMsg msg(nResult, nullptr);
            pTask->Notify(&msg);
        } else {
            BufStream stream(mTransferBuffer, mTransferSize, true);
            std::list<NetRepoRemix> remixes;
            stream >> remixes;
            RepoRemixesMsg msg(0, &remixes);
            pTask->Notify(&msg);
        }
    } else if (mTransferMode == kTransferRemix) {
        if (nResult != 0) {
            RemixDownloadMsg msg(nResult, nullptr, nullptr);
            pTask->Notify(&msg);
            RepoRemixStatusMsg status(nResult, kProgressDone);
            pTask->Notify(&status);
        } else {
            RemixInfo info;
            String note;
            BufStream stream(mTransferBuffer, mTransferSize, true);
            stream >> info >> note;
            const int nSize = info.mDataSize;
            if (nSize != mTransferSize - stream.Tell()) {
                RepoRemixStatusMsg status(RepoRemixStatusMsg::kStatusFailed, kProgressDone);
                pTask->Notify(&status);
            } else {
                info.mReadOnly = 1;
                TheGameDb->SetRemix(&info);
                TheGameDb->SetRemixBuffer(nSize);
                stream.Read(TheGameDb->GetRemixBuffer(), nSize);
                RemixDownloadMsg msg(0, mTransferName.c_str(), note.c_str());
                pTask->Notify(&msg);
                RepoRemixStatusMsg status(RepoRemixStatusMsg::kStatusDone, kProgressDone);
                pTask->Notify(&status);
            }
        }
    }
    PoolMemFree(mTransferBuffer);
    mTransferMode = kTransferNone;
    mTransferBuffer = nullptr;
    mTransferSize = 0;
}

void LobbyRT::RequestFiles(MsgSink *pSink, int nRemixes) {
    mTransferMode = kTransferList;
    if (nRemixes != 0) {
        mTransferName = kRemixPrefix;
    } else {
        mTransferName.Clear();
    }
    mFileListTask.Start(pSink);
}

void LobbyRT::OnFileListDone(NetMultiTask *pTask) {
#ifdef VIDEO_STANDARD_PAL
    mFiles.sort();
#endif
    mTransferMode = kTransferNone;
    std::list<String> files;
    files.swap(mFiles);
    FileListMsg msg(&files);
    pTask->Notify(&msg);
}

void LobbyRT::DeleteFile(MsgSink *pSink, const char *pszName) {
    mTransferMode = kTransferDelete;
    mTransferName = pszName;
    mDeleteFileTask.Start(pSink);
}

void LobbyRT::OnDeleteFileDone(NetMultiTask *pTask) {
    (void)pTask;
    mTransferMode = kTransferNone;
}

void LobbyRT::ReportProgress(float fProgress) {
    RepoRemixStatusMsg msg(RepoRemixStatusMsg::kStatusProgress, fProgress);
#ifdef VIDEO_STANDARD_PAL
    switch (mTransferMode) {
    case kTransferDirectory:
    case kTransferRemix:
        mDownloadTask.Notify(&msg);
        break;
    case kTransferUploadRemix:
    case kTransferUploadDirectory:
        mUploadTask.Notify(&msg);
        break;
    case kTransferList:
        mFileListTask.Notify(&msg);
        break;
    default:
        break;
    }
#else
    if (mTransferMode <= kTransferNone) {
        return;
    }
    if (mTransferMode < kTransferUploadRemix) {
        mDownloadTask.Notify(&msg);
    } else if (mTransferMode < kTransferList) {
        mUploadTask.Notify(&msg);
    }
#endif
}

void LobbyRT::Poll() {
    if (mRtReady != 0) {
        if (mCheckInet.Poll() == Task::kStateFailed) {
            DebugPrint("Inet failure\n");
            mCheckInet.Suspend();
            mDisconnected = 1;
            Fail(GetSink(), LobbyConnectResultMsg::kResultLostInternet, kReasonLost);
        }
        const int nError = rtMediusUpdate();
        if (nError != 0) {
            mDisconnected = 1;
            DebugPrint("Medius Error:%d\n", nError);
            if (mDisconnectTask.GetState() != Task::kStateRunning) {
                Fail(GetSink(), LobbyConnectResultMsg::kResultFailed, kReasonLost);
            }
        }
    }

    int nState = mConnectTask.Poll();
    if (nState == Task::kStateFailed) {
        mConnectTask.Stop();
        Fail(mConnectTask.GetSink(), mConnectTask.GetResult(), kReasonConnect);
    } else if (nState == Task::kStateDone) {
        LobbyConnectResultMsg msg(LobbyConnectResultMsg::kResultSuccess);
        if (mLocations.size() != 0) {
            msg.GetLocations().insert(
                msg.GetLocations().end(), mLocations.begin(), mLocations.end());
        } else {
            msg.GetLocations().push_back(LobbyLocation{String(kDefaultLocation), 0});
        }
        mConnectTask.Stop();
        mConnectTask.Notify(&msg);
    }

    nState = mLoginTask.Poll();
    if (nState == Task::kStateFailed) {
        mLoginTask.Stop();
        Fail(mLoginTask.GetSink(), mLoginTask.GetResult(), kReasonLogin);
    } else if (nState == Task::kStateDone) {
        mLoginTask.Stop();
        LoginResultMsg msg(
            LoginResultMsg::kResultSuccess, mAnnouncement.c_str(), mPolicy.c_str(), mChatroom);
        mLoginTask.Notify(&msg);
    }

    nState = mJoinChatroomTask.Poll();
    if (nState == Task::kStateFailed) {
        mJoinChatroomTask.Stop();
#ifdef VIDEO_STANDARD_PAL
        // A failed reconnection to the lobby server of the room loses the lobby.
        if (mJoinChatroomTask.GetResult() == LobbyConnectResultMsg::kResultConnectFailed) {
            Fail(GetSink(), LobbyConnectResultMsg::kResultLostInternet, kReasonLost);
        } else {
            JoinChatroomResultMsg msg(mJoinChatroomTask.GetResult());
            mJoinChatroomTask.Notify(&msg);
        }
#else
        JoinChatroomResultMsg msg(mJoinChatroomTask.GetResult());
        mJoinChatroomTask.Notify(&msg);
#endif
    } else if (nState == Task::kStateDone) {
        mJoinChatroomTask.Stop();
        JoinChatroomResultMsg msg(JoinChatroomResultMsg::kResultSuccess, mChatroom);
        mJoinChatroomTask.Notify(&msg);
    }

    nState = mCreateChatroomTask.Poll();
    if (nState == Task::kStateFailed) {
        mCreateChatroomTask.Stop();
#ifdef VIDEO_STANDARD_PAL
        if (mCreateChatroomTask.GetResult() == LobbyConnectResultMsg::kResultConnectFailed) {
            Fail(GetSink(), LobbyConnectResultMsg::kResultLostInternet, kReasonLost);
        } else {
            JoinChatroomResultMsg msg(mCreateChatroomTask.GetResult());
            mCreateChatroomTask.Notify(&msg);
        }
#else
        JoinChatroomResultMsg msg(mCreateChatroomTask.GetResult());
        mCreateChatroomTask.Notify(&msg);
#endif
    } else if (nState == Task::kStateDone) {
        mCreateChatroomTask.Stop();
        JoinChatroomResultMsg msg(JoinChatroomResultMsg::kResultSuccess, mChatroom);
        mCreateChatroomTask.Notify(&msg);
    }

    if (mDisconnectTask.Poll() == Task::kStateDone) {
        mDisconnectTask.Stop();
        switch (mDisconnectReason) {
        case kReasonDisconnect: {
            LobbyDisconnectResultMsg msg;
            mDisconnectTask.Notify(&msg);
            break;
        }
        case kReasonConnect: {
            LobbyConnectResultMsg msg(mDisconnectTask.GetResult());
            mDisconnectTask.Notify(&msg);
            break;
        }
        case kReasonLogin: {
            LoginResultMsg msg(mDisconnectTask.GetResult());
            mDisconnectTask.Notify(&msg);
            break;
        }
        case kReasonLost: {
            LobbyConnectionLostMsg msg(mDisconnectTask.GetResult());
            mDisconnectTask.Notify(&msg);
            break;
        }
        default:
            break;
        }
        mDisconnected = 0;
    }

    nState = mCreateAccountTask.Poll();
    if (nState == Task::kStateDone || nState == Task::kStateFailed) {
        const int nResult = nState == Task::kStateDone ? 0 : mCreateAccountTask.GetResult();
        mCreateAccountTask.Stop();
        CreateAccountResultMsg msg(nResult);
        mCreateAccountTask.Notify(&msg);
    }

    nState = mDeleteAccountTask.Poll();
    if (nState == Task::kStateDone || nState == Task::kStateFailed) {
        const int nResult = nState == Task::kStateDone ? 0 : mDeleteAccountTask.GetResult();
        mDeleteAccountTask.Stop();
        DeleteAccountResultMsg msg(nResult);
        mDeleteAccountTask.Notify(&msg);
    }

    nState = mChangePasswordTask.Poll();
    if (nState == Task::kStateDone || nState == Task::kStateFailed) {
        const int nResult = nState == Task::kStateDone ? 0 : mChangePasswordTask.GetResult();
        mChangePasswordTask.Stop();
        ChangePasswordResultMsg msg(nResult);
        mChangePasswordTask.Notify(&msg);
    }

    if (mChatroomsTask.Poll() == Task::kStateDone) {
        MoveChatroomToFront();
        mChatroomsTask.Stop();
        LobbyChatroomsMsg msg(&mChatrooms);
        mChatroomsTask.Notify(&msg);
    }

    if (mLobbyPlayersTask.Poll() == Task::kStateDone) {
        ApplyMuted(&mLobbyPlayers);
        MoveLocalPlayerToFront(&mLobbyPlayers);
        mLobbyPlayersTask.Stop();
        LobbyPlayersMsg msg(&mLobbyPlayers, LobbyPlayersMsg::kNoLaunchpad, 0);
        mLobbyPlayersTask.Notify(&msg);
    }

    if (mLaunchpadPlayersTask.Poll() == Task::kStateDone) {
        ApplyMuted(&mLaunchpadPlayers);
        mLaunchpadPlayersTask.Stop();
        LobbyPlayersMsg msg(&mLaunchpadPlayers, mLaunchpadId, 0);
        mLaunchpadPlayersTask.Notify(&msg);
    }

    if (mRanksTask.Poll() == Task::kStateDone) {
        ApplyMuted(&mRankedPlayers);
        mRanksTask.Stop();
        LobbyPlayersMsg msg(&mRankedPlayers, LobbyPlayersMsg::kNoLaunchpad, mRanksPage);
        mRanksTask.Notify(&msg);
    }

    nState = mFindPlayerTask.Poll();
    if (nState == Task::kStateDone) {
        mFindPlayerTask.Stop();
        FindPlayerMsg msg(0, mFoundPlayer, mFoundChatroom);
        mFindPlayerTask.Notify(&msg);
    } else if (nState == Task::kStateFailed) {
        mFindPlayerTask.Stop();
        FindPlayerMsg msg(mFindPlayerTask.GetResult());
        mFindPlayerTask.Notify(&msg);
    }

    nState = mDownloadTask.Poll();
    if (nState == Task::kStateDone || nState == Task::kStateFailed) {
        if (nState == Task::kStateDone) {
            mDownloadTask.SetResult(0);
        }
        mDownloadTask.Stop();
        OnDownloadDone(&mDownloadTask);
    }

    nState = mUploadTask.Poll();
    if (nState == Task::kStateDone || nState == Task::kStateFailed) {
        if (nState == Task::kStateDone) {
            mUploadTask.SetResult(0);
        }
        mUploadTask.Stop();
        OnUploadDone(&mUploadTask);
    }

    nState = mFileListTask.Poll();
    if (nState == Task::kStateDone || nState == Task::kStateFailed) {
        if (nState == Task::kStateDone) {
            mFileListTask.SetResult(0);
        }
        mFileListTask.Stop();
        OnFileListDone(&mFileListTask);
    }

    nState = mDeleteFileTask.Poll();
    if (nState == Task::kStateDone || nState == Task::kStateFailed) {
        if (nState == Task::kStateDone) {
            mDeleteFileTask.SetResult(0);
        }
        mDeleteFileTask.Stop();
        OnDeleteFileDone(&mDeleteFileTask);
    }

    nState = mLaunchpadsTask.Poll();
    if (nState == Task::kStateDone) {
        mActiveFilters = mFilters;
        mLaunchpadsTask.Stop();
        if (mLaunchpads.empty() && mLaunchpadsPage > 0 && mLaunchpadsPageSize > 0) {
            mLaunchpadsPage = 0;
            mLaunchpadsTask.Start(mLaunchpadsTask.GetSink());
        } else {
            LobbyLaunchpadsMsg msg(&mLaunchpads, mLaunchpadsPage);
            mLaunchpadsTask.Notify(&msg);
        }
    } else if (nState == Task::kStateFailed) {
        DebugPrint("Error in setting up launchpad filters\n");
        mActiveFilters.clear();
        mActiveFilters.push_back(
            RTFilter{RTFilter::kFieldGenericField2, RTFilter::kComparisonNotEqual, kAnyValue});
        mActiveFilters.push_back(
            RTFilter{RTFilter::kFieldGenericField3, RTFilter::kComparisonNotEqual, kAnyValue});
        mActiveFilters.push_back(
            RTFilter{RTFilter::kFieldRulesSet, RTFilter::kComparisonNotEqual, kAnyValue});
        mActiveFilters.push_back(
            RTFilter{RTFilter::kFieldGameLevel, RTFilter::kComparisonNotEqual, kAnyValue});
        std::list<NetLaunchpadInfo> launchpads;
        mLaunchpadsTask.Stop();
        LobbyLaunchpadsMsg msg(&launchpads, 0);
        mLaunchpadsTask.Notify(&msg);
    }

    mTasks.Poll();
}

void LobbyRT::Fail(MsgSink *pSink, int nResult, int nReason) {
    mConnectTask.Stop();
    mLoginTask.Stop();
    mJoinChatroomTask.Stop();
    mCreateChatroomTask.Stop();
    mCreateAccountTask.Stop();
    mDeleteAccountTask.Stop();
    mChangePasswordTask.Stop();
    mChatroomsTask.Stop();
    mLobbyPlayersTask.Stop();
    mLaunchpadPlayersTask.Stop();
    mLaunchpadsTask.Stop();
    mFindPlayerTask.Stop();
    mRanksTask.Stop();
    mDownloadTask.Stop();
    mUploadTask.Stop();
    mFileListTask.Stop();
    mDeleteFileTask.Stop();
    mActiveFilters.clear();
    mFilters.clear();
    mMuted.clear();
    mDisconnectReason = nReason;
    mDisconnectTask.SetResult(nResult);
    mDisconnectTask.Start(pSink);
    PoolMemFree(mTransferBuffer); // Yes, the binary does not clear mTransferBuffer.
}

void LobbyRT::SetAccount(int nAccount, int nChannel, const rtNetConnectionInfo *pInfo) {
    mLocalPlayer.SetId(nAccount);
    mChatroom.mId = nChannel;
    mFoundPlayer.SetId(nAccount);
    mLocalPlayer.SetChannelId(nChannel);
    mLobbyInfo = *pInfo;
}

const char *LobbyRT::GetName() {
    return mLocalPlayer.GetName().c_str();
}

const char *LobbyRT::GetPassword() {
    return mPassword.c_str();
}

const char *LobbyRT::GetPlayerStats() {
    EncodePlayer(&mStatsText, &mLocalPlayer);
    return mStatsText.c_str();
}

void LobbyRT::UpdateStats(const LobbyPlayer *pPlayer) {
    const RatingRecord &record = mLocalPlayer.GetRecord();
    const RatingRecord &other = pPlayer->GetRecord();
    if (mLocalPlayer.GetRating() == pPlayer->GetRating() &&
        mLocalPlayer.GetGames() == pPlayer->GetGames() && record.mWins == other.mWins &&
        record.mGames == other.mGames && record.mOpponents == other.mOpponents &&
        record.mPoints == other.mPoints && mLocalPlayer.GetRank() == pPlayer->GetRank()) {
        return;
    }
    mLocalPlayer.SetRating(pPlayer->GetRating());
    mLocalPlayer.SetGames(pPlayer->GetGames());
    mLocalPlayer.SetRecord(pPlayer->GetRecord());
    mLocalPlayer.SetRank(pPlayer->GetRank());
    mTasks.Add(new AccountUpdateStatsTask);
}

bool LobbyRT::IsMuted(int nAccount) {
    return mMuted.find(nAccount) != mMuted.end();
}

void LobbyRT::LogNetError(int nLine, int nError) {
    (void)nLine;
    (void)nError;
}

void LobbyRT::LogMediusError(int nLine, int nError) {
    (void)nLine;
    (void)nError;
}
