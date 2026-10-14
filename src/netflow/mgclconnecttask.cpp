#include "netflow/mgclconnecttask.h"

#include <cstring>

#include <librtmgcl.h>

#include "msg/launchpadabortedmsg.h"
#include "netflow/hostpadrt.h"
#include "netflow/lobbyrt.h"

namespace {

// The world the connection parameters of the lobby server give.
constexpr int kLobbyServerWorld = 1;

} // namespace

// NTSC-U/C: 0x003b0ad8
MGCLConnectTask *MGCLConnectTask::sTask;

// NTSC-U/C: 0x003b0adc
int MGCLConnectTask::sConnecting;

void MGCLConnectTask::OnStart() {
    rtMgclConnectParams params;
    memset(&params, 0, sizeof(params));
    if (mServer != kServerGame) {
        (void)rtMgclInitConnectParams(&params);
        (void)rtMgclSetConnectParams(&params,
                                     TheLobbyRT->mServerAddress.c_str(),
                                     TheLobbyRT->mServerPort,
                                     kLobbyServerWorld,
                                     OnConnect);
        if (TheLobbyRT->mSecure != 0) {
            (void)rtMgclGetServerKey(params.info.serverKey);
        }
    } else {
        params.info = TheHostpadRT->mServerInfo;
        params.onConnect = OnConnect;
    }
    int nStatus;
    memset(&nStatus, 0, sizeof(nStatus));
    sTask = this;
    sConnecting = 1;
    if (rtMgclConnect(&params, &nStatus) != 0) {
        sConnecting = 0;
        Finish(false, LaunchpadAbortedMsg::kReasonServer);
    }
}

void MGCLConnectTask::OnStop() {
    sTask = nullptr;
}

void MGCLConnectTask::OnConnect(int nConnection, int nStatus) {
    (void)nConnection;
    sConnecting = 0;
    if (sTask != nullptr) {
        TheHostpadRT->mMgclConnected = nStatus == 0;
        sTask->Finish(nStatus == 0, LaunchpadAbortedMsg::kReasonServer);
    }
}
