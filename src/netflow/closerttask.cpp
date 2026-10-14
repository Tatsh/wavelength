#include "netflow/closerttask.h"

#include <librtdme.h>
#include <librtmedius.h>

#include "netflow/lobbyrt.h"

void CloseRTTask::OnStart() {
    (void)rtMediusClose();
    (void)rtNetClose();
    TheLobbyRT->mRtReady = 0;
    Finish(true, 0);
}
