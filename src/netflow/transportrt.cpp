#include "netflow/transportrt.h"

#include <algorithm>

#include "msg/playerabortedmsg.h"
#include "netflow/hostpadrt.h"
#include "netflow/launchpadrt.h"

namespace {

// The players of a session, which mAborted reserves.
constexpr int kMaxPlayers = 4;

} // namespace

std::unique_ptr<TransportRT> TheTransportRT;

NetTransport *TheNetTransport;

TransportRT::TransportRT() : mSink(nullptr), mOpen(0) {
    mAborted.reserve(kMaxPlayers);
    TheNetTransport = this;
}

TransportRT::~TransportRT() {
    TheNetTransport = nullptr;
}

void TransportRT::Notify(Message *pMsg) {
    if (mSink != nullptr && mOpen != 0) {
        mSink->Dispatch(pMsg);
    }
}

void TransportRT::Receive(Packet *pPacket) {
    if (mSink == nullptr || mOpen == 0) {
        return;
    }
    if (std::find(mAborted.begin(), mAborted.end(), pPacket->mSender) == mAborted.end()) {
        mSink->Dispatch(pPacket);
    }
}

void TransportRT::PlayerAborted(int nId) {
    mAborted.push_back(nId);
    if (mSink != nullptr && mOpen != 0) {
        PlayerAbortedMsg msg(nId);
        mSink->Dispatch(&msg);
    }
}

void TransportRT::SetOpen(int nOpen) {
    mOpen = nOpen;
    if (nOpen != 0) {
        mAborted.clear();
    }
}

void TransportRT::SetSink(MsgSink *pSink) {
    mSink = pSink;
}

void TransportRT::Send(ExternalPacket &packet) {
    if (TheLaunchpadRT != nullptr && mOpen != 0) {
        TheLaunchpadRT->Send(&packet);
    }
}

bool TransportRT::IsHost() {
    if (TheLaunchpadRT == nullptr) {
        return false;
    }
    return TheLaunchpadRT->GetLocalId() == LaunchpadRT::kHostId;
}

void TransportRT::Leave() {
    if (TheLaunchpadRT != nullptr) {
        TheLaunchpadRT->Leave();
    }
}

void TransportRT::ReportScores(const std::vector<NetGameScore> &scores) {
    (void)IsHost(); // Yes, the binary discards this call's result.
    if (TheHostpadRT != nullptr) {
        TheHostpadRT->SendScores(scores);
    }
}
