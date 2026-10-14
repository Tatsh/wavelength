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
    TheLobbyRT->mMediusConnected = 0;
    ChatMsg::ClearHandlers();
    Finish(true, 0);
}
