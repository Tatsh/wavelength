#include "netflow/mediusdisconnecttask.h"

#include <librtmedius.h>

#include "msg/chatmsg.h"
#include "netflow/lobbyrt.h"
#include "netflow/mediusconnecttask.h"

void MediusDisconnectTask::OnPoll() {
    if (MediusConnectTask::sConnecting != 0) {
        return;
    }
    (void)rtMediusDisconnect();
    TheLobbyRT->SetMediusConnected(0);
    ChatMsg::ClearPartialMessages();
    Finish(true, 0);
}
