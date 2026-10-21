#include "netflow/mediusconnecttask.h"

#include <cstring>

#include <librtmedius.h>
#include <librtmgcl.h>

#include "msg/lobbyconnectresultmsg.h"
#include "netflow/lobbyrt.h"
#include "os/system.h"

namespace {

// The world the connection parameters of the lobby server give.
constexpr int kLobbyServerWorld = 1;

// The players of the connection.
constexpr int kMaxPlayers = 32;

// The connection a failed connect reports.
constexpr int kNoConnection = -1;

#ifdef VIDEO_STANDARD_PAL
// The milliseconds the server may take to connect.
constexpr float kConnectWaitMs = 20000.0f;
#endif

} // namespace

// NTSC-U/C: 0x003b0afc, PAL: 0x0042f9c8
MediusConnectTask *MediusConnectTask::sTask;

// NTSC-U/C: 0x003b0b00, PAL: 0x0042f9cc
int MediusConnectTask::sConnecting;

void MediusConnectTask::OnStart() {
#ifdef VIDEO_STANDARD_PAL
    mDeadline = SystemMs() + kConnectWaitMs;
#endif
    rtMediusConnectParams params;
    memset(&params, 0, sizeof(params));
    if (mServer != kServerLocation) {
        rtMediusSetConnectParams(&params,
                                 nullptr,
                                 TheLobbyRT->GetServerAddress().c_str(),
                                 TheLobbyRT->GetServerPort(),
                                 kLobbyServerWorld);
        if (TheLobbyRT->GetSecure() != 0) {
            unsigned char key[RT_NET_SERVER_KEY_SIZE];
            (void)rtMgclGetServerKey(key);
            memcpy(params.info.serverKey, key, sizeof(key));
        }
    } else {
        params.info = TheLobbyRT->GetLobbyInfo();
    }
    params.maxPlayers = kMaxPlayers;
    params.onConnect = OnConnect;
    params.onClientJoin = nullptr;
    params.onClientLeave = nullptr;
    rtMediusConnectStatus status;
    memset(&status, 0, sizeof(status));
    sTask = this;
    sConnecting = 1;
    const int nError = rtMediusConnect(&params, &status);
#ifdef VIDEO_STANDARD_PAL
    LobbyRT::ReportMediusError(341, nError);
#else
    LobbyRT::ReportMediusError(335, nError);
#endif
    if (nError != 0) {
        sConnecting = 0;
        Finish(false, LobbyConnectResultMsg::kResultConnectFailed);
    }
}

void MediusConnectTask::OnStop() {
    sTask = nullptr;
}

#ifdef VIDEO_STANDARD_PAL
void MediusConnectTask::OnPoll() {
    if (mDeadline < SystemMs()) {
        Finish(false, LobbyConnectResultMsg::kResultConnectFailed);
        sConnecting = 0;
    }
}
#endif

void MediusConnectTask::OnConnect(int nConnection) {
    sConnecting = 0;
    if (sTask != nullptr) {
        const bool bConnected = nConnection != kNoConnection;
        sTask->Finish(bConnected, LobbyConnectResultMsg::kResultConnectFailed);
        TheLobbyRT->SetMediusConnected(bConnected);
    }
}
