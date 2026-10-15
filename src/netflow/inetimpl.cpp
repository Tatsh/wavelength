#include "netflow/inetimpl.h"

#include <algorithm>
#include <list>
#include <stdio.h>
#include <string.h>
#include <vector>

#include <eekernel.h>
#include <libcdvd.h>
#include <libezctl.h>
#include <libeznetcnf.h>
#include <sifdev.h>

#include "app/msgsink.h"
#include "msg/inetdisconnectresultmsg.h"
#include "netflow/inetcheckconnectionresultmsg.h"
#include "netflow/inetconfig.h"
#include "netflow/inetconfigsresultmsg.h"
#include "netflow/inetconnectresultmsg.h"
#include "netflow/inetconnectstatusmsg.h"
#include "netflow/inetdnslookupresultmsg.h"
#include "os/asynctask.h"
#include "os/debug.h"
#include "os/file.h"
#include "os/irx.h"
#include "os/locale.h"
#include "os/system.h"
#include "script/dataarray.h"

namespace {

// The milliseconds a connection may take before Connect() is configured.
constexpr float kDefaultTimeout = 20000.0f;

// The milliseconds a modem and an Ethernet adaptor may take to connect.
constexpr float kModemTimeout = 120000.0f;
constexpr float kEthernetTimeout = 30000.0f;

// The link status that differs from every report.
constexpr int kNoLinkStatus = 999;

// The capacity each module vector reserves.
constexpr int kModuleCapacity = 10;

// The interface that DownInterface() and ConnectTask() switch.
constexpr int kInterface = 0;

// The configuration file on the memory card.
const char kConfigPath[] = "mc0:";

// The error of ezNetCnfGetList() and ezNetCnfLoadEntry() for a configuration of another console.
constexpr int kNetCnfWrongConsole = -15;

// The value of interfaceType and deviceType that selects PPP and a modem.
constexpr int kInterfaceTypePpp = 2;
constexpr int kDeviceTypeModem = 2;

// The values of pppoe that mean PPP over Ethernet is off.
constexpr unsigned char kPppoeOff = 0;
constexpr unsigned char kPppoeUnset = 255;

// The vendor of the Sony network adaptor, and the characters of the vendor compared.
const char kSonyVendor[] = "SCE";
constexpr unsigned int kVendorLength = 3;

// The states of ezNetCtlStatus_t that Poll() handles.
constexpr int kInterfaceStarting = 2;
constexpr int kInterfaceUp = 3;
constexpr int kInterfaceFailed = 5;

// The link status values of ezNetCtlStatus_t.
constexpr int kLinkInvalid = 0;
constexpr int kLinkValid = 1;

#ifdef VIDEO_STANDARD_PAL
// The start of the message of a failed interface whose provider refused the login.
const char kAuthFailureText[] = "Authentication fail";

// The tokens of the texts InetConnectStatusMsg reports while the interface comes up.
const char kConnectingToken[] = "eznetctl_Connecting";
const char kInvalidEthernetToken[] = "eznetctl_invalid_ethernet";
#endif

// Results that CheckConnection() and LookupHost() report.
constexpr int kCheckFailed = -91;
constexpr int kLookupFailed = -90;

// The request of sceDevctl() that turns the network adaptor off.
constexpr int kDev9PowerOff = 0x4402;

// The argument count LaunchConfigTool() passes.
constexpr int kLaunchArgCount = 1;

// An IOP module the connection uses.
class Mod {
public:
    const char *mName; // The file name.
    int mId;           // The identifier of the loaded module, or 0 before it loads.

    // Compare the file names.
    bool operator==(const Mod &other) const {
        return strcmp(mName, other.mName) == 0;
    }
};

} // namespace

InetImpl *InetImpl::sInstance;

NetInet *TheNetInet;

// NTSC-U/C: 0x0043ca80
// The arguments LaunchConfigTool() passes.
static char *sLaunchArgs[kLaunchArgCount];

// NTSC-U/C: 0x0043cac0
// The combinations of the configuration file.
static ezNetCnfCombinationList_t sConfigList;

// NTSC-U/C: 0x0043cf00
// The configuration ConnectTask() loads.
static ezNetCnfData_t sConfigData;

// NTSC-U/C: 0x0043e240
// The state of the interface.
static ezNetCtlStatus_t sStatus;

// The two vectors below are constructed and destroyed by one routine at 0x00254b48. The image's
// static construction and destruction entries at 0x00254c50 and 0x00254c70 call it.
// NTSC-U/C: 0x00254b48, PAL: 0x0025da60
// NTSC-U/C: 0x00254c50, PAL: 0x0025db68
// NTSC-U/C: 0x00254c70, PAL: 0x0025db88

// NTSC-U/C: 0x0043ca88
// The modules loaded.
static std::vector<Mod> sLoadedModules;

// NTSC-U/C: 0x0043ca98
// The modules the configuration needs.
static std::vector<Mod> sWantedModules;

InetImpl::InetImpl()
    : mState(kStateIdle), mStartTime(0.0f), mTimeout(kDefaultTimeout),
      mConnectionType(kConnectionOffline), mPowerOffRequested(0), mLookupPending(0),
#ifdef VIDEO_STANDARD_PAL
      mDNASPending(1),
#endif
      mConnectSink(nullptr), mLookupSink(nullptr), mCheckSink(nullptr), mConfigsSink(nullptr),
      mDisconnectSink(nullptr) {
    TheNetInet = this;
    sInstance = this;
    ezNetCnfInit();
    ezNetCtlInit();
    sLoadedModules.reserve(kModuleCapacity);
    sWantedModules.reserve(kModuleCapacity);
}

InetImpl::~InetImpl() {
    TheNetInet = nullptr;
    sInstance = nullptr;
}

int InetImpl::DownInterface() {
    ezNetCtlDownInterface(kInterface);
    sInstance->mConnectionType = kConnectionOffline;
    sInstance->mState = kStateIdle;
    return 0;
}

void InetImpl::LaunchConfigTool() {
#ifdef VIDEO_STANDARD_PAL
    // The European disc has one configuration tool, whatever the kind of disc.
    const char *pszPath = "cdrom0:\\NETGUI\\NTGUCDVD.ELF;1";
#else
    const char *pszPath;
    const int nType = sceCdGetDiskType();
    if (nType == SCECdPS2CD) {
        pszPath = "cdrom0:\\NETGUI\\NTGUICD.ELF;1";
    } else if (nType == SCECdNODISC) {
        pszPath = "host0:cdbuild/netgui/ntguihost.elf";
    } else {
        pszPath = "cdrom0:\\NETGUI\\NTGUIDVD.ELF;1";
    }
#endif
    String bootPath;
    MakeDevicePath(bootPath, FileBootExecutable());
    sLaunchArgs[0] = const_cast<char *>(bootPath.c_str());
    sceCdInit(SCECdEXIT);
    LoadExecPS2(pszPath, kLaunchArgCount, sLaunchArgs);
}

void InetImpl::RequestConfigs(MsgSink *pSink) {
    if (mConfigsSink == nullptr) {
        mConfigsSink = pSink;
        EnqueueAsyncTask(ConfigsTask, OnConfigsTaskDone);
    }
}

int InetImpl::ConfigsTask() {
    sConfigList.listLength = 0;
    return ezNetCnfGetList(kConfigPath, 0, &sConfigList);
}

void InetImpl::OnConfigsTaskDone(int nResult) {
    if (sInstance != nullptr) {
        sInstance->DeliverConfigs(nResult);
    }
}

void InetImpl::DeliverConfigs(int nResult) {
    int nCode = 0;
    if (nResult <= 0) {
        DebugPrint("get combolist error:%d\n", nResult);
        if (nResult == kNetCnfWrongConsole) {
            nCode = InetConnectResultMsg::kResultConfigWrongConsole;
        } else if (nResult != 0) {
            nCode = InetConnectResultMsg::kResultConfigError;
        } else {
            nCode = InetConnectResultMsg::kResultConfigNotFound;
        }
    }
    InetConfigsResultMsg msg(nCode);
    std::list<InetConfig> unused; // Yes, the binary builds and frees an unused list.
    for (int i = 1; i <= sConfigList.listLength; ++i) {
        const ezNetCnfCombination_t *pCombination = ezNetCnfGetCombination(&sConfigList, i);
        if (pCombination != nullptr && pCombination->status >= 0) {
            InetConfig config;
            config.mId = i;
            config.mDescription = String(pCombination->deviceName);
            config.mName = String(pCombination->interfaceName);
            config.mDefault = sConfigList.defaultNumber == i;
            msg.mConfigs.push_back(config);
        }
    }
    mConfigsSink->Dispatch(&msg);
    mConfigsSink = nullptr;
}

void InetImpl::Connect(MsgSink *pSink, int nConfig) {
    DownInterface();
    mConnectSink = pSink;
    mConfigId = nConfig;
#ifdef VIDEO_STANDARD_PAL
    mDNASPending = 1;
#endif
    mState = kStateStarting;
    EnqueueAsyncTask(ConnectTask, OnConnectTaskDone);
}

int InetImpl::ConnectTask() {
    int nFailure;
    int nStatus = 0;
    const ezNetCnfCombination_t *pCombination =
        ezNetCnfGetCombination(&sConfigList, sInstance->mConfigId);
    if (pCombination == nullptr) {
        nFailure = InetConnectResultMsg::kResultNoCombination;
        nStatus = -1;
    }
    if (nStatus >= 0) {
        nStatus = ezNetCnfLoadEntry(kConfigPath, pCombination->name, &sConfigData);
        nFailure = nStatus == kNetCnfWrongConsole ?
                       InetConnectResultMsg::kResultConfigWrongConsole :
                       InetConnectResultMsg::kResultConfigNotFound;
        if (nStatus >= 0) {
            nFailure = InetConnectResultMsg::kResultHardwareError;
            nStatus = SelectModules(&sConfigData);
            if (nStatus >= 0) {
                nStatus = ezNetCtlSetProperties(&sConfigData);
                if (nStatus >= 0) {
                    nStatus = ezNetCtlUpInterface(kInterface);
                }
            }
        }
    }
    return nStatus < 0 ? nFailure : 0;
}

void InetImpl::OnConnectTaskDone(int nResult) {
    if (nResult == 0) {
        sInstance->mState = kStateConnecting;
        const float fNow = SystemMs();
        sInstance->mLinkStatus = kNoLinkStatus;
        sInstance->mStartTime = fNow;
        sInstance->mStatusText = "";
        return;
    }
    sInstance->mState = kStateIdle;
    InetConnectResultMsg msg(nResult);
    sInstance->mConnectSink->Dispatch(&msg);
    sInstance->mConnectSink = nullptr;
}

void InetImpl::Disconnect(MsgSink *pSink) {
    mDisconnectSink = pSink;
}

void InetImpl::ClearPending() {
    mLookupSink = nullptr;
}

void InetImpl::LookupHost(MsgSink *pSink, const char *pszHost) {
    if (mLookupPending) {
        InetDNSLookupResultMsg msg(kLookupFailed, "");
        pSink->Dispatch(&msg);
        return;
    }
    mLookupSink = pSink;
    mHost = pszHost;
    mLookupPending = 1;
    EnqueueAsyncTask(LookupTask, OnLookupTaskDone);
}

int InetImpl::LookupTask() {
    return ezNetCtlLookUpName(sInstance->mHost.c_str());
}

void InetImpl::OnLookupTaskDone(int nResult) {
    MsgSink *pSink = sInstance->mLookupSink;
    if (pSink != nullptr) {
        if (nResult != 0) {
            InetDNSLookupResultMsg msg(kLookupFailed, "");
            pSink->Dispatch(&msg);
        } else {
            const char *pszAddress = ezNetCtlGetNameBuffer();
            if (pszAddress[0] == '\0') {
                InetDNSLookupResultMsg msg(kLookupFailed, "");
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
    if (mState != kStateConnected) {
        InetCheckConnectionResultMsg msg(kCheckFailed);
        pSink->Dispatch(&msg);
        return;
    }
    mCheckSink = pSink;
    EnqueueAsyncTask(StatusTask, OnStatusTaskDone);
}

int InetImpl::StatusTask() {
    return ezNetCtlGetStatus(&sStatus);
}

void InetImpl::OnStatusTaskDone(int) {
    int nResult = 0;
    if (sStatus.state != kInterfaceUp) {
        DownInterface();
        nResult = kCheckFailed;
    }
    InetCheckConnectionResultMsg msg(nResult);
    sInstance->mCheckSink->Dispatch(&msg);
    sInstance->mCheckSink = nullptr;
}

#ifdef VIDEO_STANDARD_PAL
int InetImpl::IsDNASPending() {
    return mDNASPending;
}

void InetImpl::ClearDNASPending() {
    mDNASPending = 0;
}
#endif

void InetImpl::RequestPowerOff() {
    mPowerOffRequested = 1;
}

void InetImpl::Poll() {
    if (mCheckSink != nullptr || mLookupPending) {
        return;
    }
    if (mDisconnectSink != nullptr && mState != kStateStarting) {
        DownInterface();
        InetDisconnectResultMsg msg;
        mDisconnectSink->Dispatch(&msg);
        mDisconnectSink = nullptr;
        mConnectSink = nullptr;
    }
    if (mState == kStateConnecting) {
        int nResult = 0;
        ezNetCtlGetStatus(&sStatus);
        if (sStatus.interfaceId < 0) {
            mState = kStateIdle;
            nResult = InetConnectResultMsg::kResultHardwareError;
        }
        const float fNow = SystemMs();
        if (mTimeout < fNow - mStartTime) {
            mState = kStateIdle;
            DownInterface();
            nResult = InetConnectResultMsg::kResultTimeout;
        } else if (sStatus.state == kInterfaceUp) {
            mState = kStateConnected;
            nResult = 0;
        } else if (sStatus.state == kInterfaceStarting) {
            mStartTime = fNow;
        } else if (sStatus.state == kInterfaceFailed) {
            mState = kStateIdle;
            nResult = InetConnectResultMsg::kResultFailed;
#ifdef VIDEO_STANDARD_PAL
            if (strncmp(sStatus.message, kAuthFailureText, strlen(kAuthFailureText)) == 0) {
                nResult = InetConnectResultMsg::kResultAuthFailed;
            }
#endif
        }
        if (mLinkStatus != sStatus.linkStatus || mStatusText != sStatus.message) {
#ifdef VIDEO_STANDARD_PAL
            DebugPrint("eznet link:%d msg:%s\n", sStatus.linkStatus, sStatus.message);
            String text(TheLocale.Localize(kConnectingToken, true));
            if (sStatus.linkStatus == kLinkInvalid) {
                text += TheLocale.Localize(kInvalidEthernetToken, true);
            }
#else
            String text;
            if (sStatus.linkStatus == kLinkInvalid) {
                text += "Invalid Ethernet link\n";
            } else if (sStatus.linkStatus == kLinkValid) {
                text += "Valid Ethernet link\n";
            }
            text += sStatus.message;
#endif
            InetConnectStatusMsg msg(text);
            mConnectSink->Dispatch(&msg);
            mLinkStatus = sStatus.linkStatus;
            mStatusText = sStatus.message;
        }
        if (mState != kStateConnecting) {
            InetConnectResultMsg msg(nResult);
            mConnectSink->Dispatch(&msg);
            mConnectSink = nullptr;
        }
    }
    if (mPowerOffRequested) {
        mPowerOffRequested = 0;
        printf("Powering off now\n");
        DownInterface();
        while (sceDevctl("dev9x:", kDev9PowerOff, nullptr, 0, nullptr, 0) < 0) {
        }
        printf("dev9 off\n");
        unsigned int nTray;
        sceCdTrayReq(SCECdTrayClose, &nTray);
        while (sceCdDiskReady(SCECdBlock) == SCECdNotReady) {
        }
        printf("cd drive closed\n");
        int nStat = 0;
        while (sceCdPowerOff(&nStat) == 0 || nStat != 0) {
        }
    }
}

void InetImpl::AddModule(const char *pszName) {
    sWantedModules.push_back(Mod{pszName, 0});
}

int InetImpl::LoadModules() {
    for (int i = static_cast<int>(sLoadedModules.size()) - 1; i >= 0; --i) {
        const Mod &mod = sLoadedModules[i];
        if (std::find(sWantedModules.begin(), sWantedModules.end(), mod) == sWantedModules.end()) {
            int nResult;
            if (sceSifStopModule(mod.mId, 0, nullptr, &nResult) >= 0 &&
                sceSifUnloadModule(mod.mId) >= 0) {
                sLoadedModules.erase(sLoadedModules.begin() + i);
            }
        }
    }
    const DataArray *pModules =
        SystemConfig()->FindArray("net", true)->FindArray("optional_modules", true);
    for (unsigned int i = 0; i < sWantedModules.size(); ++i) {
        const Mod &mod = sWantedModules[i];
        if (std::find(sLoadedModules.begin(), sLoadedModules.end(), mod) == sLoadedModules.end()) {
            const int nId = IrxLoadMatching(pModules, mod.mName);
            if (nId < 0) {
                return nId;
            }
            sLoadedModules.push_back(mod);
            sLoadedModules.back().mId = nId;
        }
    }
    return 0;
}

int InetImpl::SelectModules(const ezNetCnfData_t *pData) {
    sWantedModules.clear();
    if (pData->interfaceType == kInterfaceTypePpp) {
        AddModule("ppp.irx");
        if (pData->pppoe != kPppoeOff && pData->pppoe != kPppoeUnset) {
            AddModule("pppoe.irx");
        }
    }
    int nType;
    float fTimeout;
    if (pData->deviceType == kDeviceTypeModem &&
        (pData->pppoe == kPppoeOff || pData->pppoe == kPppoeUnset)) {
        if (strncmp(pData->vendor, kSonyVendor, kVendorLength) == 0) {
            AddModule("dev9.irx");
            AddModule("spduart.irx");
        } else {
            AddModule("cxtmdm.irx");
        }
        nType = kConnectionSlow;
        fTimeout = kModemTimeout;
    } else {
        // The European disc has no AN986 driver, and its Ethernet connection waits as long as a
        // modem.
        if (strncmp(pData->vendor, kSonyVendor, kVendorLength) == 0) {
            AddModule("dev9.irx");
            AddModule("smap.irx");
#ifndef VIDEO_STANDARD_PAL
        } else {
            AddModule("an986.irx");
#endif
        }
        nType = kConnectionFast;
#ifdef VIDEO_STANDARD_PAL
        fTimeout = kModemTimeout;
#else
        fTimeout = kEthernetTimeout;
#endif
    }
    sInstance->mConnectionType = nType;
    sInstance->mTimeout = fTimeout;
    if (strncmp(pData->vendor, kSonyVendor, kVendorLength) == 0) {
        sInstance->mPowerOffRequested = 0;
        sceCdPOffCallback(OnPowerOff, nullptr);
    }
    return LoadModules();
}

void InetImpl::OnPowerOff(void *) {
    sInstance->mPowerOffRequested = 1;
}
