#include "utl/MsgSink.h"

MsgSink::~MsgSink() {
}

bool MsgSink::Dispatch(Message *msg) {
    return DispatchPriv(msg);
}
