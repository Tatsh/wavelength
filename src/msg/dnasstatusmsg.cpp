#include "msg/dnasstatusmsg.h"

#ifdef VIDEO_STANDARD_PAL

Message *DNASStatusMsg::Clone() {
    return new DNASStatusMsg(*this);
}

int DNASStatusMsg::Type() {
    return g_nDNASStatusMsgType;
}

const char *DNASStatusMsg::GetName() const {
    return "DNASStatusMsg";
}

#endif
