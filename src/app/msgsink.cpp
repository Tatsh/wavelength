#include "app/msgsink.h"

MsgSink::~MsgSink() {
}

bool MsgSink::Dispatch(Message *pMsg) {
    return DispatchPriv(pMsg);
}
