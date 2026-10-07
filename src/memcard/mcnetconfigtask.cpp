#include "memcard/mcnetconfigtask.h"

#include "netflow/netinet.h"

namespace {

// Values of InetConfigsResultMsg::mResult when no configuration was found. The names are
// inferred from the outcomes they produce.
constexpr int kConfigsMissing = -99;
constexpr int kConfigsDamaged = -98;

} // namespace

void MCNetConfigTask::Set(int nPort) {
    mPort = nPort;
}

void MCNetConfigTask::OnStart() {
    mConfigs.clear();
    TheNetInet->RequestConfigs(this);
}

bool MCNetConfigTask::DispatchPriv(Message *pMsg) {
    if (pMsg->Type() == g_nInetConfigsResultMsgType) {
        return OnConfigsResult(static_cast<InetConfigsResultMsg *>(pMsg));
    }
    return false;
}

bool MCNetConfigTask::OnConfigsResult(InetConfigsResultMsg *pMsg) {
    mConfigs = pMsg->mConfigs;
    if (mConfigs.size() != 0) {
        sStatus = kStatusOk;
        Finish(true);
        return true;
    }
    switch (pMsg->mResult) {
    case kConfigsMissing:
        sStatus = kStatusNotFound;
        break;
    case kConfigsDamaged:
        sStatus = kStatusConfigsBad;
        break;
    default:
        sStatus = kStatusConfigsError;
        break;
    }
    Finish(false);
    return true;
}
