#include "game/delayer.h"

#include "msg/message.h"

Delayer::~Delayer() {
}

bool Delayer::DispatchPriv(Message *pMsg) {
    (void)pMsg->Type(); // Yes, the binary discards this call's result.
    Send(pMsg);
    return false;
}
