#include <stdio.h>
#include <string.h>

#include <librtdme.h>
#include <librtmgcl.h>

// The version text rtMgclGetVersion() reports.
static const char kVersionText[] = "Medius Game Communication Library Version 1.42.0003";

// The server version text a session begin request carries when the game leaves it empty.
static const char kServerVersion[] = "1.42.0003";

enum {
    // The error codes of the entry points.
    kErrorConnect = -2,
    kErrorDisconnect = -3,
    kErrorSend = -5,
    kErrorInitialize = -6,
    kErrorClose = -7,
    kErrorUpdate = -8,
    kErrorNotConnected = -12,
    kErrorServerType = -13,
    kErrorArgument = -13,
    kErrorAddresses = -14,
    kErrorAlreadyInitialized = -18,
    // The receiver of the messages sent to the game server.
    kServerReceiver = 0xffff,
    // The flags of the messages sent to the game server.
    kSendFlags = RT_DME_SEND_GUARANTEED,
    // The states of the request that waits for the addresses of the host.
    kRequestIdle = 0,
    kRequestReady = 1,
    kRequestFailed = 2,
    kRequestNoAddresses = 3,
    kRequestQuerying = 4,
    // The kinds of server of rtMgclSessionBeginRequest::serverType.
    kServerTypeDirect = 0,
    kServerTypeQueryFirst = 1,
    kServerTypeQuerySecond = 2,
    kServerTypeDirectOther = 4,
    // rtDmeConnect() succeeded when its callback reports this.
    kConnectSucceeded = 1,
    // The kind of connection info of a request whose addresses the library hands to rtDme.
    kConnectionReportAddresses = 2,
    // The alert levels above the last one are reported as 0.
    kAlertLevelCount = 4,
    kAddressCopySize = RT_NET_ADDRESS_SIZE + sizeof(unsigned int),
};

// A record of the game server that carries an identifier and two values. 0x20 bytes.
typedef struct {
    char messageId[RT_MGCL_MESSAGE_ID_SIZE];
    int first;
    int second;
} MgclPairRecord;

// A record of the game server that carries an identifier, a status, and a value. 0x1c bytes.
typedef struct {
    rtMgclResponse header;
    int value;
} MgclStatusValueRecord;

// A record of the game server that carries an identifier and a value. 0x1c bytes.
typedef struct {
    char messageId[RT_MGCL_MESSAGE_ID_SIZE];
    int value;
} MgclIdRecord;

// A record of the game server that carries an identifier, a value, and a flag. 0x20 bytes.
typedef struct {
    char messageId[RT_MGCL_MESSAGE_ID_SIZE];
    int value;
    char flag;
} MgclFlagRecord;

// A record of the game server that carries an identifier, three values, and a flag. 0x28 bytes.
typedef struct {
    char messageId[RT_MGCL_MESSAGE_ID_SIZE];
    int first;
    int second;
    int third;
    char flag;
} MgclQuadRecord;

// A record of the game server that carries an identifier, two values, and two addresses.
typedef struct {
    char messageId[RT_MGCL_MESSAGE_ID_SIZE];
    int first;
    int second;
    rtNetAddress addresses[RT_NET_ADDRESS_COUNT];
} MgclAddressRecord;

// A response of the game server that carries the connection info of a server. 0xb4 bytes.
typedef rtMgclAuthenticationResponse MgclConnectionResponse;

// Receivers of the responses no entry point of this version installs.
typedef void (*MgclStatusValueCallback)(int connection,
                                        int type,
                                        int size,
                                        const MgclStatusValueRecord *response);

// NTSC-U/C: 0x003c26f8
static unsigned char g_abServerKey[RT_NET_SERVER_KEY_SIZE] = {
    0x55, 0x01, 0x86, 0x14, 0x98, 0xe0, 0x6d, 0xbd, 0x11, 0x0b, 0xb3, 0x24, 0xd1, 0xd1, 0x34, 0x54,
    0x73, 0x6e, 0x57, 0xae, 0x7f, 0xb8, 0xab, 0x3e, 0x7e, 0x9e, 0x45, 0xef, 0xa0, 0x54, 0x80, 0x00,
    0x02, 0x28, 0x9d, 0x56, 0x4e, 0x26, 0x2e, 0x85, 0x41, 0x4f, 0x16, 0x9c, 0xdb, 0x11, 0x32, 0xfc,
    0x9b, 0xed, 0x85, 0xff, 0x91, 0x9f, 0x68, 0x25, 0x23, 0x5d, 0x83, 0xec, 0x16, 0x57, 0xf7, 0xc4,
};

// The registered message types, from NTSC-U/C 0x003c2738 on.
static int g_nServerReportType;
static int g_nAuthenticationRequestType;
static int g_nAuthenticationResponseType;
static int g_nSessionBeginRequestType;
static int g_nSessionBeginResponseType;
static int g_nSessionEndRequestType;
static int g_nSessionEndResponseType;
static int g_nServerRequest0Type;
static int g_nServerRequest0ReplyType;
static int g_nServerRequest1Type;
static int g_nServerReplyType;
static int g_nServerRequest2Type;
static int g_nServerRequest2ReplyType;
static int g_nServerRequest3Type;
// NTSC-U/C: 0x003c2770
// Never written. Both registrations of the fourth kind of request store into g_nServerRequest3Type.
static int g_nServerRequest3ExtendedType __attribute__((unused));
static int g_nCreateGameRequestType;
static int g_nCreateGameResponseType;
static int g_nMessage17Type;
static int g_nMessage18Type;
static int g_nMessage19Type;
static int g_nMessage20Type;
static int g_nMessage21Type;

// NTSC-U/C: 0x003c2790
static int g_nConnection;

// The receivers, from NTSC-U/C 0x003c2794 on.
static rtMgclAuthenticationCallback g_pfnAuthenticationCallback;
static rtMgclSessionBeginCallback g_pfnSessionBeginCallback;
static rtMgclSessionEndCallback g_pfnSessionEndCallback;
static rtMgclServerRequestHandler g_apfnServerRequestHandlers[RT_MGCL_HANDLER_COUNT];
static rtMgclCreateGameCallback g_pfnCreateGameCallback;
static rtMgclSessionEndCallback g_pfnMessage18Callback;
static MgclStatusValueCallback g_pfnMessage21Callback;

// NTSC-U/C: 0x003c27bc
static int g_nMaxClients = 0x100;
// NTSC-U/C: 0x003c27c0
static int g_nPendingRequestStored = 0xffffff;
// NTSC-U/C: 0x003c27c4
static int g_nServerType = 0xffffff;
// NTSC-U/C: 0x003c27c8
static int g_nPort;
// NTSC-U/C: 0x003c27cc
static short g_nInitialized;
// NTSC-U/C: 0x003c27d0
static int g_nNetMode = 1;
// NTSC-U/C: 0x003c27d4
static rtMgclConnectCallback g_pfnConnectCallback;

// NTSC-U/C: 0x0052d2f8
static int g_nRequestState;
// NTSC-U/C: 0x0052d300
static rtNetAddress g_hostAddresses[RT_NET_ADDRESS_COUNT];
// NTSC-U/C: 0x0052d330
static char g_szSessionKey[RT_MGCL_SESSION_KEY_SIZE];
// NTSC-U/C: 0x0052d348
static rtNetAddress g_serverAddress;
// NTSC-U/C: 0x0052d360
static rtMgclAuthenticationRequest g_pendingRequest;

// NTSC-U/C: 0x00305400, PAL: 0x0030f498
// Terminates a text field at its last byte, or empties it when it has no room.
static int TruncateField(char *pszField, int nSize) {
    if (pszField == NULL) {
        return -1;
    }
    if (nSize <= 0) {
        pszField[0] = '\0';
    } else {
        pszField[nSize - 1] = '\0';
    }
    return 0;
}

int rtMgclGetServerKey(unsigned char key[RT_NET_SERVER_KEY_SIZE]) {
    if (key == NULL) {
        return -1;
    }
    memcpy(key, g_abServerKey, RT_NET_SERVER_KEY_SIZE);
    return 0;
}

// NTSC-U/C: 0x00303268, PAL: 0x0030d300
static void SetRequestState(int nState) {
    g_nRequestState = nState;
}

// NTSC-U/C: 0x00303278, PAL: 0x0030d310
static int GetRequestState(void) {
    return g_nRequestState;
}

// NTSC-U/C: 0x00303288, PAL: 0x0030d320
static void SetHostAddresses(const rtNetAddress *pAddresses) {
    memcpy(g_hostAddresses, pAddresses, sizeof(g_hostAddresses));
}

// NTSC-U/C: 0x00303360, PAL: 0x0030d3f8
static void GetHostAddresses(rtNetAddress *pAddresses) {
    memcpy(pAddresses, g_hostAddresses, sizeof(g_hostAddresses));
}

// NTSC-U/C: 0x003033d0, PAL: 0x0030d468
// Receives the addresses the server sees for the host.
static void OnAddressResult(const rtDmeAddressResult *pResult) {
    if (pResult->error != 0) {
        SetRequestState(kRequestNoAddresses);
        return;
    }
    if (pResult->complete == 0) {
        SetRequestState(kRequestNoAddresses);
        return;
    }
    SetHostAddresses(pResult->addresses);
    SetRequestState(kRequestReady);
}

// NTSC-U/C: 0x00303428, PAL: 0x0030d4c0
// Stores the request that waits for the addresses. A non-zero flag stores nothing.
static void StorePendingRequest(int nStored, const rtMgclAuthenticationRequest *pRequest) {
    g_nPendingRequestStored = nStored;
    if (nStored == 0) {
        g_pendingRequest = *pRequest;
    }
}

// NTSC-U/C: 0x00303508, PAL: 0x0030d5a0
static void LoadPendingRequest(rtMgclAuthenticationRequest *pRequest) {
    if (g_nPendingRequestStored == 0) {
        *pRequest = g_pendingRequest;
    }
}

// NTSC-U/C: 0x003035e8, PAL: 0x0030d680
static void GetPendingRequestStored(int *pnStored) {
    *pnStored = g_nPendingRequestStored;
}

// NTSC-U/C: 0x003035f8, PAL: 0x0030d690
// Sends the request that waited for the addresses once they arrived.
static int SendPendingRequest(void) {
    int nResult = 0;

    if (GetRequestState() == kRequestReady) {
        rtMgclAuthenticationRequest request;
        int nStored;

        SetRequestState(kRequestIdle);
        GetPendingRequestStored(&nStored);
        if (nStored == 0) {
            LoadPendingRequest(&request);
            GetHostAddresses(request.addresses);
            nResult = rtDmeSendClassMessage(kSendFlags,
                                            g_nConnection,
                                            kServerReceiver,
                                            RT_DME_CLASS_SERVER,
                                            g_nAuthenticationRequestType,
                                            sizeof(request),
                                            &request);
        }
        return (nResult == 0) ? 0 : kErrorSend;
    }
    if (GetRequestState() == kRequestNoAddresses) {
        return kErrorAddresses;
    }
    if (GetRequestState() == kRequestFailed) {
        return kErrorSend;
    }
    return 0;
}

// NTSC-U/C: 0x003036c0, PAL: 0x0030d758
static void SetConnectCallback(rtMgclConnectCallback pfnCallback) {
    g_pfnConnectCallback = pfnCallback;
}

// NTSC-U/C: 0x003036d0, PAL: 0x0030d768
static rtMgclConnectCallback GetConnectCallback(void) {
    return g_pfnConnectCallback;
}

// NTSC-U/C: 0x003036e0, PAL: 0x0030d778
static int GetServerType(void) {
    return g_nServerType;
}

// NTSC-U/C: 0x003036f0, PAL: 0x0030d788
static void SetServerType(int nServerType) {
    g_nServerType = nServerType;
}

// NTSC-U/C: 0x00303700, PAL: 0x0030d798
static int GetPort(void) {
    return g_nPort;
}

// NTSC-U/C: 0x00303710, PAL: 0x0030d7a8
static void SetPort(int nPort) {
    g_nPort = nPort;
}

// NTSC-U/C: 0x00303720, PAL: 0x0030d7b8
static void GetSessionKey(char *pszKey) {
    if (pszKey != NULL) {
        strncpy(pszKey, g_szSessionKey, RT_MGCL_SESSION_KEY_SIZE);
    }
}

// NTSC-U/C: 0x00303758, PAL: 0x0030d7f0
static void SetSessionKey(const char *pszKey) {
    if (pszKey != NULL) {
        strncpy(g_szSessionKey, pszKey, RT_MGCL_SESSION_KEY_SIZE);
    }
}

// NTSC-U/C: 0x00303790, PAL: 0x0030d828
static void GetServerAddress(rtNetAddress *pAddress) {
    *pAddress = g_serverAddress;
}

// NTSC-U/C: 0x003037d0, PAL: 0x0030d868
static void SetServerAddress(const rtNetAddress *pAddress) {
    g_serverAddress = *pAddress;
}

#pragma mark - Message handlers

// NTSC-U/C: 0x00303848, PAL: 0x0030d8e0
static int OnServerRequest0(int connection, int type, int sender, rtDmeMessage *message) {
    if (g_apfnServerRequestHandlers[0] != NULL) {
        g_apfnServerRequestHandlers[0](connection, type, sender, (const rtMgclResponse *)message);
    }
    return sizeof(MgclPairRecord);
}

// NTSC-U/C: 0x00303878, PAL: 0x0030d910
static int OnCreateGameResponse(int connection, int type, int sender, rtDmeMessage *message) {
    if (g_pfnCreateGameCallback != NULL) {
        g_pfnCreateGameCallback(
            connection, type, sender, (const rtMgclCreateGameResponse *)message);
    }
    return sizeof(rtMgclCreateGameResponse);
}

// NTSC-U/C: 0x003038a8, PAL: 0x0030d940
static int OnMessage18(int connection, int type, int sender, rtDmeMessage *message) {
    if (g_pfnMessage18Callback != NULL) {
        g_pfnMessage18Callback(connection, type, sender, (const rtMgclResponse *)message);
    }
    return sizeof(rtMgclResponse);
}

// NTSC-U/C: 0x003038d8, PAL: 0x0030d970
static int OnServerRequest3(int connection, int type, int sender, rtDmeMessage *message) {
    if (g_apfnServerRequestHandlers[3] != NULL) {
        g_apfnServerRequestHandlers[3](connection, type, sender, (const rtMgclResponse *)message);
    }
    return sizeof(MgclIdRecord);
}

// NTSC-U/C: 0x00303908, PAL: 0x0030d9a0
// Hands the addresses of a peer the request carries to rtDme unless the handler refuses.
static int OnServerRequest1(int connection, int type, int sender, rtDmeMessage *message) {
    const MgclConnectionResponse *pRequest = (const MgclConnectionResponse *)message;
    rtDmeAddressReport report;
    rtDmeConnectStatus status;
    int nResult = 0;

    if (g_apfnServerRequestHandlers[1] != NULL) {
        nResult = g_apfnServerRequestHandlers[1](connection, type, sender, &pRequest->header);
    }
    if (nResult == 0 && pRequest->info.type == kConnectionReportAddresses) {
        rtDmeInitAddressReport(&report);
        memcpy(report.addresses, pRequest->info.addresses, sizeof(report.addresses));
        rtDmeReportAddresses(&report, &status);
    }
    return sizeof(MgclConnectionResponse);
}

// NTSC-U/C: 0x003039d0, PAL: 0x0030da68
static int OnServerRequest2(int connection, int type, int sender, rtDmeMessage *message) {
    if (g_apfnServerRequestHandlers[2] != NULL) {
        g_apfnServerRequestHandlers[2](connection, type, sender, (const rtMgclResponse *)message);
    }
    return sizeof(MgclFlagRecord);
}

// NTSC-U/C: 0x00303a00, PAL: 0x0030da98
static int OnAuthenticationResponse(int connection, int type, int sender, rtDmeMessage *message) {
    if (g_pfnAuthenticationCallback != NULL) {
        g_pfnAuthenticationCallback(
            connection, type, sender, (const rtMgclAuthenticationResponse *)message);
    }
    return sizeof(rtMgclAuthenticationResponse);
}

// NTSC-U/C: 0x00303a30, PAL: 0x0030dac8
// Keeps the session key and the address of the server of a successful session.
static int OnSessionBeginResponse(int connection, int type, int sender, rtDmeMessage *message) {
    const MgclConnectionResponse *pResponse = (const MgclConnectionResponse *)message;

    if (pResponse->header.status == RT_MGCL_STATUS_SUCCESS) {
        SetSessionKey(pResponse->info.sessionKey);
        SetServerAddress(&pResponse->info.addresses[0]);
    }
    if (g_pfnSessionBeginCallback != NULL) {
        g_pfnSessionBeginCallback(
            connection, type, sender, (const rtMgclSessionBeginResponse *)message);
    }
    return sizeof(MgclConnectionResponse);
}

// NTSC-U/C: 0x00303ab8, PAL: 0x0030db50
static int OnSessionEndResponse(int connection, int type, int sender, rtDmeMessage *message) {
    if (g_pfnSessionEndCallback != NULL) {
        g_pfnSessionEndCallback(connection, type, sender, (const rtMgclResponse *)message);
    }
    return sizeof(rtMgclResponse);
}

// NTSC-U/C: 0x00303ae8, PAL: 0x0030db80
static int OnMessage21(int connection, int type, int sender, rtDmeMessage *message) {
    if (g_pfnMessage21Callback != NULL) {
        g_pfnMessage21Callback(connection, type, sender, (const MgclStatusValueRecord *)message);
    }
    return sizeof(MgclStatusValueRecord);
}

#pragma mark - Entry points

int rtMgclAuthenticate(rtMgclAuthenticationRequest *request,
                       rtMgclAuthenticationCallback callback) {
    TruncateField(request->messageId, RT_MGCL_MESSAGE_ID_SIZE);
    if (g_nInitialized != 1) {
        return kErrorNotConnected;
    }
    g_pfnAuthenticationCallback = callback;
    if (GetServerType() == kServerTypeQueryFirst || GetServerType() == kServerTypeQuerySecond) {
        rtDmeAddressQuery query;
        rtDmeConnectStatus status;

        if (rtDmeInitAddressQuery(&query) != 0) {
            return kErrorSend;
        }
        GetServerAddress(&query.server);
        query.onResult = OnAddressResult;
        SetRequestState(kRequestQuerying);
        StorePendingRequest(0, request);
        if (rtDmeQueryAddresses(&query, &status) == 0) {
            return 0;
        }
        SetRequestState(kRequestIdle);
        return kErrorSend;
    }
    if (GetServerType() != kServerTypeDirect && GetServerType() != kServerTypeDirectOther) {
        return kErrorServerType;
    }
    request->addresses[0].type = 1;
    rtDmeGetLocalAddress(request->addresses[0].address);
    request->addresses[0].port = GetPort();
    request->addresses[1].type = 0;
    return (rtDmeSendClassMessage(kSendFlags,
                                  g_nConnection,
                                  kServerReceiver,
                                  RT_DME_CLASS_SERVER,
                                  g_nAuthenticationRequestType,
                                  sizeof(*request),
                                  request) == 0) ?
               0 :
               kErrorSend;
}

int rtMgclSessionBegin(rtMgclSessionBeginRequest *request, rtMgclSessionBeginCallback callback) {
    if (g_nInitialized != 1) {
        return kErrorNotConnected;
    }
    g_pfnSessionBeginCallback = callback;
    SetServerType(request->serverType);
    if (request->serverType == kServerTypeDirect || request->serverType == kServerTypeDirectOther) {
        SetPort(request->port);
    } else {
        request->port = 0;
    }
    const char *pszVersion = request->serverVersion;
    if (strcmp(pszVersion, "") == 0 || pszVersion == NULL) {
        strncpy(request->serverVersion, kServerVersion, RT_MGCL_SERVER_VERSION_SIZE);
    }
    TruncateField(request->messageId, RT_MGCL_MESSAGE_ID_SIZE);
    TruncateField(request->serverVersion, RT_MGCL_SERVER_VERSION_SIZE);
    return (rtDmeSendClassMessage(kSendFlags,
                                  g_nConnection,
                                  kServerReceiver,
                                  RT_DME_CLASS_SERVER,
                                  g_nSessionBeginRequestType,
                                  sizeof(*request),
                                  request) == 0) ?
               0 :
               kErrorSend;
}

int rtMgclSessionEnd(rtMgclSessionEndRequest *request, rtMgclSessionEndCallback callback) {
    TruncateField(request->messageId, RT_MGCL_MESSAGE_ID_SIZE);
    if (g_nInitialized != 1) {
        return kErrorNotConnected;
    }
    g_pfnSessionEndCallback = callback;
    return (rtDmeSendClassMessage(kSendFlags,
                                  g_nConnection,
                                  kServerReceiver,
                                  RT_DME_CLASS_SERVER,
                                  g_nSessionEndRequestType,
                                  sizeof(*request),
                                  request) == 0) ?
               0 :
               kErrorSend;
}

int rtMgclUpdate(void) {
    int nResult;

    if (g_nInitialized != 1) {
        return kErrorNotConnected;
    }
    nResult = SendPendingRequest();
    if (nResult != 0) {
        return nResult;
    }
    nResult = rtDmeUpdate();
    if (nResult != 0) {
        printf("Error Network I/O --- DME Error %d.\n", nResult);
        return kErrorUpdate;
    }
    return 0;
}

int rtMgclClose(void) {
    int nResult = 0;

    if (g_nInitialized != 1) {
        return kErrorNotConnected;
    }
    if (g_nNetMode == 1) {
        nResult = rtNetClose();
    }
    if (nResult != 0) {
        printf("Error closing Report Library --- DME Error %d.\n", nResult);
        return kErrorClose;
    }
    g_nInitialized = 0;
    return 0;
}

// NTSC-U/C: 0x00305378, PAL: 0x0030f410
// Receives the result of the connection rtMgclConnect() started.
static void OnConnectResult(int connection, int result, int reason, int netError) {
    int nStatus;

    if (result == kConnectSucceeded) {
        g_nConnection = connection;
        nStatus = 0;
    } else {
        g_nConnection = connection;
        printf("MGCLConnect Failed - NetErrorCode = %d - Failure Reason %d\n", netError, reason);
        nStatus = kErrorConnect;
    }
    if (GetConnectCallback() != NULL) {
        GetConnectCallback()(g_nConnection, nStatus);
    }
}

int rtMgclConnect(const rtMgclConnectParams *params, int *status) {
    rtDmeConnectParams connect;
    rtDmeConnectStatus connectStatus;
    int nResult;

    if (g_nInitialized != 1) {
        return kErrorNotConnected;
    }
    rtDmeInitConnectParams(&connect);
    connect.info = params->info;
    connect.maxClients = g_nMaxClients;
    if (params->onConnect != NULL) {
        SetConnectCallback(params->onConnect);
        connect.onConnect = OnConnectResult;
    } else {
        SetConnectCallback(NULL);
    }
    nResult = rtDmeConnect(&connect, &connectStatus);
    if (nResult != 0) {
        printf("Error connecting to Medius Server at IP= %s Port= %d World= %d --- DME Error %d.\n",
               params->info.addresses[0].address,
               params->info.addresses[0].port,
               params->info.worldId,
               nResult);
        *status = kErrorConnect;
        return kErrorConnect;
    }
    if (params->onConnect == NULL) {
        g_nConnection = connectStatus.reserved[2]; // The connection the status reports.
        *status = 0;
    }
    return 0;
}

int rtMgclDisconnect(void) {
    int nResult;

    if (g_nInitialized != 1) {
        return kErrorNotConnected;
    }
    nResult = rtDmeDisconnect(g_nConnection);
    g_nConnection = -1;
    if (nResult != 0) {
        printf("Error Disconnecting from Medius Server --- DME Error %d.\n", nResult);
        return kErrorDisconnect;
    }
    return 0;
}

int rtMgclInitialize(const rtMgclInitParams *params, int *status) {
    rtMgclServerReport serverReport;
    rtMgclAuthenticationRequest authenticationRequest;
    rtMgclAuthenticationResponse authenticationResponse;
    rtMgclSessionBeginRequest sessionBeginRequest;
    MgclConnectionResponse sessionBeginResponse;
    rtMgclSessionEndRequest sessionEndRequest;
    rtMgclResponse sessionEndResponse;
    MgclPairRecord serverRequest0;
    MgclStatusValueRecord serverRequest0Reply;
    MgclConnectionResponse serverRequest1;
    rtMgclServerReply serverReply;
    MgclFlagRecord serverRequest2;
    rtMgclResponse serverRequest2Reply;
    MgclIdRecord serverRequest3;
    MgclQuadRecord serverRequest3Extended;
    rtMgclCreateGameRequest createGameRequest;
    rtMgclCreateGameResponse createGameResponse;
    MgclIdRecord message17;
    rtMgclResponse message18;
    rtMgclCreateGameRequest message19;
    MgclAddressRecord message20;
    MgclStatusValueRecord message21;
    rtNetSettings settings;
    rtNetInitStatus initStatus;
    int nInitResult = 0;
    int nRegisterResult = 0;

    if (g_nInitialized == 1) {
        *status = kErrorInitialize;
        return kErrorAlreadyInitialized;
    }
    rtNetDefaultSettings(&settings);
    settings.publicKey = params->publicKey;
    g_nMaxClients = params->maxClients;
    settings.applicationId = params->applicationId;
    if (params->connectMode == 1) {
        g_nNetMode = params->connectMode;
        nInitResult = rtNetInitialize(&settings, &initStatus);
    } else {
        g_nNetMode = 0;
    }
    SetRequestState(kRequestIdle);
    if (nInitResult == 0) {
        rtDmeBeginFields();
        rtDmeAddField(&serverReport,
                      &serverReport.sessionKey,
                      RT_MGCL_SESSION_KEY_SIZE,
                      RT_MGCL_SESSION_KEY_SIZE);
        rtDmeAddField(&serverReport, &serverReport.maxWorlds, sizeof(serverReport.maxWorlds), 1);
        rtDmeAddField(&serverReport,
                      &serverReport.maxPlayersPerWorld,
                      sizeof(serverReport.maxPlayersPerWorld),
                      1);
        rtDmeAddField(
            &serverReport, &serverReport.activeWorlds, sizeof(serverReport.activeWorlds), 1);
        rtDmeAddField(
            &serverReport, &serverReport.activePlayers, sizeof(serverReport.activePlayers), 1);
        rtDmeAddField(&serverReport, &serverReport.alertLevel, sizeof(serverReport.alertLevel), 1);
        rtDmeRegisterClassMessage(&g_nServerReportType, RT_DME_CLASS_SERVER, NULL);
        rtDmeBeginFields();
        rtDmeAddField(
            &authenticationRequest, &authenticationRequest.messageId, 1, RT_MGCL_MESSAGE_ID_SIZE);
        rtDmeAddField(&authenticationRequest,
                      &authenticationRequest.requestType,
                      sizeof(authenticationRequest.requestType),
                      1);
        rtDmeAddField(&authenticationRequest,
                      &authenticationRequest.addresses[0].type,
                      sizeof(authenticationRequest.addresses[0].type),
                      1);
        rtDmeAddField(&authenticationRequest,
                      &authenticationRequest.addresses[0].address,
                      RT_NET_ADDRESS_SIZE,
                      RT_NET_ADDRESS_SIZE);
        rtDmeAddField(&authenticationRequest,
                      &authenticationRequest.addresses[0].port,
                      sizeof(authenticationRequest.addresses[0].port),
                      1);
        rtDmeAddField(&authenticationRequest,
                      &authenticationRequest.addresses[1].type,
                      sizeof(authenticationRequest.addresses[1].type),
                      1);
        rtDmeAddField(&authenticationRequest,
                      &authenticationRequest.addresses[1].address,
                      RT_NET_ADDRESS_SIZE,
                      RT_NET_ADDRESS_SIZE);
        rtDmeAddField(&authenticationRequest,
                      &authenticationRequest.addresses[1].port,
                      sizeof(authenticationRequest.addresses[1].port),
                      1);
        rtDmeRegisterClassMessage(&g_nAuthenticationRequestType, RT_DME_CLASS_SERVER, NULL);
        rtDmeBeginFields();
        rtDmeAddField(&authenticationResponse,
                      &authenticationResponse.header.messageId,
                      1,
                      RT_MGCL_MESSAGE_ID_SIZE);
        rtDmeAddField(&authenticationResponse,
                      &authenticationResponse.info.type,
                      sizeof(authenticationResponse.info.type),
                      1);
        rtDmeAddField(&authenticationResponse,
                      &authenticationResponse.info.addresses[0].type,
                      sizeof(authenticationResponse.info.addresses[0].type),
                      1);
        rtDmeAddField(&authenticationResponse,
                      &authenticationResponse.info.addresses[0].address,
                      RT_NET_ADDRESS_SIZE,
                      RT_NET_ADDRESS_SIZE);
        rtDmeAddField(&authenticationResponse,
                      &authenticationResponse.info.addresses[0].port,
                      sizeof(authenticationResponse.info.addresses[0].port),
                      1);
        rtDmeAddField(&authenticationResponse,
                      &authenticationResponse.info.addresses[1].type,
                      sizeof(authenticationResponse.info.addresses[1].type),
                      1);
        rtDmeAddField(&authenticationResponse,
                      &authenticationResponse.info.addresses[1].address,
                      RT_NET_ADDRESS_SIZE,
                      RT_NET_ADDRESS_SIZE);
        rtDmeAddField(&authenticationResponse,
                      &authenticationResponse.info.addresses[1].port,
                      sizeof(authenticationResponse.info.addresses[1].port),
                      1);
        rtDmeAddField(&authenticationResponse,
                      &authenticationResponse.info.worldId,
                      sizeof(authenticationResponse.info.worldId),
                      1);
        rtDmeAddField(&authenticationResponse,
                      &authenticationResponse.info.serverKey,
                      sizeof(int),
                      sizeof(authenticationResponse.info.serverKey) / sizeof(int));
        rtDmeAddField(&authenticationResponse,
                      &authenticationResponse.info.sessionKey,
                      1,
                      RT_MGCL_SESSION_KEY_SIZE);
        rtDmeAddField(
            &authenticationResponse, &authenticationResponse.info.accessKey, 1, RT_NET_KEY_SIZE);
        rtDmeAddField(&authenticationResponse,
                      &authenticationResponse.header.status,
                      sizeof(authenticationResponse.header.status),
                      1);
        rtDmeRegisterClassMessage(
            &g_nAuthenticationResponseType, RT_DME_CLASS_SERVER, OnAuthenticationResponse);
        rtDmeBeginFields();
        rtDmeAddField(
            &sessionBeginRequest, &sessionBeginRequest.messageId, 1, RT_MGCL_MESSAGE_ID_SIZE);
        rtDmeAddField(&sessionBeginRequest,
                      &sessionBeginRequest.locationId,
                      sizeof(sessionBeginRequest.locationId),
                      1);
        rtDmeAddField(&sessionBeginRequest,
                      &sessionBeginRequest.applicationId,
                      sizeof(sessionBeginRequest.applicationId),
                      1);
        rtDmeAddField(&sessionBeginRequest,
                      &sessionBeginRequest.serverType,
                      sizeof(sessionBeginRequest.serverType),
                      1);
        rtDmeAddField(&sessionBeginRequest,
                      &sessionBeginRequest.serverVersion,
                      1,
                      RT_MGCL_SERVER_VERSION_SIZE);
        rtDmeAddField(
            &sessionBeginRequest, &sessionBeginRequest.port, sizeof(sessionBeginRequest.port), 1);
        rtDmeRegisterClassMessage(&g_nSessionBeginRequestType, RT_DME_CLASS_SERVER, NULL);
        rtDmeBeginFields();
        rtDmeAddField(&sessionBeginResponse,
                      &sessionBeginResponse.header.messageId,
                      1,
                      RT_MGCL_MESSAGE_ID_SIZE);
        rtDmeAddField(&sessionBeginResponse,
                      &sessionBeginResponse.info.type,
                      sizeof(sessionBeginResponse.info.type),
                      1);
        rtDmeAddField(&sessionBeginResponse,
                      &sessionBeginResponse.info.addresses[0].type,
                      sizeof(sessionBeginResponse.info.addresses[0].type),
                      1);
        rtDmeAddField(&sessionBeginResponse,
                      &sessionBeginResponse.info.addresses[0].address,
                      RT_NET_ADDRESS_SIZE,
                      RT_NET_ADDRESS_SIZE);
        rtDmeAddField(&sessionBeginResponse,
                      &sessionBeginResponse.info.addresses[0].port,
                      sizeof(sessionBeginResponse.info.addresses[0].port),
                      1);
        rtDmeAddField(&sessionBeginResponse,
                      &sessionBeginResponse.info.addresses[1].type,
                      sizeof(sessionBeginResponse.info.addresses[1].type),
                      1);
        rtDmeAddField(&sessionBeginResponse,
                      &sessionBeginResponse.info.addresses[1].address,
                      RT_NET_ADDRESS_SIZE,
                      RT_NET_ADDRESS_SIZE);
        rtDmeAddField(&sessionBeginResponse,
                      &sessionBeginResponse.info.addresses[1].port,
                      sizeof(sessionBeginResponse.info.addresses[1].port),
                      1);
        rtDmeAddField(&sessionBeginResponse,
                      &sessionBeginResponse.info.worldId,
                      sizeof(sessionBeginResponse.info.worldId),
                      1);
        rtDmeAddField(&sessionBeginResponse,
                      &sessionBeginResponse.info.serverKey,
                      sizeof(int),
                      sizeof(sessionBeginResponse.info.serverKey) / sizeof(int));
        rtDmeAddField(&sessionBeginResponse,
                      &sessionBeginResponse.info.sessionKey,
                      1,
                      RT_MGCL_SESSION_KEY_SIZE);
        rtDmeAddField(
            &sessionBeginResponse, &sessionBeginResponse.info.accessKey, 1, RT_NET_KEY_SIZE);
        rtDmeAddField(&sessionBeginResponse,
                      &sessionBeginResponse.header.status,
                      sizeof(sessionBeginResponse.header.status),
                      1);
        rtDmeRegisterClassMessage(
            &g_nSessionBeginResponseType, RT_DME_CLASS_SERVER, OnSessionBeginResponse);
        rtDmeBeginFields();
        rtDmeAddField(&sessionEndRequest, &sessionEndRequest.messageId, 1, RT_MGCL_MESSAGE_ID_SIZE);
        rtDmeRegisterClassMessage(&g_nSessionEndRequestType, RT_DME_CLASS_SERVER, NULL);
        rtDmeBeginFields();
        rtDmeAddField(
            &sessionEndResponse, &sessionEndResponse.messageId, 1, RT_MGCL_MESSAGE_ID_SIZE);
        rtDmeAddField(
            &sessionEndResponse, &sessionEndResponse.status, sizeof(sessionEndResponse.status), 1);
        rtDmeRegisterClassMessage(
            &g_nSessionEndResponseType, RT_DME_CLASS_SERVER, OnSessionEndResponse);
        rtDmeBeginFields();
        rtDmeAddField(&serverRequest0, &serverRequest0.messageId, 1, RT_MGCL_MESSAGE_ID_SIZE);
        rtDmeAddField(&serverRequest0, &serverRequest0.first, sizeof(serverRequest0.first), 1);
        rtDmeAddField(&serverRequest0, &serverRequest0.second, sizeof(serverRequest0.second), 1);
        rtDmeRegisterClassMessage(&g_nServerRequest0Type, RT_DME_CLASS_SERVER, OnServerRequest0);
        rtDmeBeginFields();
        rtDmeAddField(&serverRequest0Reply,
                      &serverRequest0Reply.header.messageId,
                      1,
                      RT_MGCL_MESSAGE_ID_SIZE);
        rtDmeAddField(&serverRequest0Reply,
                      &serverRequest0Reply.header.status,
                      sizeof(serverRequest0Reply.header.status),
                      1);
        rtDmeAddField(
            &serverRequest0Reply, &serverRequest0Reply.value, sizeof(serverRequest0Reply.value), 1);
        rtDmeRegisterClassMessage(&g_nServerRequest0ReplyType, RT_DME_CLASS_SERVER, NULL);
        rtDmeBeginFields();
        rtDmeAddField(
            &serverRequest1, &serverRequest1.header.messageId, 1, RT_MGCL_MESSAGE_ID_SIZE);
        rtDmeAddField(
            &serverRequest1, &serverRequest1.info.type, sizeof(serverRequest1.info.type), 1);
        rtDmeAddField(&serverRequest1,
                      &serverRequest1.info.addresses[0].type,
                      sizeof(serverRequest1.info.addresses[0].type),
                      1);
        rtDmeAddField(&serverRequest1,
                      &serverRequest1.info.addresses[0].address,
                      RT_NET_ADDRESS_SIZE,
                      RT_NET_ADDRESS_SIZE);
        rtDmeAddField(&serverRequest1,
                      &serverRequest1.info.addresses[0].port,
                      sizeof(serverRequest1.info.addresses[0].port),
                      1);
        rtDmeAddField(&serverRequest1,
                      &serverRequest1.info.addresses[1].type,
                      sizeof(serverRequest1.info.addresses[1].type),
                      1);
        rtDmeAddField(&serverRequest1,
                      &serverRequest1.info.addresses[1].address,
                      RT_NET_ADDRESS_SIZE,
                      RT_NET_ADDRESS_SIZE);
        rtDmeAddField(&serverRequest1,
                      &serverRequest1.info.addresses[1].port,
                      sizeof(serverRequest1.info.addresses[1].port),
                      1);
        rtDmeAddField(
            &serverRequest1, &serverRequest1.info.worldId, sizeof(serverRequest1.info.worldId), 1);
        rtDmeAddField(&serverRequest1,
                      &serverRequest1.info.serverKey,
                      sizeof(int),
                      sizeof(serverRequest1.info.serverKey) / sizeof(int));
        rtDmeAddField(
            &serverRequest1, &serverRequest1.info.sessionKey, 1, RT_MGCL_SESSION_KEY_SIZE);
        rtDmeAddField(&serverRequest1, &serverRequest1.info.accessKey, 1, RT_NET_KEY_SIZE);
        rtDmeRegisterClassMessage(&g_nServerRequest1Type, RT_DME_CLASS_SERVER, OnServerRequest1);
        rtDmeBeginFields();
        rtDmeAddField(&serverReply, &serverReply.messageId, 1, RT_MGCL_MESSAGE_ID_SIZE);
        rtDmeAddField(&serverReply, &serverReply.status, sizeof(serverReply.status), 1);
        rtDmeAddField(&serverReply, &serverReply.sessionKey, 1, RT_MGCL_SESSION_KEY_SIZE);
        rtDmeAddField(&serverReply,
                      &serverReply.reserved28,
                      sizeof(int),
                      sizeof(serverReply.reserved28) / sizeof(int));
        rtDmeRegisterClassMessage(&g_nServerReplyType, RT_DME_CLASS_SERVER, NULL);
        rtDmeBeginFields();
        rtDmeAddField(&serverRequest2, &serverRequest2.messageId, 1, RT_MGCL_MESSAGE_ID_SIZE);
        rtDmeAddField(&serverRequest2, &serverRequest2.value, sizeof(serverRequest2.value), 1);
        rtDmeAddField(&serverRequest2, &serverRequest2.flag, sizeof(serverRequest2.flag), 1);
        rtDmeRegisterClassMessage(&g_nServerRequest2Type, RT_DME_CLASS_SERVER, OnServerRequest2);
        rtDmeBeginFields();
        rtDmeAddField(
            &serverRequest2Reply, &serverRequest2Reply.messageId, 1, RT_MGCL_MESSAGE_ID_SIZE);
        rtDmeAddField(&serverRequest2Reply,
                      &serverRequest2Reply.status,
                      sizeof(serverRequest2Reply.status),
                      1);
        rtDmeRegisterClassMessage(&g_nServerRequest2ReplyType, RT_DME_CLASS_SERVER, NULL);
        rtDmeBeginFields();
        rtDmeAddField(&serverRequest3, &serverRequest3.messageId, 1, RT_MGCL_MESSAGE_ID_SIZE);
        rtDmeAddField(&serverRequest3, &serverRequest3.value, sizeof(serverRequest3.value), 1);
        rtDmeRegisterClassMessage(&g_nServerRequest3Type, RT_DME_CLASS_SERVER, OnServerRequest3);
        rtDmeBeginFields();
        rtDmeAddField(
            &serverRequest3Extended, &serverRequest3Extended.messageId, 1, RT_MGCL_MESSAGE_ID_SIZE);
        rtDmeAddField(&serverRequest3Extended,
                      &serverRequest3Extended.first,
                      sizeof(serverRequest3Extended.first),
                      1);
        rtDmeAddField(&serverRequest3Extended,
                      &serverRequest3Extended.second,
                      sizeof(serverRequest3Extended.second),
                      1);
        rtDmeAddField(&serverRequest3Extended,
                      &serverRequest3Extended.third,
                      sizeof(serverRequest3Extended.third),
                      1);
        rtDmeAddField(&serverRequest3Extended,
                      &serverRequest3Extended.flag,
                      sizeof(serverRequest3Extended.flag),
                      1);
        rtDmeRegisterClassMessage(&g_nServerRequest3Type, RT_DME_CLASS_SERVER, OnServerRequest3);
        rtDmeBeginFields();
        rtDmeAddField(&createGameRequest, &createGameRequest.messageId, 1, RT_MGCL_MESSAGE_ID_SIZE);
        rtDmeAddField(&createGameRequest, &createGameRequest.gameName, 1, RT_MGCL_GAME_NAME_SIZE);
        rtDmeAddField(&createGameRequest, &createGameRequest.gameStats, 1, RT_MGCL_GAME_STATS_SIZE);
        rtDmeAddField(&createGameRequest,
                      &createGameRequest.password,
                      RT_MGCL_PASSWORD_SIZE,
                      RT_MGCL_PASSWORD_SIZE);
        rtDmeAddField(&createGameRequest,
                      &createGameRequest.applicationId,
                      sizeof(createGameRequest.applicationId),
                      1);
        rtDmeAddField(&createGameRequest,
                      &createGameRequest.maxClients,
                      sizeof(createGameRequest.maxClients),
                      1);
        rtDmeAddField(&createGameRequest,
                      &createGameRequest.minClients,
                      sizeof(createGameRequest.minClients),
                      1);
        rtDmeAddField(&createGameRequest,
                      &createGameRequest.gameLevel,
                      sizeof(createGameRequest.gameLevel),
                      1);
        rtDmeAddField(&createGameRequest,
                      &createGameRequest.playerSkillLevel,
                      sizeof(createGameRequest.playerSkillLevel),
                      1);
        rtDmeAddField(
            &createGameRequest, &createGameRequest.rulesSet, sizeof(createGameRequest.rulesSet), 1);
        rtDmeAddField(&createGameRequest,
                      &createGameRequest.genericField1,
                      sizeof(createGameRequest.genericField1),
                      1);
        rtDmeAddField(&createGameRequest,
                      &createGameRequest.genericField2,
                      sizeof(createGameRequest.genericField2),
                      1);
        rtDmeAddField(&createGameRequest,
                      &createGameRequest.genericField3,
                      sizeof(createGameRequest.genericField3),
                      1);
        rtDmeAddField(&createGameRequest,
                      &createGameRequest.gameHostType,
                      sizeof(createGameRequest.gameHostType),
                      1);
        rtDmeAddField(&createGameRequest,
                      &createGameRequest.addresses[0].type,
                      sizeof(createGameRequest.addresses[0].type),
                      1);
        rtDmeAddField(&createGameRequest,
                      &createGameRequest.addresses[0].address,
                      RT_NET_ADDRESS_SIZE,
                      RT_NET_ADDRESS_SIZE);
        rtDmeAddField(&createGameRequest,
                      &createGameRequest.addresses[0].port,
                      sizeof(createGameRequest.addresses[0].port),
                      1);
        rtDmeAddField(&createGameRequest,
                      &createGameRequest.addresses[1].type,
                      sizeof(createGameRequest.addresses[1].type),
                      1);
        rtDmeAddField(&createGameRequest,
                      &createGameRequest.addresses[1].address,
                      RT_NET_ADDRESS_SIZE,
                      RT_NET_ADDRESS_SIZE);
        rtDmeAddField(&createGameRequest,
                      &createGameRequest.addresses[1].port,
                      sizeof(createGameRequest.addresses[1].port),
                      1);
        rtDmeAddField(&createGameRequest,
                      &createGameRequest.worldStatus,
                      sizeof(createGameRequest.worldStatus),
                      1);
        rtDmeRegisterClassMessage(&g_nCreateGameRequestType, RT_DME_CLASS_SERVER, NULL);
        rtDmeBeginFields();
        rtDmeAddField(
            &createGameResponse, &createGameResponse.header.messageId, 1, RT_MGCL_MESSAGE_ID_SIZE);
        rtDmeAddField(&createGameResponse,
                      &createGameResponse.header.status,
                      sizeof(createGameResponse.header.status),
                      1);
        rtDmeAddField(&createGameResponse,
                      &createGameResponse.worldId,
                      sizeof(createGameResponse.worldId),
                      1);
        rtDmeRegisterClassMessage(
            &g_nCreateGameResponseType, RT_DME_CLASS_SERVER, OnCreateGameResponse);
        rtDmeBeginFields();
        rtDmeAddField(&message17, &message17.messageId, 1, RT_MGCL_MESSAGE_ID_SIZE);
        rtDmeAddField(&message17, &message17.value, sizeof(message17.value), 1);
        rtDmeRegisterClassMessage(&g_nMessage17Type, RT_DME_CLASS_SERVER, NULL);
        rtDmeBeginFields();
        rtDmeAddField(&message18, &message18.messageId, 1, RT_MGCL_MESSAGE_ID_SIZE);
        rtDmeAddField(&message18, &message18.status, sizeof(message18.status), 1);
        rtDmeRegisterClassMessage(&g_nMessage18Type, RT_DME_CLASS_SERVER, OnMessage18);
        rtDmeBeginFields();
        rtDmeAddField(&message19, &message19.messageId, 1, RT_MGCL_MESSAGE_ID_SIZE);
        rtDmeAddField(&message19, &message19.gameName, 1, RT_MGCL_GAME_NAME_SIZE);
        rtDmeAddField(&message19, &message19.gameStats, 1, RT_MGCL_GAME_STATS_SIZE);
        rtDmeAddField(
            &message19, &message19.password, RT_MGCL_PASSWORD_SIZE, RT_MGCL_PASSWORD_SIZE);
        rtDmeAddField(&message19, &message19.applicationId, sizeof(message19.applicationId), 1);
        rtDmeAddField(&message19, &message19.maxClients, sizeof(message19.maxClients), 1);
        rtDmeAddField(&message19, &message19.minClients, sizeof(message19.minClients), 1);
        rtDmeAddField(&message19, &message19.gameLevel, sizeof(message19.gameLevel), 1);
        rtDmeAddField(
            &message19, &message19.playerSkillLevel, sizeof(message19.playerSkillLevel), 1);
        rtDmeAddField(&message19, &message19.rulesSet, sizeof(message19.rulesSet), 1);
        rtDmeAddField(&message19, &message19.genericField1, sizeof(message19.genericField1), 1);
        rtDmeAddField(&message19, &message19.genericField2, sizeof(message19.genericField2), 1);
        rtDmeAddField(&message19, &message19.genericField3, sizeof(message19.genericField3), 1);
        rtDmeAddField(&message19, &message19.gameHostType, sizeof(message19.gameHostType), 1);
        rtDmeAddField(
            &message19, &message19.addresses[0].type, sizeof(message19.addresses[0].type), 1);
        rtDmeAddField(
            &message19, &message19.addresses[0].address, sizeof(message19.addresses[0].address), 1);
        rtDmeRegisterClassMessage(&g_nMessage19Type, RT_DME_CLASS_SERVER, NULL);
        rtDmeBeginFields();
        rtDmeAddField(&message20, &message20.messageId, 1, RT_MGCL_MESSAGE_ID_SIZE);
        rtDmeAddField(&message20, &message20.first, sizeof(message20.first), 1);
        rtDmeAddField(&message20, &message20.second, sizeof(message20.second), 1);
        rtDmeAddField(
            &message20, &message20.addresses[0].type, sizeof(message20.addresses[0].type), 1);
        rtDmeAddField(
            &message20, &message20.addresses[0].address, RT_NET_ADDRESS_SIZE, RT_NET_ADDRESS_SIZE);
        rtDmeAddField(
            &message20, &message20.addresses[0].port, sizeof(message20.addresses[0].port), 1);
        rtDmeAddField(
            &message20, &message20.addresses[1].type, sizeof(message20.addresses[1].type), 1);
        rtDmeAddField(
            &message20, &message20.addresses[1].address, RT_NET_ADDRESS_SIZE, RT_NET_ADDRESS_SIZE);
        rtDmeAddField(
            &message20, &message20.addresses[1].port, sizeof(message20.addresses[1].port), 1);
        rtDmeRegisterClassMessage(&g_nMessage20Type, RT_DME_CLASS_SERVER, NULL);
        rtDmeBeginFields();
        rtDmeAddField(&message21, &message21.header.messageId, 1, RT_MGCL_MESSAGE_ID_SIZE);
        rtDmeAddField(&message21, &message21.header.status, sizeof(message21.header.status), 1);
        rtDmeAddField(&message21, &message21.value, sizeof(message21.value), 1);
        nRegisterResult =
            rtDmeRegisterClassMessage(&g_nMessage21Type, RT_DME_CLASS_SERVER, OnMessage21);
    }
    g_apfnServerRequestHandlers[0] = params->handlers[0];
    g_apfnServerRequestHandlers[1] = params->handlers[1];
    g_apfnServerRequestHandlers[2] = params->handlers[2];
    g_apfnServerRequestHandlers[3] = params->handlers[3];
    if (nRegisterResult != 0 || nInitResult != 0) {
        printf("Error Initializing Medius Communication Library --- DME Error %d.\n", 0);
        *status = kErrorInitialize;
        return kErrorInitialize;
    }
    *status = 0;
    g_nInitialized = 1;
    return 0;
}

int rtMgclSendServerReport(rtMgclServerReport *report) {
    int nResult;

    if (report->activeWorlds < 0 || report->maxWorlds < 0 || report->activePlayers < 0 ||
        report->maxPlayersPerWorld < 0) {
        return kErrorNotConnected;
    }
    if ((unsigned int)report->alertLevel >= kAlertLevelCount) {
        report->alertLevel = 0;
    }
    if (g_nInitialized != 1) {
        return kErrorNotConnected;
    }
    GetSessionKey(report->sessionKey);
    nResult = rtDmeSendClassMessage(kSendFlags,
                                    g_nConnection,
                                    kServerReceiver,
                                    RT_DME_CLASS_SERVER,
                                    g_nServerReportType,
                                    sizeof(*report),
                                    report);
    if (nResult != 0) {
        printf("Error sending Server Report --- DME Error %d.\n", nResult);
        return kErrorSend;
    }
    return 0;
}

int rtMgclGetVersion(char version[RT_MGCL_VERSION_SIZE]) {
    strncpy(version, kVersionText, RT_MGCL_VERSION_SIZE);
    return 0;
}

int rtMgclSendServerReply(rtMgclServerReply *reply) {
    TruncateField(reply->messageId, RT_MGCL_MESSAGE_ID_SIZE);
    TruncateField(reply->sessionKey, RT_MGCL_SESSION_KEY_SIZE);
    if (g_nInitialized != 1) {
        return kErrorNotConnected;
    }
    return (rtDmeSendClassMessage(kSendFlags,
                                  g_nConnection,
                                  kServerReceiver,
                                  RT_DME_CLASS_SERVER,
                                  g_nServerReplyType,
                                  sizeof(*reply),
                                  reply) == 0) ?
               0 :
               kErrorSend;
}

int rtMgclCreateGame(rtMgclCreateGameRequest *request, rtMgclCreateGameCallback callback) {
    int nResult;

    TruncateField(request->messageId, RT_MGCL_MESSAGE_ID_SIZE);
    TruncateField(request->gameName, RT_MGCL_GAME_NAME_SIZE);
    TruncateField(request->password, RT_MGCL_PASSWORD_SIZE);
    TruncateField(request->gameStats, RT_MGCL_GAME_STATS_SIZE);
    request->gameHostType = GetServerType();
    if (g_nInitialized != 1) {
        return kErrorNotConnected;
    }
    if (GetServerType() == kServerTypeQueryFirst || GetServerType() == kServerTypeQuerySecond) {
        GetHostAddresses(request->addresses);
    } else if (GetServerType() == kServerTypeDirect) {
        request->addresses[0].type = 0;
        request->addresses[1].type = 0;
    } else if (GetServerType() == kServerTypeDirectOther) {
        request->addresses[0].type = 0;
        request->addresses[1].type = 0;
    } else {
        return kErrorServerType;
    }
    nResult = rtDmeSendClassMessage(kSendFlags,
                                    g_nConnection,
                                    kServerReceiver,
                                    RT_DME_CLASS_SERVER,
                                    g_nCreateGameRequestType,
                                    sizeof(*request),
                                    request);
    g_pfnCreateGameCallback = callback;
    return (nResult == 0) ? 0 : kErrorSend;
}

int rtMgclInitConnectParams(rtMgclConnectParams *params) {
    if (params == NULL) {
        return kErrorArgument;
    }
    memset(params, 0, sizeof(*params));
    params->info.type = 0;
    params->onConnect = NULL;
    return 0;
}

int rtMgclSetConnectParams(rtMgclConnectParams *params,
                           const char *address,
                           int port,
                           int worldId,
                           rtMgclConnectCallback onConnect) {
    if (params == NULL) {
        return kErrorArgument;
    }
    params->info.addresses[0].type = 1;
    params->info.type = 1;
    // Yes, the binary copies four bytes past the address. The port that follows is stored next.
    strncpy(params->info.addresses[0].address, address, kAddressCopySize);
    params->info.addresses[0].port = port;
    params->info.worldId = worldId;
    if (onConnect != NULL) {
        params->onConnect = onConnect;
    }
    return 0;
}
