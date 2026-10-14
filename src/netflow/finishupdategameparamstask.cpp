#include "netflow/finishupdategameparamstask.h"

#include "netflow/hostpadrt.h"

void FinishUpdateGameParamsTask::OnStart() {
    if (TheHostpadRT != nullptr) {
        TheHostpadRT->SendGameParams();
    }
    Finish(true, 0);
}
