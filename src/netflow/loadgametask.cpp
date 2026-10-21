#include "netflow/loadgametask.h"

#include "msg/loadgamepkt.h"
#include "netflow/launchpadrt.h"

void LoadGameTask::OnStart() {
    if (TheLaunchpadRT != nullptr) {
        LoadGamePkt pkt;
        TheLaunchpadRT->Send(&pkt);
        static_cast<MsgSink *>(TheLaunchpadRT.get())->Dispatch(&pkt);
    }
    Finish(true, 0);
}
