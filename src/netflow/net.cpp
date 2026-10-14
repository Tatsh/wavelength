#include "netflow/net.h"

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

// NTSC-U/C: 0x003b0bd0
// The network connection InitializeNetSubsystem() builds.
InetImpl *gInetImpl;

} // namespace

void InitializeNetSubsystem() {
    if (kImageFirstWord != 0) {
        return;
    }
    IrxLoadAll(SystemConfig()->FindArray("net", true)->FindArray("startup_modules", true));
    NetCryptInit();
    gInetImpl = new InetImpl;
    (void)new LobbyRT;     // Yes, the binary keeps the service only through TheLobbyRT.
    (void)new TransportRT; // Yes, the binary keeps the transport only through TheTransportRT.
}

void TerminateNetSubsystem() {
    if (kImageFirstWord != 0) {
        return;
    }
    delete TheTransportRT;
    delete TheLaunchpadRT;
    delete TheLobbyRT;
    delete gInetImpl;
    gInetImpl = nullptr;
}

void PollNetSubsystem() {
    if (kImageFirstWord != 0) {
        return;
    }
    gInetImpl->Poll();
    TheLobbyRT->Poll();
    if (TheLaunchpadRT != nullptr && TheLaunchpadRT->Poll() != 0) {
        delete TheLaunchpadRT;
    }
}
