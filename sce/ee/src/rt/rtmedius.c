#include "rt/rtmedius.h"

#include <string.h>

// The version text rtMediusGetVersion() reports.
static const char kVersionText[] = "Medius Client Library Version 1.43.0002";

enum {
    // The size rtMediusGetVersion() copies, the terminator included.
    kVersionTextSize = 40,
    // The result of rtMediusTruncateField() for a null field.
    kErrorNullField = -99,
    // The receiver of the messages sent to the lobby server.
    kServerReceiver = 0xffff,
    // The flags of the messages sent to the lobby server.
    kSendFlags = RT_DME_SEND_GUARANTEED,
    // The connection a failed connect reports.
    kNoConnection = -1,
    // The status rtMediusInitialize() reports when it fails, and its result when the registration
    // of the messages failed.
    kErrorRegistration = -1,
    // The states of the request that waits for the addresses of the console.
    kQueryIdle = 0,
    kQueryReady = 1,
    kQueryFailed = 2,
    kQueryPending = 3,
    // The kinds of request that wait for the addresses of the console.
    kPendingMessage031 = 18,
    kPendingJoinGame = 44,
    kPendingNone = 0xffffff,
    // The kind of host of rtMediusJoinGameRequest that needs the addresses of the console first.
    kHostTypeQueryAddresses = 2,
    // The kinds of connection and address rtMediusSetConnectParams() fills.
    kConnectionTypeDirect = 1,
    kAddressTypeNone = 0,
    kAddressTypeExternal = 1,
    // The size rtMediusChangePassword() sends.
    kChangePasswordSendSize = 0x1b0,
    // The sizes of the text fields of the records of the lobby messages.
    kIpAddressSize = 20,
    kProfileStateSize = 3,
    kProfilePhoneSize = 16,
    kProfileCommentsSize = 80,
    kClanMessageSize = 200,
    kNoticeTextSize = 256,
    kNewsSize = 256,
    kUniverseNameSize = 128,
    kUniverseDnsSize = 128,
    kUniverseDescriptionSize = 256,
    kMachineSignatureSize = 128,
    // The characters of some text fields the field lists name, fewer than the fields have.
    kAnonymousLoginStatsCount = 32,
    kWorldNameCount = 32,
    kVersionServerTextCount = 56,
};

// The request of message 31 (create a game on the console), which waits for the addresses of the
// console. 0xec bytes.
typedef struct {
    rtMediusRequest header;
    int reserved28; // +0x28, of unknown purpose.
    int reserved2C; // +0x2c, of unknown purpose.
    int applicationId;
    int minPlayers;
    int maxPlayers;
    int gameLevel;
    char gameName[RT_MEDIUS_GAME_NAME_SIZE];
    char gamePassword[RT_MEDIUS_PASSWORD_SIZE];
    int playerSkillLevel;
    int rulesSet;
    int genericField1;
    int genericField2;
    int genericField3;
    int gameHostType;
    int reservedB8; // +0xb8, of unknown purpose.
    rtNetAddress addresses[RT_NET_ADDRESS_COUNT];
} MediusMessage031Request;

// The records below are the templates of the field lists of the lobby messages. The names of the
// records and of their fields are inferred from the names of the messages and the sizes of the
// fields.

// The profile of an account. 0x18a bytes.
typedef struct {
    short birthYear;
    short birthMonth;
    short birthDay;
    char firstName[RT_MEDIUS_PLAYER_NAME_SIZE];
    char lastName[RT_MEDIUS_PLAYER_NAME_SIZE];
    char company[RT_MEDIUS_PLAYER_NAME_SIZE];
    char address1[RT_MEDIUS_PLAYER_NAME_SIZE];
    char address2[RT_MEDIUS_PLAYER_NAME_SIZE];
    char address3[RT_MEDIUS_PLAYER_NAME_SIZE];
    char city[RT_MEDIUS_PLAYER_NAME_SIZE];
    char state[kProfileStateSize];
    char country[RT_MEDIUS_PLAYER_NAME_SIZE];
    char email[RT_MEDIUS_PLAYER_NAME_SIZE];
    char phone[kProfilePhoneSize];
    char comments[kProfileCommentsSize];
} MediusProfile;

// The response that reports the profile of an account. 0x1cc bytes.
typedef struct {
    rtMediusResponse header;
    int accountId;
    char accountName[RT_MEDIUS_PLAYER_NAME_SIZE];
    MediusProfile profile;
} MediusAccountGetProfileResponse;

// The request that changes the profile of the account. 0x1b0 bytes.
typedef struct {
    rtMediusRequest header;
    MediusProfile profile;
} MediusAccountUpdateProfileRequest;

// The response that reports the address of the console. 0x30 bytes.
typedef struct {
    char messageId[RT_MEDIUS_MESSAGE_ID_SIZE];
    char ipAddress[kIpAddressSize];
    int status;
} MediusGetMyIpResponse;

// The request that creates a game. 0xb0 bytes.
typedef struct {
    rtMediusRequest header;
    int applicationId;
    int minPlayers;
    int maxPlayers;
    int gameLevel;
    char gameName[RT_MEDIUS_GAME_NAME_SIZE];
    char gamePassword[RT_MEDIUS_PASSWORD_SIZE];
    int playerSkillLevel;
    int rulesSet;
    int genericField1;
    int genericField2;
    int genericField3;
    int gameHostType;
} MediusCreateGameRequest;

// A response that reports a status and one value (an account, a world, a clan, or a total). 0x20
// bytes.
typedef struct {
    rtMediusResponse header;
    int id;
} MediusIdResponse;

// A request with two values whose meaning depends on the message. 0x30 bytes.
typedef struct {
    rtMediusRequest header;
    int id;
    int value;
} MediusPairRequest;

// The response of the server to a channel the console joined. 0x3c bytes.
typedef struct {
    char messageId[RT_MEDIUS_MESSAGE_ID_SIZE];
    char sessionKey[RT_MEDIUS_SESSION_KEY_SIZE];
    char accessKey[RT_NET_KEY_SIZE];
    int worldId;
} MediusJoinChannelFwdResponse;

// The response that describes a game. 0x18c bytes.
typedef struct {
    rtMediusResponse header;
    int applicationId;
    int minPlayers;
    int maxPlayers;
    int gameLevel;
    int playerSkillLevel;
    int playerCount;
    char gameStats[RT_MEDIUS_STATS_SIZE];
    char gameName[RT_MEDIUS_GAME_NAME_SIZE];
    int rulesSet;
    int genericField1;
    int genericField2;
    int genericField3;
    int worldStatus;
    int gameHostType;
} MediusGameInfoResponse;

// The request that looks up a world by its name. 0x6c bytes.
typedef struct {
    rtMediusRequest header;
    char name[RT_MEDIUS_GAME_NAME_SIZE];
    int worldType;
} MediusFindWorldByNameRequest;

// The response that reports one world of a name. 0x90 bytes.
typedef struct {
    rtMediusResponse header;
    int applicationId;
    char applicationName[RT_MEDIUS_PLAYER_NAME_SIZE];
    int applicationType;
    int worldId;
    char worldName[RT_MEDIUS_GAME_NAME_SIZE];
    int worldStatus;
    char endOfList;
} MediusFindWorldByNameResponse;

// The response that reports one location of a player. 0x70 bytes.
typedef struct {
    rtMediusResponse header;
    int applicationId;
    char applicationName[RT_MEDIUS_PLAYER_NAME_SIZE];
    int locationType;
    int worldId;
    int online;
    char accountName[RT_MEDIUS_PLAYER_NAME_SIZE];
    char endOfList;
} MediusFindPlayerResponse;

// A line of chat the server forwards. 0x80 bytes.
typedef struct {
    char messageId[RT_MEDIUS_MESSAGE_ID_SIZE];
    int originatorAccountId;
    char originatorName[RT_MEDIUS_PLAYER_NAME_SIZE];
    int messageType;
    char text[RT_MEDIUS_CHAT_TEXT_SIZE];
} MediusChatFwdMessage;

// The response that reports one account of a buddy or ignore list. 0x48 bytes.
typedef struct {
    rtMediusResponse header;
    int accountId;
    char accountName[RT_MEDIUS_PLAYER_NAME_SIZE];
    int onlineState;
    char endOfList;
} MediusAccountListResponse;

// A response that reports an account and its name. 0x40 bytes.
typedef struct {
    rtMediusResponse header;
    int accountId;
    char accountName[RT_MEDIUS_PLAYER_NAME_SIZE];
} MediusAccountNameResponse;

// The request of another player to add the account to a buddy list. 0x3c bytes.
typedef struct {
    char messageId[RT_MEDIUS_MESSAGE_ID_SIZE];
    int originatorAccountId;
    char originatorName[RT_MEDIUS_PLAYER_NAME_SIZE];
} MediusBuddyConfirmationRequest;

// The reply of the console to MediusBuddyConfirmationRequest. 0x34 bytes.
typedef struct {
    char messageId[RT_MEDIUS_MESSAGE_ID_SIZE];
    int responseType;
    char sessionKey[RT_MEDIUS_SESSION_KEY_SIZE];
    int originatorAccountId;
} MediusBuddyConfirmationReply;

// A notice of the server. 0x104 bytes.
typedef struct {
    int code;
    char text[kNoticeTextSize];
} MediusNotice;

// The response that reports one announcement. 0x40c bytes.
typedef struct {
    rtMediusResponse header;
    int announcementId;
    char announcement[RT_MEDIUS_ANNOUNCEMENT_SIZE];
    char endOfList;
} MediusAnnouncementResponse;

// The response that reports one filter of the game list. 0x2c bytes.
typedef struct {
    rtMediusResponse header;
    int filterField;
    int comparison;
    int value;
    char endOfList;
} MediusGameListFilterResponse;

// A request with a value and a name whose meaning depends on the message. 0x4c bytes.
typedef struct {
    rtMediusRequest header;
    int id;
    char name[RT_MEDIUS_PLAYER_NAME_SIZE];
} MediusIdNameRequest;

// The response that describes a clan by its identifier. 0x168 bytes.
typedef struct {
    rtMediusResponse header;
    int applicationId;
    char clanName[RT_MEDIUS_PLAYER_NAME_SIZE];
    int leaderAccountId;
    char leaderAccountName[RT_MEDIUS_PLAYER_NAME_SIZE];
    char stats[RT_MEDIUS_STATS_SIZE];
    int clanStatus;
} MediusClanByIdResponse;

// The response that describes a clan by its name. 0x148 bytes.
typedef struct {
    rtMediusResponse header;
    int clanId;
    int leaderAccountId;
    char leaderAccountName[RT_MEDIUS_PLAYER_NAME_SIZE];
    char stats[RT_MEDIUS_STATS_SIZE];
    int clanStatus;
} MediusClanByNameResponse;

// A request with a value and a message of a clan. 0xf4 bytes.
typedef struct {
    rtMediusRequest header;
    int id;
    char message[kClanMessageSize];
} MediusClanMessageRequest;

// The response that reports one invitation to a clan. 0x118 bytes.
typedef struct {
    rtMediusResponse header;
    int clanInvitationId;
    int clanId;
    int responseStatus;
    char message[kClanMessageSize];
    int leaderAccountId;
    char leaderAccountName[RT_MEDIUS_PLAYER_NAME_SIZE];
    char endOfList;
} MediusClanInvitationResponse;

// The request that responds to an invitation or a challenge. 0xf8 bytes.
typedef struct {
    rtMediusRequest header;
    int id;
    int response;
    char message[kClanMessageSize];
} MediusClanReplyRequest;

// The request of a page of the messages of a clan. 0x34 bytes.
typedef struct {
    rtMediusRequest header;
    int clanId;
    int start;
    int pageSize;
} MediusClanPageRequest;

// The response that reports one message of a clan. 0xec bytes.
typedef struct {
    rtMediusResponse header;
    int clanId;
    char message[kClanMessageSize];
    char endOfList;
} MediusMyClanMessageResponse;

// The request that sends a message to the clan. 0xee bytes.
typedef struct {
    rtMediusRequest header;
    char message[kClanMessageSize];
} MediusSendClanMessageRequest;

// The request of the challenge history of a clan. 0x38 bytes.
typedef struct {
    rtMediusRequest header;
    int clanId;
    int threshold;
    int start;
    int pageSize;
} MediusChallengeHistoryRequest;

// The response that reports one challenge of the history of a clan. 0x28 bytes.
typedef struct {
    rtMediusResponse header;
    int againstClanId;
    int challengeStatus;
    char endOfList;
} MediusChallengeHistoryResponse;

// The response that reports one invitation the clan sent. 0x114 bytes.
typedef struct {
    rtMediusResponse header;
    int accountId;
    char accountName[RT_MEDIUS_PLAYER_NAME_SIZE];
    char message[kClanMessageSize];
    int responseStatus;
    int responseTime;
    char endOfList;
} MediusInvitationSentResponse;

// The response that describes one clan of the account. 0x170 bytes.
typedef struct {
    rtMediusResponse header;
    int clanId;
    int applicationId;
    char clanName[RT_MEDIUS_PLAYER_NAME_SIZE];
    int leaderAccountId;
    char leaderAccountName[RT_MEDIUS_PLAYER_NAME_SIZE];
    char stats[RT_MEDIUS_STATS_SIZE];
    int clanStatus;
    char endOfList;
} MediusMyClanResponse;

// The response that reports one message of every message of a clan. 0xf0 bytes.
typedef struct {
    rtMediusResponse header;
    int clanMessageId;
    char message[kClanMessageSize];
    int messageStatus;
    char endOfList;
} MediusClanMessageResponse;

// The request of the challenges of a clan. 0x3c bytes.
typedef struct {
    rtMediusRequest header;
    int clanId;
    int challengeStatus;
    int threshold;
    int start;
    int pageSize;
} MediusChallengesRequest;

// The response that reports one challenge of a clan. 0x1c0 bytes.
typedef struct {
    rtMediusResponse header;
    int challengerClanId;
    int againstClanId;
    int challengeStatus;
    int responseTime;
    char challengeMessage[kClanMessageSize];
    char responseMessage[kClanMessageSize];
    char endOfList;
} MediusChallengeResponse;

// The request that changes the statistics of a clan. 0x12c bytes.
typedef struct {
    rtMediusRequest header;
    int clanId;
    char stats[RT_MEDIUS_STATS_SIZE];
} MediusUpdateClanStatsRequest;

// The response that reports the security level of a world. 0x28 bytes.
typedef struct {
    rtMediusResponse header;
    int worldId;
    int applicationType;
    int securityLevel;
} MediusSecurityLevelResponse;

// The response that reports one location. 0x64 bytes.
typedef struct {
    char messageId[RT_MEDIUS_MESSAGE_ID_SIZE];
    int locationId;
    char locationName[RT_MEDIUS_GAME_NAME_SIZE];
    int status;
    char endOfList;
} MediusLocationResponse;

// The response that reports one member of a clan. 0x44 bytes.
typedef struct {
    rtMediusResponse header;
    char accountName[RT_MEDIUS_PLAYER_NAME_SIZE];
    int accountId;
    char endOfList;
} MediusClanMemberResponse;

// The notice that the server moved a game to another world. 8 bytes.
typedef struct {
    int oldWorldId;
    int newWorldId;
} MediusReassignWorld;

// The response that reports one player of the lobby. 0x44 bytes.
typedef struct {
    rtMediusResponse header;
    int accountId;
    char accountName[RT_MEDIUS_PLAYER_NAME_SIZE];
    char endOfList;
} MediusPlayerNameResponse;

// The response that reports the users of the system. 0x24 bytes.
typedef struct {
    rtMediusResponse header;
    int totalInSystem;
    int totalActive;
} MediusTotalUsersResponse;

// The request of the information of the universe. 0x24 bytes.
typedef struct {
    char messageId[RT_MEDIUS_MESSAGE_ID_SIZE];
    int infoType;
    int characterEncoding;
    int language;
} MediusUniverseInformationRequest;

// The response that reports one part of the news of the universe. 0x120 bytes.
typedef struct {
    rtMediusResponse header;
    char news[kNewsSize];
    char endOfList;
} MediusUniverseNewsResponse;

// The response that reports one universe. 0x230 bytes.
typedef struct {
    rtMediusResponse header;
    char universeName[kUniverseNameSize];
    char dns[kUniverseDnsSize];
    int port;
    char description[kUniverseDescriptionSize];
    int status;
    int userCount;
    int maxUsers;
    char endOfList;
} MediusUniverseStatusResponse;

// The request that posts the signature of the console. 0xa6 bytes.
typedef struct {
    rtMediusRequest header;
    char signature[kMachineSignatureSize];
} MediusMachineSignatureRequest;

// NTSC-U/C: 0x003c25c8
short g_nMediusInitialized;
// NTSC-U/C: 0x003c25cc
int g_nMediusConnection = -1;
// NTSC-U/C: 0x003c25d8
char g_szMediusSessionKey[RT_MEDIUS_SESSION_KEY_SIZE];
// NTSC-U/C: 0x003c25f0
rtMediusCallback g_pfnMediusNoticeCallback;
// NTSC-U/C: 0x003c25f4
rtMediusSessionBeginCallback g_pfnMediusSessionBeginCallback;
// NTSC-U/C: 0x003c25f8
rtMediusCallback g_pfnMediusSessionEndCallback;
// NTSC-U/C: 0x003c25fc
rtMediusAccountGetIdCallback g_pfnMediusAccountGetIdCallback;
// NTSC-U/C: 0x003c2600
rtMediusLoginCallback g_pfnMediusAccountLoginCallback;
// NTSC-U/C: 0x003c2604
rtMediusCallback g_pfnMediusMessage022Callback;
// NTSC-U/C: 0x003c2608
rtMediusCallback g_pfnMediusAccountRegistrationCallback;
// NTSC-U/C: 0x003c260c
rtMediusCallback g_pfnMediusMessage014Callback;
// NTSC-U/C: 0x003c2610
rtMediusCallback g_pfnMediusChangePasswordCallback;
// NTSC-U/C: 0x003c2614
rtMediusCallback g_pfnMediusUpdateStatsCallback;
// NTSC-U/C: 0x003c2618
rtMediusCallback g_pfnMediusMessage012Callback;
// NTSC-U/C: 0x003c261c
rtMediusCallback g_pfnMediusAccountDeleteCallback;
// NTSC-U/C: 0x003c2620
rtMediusLoginCallback g_pfnMediusAnonymousLoginCallback;
// NTSC-U/C: 0x003c2624
rtMediusChannelPlayersCallback g_pfnMediusChannelPlayersCallback;
// NTSC-U/C: 0x003c2628
rtMediusGamePlayersCallback g_pfnMediusGamePlayersCallback;
// NTSC-U/C: 0x003c262c
rtMediusCallback g_pfnMediusMessage030Callback;
// NTSC-U/C: 0x003c2630
rtMediusCallback g_pfnMediusMessage032Callback;
// NTSC-U/C: 0x003c2634
rtMediusCreateChannelCallback g_pfnMediusCreateChannelCallback;
// NTSC-U/C: 0x003c2638
rtMediusJoinGameCallback g_pfnMediusJoinGameCallback;
// NTSC-U/C: 0x003c263c
rtMediusJoinChannelCallback g_pfnMediusJoinChannelCallback;
// NTSC-U/C: 0x003c2640
rtMediusGameListCallback g_pfnMediusGameListCallback;
// NTSC-U/C: 0x003c2644
rtMediusChannelListCallback g_pfnMediusChannelListCallback;
// NTSC-U/C: 0x003c2648
rtMediusPlayerInfoCallback g_pfnMediusPlayerInfoCallback;
// NTSC-U/C: 0x003c264c
rtMediusCallback g_pfnMediusMessage052Callback;
// NTSC-U/C: 0x003c2650
rtMediusChannelInfoCallback g_pfnMediusChannelInfoCallback;
// NTSC-U/C: 0x003c2654
rtMediusCallback g_pfnMediusMessage056Callback;
// NTSC-U/C: 0x003c2658
rtMediusFindPlayerCallback g_pfnMediusFindPlayerCallback;
// NTSC-U/C: 0x003c265c
rtMediusChatHandler g_pfnMediusChatMessageCallback;
// NTSC-U/C: 0x003c2660
rtMediusCallback g_pfnMediusMessage062Callback;
// NTSC-U/C: 0x003c2664
rtMediusCallback g_pfnMediusMessage064Callback;
// NTSC-U/C: 0x003c2668
rtMediusCallback g_pfnMediusMessage066Callback;
// NTSC-U/C: 0x003c266c
rtMediusCallback g_pfnMediusInitHandler04Callback;
// NTSC-U/C: 0x003c2670
rtMediusCallback g_pfnMediusMessage068Callback;
// NTSC-U/C: 0x003c2674
rtMediusCallback g_pfnMediusMessage193Callback;
// NTSC-U/C: 0x003c2678
rtMediusCallback g_pfnMediusMessage195Callback;
// NTSC-U/C: 0x003c267c
rtMediusCallback g_pfnMediusMessage197Callback;
// NTSC-U/C: 0x003c2680
rtMediusPolicyCallback g_pfnMediusGetPolicyCallback;
// NTSC-U/C: 0x003c2684
rtMediusAnnouncementCallback g_pfnMediusGetAnnouncementsCallback;
// NTSC-U/C: 0x003c2688
rtMediusCallback g_pfnMediusSetGameListFilterCallback;
// NTSC-U/C: 0x003c268c
rtMediusCallback g_pfnMediusClearGameListFilterCallback;
// NTSC-U/C: 0x003c2690
rtMediusCallback g_pfnMediusMessage083Callback;
// NTSC-U/C: 0x003c2694
rtMediusVersionCallback g_pfnMediusVersionServerCallback;
// NTSC-U/C: 0x003c2698
rtMediusCallback g_pfnMediusMessage028Callback;
// NTSC-U/C: 0x003c269c
rtMediusCallback g_pfnMediusMessage137Callback;
// NTSC-U/C: 0x003c26a0
rtMediusCallback g_pfnMediusBanPlayerCallback;
// NTSC-U/C: 0x003c26a4
rtMediusCallback g_pfnMediusMessage141Callback;
// NTSC-U/C: 0x003c26a8
rtMediusCallback g_pfnMediusMessage143Callback;
// NTSC-U/C: 0x003c26ac
rtMediusLadderPositionCallback g_pfnMediusLadderPositionCallback;
// NTSC-U/C: 0x003c26b0
rtMediusLadderPositionCallback g_pfnMediusLadderPositionFastCallback;
// NTSC-U/C: 0x003c26b4
rtMediusLadderListCallback g_pfnMediusLadderListCallback;
// NTSC-U/C: 0x003c26b8
rtMediusCallback g_pfnMediusMessage151Callback;
// NTSC-U/C: 0x003c26bc
rtMediusTextFilterCallback g_pfnMediusTextFilterCallback;
// NTSC-U/C: 0x003c26c0
rtMediusCallback g_pfnMediusInitHandler0CCallback;
// NTSC-U/C: 0x003c26c4
rtMediusCallback g_pfnMediusMessage156Callback;
// NTSC-U/C: 0x003c26c8
rtMediusCallback g_pfnMediusMessage158Callback;
// NTSC-U/C: 0x003c26cc
rtMediusCallback g_pfnMediusMessage160Callback;
// NTSC-U/C: 0x003c26d0
rtMediusCallback g_pfnMediusMessage162Callback;
// NTSC-U/C: 0x003c26d4
rtMediusCallback g_pfnMediusMessage164Callback;
// NTSC-U/C: 0x003c26d8
rtMediusCallback g_pfnMediusMessage199Callback;
// NTSC-U/C: 0x003c26dc
rtMediusCallback g_pfnMediusMessage200Callback;
// NTSC-U/C: 0x003c26e0
rtMediusCallback g_pfnMediusMessage201Callback;
// NTSC-U/C: 0x003c26e4
rtMediusCallback g_pfnMediusMessage202Callback;
// NTSC-U/C: 0x004357f4
rtMediusFileCallback g_pfnMediusFileCreateCallback;
// NTSC-U/C: 0x004357f8
rtMediusFileTransferCallback g_pfnMediusFileUploadCallback;
// NTSC-U/C: 0x004357fc
const char *g_pMediusUploadBuffer;
// NTSC-U/C: 0x00435800
int g_nMediusUploadSize;
// NTSC-U/C: 0x00435804
rtMediusFileCallback g_pfnMediusFileCloseCallback;
// NTSC-U/C: 0x00435808
rtMediusFileTransferCallback g_pfnMediusFileDownloadCallback;
// NTSC-U/C: 0x0043580c
rtMediusCallback g_pfnMediusMessage175Callback;
// NTSC-U/C: 0x00435810
rtMediusFileDeleteCallback g_pfnMediusFileDeleteCallback;
// NTSC-U/C: 0x00435814
rtMediusFileCallback g_pfnMediusFileListCallback;
// NTSC-U/C: 0x00435818
rtMediusCallback g_pfnMediusMessage181Callback;
// NTSC-U/C: 0x0043581c
rtMediusFileAttributesCallback g_pfnMediusFileGetAttributesCallback;
// NTSC-U/C: 0x00435820
rtMediusCallback g_pfnMediusMessage185Callback;
// NTSC-U/C: 0x00435824
rtMediusCallback g_pfnMediusMessage187Callback;
// NTSC-U/C: 0x00435828
rtMediusCallback g_pfnMediusMessage189Callback;
// NTSC-U/C: 0x0043582c
rtMediusCallback g_pfnMediusMessage191Callback;
// NTSC-U/C: 0x00435830
int g_nMediusFileCreateType;
// NTSC-U/C: 0x00435834
int g_nMediusFileUploadType;
// NTSC-U/C: 0x00435838
int g_nMediusFileCloseType;
// NTSC-U/C: 0x0043583c
int g_nMediusFileDownloadType;
// NTSC-U/C: 0x00435840
static int g_nMediusFileDownloadStreamType;
// NTSC-U/C: 0x00435844
int g_nMediusFileDeleteType;
// NTSC-U/C: 0x00435848
int g_nMediusFileListType;
// NTSC-U/C: 0x0043584c
static int g_nMediusFileUpdateAttributesType;
// NTSC-U/C: 0x00435850
int g_nMediusFileGetAttributesType;
// NTSC-U/C: 0x00435854
static int g_nMediusFileUpdateMetaDataType;
// NTSC-U/C: 0x00435858
static int g_nMediusFileGetMetaDataType;
// NTSC-U/C: 0x0043585c
static int g_nMediusFileSearchByMetaDataType;
// NTSC-U/C: 0x00435860
static int g_nMediusFileCancelOperationType;
// NTSC-U/C: 0x00435864
int g_nMediusJoinGameType;
// NTSC-U/C: 0x00435868
int g_nMediusMessage031Type;
// NTSC-U/C: 0x0043586c
int g_nMediusVersionServerType;
// NTSC-U/C: 0x00435870
int g_nMediusUpdateUserStateType;
// NTSC-U/C: 0x00435874
int g_nMediusSessionBeginType;
// NTSC-U/C: 0x00435878
int g_nMediusSessionEndType;
// NTSC-U/C: 0x0043587c
int g_nMediusAccountLoginType;
// NTSC-U/C: 0x00435880
static int g_nMediusAccountLogoutType;
// NTSC-U/C: 0x00435884
static int g_nMediusAccountGetProfileType;
// NTSC-U/C: 0x00435888
int g_nMediusAccountRegistrationType;
// NTSC-U/C: 0x0043588c
int g_nMediusChangePasswordType;
// NTSC-U/C: 0x00435890
int g_nMediusUpdateStatsType;
// NTSC-U/C: 0x00435894
static int g_nMediusAccountUpdateProfileType;
// NTSC-U/C: 0x00435898
int g_nMediusAccountDeleteType;
// NTSC-U/C: 0x0043589c
int g_nMediusAnonymousLoginType;
// NTSC-U/C: 0x004358a0
static int g_nMediusCreateGameType;
// NTSC-U/C: 0x004358a4
int g_nMediusTextFilterType;
// NTSC-U/C: 0x004358a8
int g_nMediusCreateChannelType;
// NTSC-U/C: 0x004358ac
int g_nMediusJoinChannelType;
// NTSC-U/C: 0x004358b0
int g_nMediusGameListType;
// NTSC-U/C: 0x004358b4
int g_nMediusChannelListType;
// NTSC-U/C: 0x004358b8
int g_nMediusChannelPlayersType;
// NTSC-U/C: 0x004358bc
int g_nMediusGamePlayersType;
// NTSC-U/C: 0x004358c0
int g_nMediusPlayerInfoType;
// NTSC-U/C: 0x004358c4
static int g_nMediusGameInfoType;
// NTSC-U/C: 0x004358c8
int g_nMediusChannelInfoType;
// NTSC-U/C: 0x004358cc
int g_nMediusFindPlayerType;
// NTSC-U/C: 0x004358d0
static int g_nMediusFindWorldByNameType;
// NTSC-U/C: 0x004358d4
int g_nMediusChatMessageType;
// NTSC-U/C: 0x004358d8
static int g_nMediusGetBuddyListType;
// NTSC-U/C: 0x004358dc
static int g_nMediusAddToBuddyListType;
// NTSC-U/C: 0x004358e0
static int g_nMediusRemoveFromBuddyListType;
// NTSC-U/C: 0x004358e4
static int g_nMediusAddToBuddyListConfirmationType;
// NTSC-U/C: 0x004358e8
static int g_nMediusAddToBuddyListFwdConfirmationResponseType;
// NTSC-U/C: 0x004358ec
static int g_nMediusGetIgnoreListType;
// NTSC-U/C: 0x004358f0
static int g_nMediusAddToIgnoreListType;
// NTSC-U/C: 0x004358f4
static int g_nMediusRemoveFromIgnoreListType;
// NTSC-U/C: 0x004358f8
int g_nMediusGetPolicyType;
// NTSC-U/C: 0x004358fc
int g_nMediusPlayerReportType;
// NTSC-U/C: 0x00435900
int g_nMediusWorldReportType;
// NTSC-U/C: 0x00435904
int g_nMediusEndGameReportType;
// NTSC-U/C: 0x00435908
int g_nMediusGetAnnouncementsType;
// NTSC-U/C: 0x0043590c
static int g_nMediusGetAllAnnouncementsType;
// NTSC-U/C: 0x00435910
int g_nMediusSetGameListFilterType;
// NTSC-U/C: 0x00435914
int g_nMediusClearGameListFilterType;
// NTSC-U/C: 0x00435918
static int g_nMediusGetGameListFilterType;
// NTSC-U/C: 0x0043591c
static int g_nMediusGetMyIpType;
// NTSC-U/C: 0x00435920
static int g_nMediusGetWorldSecurityLevelType;
// NTSC-U/C: 0x00435924
int g_nMediusBanPlayerType;
// NTSC-U/C: 0x00435928
int g_nMediusAccountGetIdType;
// NTSC-U/C: 0x0043592c
static int g_nMediusGetLocationsType;
// NTSC-U/C: 0x00435930
static int g_nMediusPickLocationType;
// NTSC-U/C: 0x00435934
int g_nMediusLadderPositionType;
// NTSC-U/C: 0x00435938
int g_nMediusLadderPositionFastType;
// NTSC-U/C: 0x0043593c
int g_nMediusLadderListType;
// NTSC-U/C: 0x00435940
static int g_nMediusChatToggleType;
// NTSC-U/C: 0x00435944
static int g_nMediusGetTotalGamesType;
// NTSC-U/C: 0x00435948
static int g_nMediusGetTotalChannelsType;
// NTSC-U/C: 0x0043594c
static int g_nMediusGetLobbyPlayerNamesType;
// NTSC-U/C: 0x00435950
static int g_nMediusGetTotalUsersType;
// NTSC-U/C: 0x00435954
static int g_nMediusSetLocalizationParamsType;
// NTSC-U/C: 0x00435958
static int g_nMediusSetMessageAsReadType;
// NTSC-U/C: 0x0043595c
static int g_nMediusGetUniverseInformationType;
// NTSC-U/C: 0x00435960
static int g_nMediusSessionBeginResponseType;
// NTSC-U/C: 0x00435964
static int g_nMediusSessionEndResponseType;
// NTSC-U/C: 0x00435968
static int g_nMediusAccountLoginResponseType;
// NTSC-U/C: 0x0043596c
static int g_nMediusAccountRegistrationResponseType;
// NTSC-U/C: 0x00435970
static int g_nMediusAccountGetProfileResponseType;
// NTSC-U/C: 0x00435974
static int g_nMediusAccountUpdateProfileResponseType;
// NTSC-U/C: 0x00435978
static int g_nMediusChangePasswordResponseType;
// NTSC-U/C: 0x0043597c
static int g_nMediusUpdateStatsResponseType;
// NTSC-U/C: 0x00435980
static int g_nMediusAccountDeleteResponseType;
// NTSC-U/C: 0x00435984
static int g_nMediusAccountLogoutResponseType;
// NTSC-U/C: 0x00435988
static int g_nMediusAccountGetIdResponseType;
// NTSC-U/C: 0x0043598c
static int g_nMediusAnonymousLoginResponseType;
// NTSC-U/C: 0x00435990
static int g_nMediusGetMyIpResponseType;
// NTSC-U/C: 0x00435994
static int g_nMediusCreateGameResponseType;
// NTSC-U/C: 0x00435998
static int g_nMediusMessage032Type;
// NTSC-U/C: 0x0043599c
static int g_nMediusCreateChannelResponseType;
// NTSC-U/C: 0x004359a0
static int g_nMediusJoinGameResponseType;
// NTSC-U/C: 0x004359a4
static int g_nMediusJoinChannelResponseType;
// NTSC-U/C: 0x004359a8
static int g_nMediusJoinChannelFwdType;
// NTSC-U/C: 0x004359ac
static int g_nMediusJoinChannelFwdResponseType;
// NTSC-U/C: 0x004359b0
static int g_nMediusGameListResponseType;
// NTSC-U/C: 0x004359b4
static int g_nMediusChannelListResponseType;
// NTSC-U/C: 0x004359b8
static int g_nMediusChannelPlayersResponseType;
// NTSC-U/C: 0x004359bc
static int g_nMediusGamePlayersResponseType;
// NTSC-U/C: 0x004359c0
static int g_nMediusPlayerInfoResponseType;
// NTSC-U/C: 0x004359c4
static int g_nMediusGameInfoResponseType;
// NTSC-U/C: 0x004359c8
static int g_nMediusChannelInfoResponseType;
// NTSC-U/C: 0x004359cc
static int g_nMediusFindWorldByNameResponseType;
// NTSC-U/C: 0x004359d0
static int g_nMediusFindPlayerResponseType;
// NTSC-U/C: 0x004359d4
static int g_nMediusChatFwdMessageType;
// NTSC-U/C: 0x004359d8
static int g_nMediusGetBuddyListResponseType;
// NTSC-U/C: 0x004359dc
static int g_nMediusAddToBuddyListResponseType;
// NTSC-U/C: 0x004359e0
static int g_nMediusRemoveFromBuddyListResponseType;
// NTSC-U/C: 0x004359e4
static int g_nMediusAddToBuddyListConfirmationResponseType;
// NTSC-U/C: 0x004359e8
static int g_nMediusAddToBuddyListFwdConfirmationType;
// NTSC-U/C: 0x004359ec
static int g_nMediusGetPolicyResponseType;
// NTSC-U/C: 0x004359f0
static int g_nMediusNoticeType;
// NTSC-U/C: 0x004359f4
static int g_nMediusGetAnnouncementsResponseType;
// NTSC-U/C: 0x004359f8
static int g_nMediusSetGameListFilterResponseType;
// NTSC-U/C: 0x004359fc
static int g_nMediusClearGameListFilterResponseType;
// NTSC-U/C: 0x00435a00
static int g_nMediusGetGameListFilterResponseType;
// NTSC-U/C: 0x00435a04
static int g_nMediusCreateClanType;
// NTSC-U/C: 0x00435a08
static int g_nMediusCreateClanResponseType;
// NTSC-U/C: 0x00435a0c
static int g_nMediusDisbandClanType;
// NTSC-U/C: 0x00435a10
static int g_nMediusDisbandClanResponseType;
// NTSC-U/C: 0x00435a14
static int g_nMediusGetClanByIdType;
// NTSC-U/C: 0x00435a18
static int g_nMediusGetClanByIdResponseType;
// NTSC-U/C: 0x00435a1c
static int g_nMediusGetClanByNameType;
// NTSC-U/C: 0x00435a20
static int g_nMediusGetClanByNameResponseType;
// NTSC-U/C: 0x00435a24
static int g_nMediusTransferClanLeadershipType;
// NTSC-U/C: 0x00435a28
static int g_nMediusTransferClanLeadershipResponseType;
// NTSC-U/C: 0x00435a2c
static int g_nMediusAddPlayerToClanType;
// NTSC-U/C: 0x00435a30
static int g_nMediusAddPlayerToClanResponseType;
// NTSC-U/C: 0x00435a34
static int g_nMediusRemovePlayerFromClanType;
// NTSC-U/C: 0x00435a38
static int g_nMediusRemovePlayerFromClanResponseType;
// NTSC-U/C: 0x00435a3c
static int g_nMediusInvitePlayerToClanType;
// NTSC-U/C: 0x00435a40
static int g_nMediusInvitePlayerToClanResponseType;
// NTSC-U/C: 0x00435a44
static int g_nMediusCheckMyClanInvitationsType;
// NTSC-U/C: 0x00435a48
static int g_nMediusCheckMyClanInvitationsResponseType;
// NTSC-U/C: 0x00435a4c
static int g_nMediusRespondToClanInvitationType;
// NTSC-U/C: 0x00435a50
static int g_nMediusRespondToClanInvitationResponseType;
// NTSC-U/C: 0x00435a54
static int g_nMediusRevokeClanInvitationType;
// NTSC-U/C: 0x00435a58
static int g_nMediusRevokeClanInvitationResponseType;
// NTSC-U/C: 0x00435a5c
static int g_nMediusRequestClanTeamChallengeType;
// NTSC-U/C: 0x00435a60
static int g_nMediusRequestClanTeamChallengeResponseType;
// NTSC-U/C: 0x00435a64
static int g_nMediusGetMyClanMessagesType;
// NTSC-U/C: 0x00435a68
static int g_nMediusGetMyClanMessagesResponseType;
// NTSC-U/C: 0x00435a6c
static int g_nMediusSendClanMessageType;
// NTSC-U/C: 0x00435a70
static int g_nMediusSendClanMessageResponseType;
// NTSC-U/C: 0x00435a74
static int g_nMediusModifyClanMessageType;
// NTSC-U/C: 0x00435a78
static int g_nMediusModifyClanMessageResponseType;
// NTSC-U/C: 0x00435a7c
static int g_nMediusDeleteClanMessageType;
// NTSC-U/C: 0x00435a80
static int g_nMediusDeleteClanMessageResponseType;
// NTSC-U/C: 0x00435a84
static int g_nMediusRespondToClanTeamChallengeType;
// NTSC-U/C: 0x00435a88
static int g_nMediusRespondToClanTeamChallengeResponseType;
// NTSC-U/C: 0x00435a8c
static int g_nMediusRevokeClanTeamChallengeType;
// NTSC-U/C: 0x00435a90
static int g_nMediusRevokeClanTeamChallengeResponseType;
// NTSC-U/C: 0x00435a94
static int g_nMediusGetClanTeamChallengeHistoryType;
// NTSC-U/C: 0x00435a98
static int g_nMediusGetClanTeamChallengeHistoryResponseType;
// NTSC-U/C: 0x00435a9c
static int g_nMediusGetClanInvitationsSentType;
// NTSC-U/C: 0x00435aa0
static int g_nMediusGetClanInvitationsSentResponseType;
// NTSC-U/C: 0x00435aa4
static int g_nMediusGetMyClansType;
// NTSC-U/C: 0x00435aa8
static int g_nMediusGetMyClansResponseType;
// NTSC-U/C: 0x00435aac
static int g_nMediusGetAllClanMessagesType;
// NTSC-U/C: 0x00435ab0
static int g_nMediusGetAllClanMessagesResponseType;
// NTSC-U/C: 0x00435ab4
static int g_nMediusConfirmClanTeamChallengeType;
// NTSC-U/C: 0x00435ab8
static int g_nMediusConfirmClanTeamChallengeResponseType;
// NTSC-U/C: 0x00435abc
static int g_nMediusGetClanTeamChallengesType;
// NTSC-U/C: 0x00435ac0
static int g_nMediusGetClanTeamChallengesResponseType;
// NTSC-U/C: 0x00435ac4
static int g_nMediusUpdateClanStatsType;
// NTSC-U/C: 0x00435ac8
static int g_nMediusUpdateClanStatsResponseType;
// NTSC-U/C: 0x00435acc
static int g_nMediusVersionServerResponseType;
// NTSC-U/C: 0x00435ad0
static int g_nMediusGetWorldSecurityLevelResponseType;
// NTSC-U/C: 0x00435ad4
static int g_nMediusBanPlayerResponseType;
// NTSC-U/C: 0x00435ad8
static int g_nMediusGetLocationsResponseType;
// NTSC-U/C: 0x00435adc
static int g_nMediusPickLocationResponseType;
// NTSC-U/C: 0x00435ae0
static int g_nMediusGetClanMemberListType;
// NTSC-U/C: 0x00435ae4
static int g_nMediusGetClanMemberListResponseType;
// NTSC-U/C: 0x00435ae8
static int g_nMediusLadderPositionResponseType;
// NTSC-U/C: 0x00435aec
static int g_nMediusLadderListResponseType;
// NTSC-U/C: 0x00435af0
static int g_nMediusChatToggleResponseType;
// NTSC-U/C: 0x00435af4
static int g_nMediusTextFilterResponseType;
// NTSC-U/C: 0x00435af8
static int g_nMediusReassignWorldType;
// NTSC-U/C: 0x00435afc
static int g_nMediusGetTotalGamesResponseType;
// NTSC-U/C: 0x00435b00
static int g_nMediusGetTotalChannelsResponseType;
// NTSC-U/C: 0x00435b04
static int g_nMediusGetLobbyPlayerNamesResponseType;
// NTSC-U/C: 0x00435b08
static int g_nMediusGetTotalUsersResponseType;
// NTSC-U/C: 0x00435b0c
static int g_nMediusSetLocalizationParamsResponseType;
// NTSC-U/C: 0x00435b10
static int g_nMediusFileCreateResponseType;
// NTSC-U/C: 0x00435b14
int g_nMediusFileUploadPartType;
// NTSC-U/C: 0x00435b18
static int g_nMediusFileUploadRequestType;
// NTSC-U/C: 0x00435b1c
static int g_nMediusFileCloseResponseType;
// NTSC-U/C: 0x00435b20
static int g_nMediusFileDownloadResponseType;
// NTSC-U/C: 0x00435b24
static int g_nMediusFileDownloadStreamResponseType;
// NTSC-U/C: 0x00435b28
static int g_nMediusFileDeleteResponseType;
// NTSC-U/C: 0x00435b2c
static int g_nMediusFileListResponseType;
// NTSC-U/C: 0x00435b30
static int g_nMediusFileUpdateAttributesResponseType;
// NTSC-U/C: 0x00435b34
static int g_nMediusFileGetAttributesResponseType;
// NTSC-U/C: 0x00435b38
static int g_nMediusFileUpdateMetaDataResponseType;
// NTSC-U/C: 0x00435b3c
static int g_nMediusFileGetMetaDataResponseType;
// NTSC-U/C: 0x00435b40
static int g_nMediusFileSearchByMetaDataResponseType;
// NTSC-U/C: 0x00435b44
static int g_nMediusFileCancelOperationResponseType;
// NTSC-U/C: 0x00435b48
static int g_nMediusGetIgnoreListResponseType;
// NTSC-U/C: 0x00435b4c
static int g_nMediusAddToIgnoreListResponseType;
// NTSC-U/C: 0x00435b50
static int g_nMediusRemoveFromIgnoreListResponseType;
// NTSC-U/C: 0x00435b54
static int g_nMediusSetMessageAsReadResponseType;
// NTSC-U/C: 0x00435b58
static int g_nMediusUniverseNewsResponseType;
// NTSC-U/C: 0x00435b5c
static int g_nMediusUniverseStatusListResponseType;
// NTSC-U/C: 0x00435b60
static int g_nMediusMachineSignaturePostType;
// NTSC-U/C: 0x00435b64
static int g_nMediusLadderPositionFastResponseType;
// NTSC-U/C: 0x00435b68
rtMediusCallback g_pfnMediusMessage085Callback;
// NTSC-U/C: 0x00435b6c
rtMediusCallback g_pfnMediusMessage087Callback;
// NTSC-U/C: 0x00435b70
rtMediusCallback g_pfnMediusMessage089Callback;
// NTSC-U/C: 0x00435b74
rtMediusCallback g_pfnMediusMessage091Callback;
// NTSC-U/C: 0x00435b78
rtMediusCallback g_pfnMediusMessage093Callback;
// NTSC-U/C: 0x00435b7c
rtMediusCallback g_pfnMediusMessage095Callback;
// NTSC-U/C: 0x00435b80
rtMediusCallback g_pfnMediusMessage097Callback;
// NTSC-U/C: 0x00435b84
rtMediusCallback g_pfnMediusMessage099Callback;
// NTSC-U/C: 0x00435b88
rtMediusCallback g_pfnMediusMessage101Callback;
// NTSC-U/C: 0x00435b8c
rtMediusCallback g_pfnMediusMessage123Callback;
// NTSC-U/C: 0x00435b90
rtMediusCallback g_pfnMediusMessage103Callback;
// NTSC-U/C: 0x00435b94
rtMediusCallback g_pfnMediusMessage105Callback;
// NTSC-U/C: 0x00435b98
rtMediusCallback g_pfnMediusMessage107Callback;
// NTSC-U/C: 0x00435b9c
rtMediusCallback g_pfnMediusMessage109Callback;
// NTSC-U/C: 0x00435ba0
rtMediusCallback g_pfnMediusMessage111Callback;
// NTSC-U/C: 0x00435ba4
rtMediusCallback g_pfnMediusMessage113Callback;
// NTSC-U/C: 0x00435ba8
rtMediusCallback g_pfnMediusMessage115Callback;
// NTSC-U/C: 0x00435bac
rtMediusCallback g_pfnMediusMessage117Callback;
// NTSC-U/C: 0x00435bb0
rtMediusCallback g_pfnMediusMessage119Callback;
// NTSC-U/C: 0x00435bb4
rtMediusCallback g_pfnMediusMessage125Callback;
// NTSC-U/C: 0x00435bb8
rtMediusCallback g_pfnMediusMessage131Callback;
// NTSC-U/C: 0x00435bbc
rtMediusCallback g_pfnMediusMessage129Callback;
// NTSC-U/C: 0x00435bc0
rtMediusCallback g_pfnMediusMessage127Callback;
// NTSC-U/C: 0x00435bc4
rtMediusCallback g_pfnMediusMessage121Callback;
// NTSC-U/C: 0x00435bc8
rtMediusCallback g_pfnMediusMessage133Callback;
// NTSC-U/C: 0x00435bcc
rtMediusCallback g_pfnMediusMessage145Callback;

// NTSC-U/C: 0x003c25d0
static unsigned int g_uMediusKeepAliveMs;
// NTSC-U/C: 0x003c25d4
static unsigned int g_uMediusLastKeepAlive;
// NTSC-U/C: 0x003c25ec
static int g_nMediusLastError;
// NTSC-U/C: 0x003c26e8
static int g_nMediusQueryState;
// NTSC-U/C: 0x003c26ec
static int g_nMediusPendingKind = kPendingNone;
// NTSC-U/C: 0x003c26f0
static rtMediusConnectCallback g_pfnMediusConnectCallback;
// NTSC-U/C: 0x0052d140
static rtNetAddress g_mediusServerAddress;
// NTSC-U/C: 0x0052d158
static rtNetAddress g_mediusHostAddresses[RT_NET_ADDRESS_COUNT];
// NTSC-U/C: 0x0052d188
static rtMediusJoinGameRequest g_mediusPendingJoinGame;
// NTSC-U/C: 0x0052d208
static MediusMessage031Request g_mediusPendingMessage031;

// Reports whether the lobby client is ready to send, as the entry points check it.
static inline int rtMediusCheckConnection(void) {
    if (g_nMediusInitialized != 1) {
        return RT_MEDIUS_ERROR_NOT_INITIALIZED;
    }
    if (g_nMediusConnection < 0) {
        return RT_MEDIUS_ERROR_NOT_CONNECTED;
    }
    return 0;
}

// Sends a record to the lobby server.
static inline int rtMediusSendRecord(int nType, int nSize, const void *pRecord) {
    if (rtDmeSendClassMessage(kSendFlags,
                              g_nMediusConnection,
                              kServerReceiver,
                              RT_DME_CLASS_LOBBY,
                              nType,
                              nSize,
                              pRecord) != 0) {
        return RT_MEDIUS_ERROR_SENDING_MESSAGE;
    }
    return 0;
}

// NTSC-U/C: 0x002f9888, PAL: 0x00303888
// Terminates a text field at its last byte, or empties it when it has no room.
static int rtMediusTruncateField(char *pszField, int nSize) {
    if (pszField == NULL) {
        return kErrorNullField;
    }
    if (nSize <= 0) {
        pszField[0] = '\0';
    } else {
        pszField[nSize - 1] = '\0';
    }
    return 0;
}

// Terminates the identifier and the session key of a request.
static inline void rtMediusTruncateRequest(rtMediusRequest *pHeader) {
    rtMediusTruncateField(pHeader->messageId, sizeof(pHeader->messageId));
    rtMediusTruncateField(pHeader->sessionKey, sizeof(pHeader->sessionKey));
}

int rtMediusFileCreate(rtMediusFileCreateRequest *request, rtMediusFileCallback callback) {
    int nError;

    rtMediusTruncateField(request->messageId, sizeof(request->messageId));
    nError = rtMediusCheckConnection();
    if (nError != 0) {
        return nError;
    }
    g_pfnMediusFileCreateCallback = callback;
    return rtMediusSendRecord(g_nMediusFileCreateType, sizeof(*request), request);
}

int rtMediusFileUpload(rtMediusFileUploadRequest *request, rtMediusFileTransferCallback callback) {
    int nError;

    rtMediusTruncateField(request->messageId, sizeof(request->messageId));
    nError = rtMediusCheckConnection();
    if (nError != 0) {
        return nError;
    }
    g_pfnMediusFileUploadCallback = callback;
    g_pMediusUploadBuffer = request->buffer;
    g_nMediusUploadSize = request->bufferSize;
    return rtMediusSendRecord(g_nMediusFileUploadType, sizeof(*request), request);
}

int rtMediusFileClose(rtMediusFileRequest *request, rtMediusFileCallback callback) {
    int nError;

    rtMediusTruncateField(request->messageId, sizeof(request->messageId));
    nError = rtMediusCheckConnection();
    if (nError != 0) {
        return nError;
    }
    g_pfnMediusFileCloseCallback = callback;
    return rtMediusSendRecord(g_nMediusFileCloseType, sizeof(*request), request);
}

int rtMediusFileDownload(rtMediusFileRequest *request, rtMediusFileTransferCallback callback) {
    int nError;

    rtMediusTruncateField(request->messageId, sizeof(request->messageId));
    nError = rtMediusCheckConnection();
    if (nError != 0) {
        return nError;
    }
    g_pfnMediusFileDownloadCallback = callback;
    return rtMediusSendRecord(g_nMediusFileDownloadType, sizeof(*request), request);
}

int rtMediusFileDelete(rtMediusFileRequest *request, rtMediusFileDeleteCallback callback) {
    int nError;

    rtMediusTruncateField(request->messageId, sizeof(request->messageId));
    nError = rtMediusCheckConnection();
    if (nError != 0) {
        return nError;
    }
    g_pfnMediusFileDeleteCallback = callback;
    return rtMediusSendRecord(g_nMediusFileDeleteType, sizeof(*request), request);
}

int rtMediusFileList(rtMediusFileListRequest *request, rtMediusFileCallback callback) {
    int nError;

    rtMediusTruncateField(request->messageId, sizeof(request->messageId));
    nError = rtMediusCheckConnection();
    if (nError != 0) {
        return nError;
    }
    g_pfnMediusFileListCallback = callback;
    return rtMediusSendRecord(g_nMediusFileListType, sizeof(*request), request);
}

int rtMediusFileGetAttributes(rtMediusFileRequest *request,
                              rtMediusFileAttributesCallback callback) {
    int nError;

    rtMediusTruncateField(request->messageId, sizeof(request->messageId));
    nError = rtMediusCheckConnection();
    if (nError != 0) {
        return nError;
    }
    g_pfnMediusFileGetAttributesCallback = callback;
    return rtMediusSendRecord(g_nMediusFileGetAttributesType, sizeof(*request), request);
}

void rtMediusSetServerAddress(const rtNetAddress *address) {
    g_mediusServerAddress = *address;
}

// NTSC-U/C: 0x002f9930, PAL: 0x00303930
static void rtMediusGetServerAddress(rtNetAddress *pAddress) {
    *pAddress = g_mediusServerAddress;
}

// NTSC-U/C: 0x002f9970, PAL: 0x00303970
static void rtMediusSetQueryState(int nState) {
    g_nMediusQueryState = nState;
}

// NTSC-U/C: 0x002f9980, PAL: 0x00303980
static int rtMediusGetQueryState(void) {
    return g_nMediusQueryState;
}

// NTSC-U/C: 0x002f9990, PAL: 0x00303990
static void rtMediusSetHostAddresses(const rtNetAddress *pAddresses) {
    memcpy(g_mediusHostAddresses, pAddresses, sizeof(g_mediusHostAddresses));
}

// NTSC-U/C: 0x002f9a68, PAL: 0x00303a68
static void rtMediusGetHostAddresses(rtNetAddress *pAddresses) {
    memcpy(pAddresses, g_mediusHostAddresses, sizeof(g_mediusHostAddresses));
}

// NTSC-U/C: 0x002f9ad8, PAL: 0x00303ad8
// Receives the addresses the server sees for the console.
static void rtMediusOnAddressResult(const rtDmeAddressResult *pResult) {
    if (pResult->error != 0) {
        rtMediusSetQueryState(kQueryFailed);
        return;
    }
    if (pResult->complete == 0) {
        rtMediusSetQueryState(kQueryFailed);
        return;
    }
    rtMediusSetHostAddresses(pResult->addresses);
    rtMediusSetQueryState(kQueryReady);
}

// NTSC-U/C: 0x002f9b30, PAL: 0x00303b30
// Stores the request that waits for the addresses. Another kind stores nothing.
static void rtMediusStorePendingRequest(int nKind, const void *pRequest) {
    g_nMediusPendingKind = nKind;
    if (nKind == kPendingMessage031) {
        memcpy(&g_mediusPendingMessage031, pRequest, sizeof(g_mediusPendingMessage031));
    } else if (nKind == kPendingJoinGame) {
        memcpy(&g_mediusPendingJoinGame, pRequest, sizeof(g_mediusPendingJoinGame));
    }
}

// NTSC-U/C: 0x002f9cd0, PAL: 0x00303cd0
static void rtMediusGetPendingKind(int *pnKind) {
    *pnKind = g_nMediusPendingKind;
}

// NTSC-U/C: 0x002f9ce0, PAL: 0x00303ce0
static void rtMediusLoadPendingRequest(void *pRequest) {
    if (g_nMediusPendingKind == kPendingMessage031) {
        memcpy(pRequest, &g_mediusPendingMessage031, sizeof(g_mediusPendingMessage031));
    } else if (g_nMediusPendingKind == kPendingJoinGame) {
        memcpy(pRequest, &g_mediusPendingJoinGame, sizeof(g_mediusPendingJoinGame));
    }
}

// NTSC-U/C: 0x002f9e80, PAL: 0x00303e80
// Sends the request that waited for the addresses once they arrived.
static int rtMediusSendPendingRequest(void) {
    rtMediusJoinGameRequest joinGame;
    MediusMessage031Request message031;
    int nKind;
    int nResult = 0;

    if (rtMediusGetQueryState() == kQueryFailed) {
        rtMediusSetQueryState(kQueryIdle);
        return RT_MEDIUS_ERROR_GENERAL;
    }
    if (rtMediusGetQueryState() != kQueryReady) {
        return 0;
    }
    rtMediusSetQueryState(kQueryIdle);
    rtMediusGetPendingKind(&nKind);
    if (nKind == kPendingMessage031) {
        rtMediusLoadPendingRequest(&message031);
        rtMediusGetHostAddresses(message031.addresses);
        nResult = rtDmeSendClassMessage(kSendFlags,
                                        g_nMediusConnection,
                                        kServerReceiver,
                                        RT_DME_CLASS_LOBBY,
                                        g_nMediusMessage031Type,
                                        sizeof(message031),
                                        &message031);
    } else if (nKind == kPendingJoinGame) {
        rtMediusLoadPendingRequest(&joinGame);
        rtMediusGetHostAddresses(joinGame.addresses);
        nResult = rtDmeSendClassMessage(kSendFlags,
                                        g_nMediusConnection,
                                        kServerReceiver,
                                        RT_DME_CLASS_LOBBY,
                                        g_nMediusJoinGameType,
                                        sizeof(joinGame),
                                        &joinGame);
    }
    if (nResult != 0) {
        return RT_MEDIUS_ERROR_GENERAL;
    }
    return 0;
}

// NTSC-U/C: 0x002f9fe0, PAL: 0x00303fe0
static void rtMediusSetConnectCallback(rtMediusConnectCallback pfnCallback) {
    g_pfnMediusConnectCallback = pfnCallback;
}

// NTSC-U/C: 0x002f9ff0, PAL: 0x00303ff0
static rtMediusConnectCallback rtMediusGetConnectCallback(void) {
    return g_pfnMediusConnectCallback;
}

// NTSC-U/C: 0x002f9f98, PAL: 0x00303f98
// Receives the result of rtDmeConnect() and hands the connection to the callback of the game.
static void rtMediusOnConnectResult(int connection,
                                    int reserved1 __attribute__((unused)),
                                    int reserved2 __attribute__((unused)),
                                    int status __attribute__((unused))) {
    g_nMediusConnection = connection;
    if (rtMediusGetConnectCallback() != NULL) {
        rtMediusGetConnectCallback()(g_nMediusConnection);
    }
}

// NTSC-U/C: 0x002fa000, PAL: 0x00304000
static void rtMediusClearConnectCallback(void) {
    g_pfnMediusConnectCallback = NULL;
}

int rtMediusInitialize(const rtMediusInitParams *params, int *status) {
    if (g_nMediusInitialized == 1) {
        *status = kErrorRegistration;
        return RT_MEDIUS_ERROR_CLOSE_BARE_BEFORE_REINIT;
    }
    rtMediusSetQueryState(kQueryIdle);
    g_pfnMediusChatMessageCallback = params->onChat;
    g_pfnMediusInitHandler04Callback = params->onMessage069;
    g_pfnMediusNoticeCallback = params->onNotice;
    g_pfnMediusInitHandler0CCallback = params->onMessage154;
    g_uMediusKeepAliveMs = params->timeoutMs;
    (void)rtDmeGetTimestamp(&g_uMediusLastKeepAlive); // Yes, the binary discards the result.
    if (rtMediusRegisterMessages() != 0) {
        *status = kErrorRegistration;
        return kErrorRegistration;
    }
    g_nMediusInitialized = 1;
    *status = 0;
    return 0;
}

int rtMediusGetVersion(char *version) {
    strncpy(version, kVersionText, kVersionTextSize);
    return 0;
}

int rtMediusVersionServer(rtMediusRequest *request, rtMediusVersionCallback callback) {
    const int nError = rtMediusCheckConnection();

    if (nError != 0) {
        return nError;
    }
    g_pfnMediusVersionServerCallback = callback;
    return rtMediusSendRecord(g_nMediusVersionServerType, sizeof(*request), request);
}

int rtMediusClose(void) {
    if (g_nMediusInitialized != 1) {
        return RT_MEDIUS_ERROR_NOT_INITIALIZED;
    }
    g_nMediusInitialized = 0;
    return 0;
}

int rtMediusConnect(const rtMediusConnectParams *params, rtMediusConnectStatus *status) {
    rtDmeConnectParams connect;
    rtDmeConnectStatus connectStatus;

    if (g_nMediusInitialized != 1) {
        status->connection = kNoConnection;
        status->result = RT_MEDIUS_ERROR_NOT_INITIALIZED;
        return RT_MEDIUS_ERROR_NOT_INITIALIZED;
    }
    if (g_nMediusConnection >= 0) {
        status->connection = kNoConnection;
        status->result = RT_MEDIUS_ERROR_ALREADY_CONNECTED;
        return RT_MEDIUS_ERROR_ALREADY_CONNECTED;
    }
    rtDmeInitConnectParams(&connect);
    connect.info = params->info;
    connect.maxClients = params->maxPlayers;
    if (params->onConnect != NULL) {
        rtMediusSetConnectCallback(params->onConnect);
        connect.onConnect = rtMediusOnConnectResult;
    } else {
        rtMediusSetConnectCallback(NULL);
    }
    connect.onClientJoin = params->onClientJoin;
    connect.onClientLeave = params->onClientLeave;
    connect.voice = params->voice;
    if (rtDmeConnect(&connect, &connectStatus) != 0) {
        status->result = RT_MEDIUS_ERROR_CONNECTING;
        status->connection = connectStatus.reserved[2]; // The connection the status reports.
        rtMediusClearConnectCallback();
        return RT_MEDIUS_ERROR_CONNECTING;
    }
    if (params->onConnect == NULL) {
        g_nMediusConnection = connectStatus.reserved[2];
        status->connection = connectStatus.reserved[2];
        status->result = 0;
    }
    return 0;
}

int rtMediusDisconnect(void) {
    if (g_nMediusInitialized != 1) {
        return RT_MEDIUS_ERROR_NOT_INITIALIZED;
    }
    if (g_nMediusConnection < 0) {
        return RT_MEDIUS_ERROR_NOT_CONNECTED;
    }
    const int nResult = rtDmeDisconnect(g_nMediusConnection);
    g_nMediusConnection = kNoConnection;
    return (nResult == 0) ? 0 : RT_MEDIUS_ERROR_DISCONNECTING;
}

int rtMediusUpdate(void) {
    unsigned int uNow = 0;
    int nResult;

    (void)rtDmeGetTimestamp(&uNow); // Yes, the binary discards the result.
    if (g_nMediusInitialized != 1) {
        return RT_MEDIUS_ERROR_NOT_INITIALIZED;
    }
    nResult = rtMediusSendPendingRequest();
    if (nResult != 0) {
        return nResult;
    }
    // Yes, the binary only restarts the interval of the keep-alive and sends nothing.
    if ((uNow - g_uMediusLastKeepAlive) > g_uMediusKeepAliveMs && g_szMediusSessionKey[0] != '\0' &&
        g_nMediusConnection >= 0) {
        g_uMediusLastKeepAlive = uNow;
    }
    nResult = rtDmeUpdate();
    if (nResult != 0) {
        g_nMediusLastError = nResult;
        return RT_MEDIUS_ERROR_GENERAL;
    }
    return 0;
}

int rtMediusUpdateUserState(rtMediusUserState *state) {
    const int nError = rtMediusCheckConnection();

    if (nError != 0) {
        return nError;
    }
    rtMediusTruncateField(state->sessionKey, sizeof(state->sessionKey));
    return rtMediusSendRecord(g_nMediusUpdateUserStateType, sizeof(*state), state);
}

int rtMediusSessionBegin(rtMediusSessionBeginRequest *request,
                         rtMediusSessionBeginCallback callback) {
    const int nError = rtMediusCheckConnection();

    if (nError != 0) {
        return nError;
    }
    rtMediusTruncateField(request->messageId, sizeof(request->messageId));
    g_pfnMediusSessionBeginCallback = callback;
    return rtMediusSendRecord(g_nMediusSessionBeginType, sizeof(*request), request);
}

int rtMediusSessionEnd(rtMediusRequest *request, rtMediusCallback callback) {
    const int nError = rtMediusCheckConnection();

    if (nError != 0) {
        return nError;
    }
    rtMediusTruncateField(request->sessionKey, sizeof(request->sessionKey));
    rtMediusTruncateField(request->messageId, sizeof(request->messageId));
    g_pfnMediusSessionEndCallback = callback;
    return rtMediusSendRecord(g_nMediusSessionEndType, sizeof(*request), request);
}

int rtMediusAccountLogin(rtMediusAccountLoginRequest *request, rtMediusLoginCallback callback) {
    int nError;

    rtMediusTruncateRequest(&request->header);
    rtMediusTruncateField(request->accountName, sizeof(request->accountName));
    rtMediusTruncateField(request->password, sizeof(request->password));
    nError = rtMediusCheckConnection();
    if (nError != 0) {
        return nError;
    }
    g_pfnMediusAccountLoginCallback = callback;
    return rtMediusSendRecord(g_nMediusAccountLoginType, sizeof(*request), request);
}

int rtMediusAccountRegistration(rtMediusAccountRegistrationRequest *request,
                                rtMediusCallback callback) {
    int nError;

    rtMediusTruncateRequest(&request->header);
    rtMediusTruncateField(request->accountName, sizeof(request->accountName));
    rtMediusTruncateField(request->password, sizeof(request->password));
    nError = rtMediusCheckConnection();
    if (nError != 0) {
        return nError;
    }
    g_pfnMediusAccountRegistrationCallback = callback;
    return rtMediusSendRecord(g_nMediusAccountRegistrationType, sizeof(*request), request);
}

int rtMediusChangePassword(rtMediusChangePasswordRequest *request, rtMediusCallback callback) {
    int nError;

    rtMediusTruncateRequest(&request->header);
    rtMediusTruncateField(request->oldPassword, sizeof(request->oldPassword));
    rtMediusTruncateField(request->newPassword, sizeof(request->newPassword));
    nError = rtMediusCheckConnection();
    if (nError != 0) {
        return nError;
    }
    g_pfnMediusChangePasswordCallback = callback;
    // Yes, the binary sends 0x1b0 bytes, past the end of the 0x66-byte request.
    return rtMediusSendRecord(g_nMediusChangePasswordType, kChangePasswordSendSize, request);
}

int rtMediusUpdateStats(rtMediusUpdateStatsRequest *request, rtMediusCallback callback) {
    int nError;

    rtMediusTruncateRequest(&request->header);
    rtMediusTruncateField(request->stats, sizeof(request->stats));
    nError = rtMediusCheckConnection();
    if (nError != 0) {
        return nError;
    }
    g_pfnMediusUpdateStatsCallback = callback;
    return rtMediusSendRecord(g_nMediusUpdateStatsType, sizeof(*request), request);
}

int rtMediusAccountDelete(rtMediusAccountDeleteRequest *request, rtMediusCallback callback) {
    const int nError = rtMediusCheckConnection();

    if (nError != 0) {
        return nError;
    }
    g_pfnMediusAccountDeleteCallback = callback;
    return rtMediusSendRecord(g_nMediusAccountDeleteType, sizeof(*request), request);
}

int rtMediusAnonymousLogin(rtMediusAnonymousLoginRequest *request, rtMediusLoginCallback callback) {
    int nError;

    rtMediusTruncateRequest(&request->header);
    rtMediusTruncateField(request->accountName, sizeof(request->accountName));
    rtMediusTruncateField(request->stats, sizeof(request->stats));
    nError = rtMediusCheckConnection();
    if (nError != 0) {
        return nError;
    }
    g_pfnMediusAnonymousLoginCallback = callback;
    return rtMediusSendRecord(g_nMediusAnonymousLoginType, sizeof(*request), request);
}

int rtMediusTextFilter(rtMediusTextFilterRequest *request, rtMediusTextFilterCallback callback) {
    int nError;

    rtMediusTruncateRequest(&request->header);
    rtMediusTruncateField(request->text, sizeof(request->text));
    nError = rtMediusCheckConnection();
    if (nError != 0) {
        return nError;
    }
    g_pfnMediusTextFilterCallback = callback;
    return rtMediusSendRecord(g_nMediusTextFilterType, sizeof(*request), request);
}

int rtMediusCreateChannel(rtMediusCreateChannelRequest *request,
                          rtMediusCreateChannelCallback callback) {
    int nError;

    rtMediusTruncateRequest(&request->header);
    rtMediusTruncateField(request->name, sizeof(request->name));
    rtMediusTruncateField(request->password, sizeof(request->password));
    nError = rtMediusCheckConnection();
    if (nError != 0) {
        return nError;
    }
    g_pfnMediusCreateChannelCallback = callback;
    return rtMediusSendRecord(g_nMediusCreateChannelType, sizeof(*request), request);
}

int rtMediusJoinGame(rtMediusJoinGameRequest *request, rtMediusJoinGameCallback callback) {
    rtDmeAddressQuery query;
    rtDmeConnectStatus status;
    int nError;

    rtMediusTruncateField(request->messageId, sizeof(request->messageId));
    rtMediusTruncateField(request->sessionKey, sizeof(request->sessionKey));
    rtMediusTruncateField(request->password, sizeof(request->password));
    nError = rtMediusCheckConnection();
    if (nError != 0) {
        return nError;
    }
    g_pfnMediusJoinGameCallback = callback;
    if (request->gameHostType != kHostTypeQueryAddresses) {
        return rtMediusSendRecord(g_nMediusJoinGameType, sizeof(*request), request);
    }
    if (rtDmeInitAddressQuery(&query) != 0) {
        return RT_MEDIUS_ERROR_CONNECTING;
    }
    rtMediusGetServerAddress(&query.server);
    query.onResult = rtMediusOnAddressResult;
    rtMediusSetQueryState(kQueryPending);
    rtMediusStorePendingRequest(kPendingJoinGame, request);
    if (rtDmeQueryAddresses(&query, &status) != 0) {
        rtMediusSetQueryState(kQueryIdle);
        return RT_MEDIUS_ERROR_CONNECTING;
    }
    return 0;
}

int rtMediusJoinChannel(rtMediusJoinChannelRequest *request, rtMediusJoinChannelCallback callback) {
    int nError;

    rtMediusTruncateRequest(&request->header);
    rtMediusTruncateField(request->password, sizeof(request->password));
    nError = rtMediusCheckConnection();
    if (nError != 0) {
        return nError;
    }
    g_pfnMediusJoinChannelCallback = callback;
    return rtMediusSendRecord(g_nMediusJoinChannelType, sizeof(*request), request);
}

int rtMediusGameList(rtMediusGameListRequest *request, rtMediusGameListCallback callback) {
    int nError;

    rtMediusTruncateRequest(&request->header);
    nError = rtMediusCheckConnection();
    if (nError != 0) {
        return nError;
    }
    g_pfnMediusGameListCallback = callback;
    return rtMediusSendRecord(g_nMediusGameListType, sizeof(*request), request);
}

int rtMediusChannelList(rtMediusChannelListRequest *request, rtMediusChannelListCallback callback) {
    int nError;

    rtMediusTruncateRequest(&request->header);
    nError = rtMediusCheckConnection();
    if (nError != 0) {
        return nError;
    }
    g_pfnMediusChannelListCallback = callback;
    return rtMediusSendRecord(g_nMediusChannelListType, sizeof(*request), request);
}

int rtMediusChannelPlayers(rtMediusIdRequest *request, rtMediusChannelPlayersCallback callback) {
    int nError;

    rtMediusTruncateRequest(&request->header);
    nError = rtMediusCheckConnection();
    if (nError != 0) {
        return nError;
    }
    g_pfnMediusChannelPlayersCallback = callback;
    return rtMediusSendRecord(g_nMediusChannelPlayersType, sizeof(*request), request);
}

int rtMediusGamePlayers(rtMediusIdRequest *request, rtMediusGamePlayersCallback callback) {
    int nError;

    rtMediusTruncateRequest(&request->header);
    nError = rtMediusCheckConnection();
    if (nError != 0) {
        return nError;
    }
    g_pfnMediusGamePlayersCallback = callback;
    return rtMediusSendRecord(g_nMediusGamePlayersType, sizeof(*request), request);
}

int rtMediusPlayerInfo(rtMediusIdRequest *request, rtMediusPlayerInfoCallback callback) {
    int nError;

    rtMediusTruncateRequest(&request->header);
    nError = rtMediusCheckConnection();
    if (nError != 0) {
        return nError;
    }
    g_pfnMediusPlayerInfoCallback = callback;
    return rtMediusSendRecord(g_nMediusPlayerInfoType, sizeof(*request), request);
}

int rtMediusChannelInfo(rtMediusIdRequest *request, rtMediusChannelInfoCallback callback) {
    int nError;

    rtMediusTruncateRequest(&request->header);
    nError = rtMediusCheckConnection();
    if (nError != 0) {
        return nError;
    }
    g_pfnMediusChannelInfoCallback = callback;
    return rtMediusSendRecord(g_nMediusChannelInfoType, sizeof(*request), request);
}

int rtMediusFindPlayer(rtMediusFindPlayerRequest *request, rtMediusFindPlayerCallback callback) {
    int nError;

    rtMediusTruncateRequest(&request->header);
    rtMediusTruncateField(request->accountName, sizeof(request->accountName));
    nError = rtMediusCheckConnection();
    if (nError != 0) {
        return nError;
    }
    g_pfnMediusFindPlayerCallback = callback;
    return rtMediusSendRecord(g_nMediusFindPlayerType, sizeof(*request), request);
}

int rtMediusSendChatMessage(rtMediusChatRequest *request) {
    int nError;

    rtMediusTruncateRequest(&request->header);
    rtMediusTruncateField(request->text, sizeof(request->text));
    nError = rtMediusCheckConnection();
    if (nError != 0) {
        return nError;
    }
    return rtMediusSendRecord(g_nMediusChatMessageType, sizeof(*request), request);
}

int rtMediusGetPolicy(rtMediusIdRequest *request, rtMediusPolicyCallback callback) {
    int nError;

    rtMediusTruncateRequest(&request->header);
    nError = rtMediusCheckConnection();
    if (nError != 0) {
        return nError;
    }
    g_pfnMediusGetPolicyCallback = callback;
    return rtMediusSendRecord(g_nMediusGetPolicyType, sizeof(*request), request);
}

int rtMediusPlayerReport(rtMediusPlayerReportData *report) {
    int nError;

    rtMediusTruncateField(report->sessionKey, sizeof(report->sessionKey));
    rtMediusTruncateField(report->stats, sizeof(report->stats));
    nError = rtMediusCheckConnection();
    if (nError != 0) {
        return nError;
    }
    return rtMediusSendRecord(g_nMediusPlayerReportType, sizeof(*report), report);
}

int rtMediusWorldReport(rtMediusWorldReportData *report) {
    int nError;

    rtMediusTruncateField(report->sessionKey, sizeof(report->sessionKey));
    rtMediusTruncateField(report->gameName, sizeof(report->gameName));
    rtMediusTruncateField(report->gameStats, sizeof(report->gameStats));
    nError = rtMediusCheckConnection();
    if (nError != 0) {
        return nError;
    }
    return rtMediusSendRecord(g_nMediusWorldReportType, sizeof(*report), report);
}

int rtMediusEndGameReport(rtMediusEndGameReportData *report) {
    int nError;

    rtMediusTruncateField(report->sessionKey, sizeof(report->sessionKey));
    rtMediusTruncateField(report->winningTeam, sizeof(report->winningTeam));
    rtMediusTruncateField(report->winningPlayer, sizeof(report->winningPlayer));
    nError = rtMediusCheckConnection();
    if (nError != 0) {
        return nError;
    }
    return rtMediusSendRecord(g_nMediusEndGameReportType, sizeof(*report), report);
}

int rtMediusGetAnnouncements(rtMediusIdRequest *request, rtMediusAnnouncementCallback callback) {
    int nError;

    rtMediusTruncateRequest(&request->header);
    nError = rtMediusCheckConnection();
    if (nError != 0) {
        return nError;
    }
    g_pfnMediusGetAnnouncementsCallback = callback;
    return rtMediusSendRecord(g_nMediusGetAnnouncementsType, sizeof(*request), request);
}

int rtMediusSetGameListFilter(rtMediusGameListFilterRequest *request, rtMediusCallback callback) {
    int nError;

    rtMediusTruncateRequest(&request->header);
    nError = rtMediusCheckConnection();
    if (nError != 0) {
        return nError;
    }
    g_pfnMediusSetGameListFilterCallback = callback;
    return rtMediusSendRecord(g_nMediusSetGameListFilterType, sizeof(*request), request);
}

int rtMediusClearGameListFilter(rtMediusIdRequest *request, rtMediusCallback callback) {
    int nError;

    rtMediusTruncateRequest(&request->header);
    nError = rtMediusCheckConnection();
    if (nError != 0) {
        return nError;
    }
    g_pfnMediusClearGameListFilterCallback = callback;
    return rtMediusSendRecord(g_nMediusClearGameListFilterType, sizeof(*request), request);
}

int rtMediusBanPlayer(rtMediusBanPlayerRequest *request, rtMediusCallback callback) {
    int nError;

    rtMediusTruncateRequest(&request->header);
    nError = rtMediusCheckConnection();
    if (nError != 0) {
        return nError;
    }
    g_pfnMediusBanPlayerCallback = callback;
    return rtMediusSendRecord(g_nMediusBanPlayerType, sizeof(*request), request);
}

int rtMediusAccountGetId(rtMediusAccountGetIdRequest *request,
                         rtMediusAccountGetIdCallback callback) {
    int nError;

    rtMediusTruncateRequest(&request->header);
    nError = rtMediusCheckConnection();
    if (nError != 0) {
        return nError;
    }
    g_pfnMediusAccountGetIdCallback = callback;
    return rtMediusSendRecord(g_nMediusAccountGetIdType, sizeof(*request), request);
}

int rtMediusLadderPosition(rtMediusLadderPositionRequest *request,
                           rtMediusLadderPositionCallback callback) {
    int nError;

    rtMediusTruncateRequest(&request->header);
    nError = rtMediusCheckConnection();
    if (nError != 0) {
        return nError;
    }
    g_pfnMediusLadderPositionCallback = callback;
    return rtMediusSendRecord(g_nMediusLadderPositionType, sizeof(*request), request);
}

int rtMediusLadderPositionFast(rtMediusLadderPositionFastRequest *request,
                               rtMediusLadderPositionCallback callback) {
    int nError;

    rtMediusTruncateRequest(&request->header);
    nError = rtMediusCheckConnection();
    if (nError != 0) {
        return nError;
    }
    g_pfnMediusLadderPositionFastCallback = callback;
    return rtMediusSendRecord(g_nMediusLadderPositionFastType, sizeof(*request), request);
}

int rtMediusLadderList(rtMediusLadderListRequest *request, rtMediusLadderListCallback callback) {
    int nError;

    rtMediusTruncateRequest(&request->header);
    nError = rtMediusCheckConnection();
    if (nError != 0) {
        return nError;
    }
    g_pfnMediusLadderListCallback = callback;
    return rtMediusSendRecord(g_nMediusLadderListType, sizeof(*request), request);
}

const char *rtMediusGetErrorString(int error) {
    switch (error) {
    case RT_MEDIUS_STATUS_SUCCESS:
        return "MediusErrorNone";
    case RT_MEDIUS_ERROR_INITIALIZING:
        return "MediusErrorInitializing";
    case RT_MEDIUS_ERROR_CONNECTING:
        return "MediusErrorConnecting";
    case RT_MEDIUS_ERROR_DISCONNECTING:
        return "MediusErrorDisconnecting";
    case RT_MEDIUS_ERROR_CLOSING:
        return "MediusErrorClosing";
    case RT_MEDIUS_ERROR_SENDING_MESSAGE:
        return "MediusErrorSendingMessage";
    case RT_MEDIUS_ERROR_FILE_NOT_FOUND:
        return "MediusErrorFileNotFound";
    case RT_MEDIUS_ERROR_DEVICE_NOT_FOUND:
        return "MediusErrorDeviceNotFound";
    case RT_MEDIUS_ERROR_DIRECTORY_NOT_FOUND:
        return "MediusErrorDirectoryNotFound";
    case RT_MEDIUS_ERROR_GENERAL:
        return "MediusErrorGeneral";
    case RT_MEDIUS_ERROR_SESSION_INACTIVE:
        return "MediusErrorSessionInactive";
    case RT_MEDIUS_ERROR_ITEM_NOT_FOUND:
        return "MediusErrorItemNotFound";
    case RT_MEDIUS_ERROR_ITEM_ALREADY_EXISTS:
        return "MediusErrorItemAlreadyExists";
    case RT_MEDIUS_WORLD_CREATED_SIZE_REDUCED:
        return "MediusWorldCreatedSizeReduced";
    case RT_MEDIUS_ERROR_NO_RESULT:
        return "MediusErrorNoResult";
    case RT_MEDIUS_ERROR_NOT_INITIALIZED:
        return "MediusErrorNotInitialized";
    case RT_MEDIUS_ERROR_NOT_CONNECTED:
        return "MediusErrorNotConnected";
    case RT_MEDIUS_ERROR_ALREADY_CONNECTED:
        return "MediusErrorAlreadyConnected";
    case RT_MEDIUS_ERROR_CLOSE_BEFORE_REINIT:
        return "MediusErrorCallMediusCloseBeforeReinitializing";
    case RT_MEDIUS_ERROR_CLOSE_BARE_BEFORE_REINIT:
        return "MediusErrorCallMediusCloseBareBeforeReinitializing";
    default:
        return "UNKNOWN MEDIUSERRORCODE";
    }
}

void rtMediusSetConnectParams(rtMediusConnectParams *params,
                              const char *sessionKey,
                              const char *address,
                              int port,
                              int worldId) {
    rtDmeConnectParams defaults;

    rtDmeInitConnectParams(&defaults);
    params->voice = defaults.voice;
    params->info.type = kConnectionTypeDirect;
    params->info.worldId = worldId;
    if (sessionKey != NULL) {
        strncpy(params->info.sessionKey, sessionKey, strlen(sessionKey) + 1);
    } else {
        params->info.sessionKey[0] = '\0';
    }
    memset(params->info.serverKey, 0, sizeof(params->info.serverKey));
    params->info.accessKey[0] = '\0';
    params->info.addresses[0].type = kAddressTypeExternal;
    strncpy(params->info.addresses[0].address, address, strlen(address) + 1);
    params->info.addresses[0].port = port;
    params->onClientLeave = NULL;
    params->info.addresses[1].type = kAddressTypeNone;
    params->info.addresses[1].address[0] = '\0';
    params->info.addresses[1].port = 0;
    params->onConnect = NULL;
    params->onClientJoin = NULL;
}

static inline void rtMediusAddChars(const void *pRecord, const char *pField, int nCount) {
    rtDmeAddField(pRecord, pField, sizeof(char), nCount);
}

// Yes, the binary gives the size of these fields as the size of each of their elements too.
static inline void rtMediusAddCharsBySize(const void *pRecord, const char *pField, int nSize) {
    rtDmeAddField(pRecord, pField, nSize, nSize);
}

static inline void rtMediusAddShort(const void *pRecord, const short *pField) {
    rtDmeAddField(pRecord, pField, sizeof(short), 1);
}

static inline void rtMediusAddInt(const void *pRecord, const int *pField) {
    rtDmeAddField(pRecord, pField, sizeof(int), 1);
}

static inline void rtMediusAddRequestHeader(const void *pRecord, const rtMediusRequest *pHeader) {
    rtMediusAddChars(pRecord, pHeader->messageId, sizeof(pHeader->messageId));
    rtMediusAddChars(pRecord, pHeader->sessionKey, sizeof(pHeader->sessionKey));
}

static inline void rtMediusAddResponseHeader(const void *pRecord, const rtMediusResponse *pHeader) {
    rtMediusAddChars(pRecord, pHeader->messageId, sizeof(pHeader->messageId));
    rtMediusAddInt(pRecord, &pHeader->status);
}

static inline void rtMediusAddAddress(const void *pRecord, const rtNetAddress *pAddress) {
    rtMediusAddInt(pRecord, &pAddress->type);
    rtMediusAddCharsBySize(pRecord, pAddress->address, sizeof(pAddress->address));
    rtDmeAddField(pRecord, &pAddress->port, sizeof(pAddress->port), 1);
}

static inline void rtMediusAddConnection(const void *pRecord, const rtNetConnectionInfo *pInfo) {
    rtMediusAddAddress(pRecord, &pInfo->addresses[0]);
    rtMediusAddAddress(pRecord, &pInfo->addresses[1]);
    rtMediusAddInt(pRecord, &pInfo->worldId);
    rtDmeAddField(pRecord,
                  pInfo->serverKey,
                  sizeof(unsigned int),
                  sizeof(pInfo->serverKey) / sizeof(unsigned int));
    rtMediusAddChars(pRecord, pInfo->sessionKey, sizeof(pInfo->sessionKey));
    rtMediusAddChars(pRecord, pInfo->accessKey, sizeof(pInfo->accessKey));
}

static inline void rtMediusAddProfile(const void *pRecord, const MediusProfile *pProfile) {
    rtMediusAddShort(pRecord, &pProfile->birthYear);
    rtMediusAddShort(pRecord, &pProfile->birthMonth);
    rtMediusAddShort(pRecord, &pProfile->birthDay);
    rtMediusAddChars(pRecord, pProfile->firstName, sizeof(pProfile->firstName));
    rtMediusAddChars(pRecord, pProfile->lastName, sizeof(pProfile->lastName));
    rtMediusAddChars(pRecord, pProfile->company, sizeof(pProfile->company));
    rtMediusAddChars(pRecord, pProfile->address1, sizeof(pProfile->address1));
    rtMediusAddChars(pRecord, pProfile->address2, sizeof(pProfile->address2));
    rtMediusAddChars(pRecord, pProfile->address3, sizeof(pProfile->address3));
    rtMediusAddChars(pRecord, pProfile->city, sizeof(pProfile->city));
    rtMediusAddChars(pRecord, pProfile->state, sizeof(pProfile->state));
    rtMediusAddChars(pRecord, pProfile->country, sizeof(pProfile->country));
    rtMediusAddChars(pRecord, pProfile->email, sizeof(pProfile->email));
    rtMediusAddChars(pRecord, pProfile->phone, sizeof(pProfile->phone));
    rtMediusAddChars(pRecord, pProfile->comments, sizeof(pProfile->comments));
}

static inline void rtMediusRegisterLobbyMessage(int *pnType, rtDmeMessageHandler pfnHandler) {
    rtDmeRegisterClassMessage(pnType, RT_DME_CLASS_LOBBY, pfnHandler);
}

int rtMediusRegisterMessages(void) {
    MediusNotice notice;
    rtMediusWorldReportData worldReport;
    rtMediusPlayerReportData playerReport;
    rtMediusEndGameReportData endGameReport;
    rtMediusSessionBeginRequest sessionBegin;
    rtMediusSessionBeginResponse sessionBeginResponse;
    MediusMachineSignatureRequest machineSignature;
    rtMediusRequest sessionEnd;
    rtMediusResponse sessionEndResponse;
    rtMediusRequest accountGetProfile;
    MediusAccountGetProfileResponse accountGetProfileResponse;
    rtMediusAccountLoginRequest accountLogin;
    rtMediusLoginResponse loginResponse;
    rtMediusAccountRegistrationRequest accountRegistration;
    MediusIdResponse accountRegistrationResponse;
    MediusAccountUpdateProfileRequest accountUpdateProfile;
    rtMediusChangePasswordRequest changePassword;
    rtMediusUpdateStatsRequest updateStats;
    rtMediusAccountDeleteRequest accountDelete;
    rtMediusRequest accountLogout;
    rtMediusResponse statusResponse;
    rtMediusAnonymousLoginRequest anonymousLogin;
    rtMediusIdRequest channelPlayers;
    rtMediusChannelPlayersResponse channelPlayersResponse;
    rtMediusIdRequest gamePlayers;
    rtMediusGamePlayersResponse gamePlayersResponse;
    MediusCreateGameRequest createGame;
    MediusIdResponse createGameResponse;
    MediusMessage031Request message031;
    MediusIdResponse message032;
    rtMediusCreateChannelRequest createChannel;
    rtMediusCreateChannelResponse createChannelResponse;
    rtMediusJoinGameRequest joinGame;
    rtMediusJoinGameResponse joinGameResponse;
    rtMediusJoinChannelRequest joinChannel;
    rtMediusJoinChannelResponse joinChannelResponse;
    rtMediusRequest joinChannelFwd;
    MediusJoinChannelFwdResponse joinChannelFwdResponse;
    rtMediusGameListRequest gameList;
    rtMediusGameListResponse gameListResponse;
    rtMediusChannelListRequest channelList;
    rtMediusChannelListResponse channelListResponse;
    rtMediusIdRequest playerInfo;
    rtMediusPlayerInfoResponse playerInfoResponse;
    rtMediusIdRequest gameInfo;
    MediusGameInfoResponse gameInfoResponse;
    rtMediusIdRequest channelInfo;
    rtMediusChannelInfoResponse channelInfoResponse;
    MediusFindWorldByNameRequest findWorldByName;
    MediusFindWorldByNameResponse findWorldByNameResponse;
    rtMediusFindPlayerRequest findPlayer;
    MediusFindPlayerResponse findPlayerResponse;
    rtMediusChatRequest chatMessage;
    MediusChatFwdMessage chatFwdMessage;
    rtMediusRequest getBuddyList;
    MediusAccountListResponse getBuddyListResponse;
    rtMediusIdRequest addToBuddyList;
    rtMediusResponse addToBuddyListResponse;
    rtMediusIdRequest removeFromBuddyList;
    rtMediusResponse removeFromBuddyListResponse;
    rtMediusIdRequest buddyConfirmation;
    MediusAccountNameResponse buddyConfirmationResponse;
    MediusBuddyConfirmationRequest buddyFwdConfirmation;
    MediusBuddyConfirmationReply buddyFwdConfirmationResponse;
    rtMediusRequest getIgnoreList;
    MediusAccountListResponse getIgnoreListResponse;
    rtMediusIdRequest addToIgnoreList;
    rtMediusResponse addToIgnoreListResponse;
    rtMediusIdRequest removeFromIgnoreList;
    rtMediusResponse removeFromIgnoreListResponse;
    rtMediusIdRequest getPolicy;
    rtMediusPolicyResponse getPolicyResponse;
    rtMediusUserState updateUserState;
    rtMediusIdRequest getAnnouncements;
    rtMediusIdRequest getAllAnnouncements;
    MediusAnnouncementResponse getAnnouncementsResponse;
    rtMediusGameListFilterRequest setGameListFilter;
    rtMediusResponse setGameListFilterResponse;
    rtMediusIdRequest clearGameListFilter;
    rtMediusResponse clearGameListFilterResponse;
    rtMediusRequest getGameListFilter;
    MediusGameListFilterResponse getGameListFilterResponse;
    rtMediusRequest getMyIp;
    MediusGetMyIpResponse getMyIpResponse;
    MediusIdNameRequest createClan;
    MediusIdResponse createClanResponse;
    rtMediusIdRequest disbandClan;
    rtMediusResponse disbandClanResponse;
    MediusPairRequest getClanById;
    MediusClanByIdResponse getClanByIdResponse;
    MediusIdNameRequest getClanByName;
    MediusClanByNameResponse getClanByNameResponse;
    MediusIdNameRequest transferClanLeadership;
    rtMediusResponse transferClanLeadershipResponse;
    MediusClanMessageRequest addPlayerToClan;
    rtMediusResponse addPlayerToClanResponse;
    MediusPairRequest removePlayerFromClan;
    rtMediusResponse removePlayerFromClanResponse;
    MediusClanMessageRequest invitePlayerToClan;
    rtMediusResponse invitePlayerToClanResponse;
    MediusPairRequest checkMyClanInvitations;
    MediusClanInvitationResponse checkMyClanInvitationsResponse;
    MediusClanReplyRequest respondToClanInvitation;
    rtMediusResponse respondToClanInvitationResponse;
    rtMediusIdRequest revokeClanInvitation;
    rtMediusResponse revokeClanInvitationResponse;
    MediusClanMessageRequest requestClanTeamChallenge;
    rtMediusResponse requestClanTeamChallengeResponse;
    MediusClanPageRequest getMyClanMessages;
    MediusMyClanMessageResponse getMyClanMessagesResponse;
    MediusSendClanMessageRequest sendClanMessage;
    rtMediusResponse sendClanMessageResponse;
    MediusClanMessageRequest modifyClanMessage;
    rtMediusResponse modifyClanMessageResponse;
    rtMediusIdRequest deleteClanMessage;
    rtMediusResponse deleteClanMessageResponse;
    MediusClanReplyRequest respondToClanTeamChallenge;
    rtMediusResponse respondToClanTeamChallengeResponse;
    rtMediusIdRequest revokeClanTeamChallenge;
    rtMediusResponse revokeClanTeamChallengeResponse;
    MediusChallengeHistoryRequest getClanTeamChallengeHistory;
    MediusChallengeHistoryResponse getClanTeamChallengeHistoryResponse;
    MediusPairRequest getClanInvitationsSent;
    MediusInvitationSentResponse getClanInvitationsSentResponse;
    MediusPairRequest getMyClans;
    MediusMyClanResponse getMyClansResponse;
    MediusPairRequest getAllClanMessages;
    MediusClanMessageResponse getAllClanMessagesResponse;
    rtMediusIdRequest confirmClanTeamChallenge;
    rtMediusResponse confirmClanTeamChallengeResponse;
    MediusUpdateClanStatsRequest updateClanStats;
    rtMediusResponse updateClanStatsResponse;
    MediusChallengesRequest getClanTeamChallenges;
    MediusChallengeResponse getClanTeamChallengesResponse;
    rtMediusRequest versionServer;
    rtMediusVersionResponse versionServerResponse;
    MediusPairRequest getWorldSecurityLevel;
    MediusSecurityLevelResponse getWorldSecurityLevelResponse;
    rtMediusBanPlayerRequest banPlayer;
    rtMediusResponse banPlayerResponse;
    rtMediusAccountGetIdRequest accountGetId;
    rtMediusAccountGetIdResponse accountGetIdResponse;
    rtMediusRequest getLocations;
    MediusLocationResponse getLocationsResponse;
    rtMediusIdRequest pickLocation;
    rtMediusResponse pickLocationResponse;
    rtMediusIdRequest getClanMemberList;
    MediusClanMemberResponse getClanMemberListResponse;
    rtMediusLadderPositionRequest ladderPosition;
    rtMediusLadderPositionResponse ladderPositionResponse;
    rtMediusLadderPositionFastRequest ladderPositionFast;
    rtMediusLadderPositionResponse ladderPositionFastResponse;
    rtMediusLadderListRequest ladderList;
    rtMediusLadderListResponse ladderListResponse;
    rtMediusIdRequest chatToggle;
    rtMediusResponse chatToggleResponse;
    rtMediusTextFilterRequest textFilter;
    rtMediusTextFilterResponse textFilterResponse;
    MediusReassignWorld reassignWorld;
    rtMediusIdRequest getTotalGames;
    MediusIdResponse getTotalGamesResponse;
    rtMediusIdRequest getTotalChannels;
    MediusIdResponse getTotalChannelsResponse;
    rtMediusIdRequest getTotalUsers;
    MediusTotalUsersResponse getTotalUsersResponse;
    rtMediusIdRequest getLobbyPlayerNames;
    MediusPlayerNameResponse getLobbyPlayerNamesResponse;
    MediusPairRequest setMessageAsRead;
    MediusUniverseInformationRequest universeInformation;
    MediusUniverseNewsResponse universeNewsResponse;
    MediusUniverseStatusResponse universeStatusResponse;

    // Yes, the binary ignores the result of every call, and the routine always reports 0.
    rtDmeBeginFields();
    rtMediusAddChars(&worldReport, worldReport.sessionKey, sizeof(worldReport.sessionKey));
    rtMediusAddInt(&worldReport, &worldReport.worldId);
    rtMediusAddInt(&worldReport, &worldReport.playerCount);
    rtMediusAddChars(&worldReport, worldReport.gameName, sizeof(worldReport.gameName));
    rtMediusAddChars(&worldReport, worldReport.gameStats, sizeof(worldReport.gameStats));
    rtMediusAddInt(&worldReport, &worldReport.minPlayers);
    rtMediusAddInt(&worldReport, &worldReport.maxPlayers);
    rtMediusAddInt(&worldReport, &worldReport.gameLevel);
    rtMediusAddInt(&worldReport, &worldReport.playerSkillLevel);
    rtMediusAddInt(&worldReport, &worldReport.rulesSet);
    rtMediusAddInt(&worldReport, &worldReport.genericField1);
    rtMediusAddInt(&worldReport, &worldReport.genericField2);
    rtMediusAddInt(&worldReport, &worldReport.genericField3);
    rtMediusAddInt(&worldReport, &worldReport.worldStatus);
    rtMediusRegisterLobbyMessage(&g_nMediusWorldReportType, rtMediusOnMessage000);

    rtDmeBeginFields();
    rtMediusAddChars(&playerReport, playerReport.sessionKey, sizeof(playerReport.sessionKey));
    rtMediusAddInt(&playerReport, &playerReport.worldId);
    rtMediusAddChars(&playerReport, playerReport.stats, sizeof(playerReport.stats));
    rtMediusRegisterLobbyMessage(&g_nMediusPlayerReportType, rtMediusOnMessage001);

    rtDmeBeginFields();
    rtMediusAddChars(&endGameReport, endGameReport.sessionKey, sizeof(endGameReport.sessionKey));
    rtMediusAddInt(&endGameReport, &endGameReport.worldId);
    rtMediusAddChars(&endGameReport, endGameReport.winningTeam, sizeof(endGameReport.winningTeam));
    rtMediusAddChars(
        &endGameReport, endGameReport.winningPlayer, sizeof(endGameReport.winningPlayer));
    rtMediusAddInt(&endGameReport, &endGameReport.finalScore);
    rtMediusRegisterLobbyMessage(&g_nMediusEndGameReportType, rtMediusOnMessage002);

    rtDmeBeginFields();
    rtMediusAddChars(&sessionBegin, sessionBegin.messageId, sizeof(sessionBegin.messageId));
    rtMediusAddInt(&sessionBegin, &sessionBegin.connectionClass);
    rtMediusRegisterLobbyMessage(&g_nMediusSessionBeginType, rtMediusOnMessage003);

    rtDmeBeginFields();
    rtMediusAddResponseHeader(&sessionBeginResponse, &sessionBeginResponse.header);
    rtMediusAddChars(&sessionBeginResponse,
                     sessionBeginResponse.sessionKey,
                     sizeof(sessionBeginResponse.sessionKey));
    rtMediusRegisterLobbyMessage(&g_nMediusSessionBeginResponseType,
                                 rtMediusOnSessionBeginResponse);

    rtDmeBeginFields();
    rtMediusAddRequestHeader(&sessionEnd, &sessionEnd);
    rtMediusRegisterLobbyMessage(&g_nMediusSessionEndType, rtMediusOnMessage005);

    rtDmeBeginFields();
    rtMediusAddResponseHeader(&sessionEndResponse, &sessionEndResponse);
    rtMediusRegisterLobbyMessage(&g_nMediusSessionEndResponseType, rtMediusOnSessionEndResponse);

    rtDmeBeginFields();
    rtMediusAddRequestHeader(&accountLogin, &accountLogin.header);
    rtMediusAddChars(&accountLogin, accountLogin.accountName, sizeof(accountLogin.accountName));
    rtMediusAddChars(&accountLogin, accountLogin.password, sizeof(accountLogin.password));
    rtMediusRegisterLobbyMessage(&g_nMediusAccountLoginType, rtMediusOnMessage007);

    rtDmeBeginFields();
    rtMediusAddResponseHeader(&loginResponse, &loginResponse.header);
    rtMediusAddInt(&loginResponse, &loginResponse.accountId);
    rtMediusAddInt(&loginResponse, &loginResponse.accountType);
    rtMediusAddInt(&loginResponse, &loginResponse.worldId);
    rtMediusAddInt(&loginResponse, &loginResponse.info.type);
    rtMediusAddConnection(&loginResponse, &loginResponse.info);
    rtMediusRegisterLobbyMessage(&g_nMediusAccountLoginResponseType,
                                 rtMediusOnAccountLoginResponse);

    rtDmeBeginFields();
    rtMediusAddRequestHeader(&accountRegistration, &accountRegistration.header);
    rtMediusAddInt(&accountRegistration, &accountRegistration.accountType);
    rtMediusAddChars(&accountRegistration,
                     accountRegistration.accountName,
                     sizeof(accountRegistration.accountName));
    rtMediusAddChars(
        &accountRegistration, accountRegistration.password, sizeof(accountRegistration.password));
    rtMediusRegisterLobbyMessage(&g_nMediusAccountRegistrationType, rtMediusOnMessage009);

    rtDmeBeginFields();
    rtMediusAddResponseHeader(&accountRegistrationResponse, &accountRegistrationResponse.header);
    rtMediusAddInt(&accountRegistrationResponse, &accountRegistrationResponse.id);
    rtMediusRegisterLobbyMessage(&g_nMediusAccountRegistrationResponseType,
                                 rtMediusOnAccountRegistrationResponse);

    rtDmeBeginFields();
    rtMediusAddRequestHeader(&accountGetProfile, &accountGetProfile);
    rtMediusRegisterLobbyMessage(&g_nMediusAccountGetProfileType, rtMediusOnMessage011);

    rtDmeBeginFields();
    rtMediusAddResponseHeader(&accountGetProfileResponse, &accountGetProfileResponse.header);
    rtMediusAddInt(&accountGetProfileResponse, &accountGetProfileResponse.accountId);
    rtMediusAddProfile(&accountGetProfileResponse, &accountGetProfileResponse.profile);
    rtMediusRegisterLobbyMessage(&g_nMediusAccountGetProfileResponseType, rtMediusOnMessage012);

    rtDmeBeginFields();
    rtMediusAddRequestHeader(&accountUpdateProfile, &accountUpdateProfile.header);
    rtMediusAddProfile(&accountUpdateProfile, &accountUpdateProfile.profile);
    rtMediusRegisterLobbyMessage(&g_nMediusAccountUpdateProfileType, rtMediusOnMessage013);

    rtDmeBeginFields();
    rtMediusAddResponseHeader(&statusResponse, &statusResponse);
    rtMediusRegisterLobbyMessage(&g_nMediusAccountUpdateProfileResponseType, rtMediusOnMessage014);

    rtDmeBeginFields();
    rtMediusAddRequestHeader(&changePassword, &changePassword.header);
    rtMediusAddChars(
        &changePassword, changePassword.oldPassword, sizeof(changePassword.oldPassword));
    rtMediusAddChars(
        &changePassword, changePassword.newPassword, sizeof(changePassword.newPassword));
    rtMediusRegisterLobbyMessage(&g_nMediusChangePasswordType, rtMediusOnMessage015);

    rtDmeBeginFields();
    rtMediusAddResponseHeader(&statusResponse, &statusResponse);
    rtMediusRegisterLobbyMessage(&g_nMediusChangePasswordResponseType,
                                 rtMediusOnChangePasswordResponse);

    rtDmeBeginFields();
    rtMediusAddRequestHeader(&updateStats, &updateStats.header);
    rtMediusAddChars(&updateStats, updateStats.stats, sizeof(updateStats.stats));
    rtMediusRegisterLobbyMessage(&g_nMediusUpdateStatsType, rtMediusOnMessage017);

    rtDmeBeginFields();
    rtMediusAddResponseHeader(&statusResponse, &statusResponse);
    rtMediusRegisterLobbyMessage(&g_nMediusUpdateStatsResponseType, rtMediusOnUpdateStatsResponse);

    rtDmeBeginFields();
    rtMediusAddRequestHeader(&accountDelete, &accountDelete.header);
    rtMediusAddChars(&accountDelete, accountDelete.password, sizeof(accountDelete.password));
    rtMediusRegisterLobbyMessage(&g_nMediusAccountDeleteType, rtMediusOnMessage019);

    rtDmeBeginFields();
    rtMediusAddResponseHeader(&statusResponse, &statusResponse);
    rtMediusRegisterLobbyMessage(&g_nMediusAccountDeleteResponseType,
                                 rtMediusOnAccountDeleteResponse);

    rtDmeBeginFields();
    rtMediusAddRequestHeader(&accountLogout, &accountLogout);
    rtMediusRegisterLobbyMessage(&g_nMediusAccountLogoutType, rtMediusOnMessage021);

    rtDmeBeginFields();
    rtMediusAddResponseHeader(&statusResponse, &statusResponse);
    rtMediusRegisterLobbyMessage(&g_nMediusAccountLogoutResponseType, rtMediusOnMessage022);

    rtDmeBeginFields();
    rtMediusAddRequestHeader(&accountGetId, &accountGetId.header);
    rtMediusAddChars(&accountGetId, accountGetId.accountName, sizeof(accountGetId.accountName));
    rtMediusRegisterLobbyMessage(&g_nMediusAccountGetIdType, rtMediusOnMessage023);

    rtDmeBeginFields();
    rtMediusAddChars(&accountGetIdResponse,
                     accountGetIdResponse.messageId,
                     sizeof(accountGetIdResponse.messageId));
    rtMediusAddInt(&accountGetIdResponse, &accountGetIdResponse.accountId);
    rtMediusAddInt(&accountGetIdResponse, &accountGetIdResponse.status);
    rtMediusRegisterLobbyMessage(&g_nMediusAccountGetIdResponseType,
                                 rtMediusOnAccountGetIdResponse);

    rtDmeBeginFields();
    rtMediusAddRequestHeader(&anonymousLogin, &anonymousLogin.header);
    rtMediusAddChars(
        &anonymousLogin, anonymousLogin.accountName, sizeof(anonymousLogin.accountName));
    rtMediusAddChars(&anonymousLogin, anonymousLogin.stats, kAnonymousLoginStatsCount);
    rtMediusRegisterLobbyMessage(&g_nMediusAnonymousLoginType, rtMediusOnMessage025);

    rtDmeBeginFields();
    rtMediusAddResponseHeader(&loginResponse, &loginResponse.header);
    rtMediusAddInt(&loginResponse, &loginResponse.accountId);
    rtMediusAddInt(&loginResponse, &loginResponse.accountType);
    rtMediusAddInt(&loginResponse, &loginResponse.worldId);
    rtMediusAddInt(&loginResponse, &loginResponse.info.type);
    rtMediusAddConnection(&loginResponse, &loginResponse.info);
    rtMediusRegisterLobbyMessage(&g_nMediusAnonymousLoginResponseType,
                                 rtMediusOnAnonymousLoginResponse);

    rtDmeBeginFields();
    rtMediusAddRequestHeader(&getMyIp, &getMyIp);
    rtMediusRegisterLobbyMessage(&g_nMediusGetMyIpType, rtMediusOnMessage027);

    rtDmeBeginFields();
    rtMediusAddChars(
        &getMyIpResponse, getMyIpResponse.messageId, sizeof(getMyIpResponse.messageId));
    rtMediusAddChars(
        &getMyIpResponse, getMyIpResponse.ipAddress, sizeof(getMyIpResponse.ipAddress));
    rtMediusAddInt(&getMyIpResponse, &getMyIpResponse.status);
    rtMediusRegisterLobbyMessage(&g_nMediusGetMyIpResponseType, rtMediusOnMessage028);

    rtDmeBeginFields();
    rtMediusAddRequestHeader(&createGame, &createGame.header);
    rtMediusAddInt(&createGame, &createGame.applicationId);
    rtMediusAddInt(&createGame, &createGame.minPlayers);
    rtMediusAddInt(&createGame, &createGame.maxPlayers);
    rtMediusAddInt(&createGame, &createGame.gameLevel);
    rtMediusAddChars(&createGame, createGame.gameName, sizeof(createGame.gameName));
    rtMediusAddChars(&createGame, createGame.gamePassword, sizeof(createGame.gamePassword));
    rtMediusAddInt(&createGame, &createGame.playerSkillLevel);
    rtMediusAddInt(&createGame, &createGame.rulesSet);
    rtMediusAddInt(&createGame, &createGame.genericField1);
    rtMediusAddInt(&createGame, &createGame.genericField2);
    rtMediusAddInt(&createGame, &createGame.genericField3);
    rtMediusAddInt(&createGame, &createGame.gameHostType);
    rtMediusRegisterLobbyMessage(&g_nMediusCreateGameType, rtMediusOnMessage029);

    rtDmeBeginFields();
    rtMediusAddResponseHeader(&createGameResponse, &createGameResponse.header);
    rtMediusAddInt(&createGameResponse, &createGameResponse.id);
    rtMediusRegisterLobbyMessage(&g_nMediusCreateGameResponseType, rtMediusOnMessage030);

    rtDmeBeginFields();
    rtMediusAddRequestHeader(&message031, &message031.header);
    rtMediusAddInt(&message031, &message031.applicationId);
    rtMediusAddInt(&message031, &message031.minPlayers);
    rtMediusAddInt(&message031, &message031.maxPlayers);
    rtMediusAddInt(&message031, &message031.gameLevel);
    rtMediusAddChars(&message031, message031.gameName, sizeof(message031.gameName));
    rtMediusAddChars(&message031, message031.gamePassword, sizeof(message031.gamePassword));
    rtMediusAddInt(&message031, &message031.playerSkillLevel);
    rtMediusAddInt(&message031, &message031.rulesSet);
    rtMediusAddInt(&message031, &message031.genericField1);
    rtMediusAddInt(&message031, &message031.genericField2);
    rtMediusAddInt(&message031, &message031.genericField3);
    rtMediusAddInt(&message031, &message031.gameHostType);
    rtMediusAddInt(&message031, &message031.reservedB8);
    rtMediusAddInt(&message031, &message031.addresses[0].type);
    rtMediusAddChars(
        &message031, message031.addresses[0].address, sizeof(message031.addresses[0].address));
    rtDmeAddField(
        &message031, &message031.addresses[0].port, sizeof(message031.addresses[0].port), 1);
    rtMediusAddInt(&message031, &message031.addresses[1].type);
    rtMediusAddChars(
        &message031, message031.addresses[1].address, sizeof(message031.addresses[1].address));
    rtDmeAddField(
        &message031, &message031.addresses[1].port, sizeof(message031.addresses[1].port), 1);
    rtMediusRegisterLobbyMessage(&g_nMediusMessage031Type, rtMediusOnMessage031);

    rtDmeBeginFields();
    rtMediusAddResponseHeader(&message032, &message032.header);
    rtMediusAddInt(&message032, &message032.id);
    rtMediusRegisterLobbyMessage(&g_nMediusMessage032Type, rtMediusOnMessage032);

    rtDmeBeginFields();
    rtMediusAddRequestHeader(&createChannel, &createChannel.header);
    rtMediusAddInt(&createChannel, &createChannel.applicationId);
    rtMediusAddInt(&createChannel, &createChannel.maxPlayers);
    rtMediusAddChars(&createChannel, createChannel.name, sizeof(createChannel.name));
    rtMediusAddChars(&createChannel, createChannel.password, sizeof(createChannel.password));
    rtMediusRegisterLobbyMessage(&g_nMediusCreateChannelType, rtMediusOnMessage033);

    rtDmeBeginFields();
    rtMediusAddResponseHeader(&createChannelResponse, &createChannelResponse.header);
    rtMediusAddInt(&createChannelResponse, &createChannelResponse.worldId);
    rtMediusRegisterLobbyMessage(&g_nMediusCreateChannelResponseType,
                                 rtMediusOnCreateChannelResponse);

    rtDmeBeginFields();
    rtMediusAddChars(&joinGame, joinGame.messageId, sizeof(joinGame.messageId));
    rtMediusAddChars(&joinGame, joinGame.sessionKey, sizeof(joinGame.sessionKey));
    rtMediusAddInt(&joinGame, &joinGame.worldId);
    rtMediusAddInt(&joinGame, &joinGame.gameHostType);
    rtMediusAddAddress(&joinGame, &joinGame.addresses[0]);
    rtMediusAddInt(&joinGame, &joinGame.addresses[1].type);
    rtMediusAddCharsBySize(
        &joinGame, joinGame.addresses[1].address, sizeof(joinGame.addresses[1].address));
    rtMediusRegisterLobbyMessage(&g_nMediusJoinGameType, rtMediusOnMessage035);

    rtDmeBeginFields();
    rtMediusAddResponseHeader(&joinGameResponse, &joinGameResponse.header);
    rtMediusAddInt(&joinGameResponse, &joinGameResponse.reserved1C);
    rtMediusAddInt(&joinGameResponse, &joinGameResponse.info.type);
    rtMediusAddConnection(&joinGameResponse, &joinGameResponse.info);
    rtMediusRegisterLobbyMessage(&g_nMediusJoinGameResponseType, rtMediusOnJoinGameResponse);

    rtDmeBeginFields();
    rtMediusAddRequestHeader(&joinChannel, &joinChannel.header);
    rtMediusAddInt(&joinChannel, &joinChannel.worldId);
    rtMediusRegisterLobbyMessage(&g_nMediusJoinChannelType, rtMediusOnMessage037);

    rtDmeBeginFields();
    rtMediusAddResponseHeader(&joinChannelResponse, &joinChannelResponse.header);
    rtMediusAddConnection(&joinChannelResponse, &joinChannelResponse.info);
    rtMediusRegisterLobbyMessage(&g_nMediusJoinChannelResponseType, rtMediusOnJoinChannelResponse);

    rtDmeBeginFields();
    rtMediusAddRequestHeader(&joinChannelFwd, &joinChannelFwd);
    rtMediusRegisterLobbyMessage(&g_nMediusJoinChannelFwdType, rtMediusOnMessage039);

    rtDmeBeginFields();
    rtMediusAddChars(&joinChannelFwdResponse,
                     joinChannelFwdResponse.messageId,
                     sizeof(joinChannelFwdResponse.messageId));
    rtMediusAddChars(&joinChannelFwdResponse,
                     joinChannelFwdResponse.sessionKey,
                     sizeof(joinChannelFwdResponse.sessionKey));
    rtMediusAddChars(&joinChannelFwdResponse,
                     joinChannelFwdResponse.accessKey,
                     sizeof(joinChannelFwdResponse.accessKey));
    rtMediusAddInt(&joinChannelFwdResponse, &joinChannelFwdResponse.worldId);
    rtMediusRegisterLobbyMessage(&g_nMediusJoinChannelFwdResponseType, rtMediusOnMessage040);

    rtDmeBeginFields();
    rtMediusAddRequestHeader(&gameList, &gameList.header);
    rtMediusAddInt(&gameList, &gameList.applicationId);
    rtMediusAddShort(&gameList, &gameList.pageId);
    rtMediusAddShort(&gameList, &gameList.pageSize);
    rtMediusRegisterLobbyMessage(&g_nMediusGameListType, rtMediusOnMessage041);

    rtDmeBeginFields();
    rtMediusAddResponseHeader(&gameListResponse, &gameListResponse.header);
    rtMediusAddInt(&gameListResponse, &gameListResponse.worldId);
    rtMediusAddChars(
        &gameListResponse, gameListResponse.gameName, sizeof(gameListResponse.gameName));
    rtMediusAddInt(&gameListResponse, &gameListResponse.worldStatus);
    rtMediusAddInt(&gameListResponse, &gameListResponse.reserved64);
    rtMediusAddInt(&gameListResponse, &gameListResponse.reserved68);
    rtMediusAddChars(&gameListResponse, &gameListResponse.endOfList, 1);
    rtMediusRegisterLobbyMessage(&g_nMediusGameListResponseType, rtMediusOnGameListResponse);

    rtDmeBeginFields();
    rtMediusAddRequestHeader(&channelList, &channelList.header);
    rtMediusAddShort(&channelList, &channelList.pageId);
    rtMediusAddShort(&channelList, &channelList.pageSize);
    rtMediusRegisterLobbyMessage(&g_nMediusChannelListType, rtMediusOnMessage043);

    rtDmeBeginFields();
    rtMediusAddResponseHeader(&channelListResponse, &channelListResponse.header);
    rtMediusAddInt(&channelListResponse, &channelListResponse.worldId);
    rtMediusAddChars(
        &channelListResponse, channelListResponse.name, sizeof(channelListResponse.name));
    rtMediusAddInt(&channelListResponse, &channelListResponse.playerCount);
    rtMediusAddChars(&channelListResponse, &channelListResponse.endOfList, 1);
    rtMediusRegisterLobbyMessage(&g_nMediusChannelListResponseType, rtMediusOnChannelListResponse);

    rtDmeBeginFields();
    rtMediusAddRequestHeader(&channelPlayers, &channelPlayers.header);
    rtMediusAddInt(&channelPlayers, &channelPlayers.id);
    rtMediusRegisterLobbyMessage(&g_nMediusChannelPlayersType, rtMediusOnMessage045);

    rtDmeBeginFields();
    rtMediusAddResponseHeader(&channelPlayersResponse, &channelPlayersResponse.header);
    rtMediusAddInt(&channelPlayersResponse, &channelPlayersResponse.reserved1C);
    rtMediusAddInt(&channelPlayersResponse, &channelPlayersResponse.accountId);
    rtMediusAddChars(&channelPlayersResponse,
                     channelPlayersResponse.accountName,
                     sizeof(channelPlayersResponse.accountName));
    rtMediusAddChars(&channelPlayersResponse,
                     channelPlayersResponse.stats,
                     sizeof(channelPlayersResponse.stats));
    rtMediusAddInt(&channelPlayersResponse, &channelPlayersResponse.connectionClass);
    rtMediusAddChars(&channelPlayersResponse, &channelPlayersResponse.endOfList, 1);
    rtMediusRegisterLobbyMessage(&g_nMediusChannelPlayersResponseType,
                                 rtMediusOnChannelPlayersResponse);

    rtDmeBeginFields();
    rtMediusAddRequestHeader(&gamePlayers, &gamePlayers.header);
    rtMediusAddInt(&gamePlayers, &gamePlayers.id);
    rtMediusRegisterLobbyMessage(&g_nMediusGamePlayersType, rtMediusOnMessage047);

    rtDmeBeginFields();
    rtMediusAddResponseHeader(&gamePlayersResponse, &gamePlayersResponse.header);
    rtMediusAddInt(&gamePlayersResponse, &gamePlayersResponse.accountId);
    rtMediusAddChars(&gamePlayersResponse,
                     gamePlayersResponse.accountName,
                     sizeof(gamePlayersResponse.accountName));
    rtMediusAddChars(
        &gamePlayersResponse, gamePlayersResponse.stats, sizeof(gamePlayersResponse.stats));
    rtMediusAddInt(&gamePlayersResponse, &gamePlayersResponse.connectionClass);
    rtMediusAddChars(&gamePlayersResponse, &gamePlayersResponse.endOfList, 1);
    rtMediusRegisterLobbyMessage(&g_nMediusGamePlayersResponseType, rtMediusOnGamePlayersResponse);

    rtDmeBeginFields();
    rtMediusAddRequestHeader(&playerInfo, &playerInfo.header);
    rtMediusAddInt(&playerInfo, &playerInfo.id);
    rtMediusRegisterLobbyMessage(&g_nMediusPlayerInfoType, rtMediusOnMessage049);

    rtDmeBeginFields();
    rtMediusAddResponseHeader(&playerInfoResponse, &playerInfoResponse.header);
    rtMediusAddInt(&playerInfoResponse, &playerInfoResponse.reserved3C);
    rtMediusAddChars(&playerInfoResponse,
                     playerInfoResponse.accountName,
                     sizeof(playerInfoResponse.accountName));
    rtMediusAddInt(&playerInfoResponse, &playerInfoResponse.reserved40);
    rtMediusAddChars(
        &playerInfoResponse, playerInfoResponse.stats, sizeof(playerInfoResponse.stats));
    rtMediusAddInt(&playerInfoResponse, &playerInfoResponse.connectionClass);
    rtMediusRegisterLobbyMessage(&g_nMediusPlayerInfoResponseType, rtMediusOnPlayerInfoResponse);

    rtDmeBeginFields();
    rtMediusAddRequestHeader(&gameInfo, &gameInfo.header);
    rtMediusAddInt(&gameInfo, &gameInfo.id);
    rtMediusRegisterLobbyMessage(&g_nMediusGameInfoType, rtMediusOnMessage051);

    rtDmeBeginFields();
    rtMediusAddResponseHeader(&gameInfoResponse, &gameInfoResponse.header);
    rtMediusAddInt(&gameInfoResponse, &gameInfoResponse.applicationId);
    rtMediusAddInt(&gameInfoResponse, &gameInfoResponse.minPlayers);
    rtMediusAddInt(&gameInfoResponse, &gameInfoResponse.maxPlayers);
    rtMediusAddInt(&gameInfoResponse, &gameInfoResponse.gameLevel);
    rtMediusAddInt(&gameInfoResponse, &gameInfoResponse.playerSkillLevel);
    rtMediusAddInt(&gameInfoResponse, &gameInfoResponse.playerCount);
    rtMediusAddChars(
        &gameInfoResponse, gameInfoResponse.gameStats, sizeof(gameInfoResponse.gameStats));
    rtMediusAddChars(
        &gameInfoResponse, gameInfoResponse.gameName, sizeof(gameInfoResponse.gameName));
    rtMediusAddInt(&gameInfoResponse, &gameInfoResponse.rulesSet);
    rtMediusAddInt(&gameInfoResponse, &gameInfoResponse.genericField1);
    rtMediusAddInt(&gameInfoResponse, &gameInfoResponse.genericField2);
    rtMediusAddInt(&gameInfoResponse, &gameInfoResponse.genericField3);
    rtMediusAddInt(&gameInfoResponse, &gameInfoResponse.worldStatus);
    rtMediusAddInt(&gameInfoResponse, &gameInfoResponse.gameHostType);
    rtMediusRegisterLobbyMessage(&g_nMediusGameInfoResponseType, rtMediusOnMessage052);

    rtDmeBeginFields();
    rtMediusAddRequestHeader(&channelInfo, &channelInfo.header);
    rtMediusAddInt(&channelInfo, &channelInfo.id);
    rtMediusRegisterLobbyMessage(&g_nMediusChannelInfoType, rtMediusOnMessage053);

    rtDmeBeginFields();
    rtMediusAddResponseHeader(&channelInfoResponse, &channelInfoResponse.header);
    rtMediusAddChars(
        &channelInfoResponse, channelInfoResponse.name, sizeof(channelInfoResponse.name));
    rtMediusAddInt(&channelInfoResponse, &channelInfoResponse.playerCount);
    rtMediusAddInt(&channelInfoResponse, &channelInfoResponse.maxPlayers);
    rtMediusRegisterLobbyMessage(&g_nMediusChannelInfoResponseType, rtMediusOnChannelInfoResponse);

    rtDmeBeginFields();
    rtMediusAddRequestHeader(&findWorldByName, &findWorldByName.header);
    rtMediusAddChars(&findWorldByName, findWorldByName.name, sizeof(findWorldByName.name));
    rtMediusAddInt(&findWorldByName, &findWorldByName.worldType);
    rtMediusRegisterLobbyMessage(&g_nMediusFindWorldByNameType, rtMediusOnMessage055);

    rtDmeBeginFields();
    rtMediusAddResponseHeader(&findWorldByNameResponse, &findWorldByNameResponse.header);
    rtMediusAddInt(&findWorldByNameResponse, &findWorldByNameResponse.applicationId);
    rtMediusAddChars(&findWorldByNameResponse,
                     findWorldByNameResponse.applicationName,
                     sizeof(findWorldByNameResponse.applicationName));
    rtMediusAddInt(&findWorldByNameResponse, &findWorldByNameResponse.applicationType);
    rtMediusAddInt(&findWorldByNameResponse, &findWorldByNameResponse.worldId);
    rtMediusAddChars(&findWorldByNameResponse, findWorldByNameResponse.worldName, kWorldNameCount);
    rtMediusAddInt(&findWorldByNameResponse, &findWorldByNameResponse.worldStatus);
    rtMediusAddChars(&findWorldByNameResponse, &findWorldByNameResponse.endOfList, 1);
    rtMediusRegisterLobbyMessage(&g_nMediusFindWorldByNameResponseType, rtMediusOnMessage056);

    rtDmeBeginFields();
    rtMediusAddRequestHeader(&findPlayer, &findPlayer.header);
    rtMediusAddInt(&findPlayer, &findPlayer.searchType);
    rtMediusAddInt(&findPlayer, &findPlayer.accountId);
    rtMediusAddChars(&findPlayer, findPlayer.accountName, sizeof(findPlayer.accountName));
    rtMediusRegisterLobbyMessage(&g_nMediusFindPlayerType, rtMediusOnMessage057);

    rtDmeBeginFields();
    rtMediusAddResponseHeader(&findPlayerResponse, &findPlayerResponse.header);
    rtMediusAddInt(&findPlayerResponse, &findPlayerResponse.applicationId);
    rtMediusAddChars(&findPlayerResponse,
                     findPlayerResponse.applicationName,
                     sizeof(findPlayerResponse.applicationName));
    rtMediusAddInt(&findPlayerResponse, &findPlayerResponse.locationType);
    rtMediusAddInt(&findPlayerResponse, &findPlayerResponse.worldId);
    rtMediusAddInt(&findPlayerResponse, &findPlayerResponse.online);
    rtMediusAddChars(&findPlayerResponse,
                     findPlayerResponse.accountName,
                     sizeof(findPlayerResponse.accountName));
    rtMediusRegisterLobbyMessage(&g_nMediusFindPlayerResponseType, rtMediusOnFindPlayerResponse);

    rtDmeBeginFields();
    rtMediusAddRequestHeader(&chatMessage, &chatMessage.header);
    rtMediusAddInt(&chatMessage, &chatMessage.toAccount);
    rtMediusAddInt(&chatMessage, &chatMessage.targetAccountId);
    rtMediusAddChars(&chatMessage, chatMessage.text, sizeof(chatMessage.text));
    rtMediusRegisterLobbyMessage(&g_nMediusChatMessageType, rtMediusOnMessage059);

    rtDmeBeginFields();
    rtMediusAddChars(&chatFwdMessage, chatFwdMessage.messageId, sizeof(chatFwdMessage.messageId));
    rtMediusAddInt(&chatFwdMessage, &chatFwdMessage.originatorAccountId);
    rtMediusAddChars(
        &chatFwdMessage, chatFwdMessage.originatorName, sizeof(chatFwdMessage.originatorName));
    rtMediusAddInt(&chatFwdMessage, &chatFwdMessage.messageType);
    rtMediusAddChars(&chatFwdMessage, chatFwdMessage.text, sizeof(chatFwdMessage.text));
    rtMediusRegisterLobbyMessage(&g_nMediusChatFwdMessageType, rtMediusOnChatMessage);

    rtDmeBeginFields();
    rtMediusAddRequestHeader(&getBuddyList, &getBuddyList);
    rtMediusRegisterLobbyMessage(&g_nMediusGetBuddyListType, rtMediusOnMessage061);

    rtDmeBeginFields();
    rtMediusAddResponseHeader(&getBuddyListResponse, &getBuddyListResponse.header);
    rtMediusAddInt(&getBuddyListResponse, &getBuddyListResponse.accountId);
    rtMediusAddChars(&getBuddyListResponse,
                     getBuddyListResponse.accountName,
                     sizeof(getBuddyListResponse.accountName));
    rtMediusAddInt(&getBuddyListResponse, &getBuddyListResponse.onlineState);
    rtMediusAddChars(&getBuddyListResponse, &getBuddyListResponse.endOfList, 1);
    rtMediusRegisterLobbyMessage(&g_nMediusGetBuddyListResponseType, rtMediusOnMessage062);

    rtDmeBeginFields();
    rtMediusAddRequestHeader(&addToBuddyList, &addToBuddyList.header);
    rtMediusAddInt(&addToBuddyList, &addToBuddyList.id);
    rtMediusRegisterLobbyMessage(&g_nMediusAddToBuddyListType, rtMediusOnMessage063);

    rtDmeBeginFields();
    rtMediusAddResponseHeader(&addToBuddyListResponse, &addToBuddyListResponse);
    rtMediusRegisterLobbyMessage(&g_nMediusAddToBuddyListResponseType, rtMediusOnMessage064);

    rtDmeBeginFields();
    rtMediusAddRequestHeader(&removeFromBuddyList, &removeFromBuddyList.header);
    rtMediusAddInt(&removeFromBuddyList, &removeFromBuddyList.id);
    rtMediusRegisterLobbyMessage(&g_nMediusRemoveFromBuddyListType, rtMediusOnMessage065);

    rtDmeBeginFields();
    rtMediusAddResponseHeader(&removeFromBuddyListResponse, &removeFromBuddyListResponse);
    rtMediusRegisterLobbyMessage(&g_nMediusRemoveFromBuddyListResponseType, rtMediusOnMessage066);

    rtDmeBeginFields();
    rtMediusAddRequestHeader(&buddyConfirmation, &buddyConfirmation.header);
    rtMediusAddInt(&buddyConfirmation, &buddyConfirmation.id);
    rtMediusRegisterLobbyMessage(&g_nMediusAddToBuddyListConfirmationType, rtMediusOnMessage067);

    rtDmeBeginFields();
    rtMediusAddResponseHeader(&buddyConfirmationResponse, &buddyConfirmationResponse.header);
    rtMediusAddInt(&buddyConfirmationResponse, &buddyConfirmationResponse.accountId);
    rtMediusAddChars(&buddyConfirmationResponse,
                     buddyConfirmationResponse.accountName,
                     sizeof(buddyConfirmationResponse.accountName));
    rtMediusRegisterLobbyMessage(&g_nMediusAddToBuddyListConfirmationResponseType,
                                 rtMediusOnMessage068);

    rtDmeBeginFields();
    rtMediusAddChars(&buddyFwdConfirmation,
                     buddyFwdConfirmation.messageId,
                     sizeof(buddyFwdConfirmation.messageId));
    rtMediusAddInt(&buddyFwdConfirmation, &buddyFwdConfirmation.originatorAccountId);
    rtMediusAddChars(&buddyFwdConfirmation,
                     buddyFwdConfirmation.originatorName,
                     sizeof(buddyFwdConfirmation.originatorName));
    rtMediusRegisterLobbyMessage(&g_nMediusAddToBuddyListFwdConfirmationType,
                                 rtMediusOnInitHandler04);

    rtDmeBeginFields();
    rtMediusAddChars(&buddyFwdConfirmationResponse,
                     buddyFwdConfirmationResponse.messageId,
                     sizeof(buddyFwdConfirmationResponse.messageId));
    rtMediusAddInt(&buddyFwdConfirmationResponse, &buddyFwdConfirmationResponse.responseType);
    rtMediusAddChars(&buddyFwdConfirmationResponse,
                     buddyFwdConfirmationResponse.sessionKey,
                     sizeof(buddyFwdConfirmationResponse.sessionKey));
    rtMediusAddInt(&buddyFwdConfirmationResponse,
                   &buddyFwdConfirmationResponse.originatorAccountId);
    rtMediusRegisterLobbyMessage(&g_nMediusAddToBuddyListFwdConfirmationResponseType,
                                 rtMediusOnMessage070);

    rtDmeBeginFields();
    rtMediusAddRequestHeader(&getPolicy, &getPolicy.header);
    rtMediusAddInt(&getPolicy, &getPolicy.id);
    rtMediusRegisterLobbyMessage(&g_nMediusGetPolicyType, rtMediusOnMessage071);

    rtDmeBeginFields();
    rtMediusAddResponseHeader(&getPolicyResponse, &getPolicyResponse.header);
    rtMediusAddChars(
        &getPolicyResponse, getPolicyResponse.policy, sizeof(getPolicyResponse.policy));
    rtMediusAddChars(&getPolicyResponse, &getPolicyResponse.endOfText, 1);
    rtMediusRegisterLobbyMessage(&g_nMediusGetPolicyResponseType, rtMediusOnGetPolicyResponse);

    rtDmeBeginFields();
    rtMediusAddChars(
        &updateUserState, updateUserState.sessionKey, sizeof(updateUserState.sessionKey));
    rtMediusAddInt(&updateUserState, &updateUserState.userAction);
    rtMediusRegisterLobbyMessage(&g_nMediusUpdateUserStateType, rtMediusOnMessage073);

    rtDmeBeginFields();
    rtMediusAddInt(&notice, &notice.code);
    rtMediusAddChars(&notice, notice.text, sizeof(notice.text));
    rtMediusRegisterLobbyMessage(&g_nMediusNoticeType, rtMediusOnNotice);

    rtDmeBeginFields();
    rtMediusAddRequestHeader(&getAnnouncements, &getAnnouncements.header);
    rtMediusAddInt(&getAnnouncements, &getAnnouncements.id);
    rtMediusRegisterLobbyMessage(&g_nMediusGetAnnouncementsType, rtMediusOnMessage075);

    rtDmeBeginFields();
    rtMediusAddRequestHeader(&getAllAnnouncements, &getAllAnnouncements.header);
    rtMediusRegisterLobbyMessage(&g_nMediusGetAllAnnouncementsType, rtMediusOnMessage076);

    rtDmeBeginFields();
    rtMediusAddResponseHeader(&getAnnouncementsResponse, &getAnnouncementsResponse.header);
    rtMediusAddInt(&getAnnouncementsResponse, &getAnnouncementsResponse.announcementId);
    rtMediusAddChars(&getAnnouncementsResponse,
                     getAnnouncementsResponse.announcement,
                     sizeof(getAnnouncementsResponse.announcement));
    rtMediusAddChars(&getAnnouncementsResponse, &getAnnouncementsResponse.endOfList, 1);
    rtMediusRegisterLobbyMessage(&g_nMediusGetAnnouncementsResponseType,
                                 rtMediusOnGetAnnouncementsResponse);

    rtDmeBeginFields();
    rtMediusAddRequestHeader(&setGameListFilter, &setGameListFilter.header);
    rtMediusAddInt(&setGameListFilter, &setGameListFilter.filterField);
    rtMediusAddInt(&setGameListFilter, &setGameListFilter.comparison);
    rtMediusAddInt(&setGameListFilter, &setGameListFilter.value);
    rtMediusRegisterLobbyMessage(&g_nMediusSetGameListFilterType, rtMediusOnMessage078);

    rtDmeBeginFields();
    rtMediusAddResponseHeader(&setGameListFilterResponse, &setGameListFilterResponse);
    rtMediusRegisterLobbyMessage(&g_nMediusSetGameListFilterResponseType,
                                 rtMediusOnSetGameListFilterResponse);

    rtDmeBeginFields();
    rtMediusAddRequestHeader(&clearGameListFilter, &clearGameListFilter.header);
    rtMediusAddInt(&clearGameListFilter, &clearGameListFilter.id);
    rtMediusRegisterLobbyMessage(&g_nMediusClearGameListFilterType, rtMediusOnMessage080);

    rtDmeBeginFields();
    rtMediusAddResponseHeader(&clearGameListFilterResponse, &clearGameListFilterResponse);
    rtMediusRegisterLobbyMessage(&g_nMediusClearGameListFilterResponseType,
                                 rtMediusOnClearGameListFilterResponse);

    rtDmeBeginFields();
    rtMediusAddRequestHeader(&getGameListFilter, &getGameListFilter);
    rtMediusRegisterLobbyMessage(&g_nMediusGetGameListFilterType, rtMediusOnMessage082);

    rtDmeBeginFields();
    rtMediusAddResponseHeader(&getGameListFilterResponse, &getGameListFilterResponse.header);
    rtMediusAddInt(&getGameListFilterResponse, &getGameListFilterResponse.filterField);
    rtMediusAddInt(&getGameListFilterResponse, &getGameListFilterResponse.comparison);
    rtMediusAddInt(&getGameListFilterResponse, &getGameListFilterResponse.value);
    rtMediusRegisterLobbyMessage(&g_nMediusGetGameListFilterResponseType, rtMediusOnMessage083);

    rtDmeBeginFields();
    rtMediusAddRequestHeader(&createClan, &createClan.header);
    rtMediusAddInt(&createClan, &createClan.id);
    // Yes, the binary adds the clan name of the record of message 89 to this list.
    rtMediusAddChars(
        &getClanByIdResponse, getClanByIdResponse.clanName, sizeof(getClanByIdResponse.clanName));
    rtMediusRegisterLobbyMessage(&g_nMediusCreateClanType, rtMediusOnMessage084);

    rtDmeBeginFields();
    rtMediusAddResponseHeader(&createClanResponse, &createClanResponse.header);
    rtMediusAddInt(&createClanResponse, &createClanResponse.id);
    rtMediusRegisterLobbyMessage(&g_nMediusCreateClanResponseType, rtMediusOnMessage085);

    rtDmeBeginFields();
    rtMediusAddRequestHeader(&disbandClan, &disbandClan.header);
    rtMediusAddInt(&disbandClan, &disbandClan.id);
    rtMediusRegisterLobbyMessage(&g_nMediusDisbandClanType, rtMediusOnMessage086);

    rtDmeBeginFields();
    rtMediusAddResponseHeader(&disbandClanResponse, &disbandClanResponse);
    rtMediusRegisterLobbyMessage(&g_nMediusDisbandClanResponseType, rtMediusOnMessage087);

    rtDmeBeginFields();
    rtMediusAddRequestHeader(&getClanById, &getClanById.header);
    rtMediusAddInt(&getClanById, &getClanById.value);
    rtMediusRegisterLobbyMessage(&g_nMediusGetClanByIdType, rtMediusOnMessage088);

    rtDmeBeginFields();
    rtMediusAddResponseHeader(&getClanByIdResponse, &getClanByIdResponse.header);
    rtMediusAddInt(&getClanByIdResponse, &getClanByIdResponse.applicationId);
    rtMediusAddChars(
        &getClanByIdResponse, getClanByIdResponse.clanName, sizeof(getClanByIdResponse.clanName));
    rtMediusAddInt(&getClanByIdResponse, &getClanByIdResponse.leaderAccountId);
    rtMediusAddChars(&getClanByIdResponse,
                     getClanByIdResponse.leaderAccountName,
                     sizeof(getClanByIdResponse.leaderAccountName));
    rtMediusAddChars(
        &getClanByIdResponse, getClanByIdResponse.stats, sizeof(getClanByIdResponse.stats));
    rtMediusAddInt(&getClanByIdResponse, &getClanByIdResponse.clanStatus);
    rtMediusRegisterLobbyMessage(&g_nMediusGetClanByIdResponseType, rtMediusOnMessage089);

    rtDmeBeginFields();
    rtMediusAddRequestHeader(&getClanByName, &getClanByName.header);
    rtMediusAddInt(&getClanByName, &getClanByName.id);
    rtMediusAddChars(&getClanByName, getClanByName.name, sizeof(getClanByName.name));
    rtMediusRegisterLobbyMessage(&g_nMediusGetClanByNameType, rtMediusOnMessage090);

    rtDmeBeginFields();
    rtMediusAddResponseHeader(&getClanByNameResponse, &getClanByNameResponse.header);
    rtMediusAddInt(&getClanByNameResponse, &getClanByNameResponse.clanId);
    rtMediusAddInt(&getClanByNameResponse, &getClanByNameResponse.leaderAccountId);
    rtMediusAddChars(&getClanByNameResponse,
                     getClanByNameResponse.leaderAccountName,
                     sizeof(getClanByNameResponse.leaderAccountName));
    rtMediusAddChars(
        &getClanByNameResponse, getClanByNameResponse.stats, sizeof(getClanByNameResponse.stats));
    rtMediusAddInt(&getClanByNameResponse, &getClanByNameResponse.clanStatus);
    rtMediusRegisterLobbyMessage(&g_nMediusGetClanByNameResponseType, rtMediusOnMessage091);

    rtDmeBeginFields();
    rtMediusAddRequestHeader(&transferClanLeadership, &transferClanLeadership.header);
    rtMediusAddInt(&transferClanLeadership, &transferClanLeadership.id);
    rtMediusAddChars(
        &transferClanLeadership, transferClanLeadership.name, sizeof(transferClanLeadership.name));
    rtMediusRegisterLobbyMessage(&g_nMediusTransferClanLeadershipType, rtMediusOnMessage092);

    rtDmeBeginFields();
    rtMediusAddResponseHeader(&transferClanLeadershipResponse, &transferClanLeadershipResponse);
    rtMediusRegisterLobbyMessage(&g_nMediusTransferClanLeadershipResponseType,
                                 rtMediusOnMessage093);

    rtDmeBeginFields();
    rtMediusAddRequestHeader(&addPlayerToClan, &addPlayerToClan.header);
    rtMediusAddInt(&addPlayerToClan, &addPlayerToClan.id);
    rtMediusAddChars(&addPlayerToClan, addPlayerToClan.message, sizeof(addPlayerToClan.message));
    rtMediusRegisterLobbyMessage(&g_nMediusAddPlayerToClanType, rtMediusOnMessage094);

    rtDmeBeginFields();
    rtMediusAddResponseHeader(&addPlayerToClanResponse, &addPlayerToClanResponse);
    rtMediusRegisterLobbyMessage(&g_nMediusAddPlayerToClanResponseType, rtMediusOnMessage095);

    rtDmeBeginFields();
    rtMediusAddRequestHeader(&removePlayerFromClan, &removePlayerFromClan.header);
    rtMediusAddInt(&removePlayerFromClan, &removePlayerFromClan.id);
    rtMediusAddInt(&removePlayerFromClan, &removePlayerFromClan.value);
    rtMediusRegisterLobbyMessage(&g_nMediusRemovePlayerFromClanType, rtMediusOnMessage096);

    rtDmeBeginFields();
    rtMediusAddResponseHeader(&removePlayerFromClanResponse, &removePlayerFromClanResponse);
    rtMediusRegisterLobbyMessage(&g_nMediusRemovePlayerFromClanResponseType, rtMediusOnMessage097);

    rtDmeBeginFields();
    rtMediusAddRequestHeader(&invitePlayerToClan, &invitePlayerToClan.header);
    rtMediusAddInt(&invitePlayerToClan, &invitePlayerToClan.id);
    rtMediusAddChars(
        &invitePlayerToClan, invitePlayerToClan.message, sizeof(invitePlayerToClan.message));
    rtMediusRegisterLobbyMessage(&g_nMediusInvitePlayerToClanType, rtMediusOnMessage098);

    rtDmeBeginFields();
    rtMediusAddResponseHeader(&invitePlayerToClanResponse, &invitePlayerToClanResponse);
    rtMediusRegisterLobbyMessage(&g_nMediusInvitePlayerToClanResponseType, rtMediusOnMessage099);

    rtDmeBeginFields();
    rtMediusAddRequestHeader(&checkMyClanInvitations, &checkMyClanInvitations.header);
    rtMediusAddInt(&checkMyClanInvitations, &checkMyClanInvitations.id);
    rtMediusAddInt(&checkMyClanInvitations, &checkMyClanInvitations.value);
    rtMediusRegisterLobbyMessage(&g_nMediusCheckMyClanInvitationsType, rtMediusOnMessage100);

    rtDmeBeginFields();
    rtMediusAddResponseHeader(&checkMyClanInvitationsResponse,
                              &checkMyClanInvitationsResponse.header);
    rtMediusAddInt(&checkMyClanInvitationsResponse,
                   &checkMyClanInvitationsResponse.clanInvitationId);
    rtMediusAddInt(&checkMyClanInvitationsResponse, &checkMyClanInvitationsResponse.clanId);
    rtMediusAddInt(&checkMyClanInvitationsResponse, &checkMyClanInvitationsResponse.responseStatus);
    rtMediusAddChars(&checkMyClanInvitationsResponse,
                     checkMyClanInvitationsResponse.message,
                     sizeof(checkMyClanInvitationsResponse.message));
    rtMediusAddInt(&checkMyClanInvitationsResponse,
                   &checkMyClanInvitationsResponse.leaderAccountId);
    rtMediusAddChars(&checkMyClanInvitationsResponse,
                     checkMyClanInvitationsResponse.leaderAccountName,
                     sizeof(checkMyClanInvitationsResponse.leaderAccountName));
    rtMediusAddChars(&checkMyClanInvitationsResponse, &checkMyClanInvitationsResponse.endOfList, 1);
    rtMediusRegisterLobbyMessage(&g_nMediusCheckMyClanInvitationsResponseType,
                                 rtMediusOnMessage101);

    rtDmeBeginFields();
    rtMediusAddRequestHeader(&respondToClanInvitation, &respondToClanInvitation.header);
    rtMediusAddInt(&respondToClanInvitation, &respondToClanInvitation.id);
    rtMediusAddInt(&respondToClanInvitation, &respondToClanInvitation.response);
    rtMediusAddChars(&respondToClanInvitation,
                     respondToClanInvitation.message,
                     sizeof(respondToClanInvitation.message));
    rtMediusRegisterLobbyMessage(&g_nMediusRespondToClanInvitationType, rtMediusOnMessage102);

    rtDmeBeginFields();
    rtMediusAddResponseHeader(&respondToClanInvitationResponse, &respondToClanInvitationResponse);
    rtMediusRegisterLobbyMessage(&g_nMediusRespondToClanInvitationResponseType,
                                 rtMediusOnMessage103);

    rtDmeBeginFields();
    rtMediusAddRequestHeader(&revokeClanInvitation, &revokeClanInvitation.header);
    rtMediusAddInt(&revokeClanInvitation, &revokeClanInvitation.id);
    rtMediusRegisterLobbyMessage(&g_nMediusRevokeClanInvitationType, rtMediusOnMessage104);

    rtDmeBeginFields();
    rtMediusAddResponseHeader(&revokeClanInvitationResponse, &revokeClanInvitationResponse);
    rtMediusRegisterLobbyMessage(&g_nMediusRevokeClanInvitationResponseType, rtMediusOnMessage105);

    rtDmeBeginFields();
    rtMediusAddRequestHeader(&requestClanTeamChallenge, &requestClanTeamChallenge.header);
    rtMediusAddInt(&requestClanTeamChallenge, &requestClanTeamChallenge.id);
    rtMediusAddChars(&requestClanTeamChallenge,
                     requestClanTeamChallenge.message,
                     sizeof(requestClanTeamChallenge.message));
    rtMediusRegisterLobbyMessage(&g_nMediusRequestClanTeamChallengeType, rtMediusOnMessage106);

    rtDmeBeginFields();
    rtMediusAddResponseHeader(&requestClanTeamChallengeResponse, &requestClanTeamChallengeResponse);
    rtMediusRegisterLobbyMessage(&g_nMediusRequestClanTeamChallengeResponseType,
                                 rtMediusOnMessage107);

    rtDmeBeginFields();
    rtMediusAddRequestHeader(&getMyClanMessages, &getMyClanMessages.header);
    rtMediusAddInt(&getMyClanMessages, &getMyClanMessages.clanId);
    rtMediusAddInt(&getMyClanMessages, &getMyClanMessages.start);
    rtMediusAddInt(&getMyClanMessages, &getMyClanMessages.pageSize);
    rtMediusRegisterLobbyMessage(&g_nMediusGetMyClanMessagesType, rtMediusOnMessage108);

    rtDmeBeginFields();
    rtMediusAddResponseHeader(&getMyClanMessagesResponse, &getMyClanMessagesResponse.header);
    rtMediusAddInt(&getMyClanMessagesResponse, &getMyClanMessagesResponse.clanId);
    rtMediusAddChars(&getMyClanMessagesResponse,
                     getMyClanMessagesResponse.message,
                     sizeof(getMyClanMessagesResponse.message));
    rtMediusAddChars(&getMyClanMessagesResponse, &getMyClanMessagesResponse.endOfList, 1);
    rtMediusRegisterLobbyMessage(&g_nMediusGetMyClanMessagesResponseType, rtMediusOnMessage109);

    rtDmeBeginFields();
    rtMediusAddRequestHeader(&sendClanMessage, &sendClanMessage.header);
    rtMediusAddChars(&sendClanMessage, sendClanMessage.message, sizeof(sendClanMessage.message));
    rtMediusRegisterLobbyMessage(&g_nMediusSendClanMessageType, rtMediusOnMessage110);

    rtDmeBeginFields();
    rtMediusAddChars(&sendClanMessageResponse,
                     sendClanMessageResponse.messageId,
                     sizeof(sendClanMessageResponse.messageId));
    // Yes, the binary adds the identifier a second time, as one element of its size.
    rtDmeAddField(&sendClanMessageResponse,
                  sendClanMessageResponse.messageId,
                  sizeof(sendClanMessageResponse.messageId),
                  1);
    rtMediusAddInt(&sendClanMessageResponse, &sendClanMessageResponse.status);
    rtMediusRegisterLobbyMessage(&g_nMediusSendClanMessageResponseType, rtMediusOnMessage111);

    rtDmeBeginFields();
    rtMediusAddRequestHeader(&modifyClanMessage, &modifyClanMessage.header);
    rtMediusAddInt(&modifyClanMessage, &modifyClanMessage.id);
    rtMediusAddChars(
        &modifyClanMessage, modifyClanMessage.message, sizeof(modifyClanMessage.message));
    rtMediusRegisterLobbyMessage(&g_nMediusModifyClanMessageType, rtMediusOnMessage112);

    rtDmeBeginFields();
    rtMediusAddResponseHeader(&modifyClanMessageResponse, &modifyClanMessageResponse);
    rtMediusRegisterLobbyMessage(&g_nMediusModifyClanMessageResponseType, rtMediusOnMessage113);

    rtDmeBeginFields();
    rtMediusAddRequestHeader(&deleteClanMessage, &deleteClanMessage.header);
    rtMediusAddInt(&deleteClanMessage, &deleteClanMessage.id);
    rtMediusRegisterLobbyMessage(&g_nMediusDeleteClanMessageType, rtMediusOnMessage114);

    rtDmeBeginFields();
    rtMediusAddResponseHeader(&deleteClanMessageResponse, &deleteClanMessageResponse);
    rtMediusRegisterLobbyMessage(&g_nMediusDeleteClanMessageResponseType, rtMediusOnMessage115);

    rtDmeBeginFields();
    rtMediusAddRequestHeader(&respondToClanTeamChallenge, &respondToClanTeamChallenge.header);
    rtMediusAddInt(&respondToClanTeamChallenge, &respondToClanTeamChallenge.id);
    rtMediusAddInt(&respondToClanTeamChallenge, &respondToClanTeamChallenge.response);
    rtMediusAddChars(&respondToClanTeamChallenge,
                     respondToClanTeamChallenge.message,
                     sizeof(respondToClanTeamChallenge.message));
    rtMediusRegisterLobbyMessage(&g_nMediusRespondToClanTeamChallengeType, rtMediusOnMessage116);

    rtDmeBeginFields();
    rtMediusAddResponseHeader(&respondToClanTeamChallengeResponse,
                              &respondToClanTeamChallengeResponse);
    rtMediusRegisterLobbyMessage(&g_nMediusRespondToClanTeamChallengeResponseType,
                                 rtMediusOnMessage117);

    rtDmeBeginFields();
    rtMediusAddRequestHeader(&revokeClanTeamChallenge, &revokeClanTeamChallenge.header);
    rtMediusAddInt(&revokeClanTeamChallenge, &revokeClanTeamChallenge.id);
    rtMediusRegisterLobbyMessage(&g_nMediusRevokeClanTeamChallengeType, rtMediusOnMessage118);

    rtDmeBeginFields();
    rtMediusAddResponseHeader(&revokeClanTeamChallengeResponse, &revokeClanTeamChallengeResponse);
    rtMediusRegisterLobbyMessage(&g_nMediusRevokeClanTeamChallengeResponseType,
                                 rtMediusOnMessage119);

    rtDmeBeginFields();
    rtMediusAddRequestHeader(&getClanTeamChallengeHistory, &getClanTeamChallengeHistory.header);
    rtMediusAddInt(&getClanTeamChallengeHistory, &getClanTeamChallengeHistory.clanId);
    rtMediusAddInt(&getClanTeamChallengeHistory, &getClanTeamChallengeHistory.threshold);
    rtMediusAddInt(&getClanTeamChallengeHistory, &getClanTeamChallengeHistory.start);
    rtMediusAddInt(&getClanTeamChallengeHistory, &getClanTeamChallengeHistory.pageSize);
    rtMediusRegisterLobbyMessage(&g_nMediusGetClanTeamChallengeHistoryType, rtMediusOnMessage120);

    rtDmeBeginFields();
    rtMediusAddResponseHeader(&getClanTeamChallengeHistoryResponse,
                              &getClanTeamChallengeHistoryResponse.header);
    rtMediusAddInt(&getClanTeamChallengeHistoryResponse,
                   &getClanTeamChallengeHistoryResponse.againstClanId);
    rtMediusAddInt(&getClanTeamChallengeHistoryResponse,
                   &getClanTeamChallengeHistoryResponse.challengeStatus);
    rtMediusAddChars(
        &getClanTeamChallengeHistoryResponse, &getClanTeamChallengeHistoryResponse.endOfList, 1);
    rtMediusRegisterLobbyMessage(&g_nMediusGetClanTeamChallengeHistoryResponseType,
                                 rtMediusOnMessage121);

    rtDmeBeginFields();
    rtMediusAddRequestHeader(&getClanInvitationsSent, &getClanInvitationsSent.header);
    rtMediusAddInt(&getClanInvitationsSent, &getClanInvitationsSent.id);
    rtMediusAddInt(&getClanInvitationsSent, &getClanInvitationsSent.value);
    rtMediusRegisterLobbyMessage(&g_nMediusGetClanInvitationsSentType, rtMediusOnMessage122);

    rtDmeBeginFields();
    rtMediusAddResponseHeader(&getClanInvitationsSentResponse,
                              &getClanInvitationsSentResponse.header);
    rtMediusAddChars(&getClanInvitationsSentResponse,
                     getClanInvitationsSentResponse.accountName,
                     sizeof(getClanInvitationsSentResponse.accountName));
    rtMediusAddChars(&getClanInvitationsSentResponse,
                     getClanInvitationsSentResponse.message,
                     sizeof(getClanInvitationsSentResponse.message));
    rtMediusAddInt(&getClanInvitationsSentResponse, &getClanInvitationsSentResponse.responseStatus);
    rtMediusAddInt(&getClanInvitationsSentResponse, &getClanInvitationsSentResponse.responseTime);
    rtMediusAddChars(&getClanInvitationsSentResponse, &getClanInvitationsSentResponse.endOfList, 1);
    rtMediusRegisterLobbyMessage(&g_nMediusGetClanInvitationsSentResponseType,
                                 rtMediusOnMessage123);

    rtDmeBeginFields();
    rtMediusAddRequestHeader(&getMyClans, &getMyClans.header);
    rtMediusAddInt(&getMyClans, &getMyClans.id);
    rtMediusAddInt(&getMyClans, &getMyClans.value);
    rtMediusRegisterLobbyMessage(&g_nMediusGetMyClansType, rtMediusOnMessage124);

    rtDmeBeginFields();
    rtMediusAddResponseHeader(&getMyClansResponse, &getMyClansResponse.header);
    rtMediusAddInt(&getMyClansResponse, &getMyClansResponse.clanId);
    rtMediusAddInt(&getMyClansResponse, &getMyClansResponse.applicationId);
    rtMediusAddChars(
        &getMyClansResponse, getMyClansResponse.clanName, sizeof(getMyClansResponse.clanName));
    rtMediusAddInt(&getMyClansResponse, &getMyClansResponse.leaderAccountId);
    rtMediusAddChars(&getMyClansResponse,
                     getMyClansResponse.leaderAccountName,
                     sizeof(getMyClansResponse.leaderAccountName));
    rtMediusAddChars(
        &getMyClansResponse, getMyClansResponse.stats, sizeof(getMyClansResponse.stats));
    rtMediusAddChars(&getMyClansResponse, &getMyClansResponse.endOfList, 1);
    rtMediusRegisterLobbyMessage(&g_nMediusGetMyClansResponseType, rtMediusOnMessage125);

    rtDmeBeginFields();
    rtMediusAddRequestHeader(&getAllClanMessages, &getAllClanMessages.header);
    rtMediusAddInt(&getAllClanMessages, &getAllClanMessages.id);
    rtMediusAddInt(&getAllClanMessages, &getAllClanMessages.value);
    rtMediusRegisterLobbyMessage(&g_nMediusGetAllClanMessagesType, rtMediusOnMessage126);

    rtDmeBeginFields();
    rtMediusAddResponseHeader(&getAllClanMessagesResponse, &getAllClanMessagesResponse.header);
    rtMediusAddInt(&getAllClanMessagesResponse, &getAllClanMessagesResponse.clanMessageId);
    rtMediusAddChars(&getAllClanMessagesResponse,
                     getAllClanMessagesResponse.message,
                     sizeof(getAllClanMessagesResponse.message));
    rtMediusAddInt(&getAllClanMessagesResponse, &getAllClanMessagesResponse.messageStatus);
    rtMediusAddChars(&getAllClanMessagesResponse, &getAllClanMessagesResponse.endOfList, 1);
    rtMediusRegisterLobbyMessage(&g_nMediusGetAllClanMessagesResponseType, rtMediusOnMessage127);

    rtDmeBeginFields();
    rtMediusAddRequestHeader(&confirmClanTeamChallenge, &confirmClanTeamChallenge.header);
    rtMediusAddInt(&confirmClanTeamChallenge, &confirmClanTeamChallenge.id);
    rtMediusRegisterLobbyMessage(&g_nMediusConfirmClanTeamChallengeType, rtMediusOnMessage128);

    rtDmeBeginFields();
    rtMediusAddResponseHeader(&confirmClanTeamChallengeResponse, &confirmClanTeamChallengeResponse);
    rtMediusRegisterLobbyMessage(&g_nMediusConfirmClanTeamChallengeResponseType,
                                 rtMediusOnMessage129);

    rtDmeBeginFields();
    rtMediusAddRequestHeader(&getClanTeamChallenges, &getClanTeamChallenges.header);
    rtMediusAddInt(&getClanTeamChallenges, &getClanTeamChallenges.clanId);
    rtMediusAddInt(&getClanTeamChallenges, &getClanTeamChallenges.challengeStatus);
    rtMediusAddInt(&getClanTeamChallenges, &getClanTeamChallenges.threshold);
    rtMediusAddInt(&getClanTeamChallenges, &getClanTeamChallenges.start);
    rtMediusAddInt(&getClanTeamChallenges, &getClanTeamChallenges.pageSize);
    rtMediusRegisterLobbyMessage(&g_nMediusGetClanTeamChallengesType, rtMediusOnMessage130);

    rtDmeBeginFields();
    rtMediusAddResponseHeader(&getClanTeamChallengesResponse,
                              &getClanTeamChallengesResponse.header);
    rtMediusAddInt(&getClanTeamChallengesResponse, &getClanTeamChallengesResponse.challengerClanId);
    rtMediusAddInt(&getClanTeamChallengesResponse, &getClanTeamChallengesResponse.againstClanId);
    rtMediusAddInt(&getClanTeamChallengesResponse, &getClanTeamChallengesResponse.challengeStatus);
    rtMediusAddInt(&getClanTeamChallengesResponse, &getClanTeamChallengesResponse.responseTime);
    rtMediusAddChars(&getClanTeamChallengesResponse,
                     getClanTeamChallengesResponse.challengeMessage,
                     sizeof(getClanTeamChallengesResponse.challengeMessage));
    rtMediusAddChars(&getClanTeamChallengesResponse,
                     getClanTeamChallengesResponse.responseMessage,
                     sizeof(getClanTeamChallengesResponse.responseMessage));
    rtMediusAddChars(&getClanTeamChallengesResponse, &getClanTeamChallengesResponse.endOfList, 1);
    rtMediusRegisterLobbyMessage(&g_nMediusGetClanTeamChallengesResponseType, rtMediusOnMessage131);

    rtDmeBeginFields();
    rtMediusAddRequestHeader(&updateClanStats, &updateClanStats.header);
    rtMediusAddInt(&updateClanStats, &updateClanStats.clanId);
    rtMediusAddCharsBySize(&updateClanStats, updateClanStats.stats, sizeof(updateClanStats.stats));
    rtMediusRegisterLobbyMessage(&g_nMediusUpdateClanStatsType, rtMediusOnMessage132);

    rtDmeBeginFields();
    rtMediusAddResponseHeader(&updateClanStatsResponse, &updateClanStatsResponse);
    rtMediusRegisterLobbyMessage(&g_nMediusUpdateClanStatsResponseType, rtMediusOnMessage133);

    rtDmeBeginFields();
    rtMediusAddRequestHeader(&versionServer, &versionServer);
    rtMediusRegisterLobbyMessage(&g_nMediusVersionServerType, rtMediusOnMessage134);

    rtDmeBeginFields();
    rtMediusAddChars(&versionServerResponse,
                     versionServerResponse.messageId,
                     sizeof(versionServerResponse.messageId));
    rtMediusAddChars(
        &versionServerResponse, versionServerResponse.version, kVersionServerTextCount);
    rtMediusRegisterLobbyMessage(&g_nMediusVersionServerResponseType,
                                 rtMediusOnVersionServerResponse);

    rtDmeBeginFields();
    rtMediusAddRequestHeader(&getWorldSecurityLevel, &getWorldSecurityLevel.header);
    rtMediusAddInt(&getWorldSecurityLevel, &getWorldSecurityLevel.id);
    rtMediusAddInt(&getWorldSecurityLevel, &getWorldSecurityLevel.value);
    rtMediusRegisterLobbyMessage(&g_nMediusGetWorldSecurityLevelType, rtMediusOnMessage136);

    rtDmeBeginFields();
    rtMediusAddResponseHeader(&getWorldSecurityLevelResponse,
                              &getWorldSecurityLevelResponse.header);
    rtMediusAddInt(&getWorldSecurityLevelResponse, &getWorldSecurityLevelResponse.worldId);
    rtMediusAddInt(&getWorldSecurityLevelResponse, &getWorldSecurityLevelResponse.applicationType);
    rtMediusAddInt(&getWorldSecurityLevelResponse, &getWorldSecurityLevelResponse.securityLevel);
    rtMediusRegisterLobbyMessage(&g_nMediusGetWorldSecurityLevelResponseType, rtMediusOnMessage137);

    rtDmeBeginFields();
    rtMediusAddRequestHeader(&banPlayer, &banPlayer.header);
    rtMediusAddInt(&banPlayer, &banPlayer.worldId);
    rtMediusAddInt(&banPlayer, &banPlayer.reserved34);
    rtMediusAddInt(&banPlayer, &banPlayer.accountId);
    rtMediusAddInt(&banPlayer, &banPlayer.duration);
    rtMediusRegisterLobbyMessage(&g_nMediusBanPlayerType, rtMediusOnMessage138);

    rtDmeBeginFields();
    rtMediusAddResponseHeader(&banPlayerResponse, &banPlayerResponse);
    rtMediusRegisterLobbyMessage(&g_nMediusBanPlayerResponseType, rtMediusOnBanPlayerResponse);

    rtDmeBeginFields();
    rtMediusAddRequestHeader(&getLocations, &getLocations);
    rtMediusRegisterLobbyMessage(&g_nMediusGetLocationsType, rtMediusOnMessage140);

    rtDmeBeginFields();
    rtMediusAddChars(&getLocationsResponse,
                     getLocationsResponse.messageId,
                     sizeof(getLocationsResponse.messageId));
    rtMediusAddInt(&getLocationsResponse, &getLocationsResponse.locationId);
    rtMediusAddCharsBySize(&getLocationsResponse,
                           getLocationsResponse.locationName,
                           sizeof(getLocationsResponse.locationName));
    rtMediusAddInt(&getLocationsResponse, &getLocationsResponse.status);
    rtMediusAddChars(&getLocationsResponse, &getLocationsResponse.endOfList, 1);
    rtMediusRegisterLobbyMessage(&g_nMediusGetLocationsResponseType, rtMediusOnMessage141);

    rtDmeBeginFields();
    rtMediusAddRequestHeader(&pickLocation, &pickLocation.header);
    rtMediusAddInt(&pickLocation, &pickLocation.id);
    rtMediusRegisterLobbyMessage(&g_nMediusPickLocationType, rtMediusOnMessage142);

    rtDmeBeginFields();
    rtMediusAddResponseHeader(&pickLocationResponse, &pickLocationResponse);
    rtMediusRegisterLobbyMessage(&g_nMediusPickLocationResponseType, rtMediusOnMessage143);

    rtDmeBeginFields();
    rtMediusAddRequestHeader(&getClanMemberList, &getClanMemberList.header);
    rtMediusAddInt(&getClanMemberList, &getClanMemberList.id);
    rtMediusRegisterLobbyMessage(&g_nMediusGetClanMemberListType, rtMediusOnMessage144);

    rtDmeBeginFields();
    rtMediusAddResponseHeader(&getClanMemberListResponse, &getClanMemberListResponse.header);
    rtMediusAddChars(&getClanMemberListResponse,
                     getClanMemberListResponse.accountName,
                     sizeof(getClanMemberListResponse.accountName));
    rtMediusAddInt(&getClanMemberListResponse, &getClanMemberListResponse.accountId);
    rtMediusAddChars(&getClanMemberListResponse, &getClanMemberListResponse.endOfList, 1);
    rtMediusRegisterLobbyMessage(&g_nMediusGetClanMemberListResponseType, rtMediusOnMessage145);

    rtDmeBeginFields();
    rtMediusAddRequestHeader(&ladderPosition, &ladderPosition.header);
    rtMediusAddInt(&ladderPosition, &ladderPosition.accountId);
    rtMediusAddInt(&ladderPosition, &ladderPosition.ladderType);
    rtMediusAddInt(&ladderPosition, &ladderPosition.statId);
    rtMediusAddInt(&ladderPosition, &ladderPosition.reserved34);
    rtMediusRegisterLobbyMessage(&g_nMediusLadderPositionType, rtMediusOnMessage146);

    rtDmeBeginFields();
    rtMediusAddChars(&ladderPositionResponse,
                     ladderPositionResponse.messageId,
                     sizeof(ladderPositionResponse.messageId));
    rtMediusAddInt(&ladderPositionResponse, &ladderPositionResponse.rank);
    rtMediusAddInt(&ladderPositionResponse, &ladderPositionResponse.status);
    rtMediusRegisterLobbyMessage(&g_nMediusLadderPositionResponseType,
                                 rtMediusOnLadderPositionResponse);

    rtDmeBeginFields();
    rtMediusAddRequestHeader(&ladderList, &ladderList.header);
    rtMediusAddInt(&ladderList, &ladderList.ladderType);
    rtMediusAddInt(&ladderList, &ladderList.statId);
    rtMediusAddInt(&ladderList, &ladderList.reserved30);
    rtMediusAddInt(&ladderList, &ladderList.startPosition);
    rtMediusAddInt(&ladderList, &ladderList.pageSize);
    rtMediusRegisterLobbyMessage(&g_nMediusLadderListType, rtMediusOnMessage148);

    rtDmeBeginFields();
    rtMediusAddChars(
        &ladderListResponse, ladderListResponse.messageId, sizeof(ladderListResponse.messageId));
    rtMediusAddInt(&ladderListResponse, &ladderListResponse.accountId);
    rtMediusAddChars(&ladderListResponse,
                     ladderListResponse.accountName,
                     sizeof(ladderListResponse.accountName));
    rtMediusAddInt(&ladderListResponse, &ladderListResponse.rank);
    rtMediusAddChars(&ladderListResponse, &ladderListResponse.endOfList, 1);
    rtMediusAddInt(&ladderListResponse, &ladderListResponse.status);
    rtMediusRegisterLobbyMessage(&g_nMediusLadderListResponseType, rtMediusOnLadderListResponse);

    rtDmeBeginFields();
    rtMediusAddRequestHeader(&chatToggle, &chatToggle.header);
    rtMediusAddInt(&chatToggle, &chatToggle.id);
    rtMediusRegisterLobbyMessage(&g_nMediusChatToggleType, rtMediusOnMessage150);

    rtDmeBeginFields();
    rtMediusAddResponseHeader(&chatToggleResponse, &chatToggleResponse);
    rtMediusRegisterLobbyMessage(&g_nMediusChatToggleResponseType, rtMediusOnMessage151);

    rtDmeBeginFields();
    rtMediusAddRequestHeader(&textFilter, &textFilter.header);
    rtMediusAddInt(&textFilter, &textFilter.filterLevel);
    rtMediusAddChars(&textFilter, textFilter.text, sizeof(textFilter.text));
    rtMediusRegisterLobbyMessage(&g_nMediusTextFilterType, rtMediusOnMessage152);

    rtDmeBeginFields();
    rtMediusAddChars(
        &textFilterResponse, textFilterResponse.messageId, sizeof(textFilterResponse.messageId));
    rtMediusAddChars(&textFilterResponse, textFilterResponse.text, sizeof(textFilterResponse.text));
    rtMediusAddInt(&textFilterResponse, &textFilterResponse.result);
    rtMediusRegisterLobbyMessage(&g_nMediusTextFilterResponseType, rtMediusOnTextFilterResponse);

    rtDmeBeginFields();
    rtMediusAddInt(&reassignWorld, &reassignWorld.oldWorldId);
    rtMediusAddInt(&reassignWorld, &reassignWorld.newWorldId);
    rtMediusRegisterLobbyMessage(&g_nMediusReassignWorldType, rtMediusOnInitHandler0C);

    rtDmeBeginFields();
    rtMediusAddRequestHeader(&getTotalGames, &getTotalGames.header);
    rtMediusAddInt(&getTotalGames, &getTotalGames.id);
    rtMediusRegisterLobbyMessage(&g_nMediusGetTotalGamesType, rtMediusOnMessage155);

    rtDmeBeginFields();
    rtMediusAddResponseHeader(&getTotalGamesResponse, &getTotalGamesResponse.header);
    rtMediusAddInt(&getTotalGamesResponse, &getTotalGamesResponse.id);
    rtMediusRegisterLobbyMessage(&g_nMediusGetTotalGamesResponseType, rtMediusOnMessage156);

    rtDmeBeginFields();
    rtMediusAddRequestHeader(&getTotalChannels, &getTotalChannels.header);
    rtMediusAddInt(&getTotalChannels, &getTotalChannels.id);
    rtMediusRegisterLobbyMessage(&g_nMediusGetTotalChannelsType, rtMediusOnMessage157);

    rtDmeBeginFields();
    rtMediusAddResponseHeader(&getTotalChannelsResponse, &getTotalChannelsResponse.header);
    rtMediusAddInt(&getTotalChannelsResponse, &getTotalChannelsResponse.id);
    rtMediusRegisterLobbyMessage(&g_nMediusGetTotalChannelsResponseType, rtMediusOnMessage158);

    rtDmeBeginFields();
    rtMediusAddRequestHeader(&getLobbyPlayerNames, &getLobbyPlayerNames.header);
    rtMediusAddInt(&getLobbyPlayerNames, &getLobbyPlayerNames.id);
    rtMediusRegisterLobbyMessage(&g_nMediusGetLobbyPlayerNamesType, rtMediusOnMessage159);

    rtDmeBeginFields();
    rtMediusAddResponseHeader(&getLobbyPlayerNamesResponse, &getLobbyPlayerNamesResponse.header);
    rtMediusAddInt(&getLobbyPlayerNamesResponse, &getLobbyPlayerNamesResponse.accountId);
    rtMediusAddChars(&getLobbyPlayerNamesResponse,
                     getLobbyPlayerNamesResponse.accountName,
                     sizeof(getLobbyPlayerNamesResponse.accountName));
    rtMediusAddChars(&getLobbyPlayerNamesResponse, &getLobbyPlayerNamesResponse.endOfList, 1);
    rtMediusRegisterLobbyMessage(&g_nMediusGetLobbyPlayerNamesResponseType, rtMediusOnMessage160);

    rtDmeBeginFields();
    rtMediusAddRequestHeader(&getTotalUsers, &getTotalUsers.header);
    rtMediusAddInt(&getTotalUsers, &getTotalUsers.id);
    rtMediusRegisterLobbyMessage(&g_nMediusGetTotalUsersType, rtMediusOnMessage161);

    rtDmeBeginFields();
    rtMediusAddResponseHeader(&getTotalUsersResponse, &getTotalUsersResponse.header);
    rtMediusAddInt(&getTotalUsersResponse, &getTotalUsersResponse.totalInSystem);
    rtMediusAddInt(&getTotalUsersResponse, &getTotalUsersResponse.totalActive);
    rtMediusRegisterLobbyMessage(&g_nMediusGetTotalUsersResponseType, rtMediusOnMessage162);

    // The binary describes the fields of the localization and file messages with empty lists.
    rtDmeBeginFields();
    rtMediusRegisterLobbyMessage(&g_nMediusSetLocalizationParamsType, rtMediusOnMessage163);
    rtDmeBeginFields();
    rtMediusRegisterLobbyMessage(&g_nMediusSetLocalizationParamsResponseType, rtMediusOnMessage164);
    rtDmeBeginFields();
    rtMediusRegisterLobbyMessage(&g_nMediusFileCreateType, rtMediusOnMessage165);
    rtDmeBeginFields();
    rtMediusRegisterLobbyMessage(&g_nMediusFileCreateResponseType, rtMediusOnFileCreateResponse);
    rtDmeBeginFields();
    rtMediusRegisterLobbyMessage(&g_nMediusFileUploadType, rtMediusOnMessage167);
    rtDmeBeginFields();
    rtMediusRegisterLobbyMessage(&g_nMediusFileUploadPartType, rtMediusOnMessage168);
    rtDmeBeginFields();
    rtMediusRegisterLobbyMessage(&g_nMediusFileUploadRequestType, rtMediusOnFileUploadRequest);
    rtDmeBeginFields();
    rtMediusRegisterLobbyMessage(&g_nMediusFileCloseType, rtMediusOnMessage170);
    rtDmeBeginFields();
    rtMediusRegisterLobbyMessage(&g_nMediusFileCloseResponseType, rtMediusOnFileCloseResponse);
    rtDmeBeginFields();
    rtMediusRegisterLobbyMessage(&g_nMediusFileDownloadType, rtMediusOnMessage172);
    rtDmeBeginFields();
    rtMediusRegisterLobbyMessage(&g_nMediusFileDownloadResponseType,
                                 rtMediusOnFileDownloadResponse);
    rtDmeBeginFields();
    rtMediusRegisterLobbyMessage(&g_nMediusFileDownloadStreamType, rtMediusOnMessage174);
    rtDmeBeginFields();
    rtMediusRegisterLobbyMessage(&g_nMediusFileDownloadStreamResponseType, rtMediusOnMessage175);
    rtDmeBeginFields();
    rtMediusRegisterLobbyMessage(&g_nMediusFileDeleteType, rtMediusOnMessage176);
    rtDmeBeginFields();
    rtMediusRegisterLobbyMessage(&g_nMediusFileDeleteResponseType, rtMediusOnFileDeleteResponse);
    rtDmeBeginFields();
    rtMediusRegisterLobbyMessage(&g_nMediusFileListType, rtMediusOnMessage178);
    rtDmeBeginFields();
    rtMediusRegisterLobbyMessage(&g_nMediusFileListResponseType, rtMediusOnFileListResponse);
    rtDmeBeginFields();
    rtMediusRegisterLobbyMessage(&g_nMediusFileUpdateAttributesType, rtMediusOnMessage180);
    rtDmeBeginFields();
    rtMediusRegisterLobbyMessage(&g_nMediusFileUpdateAttributesResponseType, rtMediusOnMessage181);
    rtDmeBeginFields();
    rtMediusRegisterLobbyMessage(&g_nMediusFileGetAttributesType, rtMediusOnMessage182);
    rtDmeBeginFields();
    rtMediusRegisterLobbyMessage(&g_nMediusFileGetAttributesResponseType,
                                 rtMediusOnFileGetAttributesResponse);
    rtDmeBeginFields();
    rtMediusRegisterLobbyMessage(&g_nMediusFileUpdateMetaDataType, rtMediusOnMessage184);
    rtDmeBeginFields();
    rtMediusRegisterLobbyMessage(&g_nMediusFileUpdateMetaDataResponseType, rtMediusOnMessage185);
    rtDmeBeginFields();
    rtMediusRegisterLobbyMessage(&g_nMediusFileGetMetaDataType, rtMediusOnMessage186);
    rtDmeBeginFields();
    rtMediusRegisterLobbyMessage(&g_nMediusFileGetMetaDataResponseType, rtMediusOnMessage187);
    rtDmeBeginFields();
    rtMediusRegisterLobbyMessage(&g_nMediusFileSearchByMetaDataType, rtMediusOnMessage188);
    rtDmeBeginFields();
    rtMediusRegisterLobbyMessage(&g_nMediusFileSearchByMetaDataResponseType, rtMediusOnMessage189);
    rtDmeBeginFields();
    rtMediusRegisterLobbyMessage(&g_nMediusFileCancelOperationType, rtMediusOnMessage190);
    rtDmeBeginFields();
    rtMediusRegisterLobbyMessage(&g_nMediusFileCancelOperationResponseType, rtMediusOnMessage191);

    rtDmeBeginFields();
    rtMediusAddRequestHeader(&getIgnoreList, &getIgnoreList);
    rtMediusRegisterLobbyMessage(&g_nMediusGetIgnoreListType, rtMediusOnMessage192);

    rtDmeBeginFields();
    rtMediusAddResponseHeader(&getIgnoreListResponse, &getIgnoreListResponse.header);
    rtMediusAddInt(&getIgnoreListResponse, &getIgnoreListResponse.accountId);
    rtMediusAddChars(&getIgnoreListResponse,
                     getIgnoreListResponse.accountName,
                     sizeof(getIgnoreListResponse.accountName));
    rtMediusAddInt(&getIgnoreListResponse, &getIgnoreListResponse.onlineState);
    rtMediusAddChars(&getIgnoreListResponse, &getIgnoreListResponse.endOfList, 1);
    rtMediusRegisterLobbyMessage(&g_nMediusGetIgnoreListResponseType, rtMediusOnMessage193);

    rtDmeBeginFields();
    rtMediusAddRequestHeader(&addToIgnoreList, &addToIgnoreList.header);
    rtMediusAddInt(&addToIgnoreList, &addToIgnoreList.id);
    rtMediusRegisterLobbyMessage(&g_nMediusAddToIgnoreListType, rtMediusOnMessage194);

    rtDmeBeginFields();
    rtMediusAddResponseHeader(&addToIgnoreListResponse, &addToIgnoreListResponse);
    rtMediusRegisterLobbyMessage(&g_nMediusAddToIgnoreListResponseType, rtMediusOnMessage195);

    rtDmeBeginFields();
    rtMediusAddRequestHeader(&removeFromIgnoreList, &removeFromIgnoreList.header);
    rtMediusAddInt(&removeFromIgnoreList, &removeFromIgnoreList.id);
    rtMediusRegisterLobbyMessage(&g_nMediusRemoveFromIgnoreListType, rtMediusOnMessage196);

    rtDmeBeginFields();
    rtMediusAddResponseHeader(&removeFromIgnoreListResponse, &removeFromIgnoreListResponse);
    rtMediusRegisterLobbyMessage(&g_nMediusRemoveFromIgnoreListResponseType, rtMediusOnMessage197);

    rtDmeBeginFields();
    rtMediusAddRequestHeader(&setMessageAsRead, &setMessageAsRead.header);
    rtMediusAddInt(&setMessageAsRead, &setMessageAsRead.id);
    rtMediusAddInt(&setMessageAsRead, &setMessageAsRead.value);
    rtMediusRegisterLobbyMessage(&g_nMediusSetMessageAsReadType, rtMediusOnMessage198);

    rtDmeBeginFields();
    rtMediusAddResponseHeader(&statusResponse, &statusResponse);
    rtMediusRegisterLobbyMessage(&g_nMediusSetMessageAsReadResponseType, rtMediusOnMessage199);

    rtDmeBeginFields();
    rtMediusAddChars(
        &universeInformation, universeInformation.messageId, sizeof(universeInformation.messageId));
    rtMediusAddInt(&universeInformation, &universeInformation.infoType);
    rtMediusAddInt(&universeInformation, &universeInformation.characterEncoding);
    rtMediusAddInt(&universeInformation, &universeInformation.language);
    rtMediusRegisterLobbyMessage(&g_nMediusGetUniverseInformationType, rtMediusOnMessage200);

    rtDmeBeginFields();
    rtMediusAddResponseHeader(&universeNewsResponse, &universeNewsResponse.header);
    rtMediusAddCharsBySize(
        &universeNewsResponse, universeNewsResponse.news, sizeof(universeNewsResponse.news));
    rtMediusAddChars(&universeNewsResponse, &universeNewsResponse.endOfList, 1);
    rtMediusRegisterLobbyMessage(&g_nMediusUniverseNewsResponseType, rtMediusOnMessage201);

    rtDmeBeginFields();
    rtMediusAddResponseHeader(&universeStatusResponse, &universeStatusResponse.header);
    rtMediusAddChars(&universeStatusResponse,
                     universeStatusResponse.universeName,
                     sizeof(universeStatusResponse.universeName));
    rtMediusAddChars(
        &universeStatusResponse, universeStatusResponse.dns, sizeof(universeStatusResponse.dns));
    rtMediusAddInt(&universeStatusResponse, &universeStatusResponse.port);
    rtMediusAddChars(&universeStatusResponse,
                     universeStatusResponse.description,
                     sizeof(universeStatusResponse.description));
    rtMediusAddInt(&universeStatusResponse, &universeStatusResponse.status);
    rtMediusAddInt(&universeStatusResponse, &universeStatusResponse.userCount);
    rtMediusAddInt(&universeStatusResponse, &universeStatusResponse.maxUsers);
    rtMediusAddChars(&universeStatusResponse, &universeStatusResponse.endOfList, 1);
    rtMediusRegisterLobbyMessage(&g_nMediusUniverseStatusListResponseType, rtMediusOnMessage202);

    rtDmeBeginFields();
    rtMediusAddRequestHeader(&machineSignature, &machineSignature.header);
    rtMediusAddChars(
        &machineSignature, machineSignature.signature, sizeof(machineSignature.signature));
    rtMediusRegisterLobbyMessage(&g_nMediusMachineSignaturePostType, rtMediusOnMessage203);

    rtDmeBeginFields();
    rtMediusAddRequestHeader(&ladderPositionFast, &ladderPositionFast.header);
    rtMediusAddInt(&ladderPositionFast, &ladderPositionFast.accountId);
    rtMediusAddInt(&ladderPositionFast, &ladderPositionFast.statValue);
    rtMediusRegisterLobbyMessage(&g_nMediusLadderPositionFastType, rtMediusOnMessage204);

    rtDmeBeginFields();
    rtMediusAddChars(&ladderPositionFastResponse,
                     ladderPositionFastResponse.messageId,
                     sizeof(ladderPositionFastResponse.messageId));
    rtMediusAddInt(&ladderPositionFastResponse, &ladderPositionFastResponse.rank);
    rtMediusAddInt(&ladderPositionFastResponse, &ladderPositionFastResponse.status);
    rtMediusRegisterLobbyMessage(&g_nMediusLadderPositionFastResponseType,
                                 rtMediusOnLadderPositionFastResponse);
    return 0;
}
