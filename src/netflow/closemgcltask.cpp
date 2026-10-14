#include "netflow/closemgcltask.h"

#include <librtmgcl.h>

#include "netflow/launchpadrt.h"

void CloseMGCLTask::OnStart() {
    (void)rtMgclClose();
    LaunchpadRT::SetMgclReady(0);
    Finish(true, 0);
}
