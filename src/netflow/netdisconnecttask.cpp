#include "netflow/netdisconnecttask.h"

#include <cstring>

#include <librtdme.h>
#include <librtmedius.h>

#include "netflow/launchpadrt.h"
#include "netflow/lobbyrt.h"
#include "os/asynctask.h"

namespace {

// The action the lobby server records for a player who left a game.
constexpr int kUserActionLeftGame = 2;

} // namespace

// NTSC-U/C: 0x003b0ae8, PAL: 0x0042f9b4
NetDisconnectTask *NetDisconnectTask::sTask;

void NetDisconnectTask::OnStart() {
    sTask = this;
    EnqueueAsyncTask(Disconnect, OnDisconnected);
}

int NetDisconnectTask::Disconnect() {
    return rtDmeDisconnect(TheLaunchpadRT->mConnection);
}

void NetDisconnectTask::OnDisconnected(int nResult) {
    (void)nResult;
    TheLaunchpadRT->mConnection = LaunchpadRT::kNoWorld;
    rtMediusUserState state;
    strcpy(state.sessionKey, TheLobbyRT->mSessionKey.c_str());
    state.userAction = kUserActionLeftGame;
    (void)rtMediusUpdateUserState(&state);
    sTask->Finish(true, 0);
    sTask = nullptr;
}
