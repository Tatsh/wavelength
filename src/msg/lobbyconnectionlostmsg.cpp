#include "msg/lobbyconnectionlostmsg.h"

Message *LobbyConnectionLostMsg::Clone() {
    return new LobbyConnectionLostMsg(*this);
}

int LobbyConnectionLostMsg::Type() {
    return g_nLobbyConnectionLostMsgType;
}

const char *LobbyConnectionLostMsg::GetName() const {
    return "LobbyConnectionLostMsg";
}
