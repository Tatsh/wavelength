#include "game/msgjoiner.h"

bool MsgJoiner::DispatchPriv(Message *pMsg) {
    Send(pMsg);
    return false;
}
