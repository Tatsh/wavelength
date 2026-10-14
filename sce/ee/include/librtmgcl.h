#ifndef LIBRTMGCL_H
#define LIBRTMGCL_H

#include <librtdme.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Game server client of the SCE-RT network library, through which the host of an online session
 * registers the session with the game server. Every entry point reports 0 for success or a
 * negative error code. A request names itself with a message identifier, and the response its
 * callback receives repeats the identifier. The names of the entry points, the records, and their
 * fields are inferred.
 */

/** The size of a message identifier, the terminator included. */
#define RT_MGCL_MESSAGE_ID_SIZE 21

/** The size of a session key, the terminator included. */
#define RT_MGCL_SESSION_KEY_SIZE 17

/** The size of the version text, the terminator included. */
#define RT_MGCL_VERSION_SIZE 64

/** The size of the name of a game, the terminator included. */
#define RT_MGCL_GAME_NAME_SIZE 64

/** The size of the statistics text of a game, the terminator included. */
#define RT_MGCL_GAME_STATS_SIZE 256

/** The size of the password of a game, the terminator included. */
#define RT_MGCL_PASSWORD_SIZE 32

/** The size of the server version text of rtMgclSessionBeginRequest. */
#define RT_MGCL_SERVER_VERSION_SIZE 16

/** The status of a response that reports success. */
#define RT_MGCL_STATUS_SUCCESS 0

/** The leading part of a response of the game server. */
typedef struct {
    char messageId[RT_MGCL_MESSAGE_ID_SIZE]; /*!< The identifier of the request. */
    signed char status;                      /*!< RT_MGCL_STATUS_SUCCESS or an error. */
} rtMgclResponse;

/**
 * Receiver of a request the game server sends this console.
 *
 * @param connection The connection.
 * @param type The kind of request.
 * @param size The size of the request.
 * @param request The request.
 */
typedef void (*rtMgclServerRequestHandler)(int connection,
                                           int type,
                                           int size,
                                           const rtMgclResponse *request);

/** The answer of the host to a request rtMgclServerRequestHandler received. 0x68 bytes. */
typedef struct {
    char messageId[RT_MGCL_MESSAGE_ID_SIZE];   /*!< The identifier of the request. */
    signed char status;                        /*!< RT_MGCL_STATUS_SUCCESS or an error. */
    char sessionKey[RT_MGCL_SESSION_KEY_SIZE]; /*!< The key of the session, or empty. */
    char reserved27[0x41];                     /* +0x27, the rest the game leaves zero. */
} rtMgclServerReply;

/** The number of request handlers of rtMgclInitParams. */
#define RT_MGCL_HANDLER_COUNT 4

/** The parameters of rtMgclInitialize(). 0x20 bytes. */
typedef struct {
    int reserved00;  /* +0x00, the game leaves it zero. */
    int connectMode; /*!< Zero for the default connection. */
    rtMgclServerRequestHandler handlers[RT_MGCL_HANDLER_COUNT]; /*!< The request handlers. */
    int maxClients;                                             /*!< The consoles a game takes. */
    int applicationId;                                          /*!< The identifier of the game. */
} rtMgclInitParams;

/**
 * Receiver of the result of rtMgclConnect().
 *
 * @param connection The connection.
 * @param status 0 for success, otherwise an error.
 */
typedef void (*rtMgclConnectCallback)(int connection, int status);

/** The parameters of rtMgclConnect(). 0xa0 bytes. */
typedef struct {
    rtNetConnectionInfo info;        /*!< The server to connect to. */
    rtMgclConnectCallback onConnect; /*!< Receives the result. */
} rtMgclConnectParams;

/** The request of rtMgclSessionBegin(). 0x38 bytes. */
typedef struct {
    char messageId[RT_MGCL_MESSAGE_ID_SIZE];         /*!< The identifier. */
    int locationId;                                  /*!< The location of the lobby. */
    int applicationId;                               /*!< The identifier of the game. */
    int serverType;                                  /*!< The kind of server. */
    char serverVersion[RT_MGCL_SERVER_VERSION_SIZE]; /*!< Filled by the library. */
    int port;                                        /*!< The port, filled by the library. */
} rtMgclSessionBeginRequest;

/** The response of rtMgclSessionBegin(). */
typedef struct {
    rtMgclResponse header;                     /*!< The identifier and the status. */
    char reserved16[0x7a];                     /* +0x16, not read by the game. */
    char sessionKey[RT_MGCL_SESSION_KEY_SIZE]; /*!< The key of the session. */
} rtMgclSessionBeginResponse;

/** The request of rtMgclSessionEnd(). */
typedef struct {
    char messageId[RT_MGCL_MESSAGE_ID_SIZE]; /*!< The identifier. */
} rtMgclSessionEndRequest;

/** The request of rtMgclAuthenticate(). 0x4c bytes. */
typedef struct {
    char messageId[RT_MGCL_MESSAGE_ID_SIZE];      /*!< The identifier. */
    rtNetAddress addresses[RT_NET_ADDRESS_COUNT]; /*!< The addresses of the host, left empty. */
    int reserved48;                               /* +0x48, the game leaves it zero. */
} rtMgclAuthenticationRequest;

/** The response of rtMgclAuthenticate(). */
typedef struct {
    rtMgclResponse header;    /*!< The identifier and the status. */
    rtNetConnectionInfo info; /*!< The game server to connect to. */
} rtMgclAuthenticationResponse;

/** The report of rtMgclSendServerReport(). 0x20 bytes. */
typedef struct {
    char sessionKey[RT_MGCL_SESSION_KEY_SIZE]; /*!< The key of the session. */
    short maxWorlds;                           /*!< The games the host runs at most. */
    short maxPlayersPerWorld;                  /*!< The players a game takes. */
    short activeWorlds;                        /*!< The games the host runs. */
    short activePlayers;                       /*!< The players of the host. */
    int alertLevel;                            /*!< The alert level of the host. */
} rtMgclServerReport;

/** The request of rtMgclCreateGame(). 0x1d4 bytes. */
typedef struct {
    char messageId[RT_MGCL_MESSAGE_ID_SIZE];      /*!< The identifier. */
    char gameName[RT_MGCL_GAME_NAME_SIZE];        /*!< The name, which encodes the settings. */
    char gameStats[RT_MGCL_GAME_STATS_SIZE];      /*!< The statistics text. */
    char password[RT_MGCL_PASSWORD_SIZE];         /*!< The password, or empty. */
    int applicationId;                            /*!< The identifier of the game. */
    int maxClients;                               /*!< The players the game takes at most. */
    int minClients;                               /*!< The players the game takes at least. */
    int gameLevel;                                /*!< The level of the game. */
    int playerSkillLevel;                         /*!< The skill level of the players. */
    int rulesSet;                                 /*!< The rules of the game. */
    int genericField1;                            /*!< A field of the game's choosing. */
    int genericField2;                            /*!< A field of the game's choosing. */
    int genericField3;                            /*!< A field of the game's choosing. */
    int gameHostType;                             /*!< The kind of host. */
    rtNetAddress addresses[RT_NET_ADDRESS_COUNT]; /*!< The addresses of the host. */
    int worldStatus;                              /*!< The state of the game. */
} rtMgclCreateGameRequest;

/** The response of rtMgclCreateGame(). */
typedef struct {
    rtMgclResponse header; /*!< The identifier and the status. */
    int worldId;           /*!< The game the server created. */
} rtMgclCreateGameResponse;

/**
 * Receivers of the responses of the requests to the game server.
 *
 * Each takes the connection, the kind of the response, the size of the response, and the
 * response.
 */
typedef void (*rtMgclSessionBeginCallback)(int connection,
                                           int type,
                                           int size,
                                           const rtMgclSessionBeginResponse *response);
typedef void (*rtMgclSessionEndCallback)(int connection,
                                         int type,
                                         int size,
                                         const rtMgclResponse *response);
typedef void (*rtMgclAuthenticationCallback)(int connection,
                                             int type,
                                             int size,
                                             const rtMgclAuthenticationResponse *response);
typedef void (*rtMgclCreateGameCallback)(int connection,
                                         int type,
                                         int size,
                                         const rtMgclCreateGameResponse *response);

/**
 * Start the library and its peer-to-peer layer.
 *
 * @param params The parameters.
 * @param status Receives a further status.
 * @return 0, or an error code.
 * @ghidraAddress NTSC-U/C: 0x003040e8
 */
int rtMgclInitialize(const rtMgclInitParams *params, int *status);

/**
 * Copy the version text of the library.
 *
 * @param version Receives the text.
 * @return 0.
 * @ghidraAddress NTSC-U/C: 0x003050d0
 */
int rtMgclGetVersion(char version[RT_MGCL_VERSION_SIZE]);

/**
 * Send the answer of the host to a request of the game server.
 *
 * @param reply The answer.
 * @return 0, or an error code.
 * @ghidraAddress NTSC-U/C: 0x00305100
 */
int rtMgclSendServerReply(rtMgclServerReply *reply);

/**
 * Stop the library.
 *
 * @return 0, or an error code.
 * @ghidraAddress NTSC-U/C: 0x00303e48
 */
int rtMgclClose(void);

/**
 * Clear the parameters of rtMgclConnect().
 *
 * @param params The parameters.
 * @return 0, or an error code for null parameters.
 * @ghidraAddress NTSC-U/C: 0x003052c0
 */
int rtMgclInitConnectParams(rtMgclConnectParams *params);

/**
 * Fill the parameters of rtMgclConnect() for a server address.
 *
 * @param params The parameters.
 * @param address The address of the server.
 * @param port The port of the server.
 * @param worldId The world of the server.
 * @param onConnect Receives the result, or null to keep the receiver.
 * @return 0, or an error code for null parameters.
 * @ghidraAddress NTSC-U/C: 0x00305300
 */
int rtMgclSetConnectParams(rtMgclConnectParams *params,
                           const char *address,
                           int port,
                           int worldId,
                           rtMgclConnectCallback onConnect);

/**
 * Copy the public key of the game server.
 *
 * @param key Receives the key.
 * @return 0, or -1 for a null buffer.
 * @ghidraAddress NTSC-U/C: 0x003031c4
 */
int rtMgclGetServerKey(unsigned char key[RT_NET_SERVER_KEY_SIZE]);

/**
 * Start connecting to the game server.
 *
 * @param params The parameters.
 * @param status Receives a further status.
 * @return 0, or an error code.
 * @ghidraAddress NTSC-U/C: 0x00303ec0
 */
int rtMgclConnect(const rtMgclConnectParams *params, int *status);

/**
 * Disconnect from the game server.
 *
 * @return 0, or an error code.
 * @ghidraAddress NTSC-U/C: 0x00304080
 */
int rtMgclDisconnect(void);

/**
 * Begin a session with the game server.
 *
 * @param request The request.
 * @param callback Receives an rtMgclSessionBeginResponse.
 * @return 0, or an error code.
 * @ghidraAddress NTSC-U/C: 0x00303c68
 */
int rtMgclSessionBegin(rtMgclSessionBeginRequest *request, rtMgclSessionBeginCallback callback);

/**
 * End the session with the game server.
 *
 * @param request The request.
 * @param callback Receives the response.
 * @return 0, or an error code.
 * @ghidraAddress NTSC-U/C: 0x00303d60
 */
int rtMgclSessionEnd(rtMgclSessionEndRequest *request, rtMgclSessionEndCallback callback);

/**
 * Authenticate the host with the game server.
 *
 * @param request The request.
 * @param callback Receives an rtMgclAuthenticationResponse.
 * @return 0, or an error code.
 * @ghidraAddress NTSC-U/C: 0x00303b18
 */
int rtMgclAuthenticate(rtMgclAuthenticationRequest *request, rtMgclAuthenticationCallback callback);

/**
 * Report the host to the game server.
 *
 * @param report The report.
 * @return 0, or an error code.
 * @ghidraAddress NTSC-U/C: 0x00305000
 */
int rtMgclSendServerReport(rtMgclServerReport *report);

/**
 * Create the game of the host on the game server.
 *
 * @param request The request.
 * @param callback Receives an rtMgclCreateGameResponse.
 * @return 0, or an error code.
 * @ghidraAddress NTSC-U/C: 0x00305190
 */
int rtMgclCreateGame(rtMgclCreateGameRequest *request, rtMgclCreateGameCallback callback);

/**
 * Service the library once per frame.
 *
 * @return 0, or an error code once the connection failed.
 * @ghidraAddress NTSC-U/C: 0x00303de8
 */
int rtMgclUpdate(void);

#ifdef __cplusplus
}
#endif

#endif
