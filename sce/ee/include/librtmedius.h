#ifndef LIBRTMEDIUS_H
#define LIBRTMEDIUS_H

#include <librtdme.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Lobby client of the SCE-RT network library, through which the game logs in to the lobby server,
 * lists and joins games, and reports them. Every entry point reports 0 for success or a negative
 * error code. A request names itself with a message identifier, and the response its callback
 * receives repeats the identifier. Each callback takes the connection, the kind of the response,
 * the size of the response, and the response. The names of the entry points, the records, and
 * their fields are inferred.
 */

/** The size of a message identifier, the terminator included. */
#define RT_MEDIUS_MESSAGE_ID_SIZE 21

/** The size of a session key, the terminator included. */
#define RT_MEDIUS_SESSION_KEY_SIZE 17

/** The size of the name of a game or a channel, the terminator included. */
#define RT_MEDIUS_GAME_NAME_SIZE 64

/** The size of a text the server filters, the terminator included. */
#define RT_MEDIUS_TEXT_SIZE 64

/** The size of the statistics text of a game or a player, the terminator included. */
#define RT_MEDIUS_STATS_SIZE 256

/** The size of a password, the terminator included. */
#define RT_MEDIUS_PASSWORD_SIZE 32

/** The size of the name of a winning team, the terminator included. */
#define RT_MEDIUS_TEAM_NAME_SIZE 64

/** The size of the name of a player or an account, the terminator included. */
#define RT_MEDIUS_PLAYER_NAME_SIZE 32

/** The size of the version text of the server, the terminator included. */
#define RT_MEDIUS_VERSION_SIZE 64

/** The size of an announcement, the terminator included. */
#define RT_MEDIUS_ANNOUNCEMENT_SIZE 1000

/** The size of a part of the policy text, the terminator included. */
#define RT_MEDIUS_POLICY_SIZE 256

/** The size of the name of a file, the terminator included. */
#define RT_MEDIUS_FILE_NAME_SIZE 128

/** The size of the bytes of one transfer of a file. */
#define RT_MEDIUS_FILE_DATA_SIZE 464

/** The size of the name of the sender of a chat line. */
#define RT_MEDIUS_CHAT_NAME_SIZE 22

/** The size of the text of a chat line, the terminator included. */
#define RT_MEDIUS_CHAT_TEXT_SIZE 64

/** The status of a response that reports success. */
#define RT_MEDIUS_STATUS_SUCCESS 0

/** The transfer status of the last part of a file. */
#define RT_MEDIUS_TRANSFER_DONE 3

/** The leading part of a request to the lobby server. */
typedef struct {
    char messageId[RT_MEDIUS_MESSAGE_ID_SIZE];   /*!< The identifier. */
    char sessionKey[RT_MEDIUS_SESSION_KEY_SIZE]; /*!< The key of the lobby session. */
} rtMediusRequest;

/** The leading part of a response of the lobby server. */
typedef struct {
    char messageId[RT_MEDIUS_MESSAGE_ID_SIZE]; /*!< The identifier of the request. */
    int status;                                /*!< RT_MEDIUS_STATUS_SUCCESS or an error. */
} rtMediusResponse;

/** Receiver of a response that reports only a status. */
typedef void (*rtMediusCallback)(int connection,
                                 int type,
                                 int size,
                                 const rtMediusResponse *response);

/** A line of chat the lobby server delivers. */
typedef struct {
    char messageType;                          /*!< The kind of line. */
    char channel;                              /*!< The channel, offset from 'a'. */
    char senderName[RT_MEDIUS_CHAT_NAME_SIZE]; /*!< The name of the sender. */
    int senderAccountId;                       /*!< The account of the sender. */
    char reserved1C[0x24];                     /* +0x1c, not read by the game. */
    char text[RT_MEDIUS_CHAT_TEXT_SIZE];       /*!< The text. */
} rtMediusChatMessage;

/** Receiver of the lines of chat. */
typedef void (*rtMediusChatHandler)(int connection,
                                    int type,
                                    int size,
                                    const rtMediusChatMessage *message);

/** The parameters of rtMediusInitialize(). 0x14 bytes. */
typedef struct {
    rtMediusChatHandler onChat;    /*!< Receives the lines of chat. */
    rtMediusCallback onMessage069; /*!< Receives message 69 of the server, or null. */
    rtMediusCallback onNotice;     /*!< Receives the notices of the server. */
    rtMediusCallback onMessage154; /*!< Receives message 154 of the server, or null. */
    int timeoutMs;                 /*!< The interval of the keep-alive, in milliseconds. */
} rtMediusInitParams;

/**
 * Receiver of the result of rtMediusConnect().
 *
 * @param connection The connection, or -1 when the connection failed.
 */
typedef void (*rtMediusConnectCallback)(int connection);

/** The parameters of rtMediusConnect(). 0x100 bytes. */
typedef struct {
    rtNetConnectionInfo info;          /*!< The lobby server to connect to. */
    rtDmeVoiceSettings voice;          /*!< The voice settings of the connection. */
    int maxPlayers;                    /*!< The players of the connection. */
    rtMediusConnectCallback onConnect; /*!< Receives the result, or null to connect at once. */
    rtDmeClientCallback onClientJoin;  /*!< Receives a peer that joined, or null. */
    rtDmeClientCallback onClientLeave; /*!< Receives a peer that left, or null. */
} rtMediusConnectParams;

/** The status rtMediusConnect() fills. 8 bytes. */
typedef struct {
    int connection; /*!< The connection, or -1 when the connection failed. */
    int result;     /*!< 0, or an error code. */
} rtMediusConnectStatus;

/** The request of rtMediusSessionBegin(). 0x1c bytes. */
typedef struct {
    char messageId[RT_MEDIUS_MESSAGE_ID_SIZE]; /*!< The identifier. */
    int connectionClass;                       /*!< The kind of network connection. */
} rtMediusSessionBeginRequest;

/** The response of rtMediusSessionBegin(). */
typedef struct {
    rtMediusResponse header;                     /*!< The identifier and the status. */
    char sessionKey[RT_MEDIUS_SESSION_KEY_SIZE]; /*!< The key of the lobby session. */
} rtMediusSessionBeginResponse;

/** Receiver of the response of rtMediusSessionBegin(). */
typedef void (*rtMediusSessionBeginCallback)(int connection,
                                             int type,
                                             int size,
                                             const rtMediusSessionBeginResponse *response);

/** The response of rtMediusVersionServer(). */
typedef struct {
    char messageId[RT_MEDIUS_MESSAGE_ID_SIZE]; /*!< The identifier of the request. */
    char version[RT_MEDIUS_VERSION_SIZE];      /*!< The version of the server. */
} rtMediusVersionResponse;

/** Receiver of the response of rtMediusVersionServer(). */
typedef void (*rtMediusVersionCallback)(int connection,
                                        int type,
                                        int size,
                                        const rtMediusVersionResponse *response);

/** The request of rtMediusTextFilter(). 0x6c bytes. */
typedef struct {
    rtMediusRequest header;         /*!< The identifier and the session. */
    int filterLevel;                /*!< The strictness of the filter. */
    char text[RT_MEDIUS_TEXT_SIZE]; /*!< The text. */
} rtMediusTextFilterRequest;

/** The response of rtMediusTextFilter(). */
typedef struct {
    char messageId[RT_MEDIUS_MESSAGE_ID_SIZE]; /*!< The identifier of the request. */
    char text[RT_MEDIUS_TEXT_SIZE];            /*!< The filtered text. */
    int result;                                /*!< Not negative for success. */
} rtMediusTextFilterResponse;

/** Receiver of the response of rtMediusTextFilter(). */
typedef void (*rtMediusTextFilterCallback)(int connection,
                                           int type,
                                           int size,
                                           const rtMediusTextFilterResponse *response);

/** The request of rtMediusAccountRegistration(). 0x6c bytes. */
typedef struct {
    rtMediusRequest header;                       /*!< The identifier and the session. */
    int accountType;                              /*!< The kind of account. */
    char accountName[RT_MEDIUS_PLAYER_NAME_SIZE]; /*!< The name of the account. */
    char password[RT_MEDIUS_PASSWORD_SIZE];       /*!< The password. */
} rtMediusAccountRegistrationRequest;

/** The request of rtMediusAccountDelete(). 0x46 bytes. */
typedef struct {
    rtMediusRequest header;                 /*!< The identifier and the session. */
    char password[RT_MEDIUS_PASSWORD_SIZE]; /*!< The password. */
} rtMediusAccountDeleteRequest;

/** The request of rtMediusAnonymousLogin(). 0x146 bytes. */
typedef struct {
    rtMediusRequest header;                       /*!< The identifier and the session. */
    char accountName[RT_MEDIUS_PLAYER_NAME_SIZE]; /*!< The name the player shows. */
    char stats[RT_MEDIUS_STATS_SIZE];             /*!< The statistics text of the player. */
} rtMediusAnonymousLoginRequest;

/** The request of rtMediusAccountLogin(). 0x66 bytes. */
typedef struct {
    rtMediusRequest header;                       /*!< The identifier and the session. */
    char accountName[RT_MEDIUS_PLAYER_NAME_SIZE]; /*!< The name of the account. */
    char password[RT_MEDIUS_PASSWORD_SIZE];       /*!< The password. */
} rtMediusAccountLoginRequest;

/** The response of rtMediusAccountLogin() and rtMediusAnonymousLogin(). */
typedef struct {
    rtMediusResponse header;  /*!< The identifier and the status. */
    int accountId;            /*!< The account. */
    int accountType;          /*!< The kind of account. */
    int worldId;              /*!< The chat channel the player starts in. */
    rtNetConnectionInfo info; /*!< The connection to the lobby server. */
} rtMediusLoginResponse;

/** Receiver of the response of a login. */
typedef void (*rtMediusLoginCallback)(int connection,
                                      int type,
                                      int size,
                                      const rtMediusLoginResponse *response);

/** The request of rtMediusUpdateStats(). 0x126 bytes. */
typedef struct {
    rtMediusRequest header;           /*!< The identifier and the session. */
    char stats[RT_MEDIUS_STATS_SIZE]; /*!< The statistics text of the player. */
} rtMediusUpdateStatsRequest;

/** The request of rtMediusChangePassword(). 0x66 bytes. */
typedef struct {
    rtMediusRequest header;                    /*!< The identifier and the session. */
    char oldPassword[RT_MEDIUS_PASSWORD_SIZE]; /*!< The password. */
    char newPassword[RT_MEDIUS_PASSWORD_SIZE]; /*!< The new password. */
} rtMediusChangePasswordRequest;

/** The request of rtMediusAccountGetId(). 0x46 bytes. */
typedef struct {
    rtMediusRequest header;                       /*!< The identifier and the session. */
    char accountName[RT_MEDIUS_PLAYER_NAME_SIZE]; /*!< The name of the account. */
} rtMediusAccountGetIdRequest;

/** The response of rtMediusAccountGetId(). */
typedef struct {
    char messageId[RT_MEDIUS_MESSAGE_ID_SIZE]; /*!< The identifier of the request. */
    int accountId;                             /*!< The account. */
    int status;                                /*!< RT_MEDIUS_STATUS_SUCCESS or an error. */
} rtMediusAccountGetIdResponse;

/** Receiver of the response of rtMediusAccountGetId(). */
typedef void (*rtMediusAccountGetIdCallback)(int connection,
                                             int type,
                                             int size,
                                             const rtMediusAccountGetIdResponse *response);

/** A request about one account or one world. 0x2c bytes. */
typedef struct {
    rtMediusRequest header; /*!< The identifier and the session. */
    int id;                 /*!< The account, the world, or the application. */
} rtMediusIdRequest;

/** The response of rtMediusPlayerInfo(). */
typedef struct {
    rtMediusResponse header;                      /*!< The identifier and the status. */
    char accountName[RT_MEDIUS_PLAYER_NAME_SIZE]; /*!< The name of the account. */
    int reserved3C;                               /* +0x3c, not read by the game. */
    int reserved40;                               /* +0x40, not read by the game. */
    int connectionClass;                          /*!< The kind of network connection. */
    char stats[RT_MEDIUS_STATS_SIZE];             /*!< The statistics text of the player. */
} rtMediusPlayerInfoResponse;

/** Receiver of the response of rtMediusPlayerInfo(). */
typedef void (*rtMediusPlayerInfoCallback)(int connection,
                                           int type,
                                           int size,
                                           const rtMediusPlayerInfoResponse *response);

/** The response of rtMediusChannelInfo(). */
typedef struct {
    rtMediusResponse header;             /*!< The identifier and the status. */
    char name[RT_MEDIUS_GAME_NAME_SIZE]; /*!< The name of the channel. */
    int playerCount;                     /*!< The players in the channel. */
    int maxPlayers;                      /*!< The players the channel takes. */
} rtMediusChannelInfoResponse;

/** Receiver of the response of rtMediusChannelInfo(). */
typedef void (*rtMediusChannelInfoCallback)(int connection,
                                            int type,
                                            int size,
                                            const rtMediusChannelInfoResponse *response);

/** The response of rtMediusGetAnnouncements(). */
typedef struct {
    rtMediusResponse header;                        /*!< The identifier and the status. */
    int announcementId;                             /*!< The announcement. */
    char announcement[RT_MEDIUS_ANNOUNCEMENT_SIZE]; /*!< The text. */
} rtMediusAnnouncementResponse;

/** Receiver of the response of rtMediusGetAnnouncements(). The game edits the text in place. */
typedef void (*rtMediusAnnouncementCallback)(int connection,
                                             int type,
                                             int size,
                                             rtMediusAnnouncementResponse *response);

/** The response of rtMediusGetPolicy(), one part of the text. */
typedef struct {
    rtMediusResponse header;            /*!< The identifier and the status. */
    char policy[RT_MEDIUS_POLICY_SIZE]; /*!< The part of the text. */
    char endOfText;                     /*!< Non-zero for the last part. */
} rtMediusPolicyResponse;

/** Receiver of the response of rtMediusGetPolicy(). */
typedef void (*rtMediusPolicyCallback)(int connection,
                                       int type,
                                       int size,
                                       const rtMediusPolicyResponse *response);

/** The request of rtMediusChannelList(). 0x2a bytes. */
typedef struct {
    rtMediusRequest header; /*!< The identifier and the session. */
    short pageId;           /*!< The page, from 1. */
    short pageSize;         /*!< The channels of a page. */
} rtMediusChannelListRequest;

/** The response of rtMediusChannelList(), one channel. */
typedef struct {
    rtMediusResponse header;             /*!< The identifier and the status. */
    int worldId;                         /*!< The channel. */
    char name[RT_MEDIUS_GAME_NAME_SIZE]; /*!< The name of the channel. */
    int playerCount;                     /*!< The players in the channel. */
    char endOfList;                      /*!< Non-zero for the last channel. */
} rtMediusChannelListResponse;

/** Receiver of the response of rtMediusChannelList(). */
typedef void (*rtMediusChannelListCallback)(int connection,
                                            int type,
                                            int size,
                                            const rtMediusChannelListResponse *response);

/** The request of rtMediusGameList(). 0x30 bytes. */
typedef struct {
    rtMediusRequest header; /*!< The identifier and the session. */
    int applicationId;      /*!< The identifier of the game. */
    short pageId;           /*!< The page, from 1. */
    short pageSize;         /*!< The games of a page. */
} rtMediusGameListRequest;

/** The response of rtMediusGameList(), one game. */
typedef struct {
    rtMediusResponse header;                 /*!< The identifier and the status. */
    int worldId;                             /*!< The game. */
    char gameName[RT_MEDIUS_GAME_NAME_SIZE]; /*!< The name, which encodes the settings. */
    int worldStatus;                         /*!< The state of the game. */
    int reserved64;                          /* +0x64, not read by the game. */
    int reserved68;                          /* +0x68, not read by the game. */
    char endOfList;                          /*!< Non-zero for the last game. */
} rtMediusGameListResponse;

/** Receiver of the response of rtMediusGameList(). */
typedef void (*rtMediusGameListCallback)(int connection,
                                         int type,
                                         int size,
                                         const rtMediusGameListResponse *response);

/** The response of rtMediusChannelPlayers(), one player. */
typedef struct {
    rtMediusResponse header;                      /*!< The identifier and the status. */
    int reserved1C;                               /* +0x1c, not read by the game. */
    int accountId;                                /*!< The account. */
    char accountName[RT_MEDIUS_PLAYER_NAME_SIZE]; /*!< The name of the account. */
    char stats[RT_MEDIUS_STATS_SIZE];             /*!< The statistics text of the player. */
    int connectionClass;                          /*!< The kind of network connection. */
    char endOfList;                               /*!< Non-zero for the last player. */
} rtMediusChannelPlayersResponse;

/** Receiver of the response of rtMediusChannelPlayers(). */
typedef void (*rtMediusChannelPlayersCallback)(int connection,
                                               int type,
                                               int size,
                                               const rtMediusChannelPlayersResponse *response);

/** The response of rtMediusGamePlayers(), one player. */
typedef struct {
    rtMediusResponse header;                      /*!< The identifier and the status. */
    int accountId;                                /*!< The account. */
    char accountName[RT_MEDIUS_PLAYER_NAME_SIZE]; /*!< The name of the account. */
    char stats[RT_MEDIUS_STATS_SIZE];             /*!< The statistics text of the player. */
    int connectionClass;                          /*!< The kind of network connection. */
    char endOfList;                               /*!< Non-zero for the last player. */
} rtMediusGamePlayersResponse;

/** Receiver of the response of rtMediusGamePlayers(). */
typedef void (*rtMediusGamePlayersCallback)(int connection,
                                            int type,
                                            int size,
                                            const rtMediusGamePlayersResponse *response);

/** The request of rtMediusLadderList(). 0x3c bytes. */
typedef struct {
    rtMediusRequest header; /*!< The identifier and the session. */
    int ladderType;         /*!< The kind of ladder. */
    int statId;             /*!< The statistic the ladder ranks. */
    int reserved30;         /* +0x30, the game leaves it zero. */
    int startPosition;      /*!< The first position, from 1. */
    int pageSize;           /*!< The positions of a page. */
} rtMediusLadderListRequest;

/** The response of rtMediusLadderList(), one position. */
typedef struct {
    char messageId[RT_MEDIUS_MESSAGE_ID_SIZE];    /*!< The identifier of the request. */
    int accountId;                                /*!< The account. */
    char accountName[RT_MEDIUS_PLAYER_NAME_SIZE]; /*!< The name of the account. */
    int rank;                                     /*!< The position. */
    int status;                                   /*!< RT_MEDIUS_STATUS_SUCCESS or an error. */
    char endOfList;                               /*!< Non-zero for the last position. */
} rtMediusLadderListResponse;

/** Receiver of the response of rtMediusLadderList(). */
typedef void (*rtMediusLadderListCallback)(int connection,
                                           int type,
                                           int size,
                                           const rtMediusLadderListResponse *response);

/** The request of rtMediusLadderPosition(). 0x38 bytes. */
typedef struct {
    rtMediusRequest header; /*!< The identifier and the session. */
    int accountId;          /*!< The account. */
    int ladderType;         /*!< The kind of ladder. */
    int statId;             /*!< The statistic the ladder ranks. */
    int reserved34;         /* +0x34, the game leaves it zero. */
} rtMediusLadderPositionRequest;

/** The request of rtMediusLadderPositionFast(). 0x30 bytes. */
typedef struct {
    rtMediusRequest header; /*!< The identifier and the session. */
    int accountId;          /*!< The account. */
    int statValue;          /*!< The value of the statistic of the account. */
} rtMediusLadderPositionFastRequest;

/** The response of rtMediusLadderPosition() and rtMediusLadderPositionFast(). */
typedef struct {
    char messageId[RT_MEDIUS_MESSAGE_ID_SIZE]; /*!< The identifier of the request. */
    int rank;                                  /*!< The position. */
    int status;                                /*!< RT_MEDIUS_STATUS_SUCCESS or an error. */
} rtMediusLadderPositionResponse;

/** Receiver of the response of a ladder position. */
typedef void (*rtMediusLadderPositionCallback)(int connection,
                                               int type,
                                               int size,
                                               const rtMediusLadderPositionResponse *response);

/** The request of rtMediusFindPlayer(). 0x50 bytes. */
typedef struct {
    rtMediusRequest header;                       /*!< The identifier and the session. */
    int searchType;                               /*!< Zero to search by account. */
    int accountId;                                /*!< The account. */
    char accountName[RT_MEDIUS_PLAYER_NAME_SIZE]; /*!< The name of the account. */
} rtMediusFindPlayerRequest;

/** The response of rtMediusFindPlayer(), one location of the player. */
typedef struct {
    rtMediusResponse header; /*!< The identifier and the status. */
    char reserved1C[0x24];   /* +0x1c, not read by the game. */
    int locationType;        /*!< 1 when the player is in a chat channel. */
    int worldId;             /*!< The chat channel of the player. */
    int online;              /*!< Non-zero while the player is online. */
    char reserved4C[0x20];   /* +0x4c, not read by the game. */
    char endOfList;          /*!< Non-zero for the last location. */
} rtMediusFindPlayerResponse;

/** Receiver of the response of rtMediusFindPlayer(). */
typedef void (*rtMediusFindPlayerCallback)(int connection,
                                           int type,
                                           int size,
                                           const rtMediusFindPlayerResponse *response);

/** The request of rtMediusJoinChannel(). 0x4c bytes. */
typedef struct {
    rtMediusRequest header;                 /*!< The identifier and the session. */
    int worldId;                            /*!< The channel. */
    char password[RT_MEDIUS_PASSWORD_SIZE]; /*!< The password, or empty. */
} rtMediusJoinChannelRequest;

/** The response of rtMediusJoinChannel(). */
typedef struct {
    rtMediusResponse header;  /*!< The identifier and the status. */
    rtNetConnectionInfo info; /*!< The connection to the channel. */
} rtMediusJoinChannelResponse;

/** Receiver of the response of rtMediusJoinChannel(). */
typedef void (*rtMediusJoinChannelCallback)(int connection,
                                            int type,
                                            int size,
                                            const rtMediusJoinChannelResponse *response);

/** The request of rtMediusCreateChannel(). 0x90 bytes. */
typedef struct {
    rtMediusRequest header;                 /*!< The identifier and the session. */
    int applicationId;                      /*!< The identifier of the game. */
    int maxPlayers;                         /*!< The players the channel takes. */
    char name[RT_MEDIUS_GAME_NAME_SIZE];    /*!< The name of the channel. */
    char password[RT_MEDIUS_PASSWORD_SIZE]; /*!< The password, or empty. */
} rtMediusCreateChannelRequest;

/** The response of rtMediusCreateChannel(). */
typedef struct {
    rtMediusResponse header; /*!< The identifier and the status. */
    int worldId;             /*!< The new channel. */
} rtMediusCreateChannelResponse;

/** Receiver of the response of rtMediusCreateChannel(). */
typedef void (*rtMediusCreateChannelCallback)(int connection,
                                              int type,
                                              int size,
                                              const rtMediusCreateChannelResponse *response);

/** The request of rtMediusSetGameListFilter(). 0x34 bytes. */
typedef struct {
    rtMediusRequest header; /*!< The identifier and the session. */
    int filterField;        /*!< The field of a game the filter compares. */
    int comparison;         /*!< The comparison. */
    int value;              /*!< The value the field is compared with. */
} rtMediusGameListFilterRequest;

/** The description of a file of the server. 0xac bytes. */
typedef struct {
    char fileName[RT_MEDIUS_FILE_NAME_SIZE]; /*!< The name. */
    unsigned char serverChecksum[16];        /*!< The checksum the server keeps. */
    int fileId;                              /*!< The identifier. */
    int fileSize;                            /*!< The size in bytes. */
    int creationTime;                        /*!< When the file was created. */
    int ownerId;                             /*!< The owning account. */
    int groupId;                             /*!< The owning group. */
    short ownerPermissions;                  /*!< The access of the owner. */
    short groupPermissions;                  /*!< The access of the group. */
    short globalPermissions;                 /*!< The access of every other account. */
    short serverOperationId;                 /*!< The operation of the server. */
} rtMediusFile;

/** A request about one file. 0xc4 bytes. */
typedef struct {
    rtMediusFile file;                         /*!< The file. */
    char messageId[RT_MEDIUS_MESSAGE_ID_SIZE]; /*!< The identifier. */
} rtMediusFileRequest;

/** The response of a request about one file. */
typedef struct {
    rtMediusFile file;                         /*!< The file. */
    int status;                                /*!< RT_MEDIUS_STATUS_SUCCESS or an error. */
    char messageId[RT_MEDIUS_MESSAGE_ID_SIZE]; /*!< The identifier of the request. */
    char endOfList;                            /*!< Non-zero for the last file of a list. */
} rtMediusFileResponse;

/** Receiver of the response of a request about one file. */
typedef void (*rtMediusFileCallback)(int connection,
                                     int type,
                                     int size,
                                     const rtMediusFileResponse *response);

/** The request of rtMediusFileCreate(). 0x1d8 bytes. */
typedef struct {
    rtMediusFile file;                         /*!< The file. */
    char attributes[0x114];                    /*!< The attributes, left empty. */
    char messageId[RT_MEDIUS_MESSAGE_ID_SIZE]; /*!< The identifier. */
} rtMediusFileCreateRequest;

/** The response of rtMediusFileGetAttributes(). */
typedef struct {
    rtMediusFile file;                         /*!< The file. */
    char attributes[0x114];                    /*!< The attributes. */
    int status;                                /*!< RT_MEDIUS_STATUS_SUCCESS or an error. */
    char messageId[RT_MEDIUS_MESSAGE_ID_SIZE]; /*!< The identifier of the request. */
} rtMediusFileAttributesResponse;

/** Receiver of the response of rtMediusFileGetAttributes(). */
typedef void (*rtMediusFileAttributesCallback)(int connection,
                                               int type,
                                               int size,
                                               const rtMediusFileAttributesResponse *response);

/** The request of rtMediusFileUpload(). 0xcc bytes. */
typedef struct {
    rtMediusFile file;                         /*!< The file. */
    const char *buffer;                        /*!< The bytes of the file. */
    int bufferSize;                            /*!< The number of bytes. */
    char messageId[RT_MEDIUS_MESSAGE_ID_SIZE]; /*!< The identifier. */
} rtMediusFileUploadRequest;

/** The response of one part of rtMediusFileUpload() and rtMediusFileDownload(). */
typedef struct {
    char data[RT_MEDIUS_FILE_DATA_SIZE];       /*!< The bytes of the part. */
    int startByteIndex;                        /*!< The offset of the part. */
    int dataSize;                              /*!< The number of bytes of the part. */
    int packetNumber;                          /*!< The number of the part. */
    int transferStatus;                        /*!< RT_MEDIUS_TRANSFER_DONE for the last part. */
    int status;                                /*!< RT_MEDIUS_STATUS_SUCCESS or an error. */
    char messageId[RT_MEDIUS_MESSAGE_ID_SIZE]; /*!< The identifier of the request. */
} rtMediusFileTransferResponse;

/** Receiver of the parts of a transfer. */
typedef void (*rtMediusFileTransferCallback)(int connection,
                                             int type,
                                             int size,
                                             const rtMediusFileTransferResponse *response);

/** The request of rtMediusFileList(). 0xb0 bytes. */
typedef struct {
    char fileNameBeginsWith[RT_MEDIUS_FILE_NAME_SIZE]; /*!< The prefix of the names. */
    int fileSizeGreaterThan;                           /*!< The smallest size, exclusive. */
    int fileSizeLessThan;                              /*!< The largest size, exclusive. */
    int ownerById;                                     /*!< The owner, or zero. */
    int newerThanTimestamp;                            /*!< The oldest creation time. */
    int startingEntry;                                 /*!< The first file. */
    int pageSize;                                      /*!< The files of a page. */
    char messageId[RT_MEDIUS_MESSAGE_ID_SIZE];         /*!< The identifier. */
} rtMediusFileListRequest;

/** The response of rtMediusFileDelete(). */
typedef struct {
    int status;                                /*!< RT_MEDIUS_STATUS_SUCCESS or an error. */
    char messageId[RT_MEDIUS_MESSAGE_ID_SIZE]; /*!< The identifier of the request. */
} rtMediusFileDeleteResponse;

/** Receiver of the response of rtMediusFileDelete(). */
typedef void (*rtMediusFileDeleteCallback)(int connection,
                                           int type,
                                           int size,
                                           const rtMediusFileDeleteResponse *response);

/** The request of rtMediusJoinGame(). 0x80 bytes. */
typedef struct {
    char messageId[RT_MEDIUS_MESSAGE_ID_SIZE];    /*!< The identifier. */
    char sessionKey[RT_MEDIUS_SESSION_KEY_SIZE];  /*!< The key of the lobby session. */
    int worldId;                                  /*!< The game. */
    char password[RT_MEDIUS_PASSWORD_SIZE];       /*!< The password, or empty. */
    int gameHostType;                             /*!< The kind of host. */
    rtNetAddress addresses[RT_NET_ADDRESS_COUNT]; /*!< The addresses of the console. */
} rtMediusJoinGameRequest;

/** The response of rtMediusJoinGame(). */
typedef struct {
    rtMediusResponse header;  /*!< The identifier and the status. */
    int reserved1C;           /* +0x1c, not read by the game. */
    rtNetConnectionInfo info; /*!< The connection to the game. */
} rtMediusJoinGameResponse;

/** Receiver of the response of rtMediusJoinGame(). */
typedef void (*rtMediusJoinGameCallback)(int connection,
                                         int type,
                                         int size,
                                         const rtMediusJoinGameResponse *response);

/** The report of rtMediusUpdateUserState(). */
typedef struct {
    char sessionKey[RT_MEDIUS_SESSION_KEY_SIZE]; /*!< The key of the lobby session. */
    int userAction;                              /*!< What the player did. */
} rtMediusUserState;

/** The report of rtMediusWorldReport(). 0x180 bytes. */
typedef struct {
    char sessionKey[RT_MEDIUS_SESSION_KEY_SIZE]; /*!< The key of the lobby session. */
    int worldId;                                 /*!< The game. */
    int playerCount;                             /*!< The players of the game. */
    char gameName[RT_MEDIUS_GAME_NAME_SIZE];     /*!< The name, which encodes the settings. */
    char gameStats[RT_MEDIUS_STATS_SIZE];        /*!< The statistics text. */
    int minPlayers;                              /*!< The players the game takes at least. */
    int maxPlayers;                              /*!< The players the game takes at most. */
    int gameLevel;                               /*!< The level of the game. */
    int playerSkillLevel;                        /*!< The skill level of the players. */
    int rulesSet;                                /*!< The rules of the game. */
    int genericField1;                           /*!< A field of the game's choosing. */
    int genericField2;                           /*!< A field of the game's choosing. */
    int genericField3;                           /*!< A field of the game's choosing. */
    int worldStatus;                             /*!< The state of the game. */
} rtMediusWorldReportData;

/** The report of rtMediusPlayerReport(). 0x118 bytes. */
typedef struct {
    char sessionKey[RT_MEDIUS_SESSION_KEY_SIZE]; /*!< The key of the lobby session. */
    int worldId;                                 /*!< The game. */
    char stats[RT_MEDIUS_STATS_SIZE];            /*!< The statistics text of the player. */
} rtMediusPlayerReportData;

/** The report of rtMediusEndGameReport(). 0x7c bytes. */
typedef struct {
    char sessionKey[RT_MEDIUS_SESSION_KEY_SIZE];    /*!< The key of the lobby session. */
    int worldId;                                    /*!< The game. */
    char winningTeam[RT_MEDIUS_TEAM_NAME_SIZE];     /*!< The winning team, or empty. */
    char winningPlayer[RT_MEDIUS_PLAYER_NAME_SIZE]; /*!< The winning player, or empty. */
    int finalScore;                                 /*!< The final score. */
} rtMediusEndGameReportData;

/** The request of rtMediusBanPlayer(). 0x38 bytes. */
typedef struct {
    rtMediusRequest header; /*!< The identifier and the session. */
    int accountId;          /*!< The player banned. */
    int duration;           /*!< The length of the ban. */
    int worldId;            /*!< The game. */
    int reserved34;         /* +0x34, the game leaves it zero. */
} rtMediusBanPlayerRequest;

/**
 * The request of rtMediusSendChatMessage(). 0x70 bytes.
 *
 * The game fills the identifier of the header with the kind of line, the channel, and the name of
 * the sender, in the layout of the first fields of rtMediusChatMessage.
 */
typedef struct {
    rtMediusRequest header;              /*!< The kind, the channel, the sender, and the session. */
    int toAccount;                       /*!< 1 to send to targetAccountId alone, otherwise 0. */
    int targetAccountId;                 /*!< The account that receives the line. */
    char text[RT_MEDIUS_CHAT_TEXT_SIZE]; /*!< The text. */
} rtMediusChatRequest;

/**
 * Start the library.
 *
 * @param params The parameters.
 * @param status Receives a further status.
 * @return 0, or an error code.
 * @ghidraAddress NTSC-U/C: 0x002fa010
 */
int rtMediusInitialize(const rtMediusInitParams *params, int *status);

/**
 * Copy the version text of the library.
 *
 * @param version Receives the text.
 * @return 0.
 * @ghidraAddress NTSC-U/C: 0x002fa0f0
 */
int rtMediusGetVersion(char *version);

/**
 * Stop the library.
 *
 * @return 0, or an error code.
 * @ghidraAddress NTSC-U/C: 0x002fa198
 */
int rtMediusClose(void);

/**
 * Service the library once per frame.
 *
 * @return 0, or an error code once the connection failed.
 * @ghidraAddress NTSC-U/C: 0x002fa4f8
 */
int rtMediusUpdate(void);

/**
 * Fill the parameters of rtMediusConnect() for a server address.
 *
 * @param params The parameters.
 * @param sessionKey The key of the session, or null for none.
 * @param address The address of the server.
 * @param port The port of the server.
 * @param worldId The world of the server.
 * @ghidraAddress NTSC-U/C: 0x002fbd10
 * @ghidraAddress PAL: 0x00305d10
 */
void rtMediusSetConnectParams(rtMediusConnectParams *params,
                              const char *sessionKey,
                              const char *address,
                              int port,
                              int worldId);

/**
 * Start connecting to the lobby server.
 *
 * @param params The parameters.
 * @param status Receives a further status.
 * @return 0, or an error code.
 * @ghidraAddress NTSC-U/C: 0x002fa1c0
 */
int rtMediusConnect(const rtMediusConnectParams *params, rtMediusConnectStatus *status);

/**
 * Disconnect from the lobby server.
 *
 * @return 0, or an error code.
 * @ghidraAddress NTSC-U/C: 0x002fa490
 */
int rtMediusDisconnect(void);

/** @ghidraAddress NTSC-U/C: 0x002fa620 */
int rtMediusSessionBegin(rtMediusSessionBeginRequest *request,
                         rtMediusSessionBeginCallback callback);

/** @ghidraAddress NTSC-U/C: 0x002fa6c0 */
int rtMediusSessionEnd(rtMediusRequest *request, rtMediusCallback callback);

/** @ghidraAddress NTSC-U/C: 0x002fa118 */
int rtMediusVersionServer(rtMediusRequest *request, rtMediusVersionCallback callback);

/** @ghidraAddress NTSC-U/C: 0x002fab98 */
int rtMediusTextFilter(rtMediusTextFilterRequest *request, rtMediusTextFilterCallback callback);

/** @ghidraAddress NTSC-U/C: 0x002fa840 */
int rtMediusAccountRegistration(rtMediusAccountRegistrationRequest *request,
                                rtMediusCallback callback);

/** @ghidraAddress NTSC-U/C: 0x002faa60 */
int rtMediusAccountDelete(rtMediusAccountDeleteRequest *request, rtMediusCallback callback);

/** @ghidraAddress NTSC-U/C: 0x002faad8 */
int rtMediusAnonymousLogin(rtMediusAnonymousLoginRequest *request, rtMediusLoginCallback callback);

/** @ghidraAddress NTSC-U/C: 0x002fa778 */
int rtMediusAccountLogin(rtMediusAccountLoginRequest *request, rtMediusLoginCallback callback);

/** @ghidraAddress NTSC-U/C: 0x002fa9b0 */
int rtMediusUpdateStats(rtMediusUpdateStatsRequest *request, rtMediusCallback callback);

/** @ghidraAddress NTSC-U/C: 0x002fa8f8 */
int rtMediusChangePassword(rtMediusChangePasswordRequest *request, rtMediusCallback callback);

/** @ghidraAddress NTSC-U/C: 0x002fb8f0 */
int rtMediusAccountGetId(rtMediusAccountGetIdRequest *request,
                         rtMediusAccountGetIdCallback callback);

/** @ghidraAddress NTSC-U/C: 0x002fb138 */
int rtMediusPlayerInfo(rtMediusIdRequest *request, rtMediusPlayerInfoCallback callback);

/** @ghidraAddress NTSC-U/C: 0x002fb1e0 */
int rtMediusChannelInfo(rtMediusIdRequest *request, rtMediusChannelInfoCallback callback);

/** @ghidraAddress NTSC-U/C: 0x002fb650 */
int rtMediusGetAnnouncements(rtMediusIdRequest *request, rtMediusAnnouncementCallback callback);

/** @ghidraAddress NTSC-U/C: 0x002fb3f8 */
int rtMediusGetPolicy(rtMediusIdRequest *request, rtMediusPolicyCallback callback);

/** @ghidraAddress NTSC-U/C: 0x002faf58 */
int rtMediusChannelList(rtMediusChannelListRequest *request, rtMediusChannelListCallback callback);

/** @ghidraAddress NTSC-U/C: 0x002faeb8 */
int rtMediusGameList(rtMediusGameListRequest *request, rtMediusGameListCallback callback);

/** @ghidraAddress NTSC-U/C: 0x002faff8 */
int rtMediusChannelPlayers(rtMediusIdRequest *request, rtMediusChannelPlayersCallback callback);

/** @ghidraAddress NTSC-U/C: 0x002fb098 */
int rtMediusGamePlayers(rtMediusIdRequest *request, rtMediusGamePlayersCallback callback);

/** @ghidraAddress NTSC-U/C: 0x002fbae8 */
int rtMediusLadderList(rtMediusLadderListRequest *request, rtMediusLadderListCallback callback);

/** @ghidraAddress NTSC-U/C: 0x002fb9a8 */
int rtMediusLadderPosition(rtMediusLadderPositionRequest *request,
                           rtMediusLadderPositionCallback callback);

/** @ghidraAddress NTSC-U/C: 0x002fba48 */
int rtMediusLadderPositionFast(rtMediusLadderPositionFastRequest *request,
                               rtMediusLadderPositionCallback callback);

/** @ghidraAddress NTSC-U/C: 0x002fb280 */
int rtMediusFindPlayer(rtMediusFindPlayerRequest *request, rtMediusFindPlayerCallback callback);

/** @ghidraAddress NTSC-U/C: 0x002fae10 */
int rtMediusJoinChannel(rtMediusJoinChannelRequest *request, rtMediusJoinChannelCallback callback);

/** @ghidraAddress NTSC-U/C: 0x002fac48 */
int rtMediusCreateChannel(rtMediusCreateChannelRequest *request,
                          rtMediusCreateChannelCallback callback);

/** @ghidraAddress NTSC-U/C: 0x002fb798 */
int rtMediusClearGameListFilter(rtMediusIdRequest *request, rtMediusCallback callback);

/** @ghidraAddress NTSC-U/C: 0x002fb6f8 */
int rtMediusSetGameListFilter(rtMediusGameListFilterRequest *request, rtMediusCallback callback);

/** @ghidraAddress NTSC-U/C: 0x002f97b8 */
int rtMediusFileGetAttributes(rtMediusFileRequest *request,
                              rtMediusFileAttributesCallback callback);

/** @ghidraAddress NTSC-U/C: 0x002f95d0 */
int rtMediusFileDownload(rtMediusFileRequest *request, rtMediusFileTransferCallback callback);

/** @ghidraAddress NTSC-U/C: 0x002f93f0 */
int rtMediusFileCreate(rtMediusFileCreateRequest *request, rtMediusFileCallback callback);

/** @ghidraAddress NTSC-U/C: 0x002f9488 */
int rtMediusFileUpload(rtMediusFileUploadRequest *request, rtMediusFileTransferCallback callback);

/** @ghidraAddress NTSC-U/C: 0x002f9538 */
int rtMediusFileClose(rtMediusFileRequest *request, rtMediusFileCallback callback);

/** @ghidraAddress NTSC-U/C: 0x002f9710 */
int rtMediusFileList(rtMediusFileListRequest *request, rtMediusFileCallback callback);

/** @ghidraAddress NTSC-U/C: 0x002f9670 */
int rtMediusFileDelete(rtMediusFileRequest *request, rtMediusFileDeleteCallback callback);

/**
 * Join a game.
 *
 * @param request The request.
 * @param callback Receives an rtMediusJoinGameResponse.
 * @return 0, or an error code.
 * @ghidraAddress NTSC-U/C: 0x002fad00
 */
int rtMediusJoinGame(rtMediusJoinGameRequest *request, rtMediusJoinGameCallback callback);

/**
 * Report what the player did.
 *
 * @param state The report.
 * @return 0, or an error code.
 * @ghidraAddress NTSC-U/C: 0x002fa598
 */
int rtMediusUpdateUserState(rtMediusUserState *state);

/**
 * Report the game of the host.
 *
 * @param report The report.
 * @return 0, or an error code.
 * @ghidraAddress NTSC-U/C: 0x002fb520
 */
int rtMediusWorldReport(rtMediusWorldReportData *report);

/**
 * Report the player in a game.
 *
 * @param report The report.
 * @return 0, or an error code.
 * @ghidraAddress NTSC-U/C: 0x002fb498
 */
int rtMediusPlayerReport(rtMediusPlayerReportData *report);

/**
 * Report the end of the game of the host.
 *
 * @param report The report.
 * @return 0, or an error code.
 * @ghidraAddress NTSC-U/C: 0x002fb5b8
 */
int rtMediusEndGameReport(rtMediusEndGameReportData *report);

/**
 * Send a line of chat through the lobby server.
 *
 * @param request The request.
 * @return 0, or an error code.
 * @ghidraAddress NTSC-U/C: 0x002fb328
 * @ghidraAddress PAL: 0x00305328
 */
int rtMediusSendChatMessage(rtMediusChatRequest *request);

/**
 * Ban a player from a game.
 *
 * @param request The request.
 * @param callback Receives the response.
 * @return 0, or an error code.
 * @ghidraAddress NTSC-U/C: 0x002fb850
 */
int rtMediusBanPlayer(rtMediusBanPlayerRequest *request, rtMediusCallback callback);

/** The error codes of the entry points. */
#define RT_MEDIUS_ERROR_INITIALIZING (-1)              /*!< MediusErrorInitializing. */
#define RT_MEDIUS_ERROR_CONNECTING (-2)                /*!< MediusErrorConnecting. */
#define RT_MEDIUS_ERROR_DISCONNECTING (-3)             /*!< MediusErrorDisconnecting. */
#define RT_MEDIUS_ERROR_CLOSING (-4)                   /*!< MediusErrorClosing. */
#define RT_MEDIUS_ERROR_SENDING_MESSAGE (-5)           /*!< MediusErrorSendingMessage. */
#define RT_MEDIUS_ERROR_FILE_NOT_FOUND (-6)            /*!< MediusErrorFileNotFound. */
#define RT_MEDIUS_ERROR_DEVICE_NOT_FOUND (-7)          /*!< MediusErrorDeviceNotFound. */
#define RT_MEDIUS_ERROR_DIRECTORY_NOT_FOUND (-8)       /*!< MediusErrorDirectoryNotFound. */
#define RT_MEDIUS_ERROR_GENERAL (-10)                  /*!< MediusErrorGeneral. */
#define RT_MEDIUS_ERROR_SESSION_INACTIVE (-11)         /*!< MediusErrorSessionInactive. */
#define RT_MEDIUS_ERROR_ITEM_NOT_FOUND (-12)           /*!< MediusErrorItemNotFound. */
#define RT_MEDIUS_ERROR_ITEM_ALREADY_EXISTS (-13)      /*!< MediusErrorItemAlreadyExists. */
#define RT_MEDIUS_ERROR_NO_RESULT (-14)                /*!< MediusErrorNoResult. */
#define RT_MEDIUS_ERROR_NOT_INITIALIZED (-15)          /*!< MediusErrorNotInitialized. */
#define RT_MEDIUS_ERROR_NOT_CONNECTED (-16)            /*!< MediusErrorNotConnected. */
#define RT_MEDIUS_ERROR_ALREADY_CONNECTED (-17)        /*!< MediusErrorAlreadyConnected. */
#define RT_MEDIUS_ERROR_CLOSE_BEFORE_REINIT (-18)      /*!< Close before initializing again. */
#define RT_MEDIUS_ERROR_CLOSE_BARE_BEFORE_REINIT (-19) /*!< Close before initializing again. */
#define RT_MEDIUS_WORLD_CREATED_SIZE_REDUCED 3 /*!< The game was created with fewer players. */

/**
 * Report the name of an error code of the entry points.
 *
 * @param error The error code.
 * @return The name, or "UNKNOWN MEDIUSERRORCODE".
 * @ghidraAddress NTSC-U/C: 0x002fbb98
 */
const char *rtMediusGetErrorString(int error);

#ifdef __cplusplus
}
#endif

#endif
