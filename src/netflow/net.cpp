#include "netflow/net.h"

#include <memory>

#include "netflow/inetimpl.h"
#include "netflow/launchpadrt.h"
#include "netflow/lobbyrt.h"
#include "netflow/netcrypt.h"
#include "netflow/transportrt.h"
#include "os/file.h"
#include "os/irx.h"
#include "os/system.h"
#include "script/dataarray.h"

namespace {

// NTSC-U/C: 0x003b0bd0, PAL: 0x0042faa0
// The network connection InitializeNetSubsystem() builds.
std::unique_ptr<InetImpl> gInetImpl;

} // namespace

void InitializeNetSubsystem() {
    if (kImageFirstWord != 0) {
        return;
    }
    IrxLoadAll(SystemConfig()->FindArray("net", true)->FindArray("startup_modules", true));
    NetCryptInit();
    gInetImpl = std::make_unique<InetImpl>();
    TheLobbyRT = std::make_unique<LobbyRT>();
    TheTransportRT = std::make_unique<TransportRT>();
}

void TerminateNetSubsystem() {
    if (kImageFirstWord != 0) {
        return;
    }
    TheTransportRT.reset();
    TheLaunchpadRT.reset();
    TheLobbyRT.reset();
    gInetImpl.reset();
}

void PollNetSubsystem() {
    if (kImageFirstWord != 0) {
        return;
    }
    gInetImpl->Poll();
    TheLobbyRT->Poll();
    if (TheLaunchpadRT != nullptr && TheLaunchpadRT->Poll() != 0) {
        TheLaunchpadRT.reset();
    }
}
