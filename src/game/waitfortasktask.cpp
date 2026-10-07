#include "game/waitfortasktask.h"

WaitForTaskTask::WaitForTaskTask(const char *pszName, TaskDoneNotifier *pNotifier)
    : mNotifier(pNotifier), mName(pszName) {
}

void WaitForTaskTask::OnStart() {
    mNotifier->NotifyWhenDone(this, mName);
}
