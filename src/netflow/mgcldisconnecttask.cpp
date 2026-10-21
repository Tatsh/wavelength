#include "netflow/mgcldisconnecttask.h"

#include <librtmgcl.h>

#include "netflow/hostpadrt.h"
#include "netflow/mgclconnecttask.h"

void MGCLDisconnectTask::OnPoll() {
    if (MGCLConnectTask::sConnecting != 0) {
        return;
    }
    (void)rtMgclDisconnect();
    TheHostpadRT->SetMgclConnected(0);
    Finish(true, 0);
}
