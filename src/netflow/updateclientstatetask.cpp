#include "netflow/updateclientstatetask.h"

#include <cstring>

#include <librtmedius.h>

#include "netflow/lobbyrt.h"

namespace {

// The action of a player that is in the lobby.
constexpr int kUserActionInLobby = 1;

} // namespace

void UpdateClientStateTask::OnStart() {
    rtMediusUserState state;
    memset(&state, 0, sizeof(state));
    strcpy(state.sessionKey, TheLobbyRT->mSessionKey.c_str());
    state.userAction = kUserActionInLobby;
    LobbyRT::ReportMediusError(1024, rtMediusUpdateUserState(&state));
    Finish(true, 0);
}
