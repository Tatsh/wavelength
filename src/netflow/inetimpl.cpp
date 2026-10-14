#include "netflow/inetimpl.h"

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <vector>

#include <eekernel.h>
#include <libcdvd.h>
#include <libezctl.h>
#include <sifdev.h>

#include "msg/inetdisconnectresultmsg.h"
#include "netflow/inetcheckconnectionresultmsg.h"
#include "netflow/inetconfigsresultmsg.h"
#include "netflow/inetconnectresultmsg.h"
#include "netflow/inetconnectstatusmsg.h"
#include "netflow/inetdnslookupresultmsg.h"
#include "os/asynctask.h"
#include "os/debug.h"
#include "os/file.h"
#include "os/irx.h"
#include "os/system.h"
#include "script/dataarray.h"

namespace {

// One IOP module of the network configuration.
struct Mod {
    // Report whether two records name the same module file.
    bool operator==(const Mod &other) const {
        return strcmp(mName, other.mName) == 0;
    }

    const char *mName; // The file of the module.
    int mId;           // The identifier of the loaded module.
};

// The wait for progress of an interface before the first configuration sets it, and the waits of
// an Ethernet adaptor and of a modem, in milliseconds.
constexpr float kDefaultTimeoutMs = 20000.0f;
constexpr float kEthernetTimeoutMs = 30000.0f;
constexpr float kModemTimeoutMs = 120000.0f;

// The modules each list reserves room for.
constexpr int kModuleReserve = 10;

// The memory card the configurations are read from.
const char *const kConfigFile = "mc0:";

// The error of the configuration server for a configuration of another console.
constexpr int kNetCnfErrorWrongConsole = -15;

// The link states and interface states of the control server.
constexpr int kLinkInvalid = 0;
constexpr int kLinkValid = 1;
constexpr int kInterfaceStarting = 2;
constexpr int kInterfaceUp = 3;
constexpr int kInterfaceFailed = 5;

// A link state the control server never reports, which makes the first status a change.
constexpr int kLinkUnknown = 999;

// The interface type of a PPP configuration and the device type of a modem.
constexpr int kInterfaceTypePpp = 2;
constexpr int kDeviceTypeModem = 2;

// The values of the PPPoE setting that mean no PPPoE.
constexpr unsigned char kPppoeOff = 0;
constexpr unsigned char kPppoeUnset = 255;

// The vendor of the adaptors Sony makes, and the length the comparison reads.
const char *const kSonyVendor = "SCE";
constexpr int kSonyVendorLength = 3;

// The device and the request that switch the network adaptor off.
const char *const kAdaptorDevice = "dev9x:";
constexpr int kDevctlAdaptorOff = 0x4402;

// The interface the control server brings up and down.
constexpr int kInterfaceId = 0;

// The configurations of the memory card.
ezNetCnfCombinationList_t gCombinations;

// The configuration Connect() loads.
ezNetCnfData_t gNetCnfData;

// The last state the control server reported.
ezNetCtlStatus_t gStatus;

// The arguments the network configuration program starts with.
char *gArgv[1];

// The IOP modules loaded, and the ones the configuration needs.
std::vector<Mod> gLoadedMods;
std::vector<Mod> gWantedMods;

} // namespace

// NTSC-U/C: 0x003b0ab4
NetInet *TheNetInet;

// NTSC-U/C: 0x003b0ab8
InetImpl *InetImpl::sInstance;

InetImpl::InetImpl()
    : mState(kStateIdle), mStartMs(0.0f), mTimeoutMs(kDefaultTimeoutMs),
      mConnectionType(kConnectionNone), mPowerOff(0), mLookupPending(0), mConnectSink(nullptr),
      mLookupSink(nullptr), mCheckSink(nullptr), mConfigsSink(nullptr), mDisconnectSink(nullptr) {
    TheNetInet = this;
    sInstance = this;
    ezNetCnfInit();
    ezNetCtlInit();
    gLoadedMods.reserve(kModuleReserve);
    gWantedMods.reserve(kModuleReserve);
}

InetImpl::~InetImpl() {
    TheNetInet = nullptr;
    sInstance = nullptr;
}

int InetImpl::StopInterface() {
    (void)ezNetCtlDownInterface(kInterfaceId);
    sInstance->mConnectionType = kConnectionNone;
    sInstance->mState = kStateIdle;
    return 0;
}

void InetImpl::LaunchConfigTool() {
    const char *pszTool;
    switch (sceCdGetDiskType()) {
    case SCECdPS2CD:
        pszTool = "cdrom0:\\NETGUI\\NTGUICD.ELF;1";
        break;
    case SCECdNODISC:
        pszTool = "host0:cdbuild/netgui/ntguihost.elf";
        break;
    default:
        pszTool = "cdrom0:\\NETGUI\\NTGUIDVD.ELF;1";
        break;
    }
    String bootPath;
    MakeDevicePath(bootPath, FileBootExecutable());
    gArgv[0] = bootPath.mBuffer;
    sceCdInit(SCECdEXIT);
    LoadExecPS2(pszTool, 1, gArgv);
}

void InetImpl::RequestConfigs(MsgSink *pSink) {
    if (mConfigsSink != nullptr) {
        return;
    }
    mConfigsSink = pSink;
    EnqueueAsyncTask(ListConfigs, OnConfigsListed);
}

int InetImpl::ListConfigs() {
    gCombinations.listLength = 0;
    return ezNetCnfGetList(kConfigFile, 0, &gCombinations);
}

void InetImpl::OnConfigsListed(int nResult) {
    if (sInstance != nullptr) {
        sInstance->ReportConfigs(nResult);
    }
}

void InetImpl::ReportConfigs(int nResult) {
    int nMsgResult = 0;
    if (nResult <= 0) {
        DebugPrint("get combolist error:%d\n", nResult);
        if (nResult == kNetCnfErrorWrongConsole) {
            nMsgResult = InetConnectResultMsg::kResultConfigWrongConsole;
        } else if (nResult != 0) {
            nMsgResult = InetConnectResultMsg::kResultConfigError;
        } else {
            nMsgResult = InetConnectResultMsg::kResultConfigNotFound;
        }
    }
    InetConfigsResultMsg msg(nMsgResult);
    for (int i = 1; i <= gCombinations.listLength; ++i) {
        const ezNetCnfCombination_t *pCombination = ezNetCnfGetCombination(&gCombinations, i);
        if (pCombination == nullptr || pCombination->status < 0) {
            continue;
        }
        const InetConfig config = {i,
                                   String(pCombination->deviceName),
                                   String(pCombination->interfaceName),
                                   gCombinations.defaultNumber == i};
        msg.mConfigs.push_back(config);
    }
    mConfigsSink->Dispatch(&msg);
    mConfigsSink = nullptr;
}

void InetImpl::Connect(MsgSink *pSink, int nConfig) {
    (void)StopInterface();
    mConnectSink = pSink;
    mConfig = nConfig;
    mState = kStateConnecting;
    EnqueueAsyncTask(OpenConfig, OnConfigOpened);
}

int InetImpl::OpenConfig() {
    int nError = InetConnectResultMsg::kResultFailed;
    int nResult = 0;
    const ezNetCnfCombination_t *pCombination =
        ezNetCnfGetCombination(&gCombinations, sInstance->mConfig);
    if (pCombination == nullptr) {
        nError = InetConnectResultMsg::kResultNoConfig;
        nResult = -1;
    }
    if (nResult >= 0) {
        nResult = ezNetCnfLoadEntry(kConfigFile, pCombination->name, &gNetCnfData);
        nError = nResult == kNetCnfErrorWrongConsole ?
                     InetConnectResultMsg::kResultConfigWrongConsole :
                     InetConnectResultMsg::kResultConfigNotFound;
        if (nResult >= 0) {
            nError = InetConnectResultMsg::kResultHardwareError;
            nResult = SetupModules(&gNetCnfData);
            if (nResult >= 0) {
                nResult = ezNetCtlSetProperties(&gNetCnfData);
                if (nResult >= 0) {
                    nResult = ezNetCtlUpInterface(kInterfaceId);
                }
            }
        }
    }
    return nResult < 0 ? nError : 0;
}

void InetImpl::OnConfigOpened(int nResult) {
    if (nResult == 0) {
        sInstance->mState = kStateStarting;
        sInstance->mStartMs = SystemMs();
        sInstance->mLinkStatus = kLinkUnknown;
        sInstance->mMessage = "";
        return;
    }
    MsgSink *pSink = sInstance->mConnectSink;
    sInstance->mState = kStateIdle;
    InetConnectResultMsg msg(nResult);
    pSink->Dispatch(&msg);
    sInstance->mConnectSink = nullptr;
}

void InetImpl::Disconnect(MsgSink *pSink) {
    mDisconnectSink = pSink;
}

void InetImpl::ClearPending() {
    mLookupSink = nullptr;
}

void InetImpl::LookupHost(MsgSink *pSink, const char *pszHost) {
    if (mLookupPending != 0) {
        InetDNSLookupResultMsg msg(InetDNSLookupResultMsg::kResultFailed, "");
        pSink->Dispatch(&msg);
        return;
    }
    mLookupSink = pSink;
    mHostName = pszHost;
    mLookupPending = 1;
    EnqueueAsyncTask(LookUpName, OnNameLookedUp);
}

int InetImpl::LookUpName() {
    return ezNetCtlLookUpName(sInstance->mHostName.c_str());
}

void InetImpl::OnNameLookedUp(int nResult) {
    if (sInstance->mLookupSink != nullptr) {
        if (nResult != 0) {
            InetDNSLookupResultMsg msg(InetDNSLookupResultMsg::kResultFailed, "");
            sInstance->mLookupSink->Dispatch(&msg);
        } else {
            const char *pszAddress = ezNetCtlGetNameBuffer();
            if (pszAddress[0] == '\0') {
                InetDNSLookupResultMsg msg(InetDNSLookupResultMsg::kResultFailed, "");
                sInstance->mLookupSink->Dispatch(&msg);
            } else {
                InetDNSLookupResultMsg msg(0, pszAddress);
                sInstance->mLookupSink->Dispatch(&msg);
            }
        }
    }
    sInstance->mLookupPending = 0;
    sInstance->mLookupSink = nullptr;
}

void InetImpl::CheckConnection(MsgSink *pSink) {
    if (mState != kStateUp) {
        InetCheckConnectionResultMsg msg(InetCheckConnectionResultMsg::kResultDown);
        pSink->Dispatch(&msg);
        return;
    }
    mCheckSink = pSink;
    EnqueueAsyncTask(ReadStatus, OnStatusRead);
}

int InetImpl::ReadStatus() {
    return ezNetCtlGetStatus(&gStatus);
}

void InetImpl::OnStatusRead(int nResult) {
    (void)nResult;
    int nCheck = 0;
    if (gStatus.state != kInterfaceUp) {
        (void)StopInterface();
        nCheck = InetCheckConnectionResultMsg::kResultDown;
    }
    InetCheckConnectionResultMsg msg(nCheck);
    sInstance->mCheckSink->Dispatch(&msg);
    sInstance->mCheckSink = nullptr;
}

void InetImpl::PowerOff() {
    mPowerOff = 1;
}

void InetImpl::Poll() {
    if (mCheckSink != nullptr || mLookupPending != 0) {
        return;
    }
    if (mDisconnectSink != nullptr && mState != kStateConnecting) {
        (void)StopInterface();
        InetDisconnectResultMsg msg;
        mDisconnectSink->Dispatch(&msg);
        mDisconnectSink = nullptr;
        mConnectSink = nullptr;
    }
    if (mState == kStateStarting) {
        int nResult = 0;
        (void)ezNetCtlGetStatus(&gStatus);
        if (gStatus.interfaceId < 0) {
            mState = kStateIdle;
            nResult = InetConnectResultMsg::kResultHardwareError;
        }
        const float fNow = SystemMs();
        if (mTimeoutMs < fNow - mStartMs) {
            mState = kStateIdle;
            (void)StopInterface();
            nResult = InetConnectResultMsg::kResultTimeout;
        } else {
            switch (gStatus.state) {
            case kInterfaceUp:
                mState = kStateUp;
                nResult = 0;
                break;
            case kInterfaceStarting:
                mStartMs = fNow;
                break;
            case kInterfaceFailed:
                mState = kStateIdle;
                nResult = InetConnectResultMsg::kResultInterfaceDown;
                break;
            default:
                break;
            }
        }
        if (mLinkStatus != gStatus.linkStatus || mMessage != gStatus.message) {
            String text;
            switch (gStatus.linkStatus) {
            case kLinkInvalid:
                text += "Invalid Ethernet link\n";
                break;
            case kLinkValid:
                text += "Valid Ethernet link\n";
                break;
            default:
                break;
            }
            text += gStatus.message;
            InetConnectStatusMsg msg(text);
            mConnectSink->Dispatch(&msg);
            mLinkStatus = gStatus.linkStatus;
            mMessage = gStatus.message;
        }
        if (mState != kStateStarting) {
            InetConnectResultMsg msg(nResult);
            mConnectSink->Dispatch(&msg);
            mConnectSink = nullptr;
        }
    }
    if (mPowerOff == 0) {
        return;
    }
    mPowerOff = 0;
    printf("Powering off now\n");
    (void)StopInterface();
    while (sceDevctl(kAdaptorDevice, kDevctlAdaptorOff, nullptr, 0, nullptr, 0) < 0) {
    }
    printf("dev9 off\n");
    unsigned int nTrayCount;
    sceCdTrayReq(SCECdTrayClose, &nTrayCount);
    while (sceCdDiskReady(SCECdBlock) == SCECdNotReady) {
    }
    printf("cd drive closed\n");
    int nDriveResult = 0;
    while (sceCdPowerOff(&nDriveResult) == 0 || nDriveResult != 0) {
    }
}

void InetImpl::WantModule(const char *pszName) {
    gWantedMods.push_back(Mod{pszName, 0});
}

int InetImpl::SyncModules() {
    for (int i = static_cast<int>(gLoadedMods.size()) - 1; i >= 0; --i) {
        const Mod &mod = gLoadedMods[i];
        if (std::find(gWantedMods.begin(), gWantedMods.end(), mod) != gWantedMods.end()) {
            continue;
        }
        int nStopResult;
        if (sceSifStopModule(mod.mId, 0, nullptr, &nStopResult) < 0) {
            continue;
        }
        if (sceSifUnloadModule(mod.mId) < 0) {
            continue;
        }
        gLoadedMods.erase(gLoadedMods.begin() + i);
    }
    const DataArray *pModules =
        SystemConfig()->FindArray("net", true)->FindArray("optional_modules", true);
    for (unsigned int i = 0; i < gWantedMods.size(); ++i) {
        const Mod &mod = gWantedMods[i];
        if (std::find(gLoadedMods.begin(), gLoadedMods.end(), mod) != gLoadedMods.end()) {
            continue;
        }
        const int nId = IrxLoadMatching(pModules, mod.mName);
        if (nId < 0) {
            return nId;
        }
        gLoadedMods.push_back(mod);
        gLoadedMods.back().mId = nId;
    }
    return 0;
}

int InetImpl::SetupModules(const ezNetCnfData_t *pData) {
    gWantedMods.clear();
    if (pData->interfaceType == kInterfaceTypePpp) {
        WantModule("ppp.irx");
        if (pData->pppoe != kPppoeOff && pData->pppoe != kPppoeUnset) {
            WantModule("pppoe.irx");
        }
    }
    const bool bSony = strncmp(pData->vendor, kSonyVendor, kSonyVendorLength) == 0;
    if (pData->deviceType == kDeviceTypeModem &&
        (pData->pppoe == kPppoeOff || pData->pppoe == kPppoeUnset)) {
        if (bSony) {
            WantModule("dev9.irx");
            WantModule("spduart.irx");
        } else {
            WantModule("cxtmdm.irx");
        }
        sInstance->mConnectionType = kConnectionSlow;
        sInstance->mTimeoutMs = kModemTimeoutMs;
    } else {
        if (bSony) {
            WantModule("dev9.irx");
            WantModule("smap.irx");
        } else {
            WantModule("an986.irx");
        }
        sInstance->mConnectionType = kConnectionFast;
        sInstance->mTimeoutMs = kEthernetTimeoutMs;
    }
    if (strncmp(pData->vendor, kSonyVendor, kSonyVendorLength) == 0) {
        sInstance->mPowerOff = 0;
        (void)sceCdPOffCallback(OnPowerOffButton, nullptr);
    }
    return SyncModules();
}

void InetImpl::OnPowerOffButton(void *pArg) {
    (void)pArg;
    sInstance->mPowerOff = 1;
}
